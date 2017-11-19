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

/*
 * This class implements all autocall functions that a definition must implement for autocall.
 * 
 * All AutoCall specialization extends this one.
 */
struct autocall_base {
	template<typename T, typename F, typename... Ts, int_t<enable_if_t<!is_map<F>::value>, enable_if_t<!is_map<Ts>::value>...> = 0>
	void do_autocall(inject_t<Ts>... others) {
		static_cast<T*>(this)->call(F::value, std::forward<inject_t<Ts>>(others).forward()...);
	}
	
	template<typename T, typename F, typename Map, enable_if_t<is_map<Map>::value && !is_map<F>::value, int> = 0>
	void do_autocall(inject_t<container_service> cs) {
		autocall_helper<T, Map, F>(detail::function_seq<typename F::value_type>{}, std::move(cs));
	}
	
private:
	template<typename T, typename Map, typename F, std::size_t... S, enable_if_t<is_map<Map>::value && !is_map<F>::value, int> = 0>
	void autocall_helper(detail::seq<S...>, inject_t<container_service> cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, typename F::value_type>... args){
			static_cast<T*>(this)->call(F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
};

} // namespace detail

/*
 * This alias simply to transform a member function address to a type.
 */
template<typename T, T t>
using method = std::integral_constant<T, t>;

/*
 * This class wraps a method and tell explicitly which services are needed.
 */
template<typename M, typename... Ps>
struct invoke : M {
	using parameters = detail::meta_list<Ps...>;
};

/*
 * The class that defines AutoCall.
 * 
 * It is intended to be extended by user definition.
 * 
 * It should be the only way to enable autocall in a service.
 */
template<typename... Ts>
struct autocall : detail::autocall_base {
	using autocall_functions = detail::meta_list<Ts...>;
	
	using map = kgr::map<>;
};

/*
 * Specialization of AutoCall when a map is sent as first parameter.
 */
template<typename... Maps, typename... Ts>
struct autocall<map<Maps...>, Ts...> : detail::autocall_base {
	using autocall_functions = detail::meta_list<Ts...>;
	
	using map = kgr::map<Maps...>;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_AUTOCALL_HPP
