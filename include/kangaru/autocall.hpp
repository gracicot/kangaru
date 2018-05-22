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

template<typename, typename, typename>
struct autocall_function_helper;

/*
 * This class implements all autocall functions that a definition must implement for autocall.
 * 
 * All autocall specialization extends this one.
 * 
 * It extending autocall is only way to enable autocall in a service.
 */
template<typename Map, typename... Ts>
struct autocall_base {
	using autocall_functions = detail::meta_list<Ts...>;
	using map = Map;
	
private:
	template<typename, typename, typename> friend struct autocall_function_helper;
	
	// TODO: Remove this function since it doesn't belong here anymore.
	template<typename T, typename M, typename F, std::size_t... S>
	static void autocall_helper(detail::seq<S...>, inject_t<container_service> cs, T& service) {
		cs.forward().invoke<M>([&](detail::function_argument_t<S, typename F::value_type>... args) {
			service.call(F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
};

} // namespace detail

/*
 * This alias simply to transform a member function address to a type.
 */
template<typename T, typename std::decay<T>::type t>
using method = std::integral_constant<typename std::decay<T>::type, t>;

/*
 * This class wraps a method and tell explicitly which services are needed.
 */
template<typename M, typename... Ps>
struct invoke : M {
	using parameters = detail::meta_list<Ps...>;
};

/*
 * The class that defines autocall with the default map.
 */
template<typename... Ts>
struct autocall : detail::autocall_base<map<>, Ts...> {};

/*
 * Specialization of autocall when a map is sent as first parameter.
 */
template<typename... Maps, typename... Ts>
struct autocall<map<Maps...>, Ts...> : detail::autocall_base<map<Maps...>, Ts...> {};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_AUTOCALL_HPP
