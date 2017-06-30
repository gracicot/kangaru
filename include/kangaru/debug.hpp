#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP

#include "detail/error.hpp"

namespace kgr {
namespace debug {

template<typename T, typename U, typename... Args>
auto service(U&& u, Args&&...) -> detail::enable_if_t<std::is_constructible<detail::ServiceError<T, Args...>, U>::value> {
	(void) detail::ServiceError<T, Args...> {std::forward<U>(u)};
}

template<typename T, typename U, typename... Args>
auto service(U&&, Args&&...) -> detail::enable_if_t<!std::is_constructible<detail::ServiceError<T, Args...>, U>::value> {
	static_assert(detail::false_t<T>::value, "No error detected.");
}

template<typename T>
auto service() -> detail::enable_if_t<std::is_default_constructible<detail::ServiceError<T>>::value> {
	(void) detail::ServiceError<T> {};
}

template<typename T>
auto service() -> detail::enable_if_t<!std::is_default_constructible<detail::ServiceError<T>>::value> {
	static_assert(detail::false_t<T>::value, "No error detected.");
}

}
}

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP
