#ifndef KGR_INCLUDE_KANGARU_DETAIL_ERROR_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_ERROR_HPP

#include "traits.hpp"
#include "service_traits.hpp"

namespace kgr {
namespace detail {

template<typename T, typename... Args>
struct ServiceError {
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Arg>::value &&
		!is_dependencies_constructible<Service, Arg, Args...>::value &&
		is_service_constructible<Service, Arg, Args...>::value, int> = 0>
	ServiceError(Arg&&) {
		static_assert(false_t<Arg>::value,
			"One or more dependencies are not constructible with it's dependencies as constructor argument. "
			"Please check that dependency's constructor and it's dependencies."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_dependencies_constructible<Service>::value &&
		is_service_constructible<Service>::value, int> = 0>
	ServiceError() {
		static_assert(false_t<Service>::value,
			"One or more dependencies are not constructible with it's dependencies as constructor argument. "
			"Please check that dependency's constructor and dependencies."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_service_constructible<Service, Arg, Args...>::value &&
		!is_service_constructible<Service>::value, int> = 0>
	ServiceError(Arg&&) {
		static_assert(false_t<Arg>::value,
			"The service type is not constructible given it's dependencies and passed arguments. "
			"Ensure that dependencies are correctly configured and you pass the right set of parameters."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_service_constructible<Service, Arg, Args...>::value &&
		is_service_constructible<Service>::value, int> = 0>
	ServiceError(Arg&&) {
		static_assert(false_t<Arg>::value, "The service type is not constructible given passed arguments to kgr::Container::service(...).");
	}
	
	template<typename Service = T, enable_if_t<is_service<Service>::value && !is_service_constructible<Service>::value, int> = 0>
	ServiceError() {
		static_assert(false_t<Service>::value,
			"The service type is not constructible given it's dependencies. "
			"Check if dependencies are configured correctly and if the service has the required constructor."
		);
	}
	
	template<typename Service = T, enable_if_t<!is_service<Service>::value && !has_forward<Service>::value, int> = 0>
	ServiceError() {
		static_assert(false_t<Service>::value, "The type sent to kgr::Container::service(...) is not a service.");
	}
	
	template<typename Service = T, enable_if_t<!is_service<Service>::value && has_forward<Service>::value, int> = 0>
	ServiceError() {
		static_assert(false_t<Service>::value, "The service type must not not contain any virtual function or must be abstract.");
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
