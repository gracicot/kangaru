#ifndef KGR_INCLUDE_KANGARU_DETAIL_ERROR_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_ERROR_HPP

#include "traits.hpp"
#include "service_traits.hpp"

namespace kgr {
namespace detail {

template<typename T, typename... Args>
struct ServiceError {
	template<typename U, enable_if_t<
		is_service<U>::value &&
		!is_dependencies_constructible<T, U, Args...>::value &&
		is_service_constructible<T, U, Args...>::value, int> = 0>
	ServiceError(U&&, ...) {
		static_assert(false_t<U>::value,
			"One or more dependencies are not constructible with it's dependencies as constructor argument. "
			"Please check that dependency's constructor and it's dependencies."
		);
	}
	
	template<typename U = T, enable_if_t<
		is_service<U>::value &&
		!is_dependencies_constructible<U>::value &&
		is_service_constructible<U>::value, int> = 0>
	ServiceError(...) {
		static_assert(false_t<U>::value,
			"One or more dependencies are not constructible with it's dependencies as constructor argument. "
			"Please check that dependency's constructor and dependencies."
		);
	}
	
	template<typename U, enable_if_t<
		is_service<U>::value &&
		!is_service_constructible<T, U, Args...>::value &&
		!is_service_constructible<T>::value, int> = 0>
	ServiceError(U&&, ...) {
		static_assert(false_t<U>::value,
			"The service type is not constructible given it's dependencies and passed arguments. "
			"Ensure that dependencies are correctly configured and you pass the right set of parameters."
		);
	}
	
	template<typename U, enable_if_t<
		is_service<U>::value &&
		!is_service_constructible<T, U, Args...>::value &&
		is_service_constructible<T>::value, int> = 0>
	ServiceError(U&&, ...) {
		static_assert(false_t<U>::value, "The service type is not constructible given passed arguments to kgr::Container::service(...).");
	}
	
	template<typename U = T, enable_if_t<is_service<U>::value && !is_service_constructible<U>::value, int> = 0>
	ServiceError(...) {
		static_assert(false_t<U>::value,
			"The service type is not constructible given it's dependencies."
			"Check if dependencies are configured correctly and if the service has the required constructor."
		);
	}
	
	template<typename U = T, enable_if_t<!is_service<U>::value && !has_forward<U>::value, int> = 0>
	ServiceError(...) {
		static_assert(false_t<U>::value, "The type sent to kgr::Container::service(...) is not a service.");
	}
	
	template<typename U = T, enable_if_t<!is_service<U>::value && has_forward<U>::value, int> = 0>
	ServiceError(...) {
		static_assert(false_t<U>::value, "The service type must not not contain any virtual function or must be abstract.");
	}
};

struct NotInvokableError {
	template<typename T = void>
	NotInvokableError(...) {
		static_assert(false_t<T>::value,
			"The function sent is not invokable. Ensure to include all services definitions "
			"you need and that received parameters are correct."
		);
	}
};

} // namespace detail
} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_ERROR_HPP
