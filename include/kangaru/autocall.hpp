#ifndef KGR_KANGARU_INCLUDE_KANGARU_AUTOCALL_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_AUTOCALL_HPP

#include "detail/meta_list.hpp"
#include "detail/utils.hpp"
#include "detail/traits.hpp"
#include "detail/injected.hpp"
#include "detail/container_service.hpp"
#include "container.hpp"

namespace kgr {
namespace detail {

struct AutoCallBase {
	template<typename T, typename F, typename... Ts, int_t<enable_if_t<!is_map<F>::value>, enable_if_t<!is_map<Ts>::value>...> = 0>
	void autocall(Inject<Ts>... others) {
		static_cast<T*>(this)->call(F::value, std::forward<Inject<Ts>>(others).forward()...);
	}
	
	template<typename T, typename Map, typename F, enable_if_t<is_map<Map>::value && !is_map<F>::value, int> = 0>
	void autocall(Inject<ContainerService> cs) {
		autocall<T, Map, F>(detail::function_seq<typename F::value_type>{}, std::move(cs));
	}
	
	template<typename T, typename Map, typename F, std::size_t... S, enable_if_t<is_map<Map>::value && !is_map<F>::value, int> = 0>
	void autocall(detail::seq<S...>, Inject<ContainerService> cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, typename F::value_type>... args){
			static_cast<T*>(this)->call(F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
};

} // namespace detail

template<typename T, T t>
using Method = std::integral_constant<T, t>;

template<typename M, typename... Ps>
struct Invoke : M {
	using Parameters = detail::meta_list<Ps...>;
};

template<typename... Ts>
struct AutoCall : detail::AutoCallBase {
	using Autocall = detail::meta_list<Ts...>;
	
	using Map = kgr::Map<>;
};

template<typename... Maps, typename... Ts>
struct AutoCall<kgr::Map<Maps...>, Ts...> : detail::AutoCallBase {
	using Autocall = detail::meta_list<Ts...>;
	
	using Map = kgr::Map<Maps...>;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_AUTOCALL_HPP
