#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP

#include "detail/error.hpp"

namespace kgr {
namespace debug {

template<typename T, typename U, typename... Args>
auto service(U&& u, Args&&...) -> detail::enable_if_t<std::is_constructible<detail::ServiceError<T, Args...>, U>::value> {
	detail::ServiceError<T, Args...> error{std::forward<U>(u)};
	
	static_cast<void>(error);
}

template<typename T, typename U, typename... Args>
auto service(U&&, Args&&...) -> detail::enable_if_t<!std::is_constructible<detail::ServiceError<T, Args...>, U>::value> = delete;

template<typename T>
auto service() -> detail::enable_if_t<std::is_default_constructible<detail::ServiceError<T>>::value> {
	detail::ServiceError<T> error{};
	
	static_cast<void>(error);
}

template<typename T>
auto service() -> detail::enable_if_t<!std::is_default_constructible<detail::ServiceError<T>>::value> = delete;

}
}

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEBUG_HPP
