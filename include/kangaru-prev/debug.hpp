#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP

#include "detail/error.hpp"

namespace kgr {
namespace debug {

/*
 * Debug function for services that takes argument.
 * 
 * This overload takes a ill formed service and static_assert the detected error.
 */
template<typename T, typename U, typename... Args>
auto service(U&& u, Args&&...) -> detail::enable_if_t<std::is_constructible<detail::service_error<T, Args...>, U>::value> {
	(void) detail::service_error<T, Args...> {std::forward<U>(u)};
}

/*
 * This overload is called when no error is found in a service that takes arguments.
 * 
 * Output as a static assert that no error is detected.
 */
template<typename T, typename U, typename... Args>
auto service(U&&, Args&&...) -> detail::enable_if_t<!std::is_constructible<detail::service_error<T, Args...>, U>::value> {
	static_assert(detail::false_t<T>::value, "No known error detected.");
}

/*
 * Debug function for a service without arguments.
 * 
 * Outputs the detected error as a static_assert
 */
template<typename T>
auto service() -> detail::enable_if_t<std::is_default_constructible<detail::service_error<T>>::value> {
	(void) detail::service_error<T> {};
}

/*
 * This overload is choosen when no error is detected.
 * 
 * Output as a static assert that no error is detected.
 */
template<typename T>
auto service() -> detail::enable_if_t<!std::is_default_constructible<detail::service_error<T>>::value> {
	static_assert(detail::false_t<T>::value, "No known error detected.");
}

} // namespace debug
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP
