#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_ERROR_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_ERROR_HPP

#include "traits.hpp"
#include "single.hpp"
#include "service_check.hpp"
#include "autocall_traits.hpp"

namespace kgr {
namespace detail {

/*
 * This class is only constructible if a service has an error.
 * It will call the right constructor and trigger the correct static_assert for the situation.
 */
template<typename T, typename... Args>
struct service_error {
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_override_convertible<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The service injected type cannot be converted to the overriding type. "
			"Check if the service is overriding the right service and if types are compatible."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_autocall_valid<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The service has problem with autocall. Injected services can be invalid, or the service map can be incoplete. "
			"Check if all required services are included and if every services are valid."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_autocall_valid<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"The service has problem with autocall. Injected services can be invalid, or the service map can be incoplete. "
			"Check if all required services are included and if every services are valid."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_service_constructible<Service, Arg, Args...>::value &&
		is_autocall_valid<Service>::value &&
		!dependency_trait<is_autocall_valid, Service, Arg, Args...>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"A dependency of this service has problem with autocall. Injected services can be invalid, or the service map can be incoplete. "
			"Check if all required services are included and if every services are valid."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_service_constructible<Service>::value &&
		is_autocall_valid<Service>::value &&
		!dependency_trait<is_autocall_valid, Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"A dependency of this service has problem with autocall. Injected services can be invalid, or the service map can be incoplete. "
			"Check if all required services are included and if every services are valid."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_override_convertible<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"The service injected type cannot be converted to the overriding type. "
			"Check if the service is overriding the right service and if types are compatible."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_override_convertible<Service>::value &&
		!is_override_polymorphic<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"An overriden service is not polymorphic, it therefore cannot be overriden."
			"The overriden service should be abstract or extend kgr::Virtual."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_override_convertible<Service>::value &&
		!is_override_polymorphic<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"An overriden service is not polymorphic, it therefore cannot be overriden."
			"The overriden service should be abstract or extend kgr::Virtual."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_override_convertible<Service>::value &&
		is_override_polymorphic<Service>::value &&
		!is_override_not_final<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"An overriden service is marked as final, thus this service annot override it."
			"The overriden service should be polymorphic or abtrant and not be final."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_override_convertible<Service>::value &&
		is_override_polymorphic<Service>::value &&
		!is_override_not_final<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"An overriden service is marked as final, thus this service annot override it."
			"The overriden service should be polymorphic or abtrant and not be final."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service>::value &&
		!dependency_trait<is_override_convertible, Service>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"One or more dependencies injected type cannot be converted to one of it's overriding type. "
			"Check if that service is overriding the right service and if types are compatible."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service, Arg, Args...>::value &&
		!dependency_trait<is_override_convertible, Service, Arg, Args...>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"One or more dependencies injected type cannot be converted to one of it's overriding type. "
			"Check if that service is overriding the right service and if types are compatible."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service>::value &&
		!dependency_trait<is_override_polymorphic, Service>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"One or more dependencies overrides a non-polymorphic service. "
			"Check if every dependencies overrides an abstract service or a service that extends kgr::Virtual."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service, Arg, Args...>::value &&
		!dependency_trait<is_override_polymorphic, Service, Arg, Args...>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"One or more dependencies overrides a non-polymorphic service. "
			"Check if every dependencies overrides an abstract service or a service that extends kgr::Virtual."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service>::value &&
		dependency_trait<is_override_polymorphic, Service>::value &&
		!dependency_trait<is_override_not_final, Service>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"One or more dependencies overrides a final service. "
			"Check if every dependencies overrides a non final service"
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service, Arg, Args...>::value &&
		dependency_trait<is_override_polymorphic, Service, Arg, Args...>::value &&
		!dependency_trait<is_override_not_final, Service, Arg, Args...>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"One or more dependencies overrides a final service. "
			"Check if every dependencies overrides a non final service"
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_abstract_service<Service>::value &&
		dependency_trait<is_default_service_valid, Service>::value &&
		!is_default_service_valid<Service>::value &&
		is_default_convertible<Service>::value &&
		is_default_overrides_abstract<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The default implementation of this abstract service is not a well defined service. "
			"Please check that types are complete and exposing a valid service definition interface."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_abstract_service<Service>::value &&
		dependency_trait<is_default_service_valid, Service, Arg, Args...>::value &&
		is_default_convertible<Service>::value &&
		!is_final_service<Service>::value &&
		is_default_overrides_abstract<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"The default implementation of this abstract service is not a well defined service. "
			"Please check that types are complete and exposing a valid service definition interface."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_abstract_service<Service>::value &&
		dependency_trait<is_default_service_valid, Service>::value &&
		!is_default_convertible<Service>::value &&
		is_default_overrides_abstract<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The service type of the default implementation of this abstract service is not convertible to the abstract service type. "
			"Please check that the service type of the default implementation of that abstract service is a compatible type."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_abstract_service<Service>::value &&
		dependency_trait<is_default_service_valid, Service, Arg, Args...>::value &&
		!is_default_service_valid<Service>::value &&
		!is_default_convertible<Service>::value &&
		is_default_overrides_abstract<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"The service type of the default implementation of this abstract service is not convertible to the abstract service type. "
			"Please check that the service type of the default implementation of that abstract service is a compatible type."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_abstract_service<Service>::value &&
		dependency_trait<is_default_service_valid, Service>::value &&
		!is_default_service_valid<Service>::value &&
		!is_default_overrides_abstract<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The default implementation of this abstract service is not overriding that abstract service. "
			"Ensure that the default implementation really override this abstract service."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_abstract_service<Service>::value &&
		dependency_trait<is_default_service_valid, Service, Arg, Args...>::value &&
		!is_default_service_valid<Service>::value &&
		!is_default_overrides_abstract<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"The default implementation of this abstract service is not overriding that abstract service. "
			"Ensure that the default implementation really override this abstract service."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_abstract_service<Service>::value &&
		!is_default_service_valid<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"Non-abstract service cannot have a default implementation."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_abstract_service<Service>::value &&
		!is_default_service_valid<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"Non-abstract service cannot have a default implementation."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service>::value &&
		is_service_constructible<Service>::value &&
		!dependency_trait<is_default_service_valid, Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The default implementation of a dependency is not valid. "
			"Ensure that every default implementation are valid services that override properly thier abstract services."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service, Arg, Args...>::value &&
		is_service_constructible<Service, Arg, Args...>::value &&
		!dependency_trait<is_default_service_valid, Service, Arg, Args...>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"The default implementation of a dependency is not valid. "
			"Ensure that every default implementation are valid services that override properly thier abstract services."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_abstract_not_final<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"An abstract service cannot be final"
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_abstract_not_final<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"An abstract service cannot be final"
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service>::value &&
		is_service_constructible<Service>::value &&
		!dependency_trait<is_abstract_not_final, Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"One or more dependencies are final abstract service or depends on a final abstract service"
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_service, Service, Arg, Args...>::value &&
		is_service_constructible<Service, Arg, Args...>::value &&
		!dependency_trait<is_abstract_not_final, Service, Arg, Args...>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"One or more dependencies are final abstract service or depends on a final abstract service"
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Arg>::value &&
		dependency_trait<is_construct_function_callable, Service, Arg, Args...>::value &&
		!dependency_trait<is_service_constructible, Service, Arg, Args...>::value &&
		is_service_constructible<Service, Arg, Args...>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Arg>::value,
			"One or more dependencies are not constructible with it's dependencies as constructor argument. "
			"Please check that dependency's constructor and it's dependencies."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		dependency_trait<is_construct_function_callable, Service>::value &&
		!dependency_trait<is_service_constructible, Service>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"One or more dependencies are not constructible with it's dependencies as constructor argument. "
			"Please check that dependency's constructor and dependencies."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Arg>::value &&
		!dependency_trait<is_construct_function_callable, Service>::value &&
		is_construct_function_callable<Service, Arg, Args...>::value &&
		is_service_constructible<Service, Arg, Args...>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Arg>::value,
			"One or more dependencies construct function cannot be called when calling kgr::Container::service<Dependency>(). "
			"Please check that dependency's construct function and ensure that is well formed."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!dependency_trait<is_construct_function_callable, Service>::value &&
		is_construct_function_callable<Service>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"One or more dependencies construct function cannot be called when calling kgr::Container::service<Dependency>(). "
			"Please check that dependency's construct function and ensure that is well formed."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_service_constructible<Service, Arg, Args...>::value &&
		!is_service_constructible<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Arg>::value,
			"The service type is not constructible given it's dependencies and passed arguments. "
			"Ensure that dependencies are correctly configured and you pass the right set of parameters."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_service_constructible<Service, Arg, Args...>::value &&
		is_service_constructible<Service>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Arg>::value, "The service type is not constructible given passed arguments to kgr::Container::service(...).");
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_construct_function_callable<Service>::value &&
		!is_service_constructible<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The service type is not constructible given it's dependencies. "
			"Check if dependencies are configured correctly and if the service has the required constructor."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		is_single<Service>::value &&
		!is_construct_function_callable<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The service construct function cannot be called. "
			"Check if the construct function is well formed, receive injected arguments first and additional parameters at the end."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_single<Service>::value &&
		dependency_trait<is_service, Service>::value &&
		!is_construct_function_callable<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"The service construct function cannot be called without arguments, or is not well formed. "
			"Check if the construct function is well formed, receive injected arguments first and additional parameters at the end."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		!is_single<Service>::value &&
		dependency_trait<is_service, Service, Arg, Args...>::value &&
		!is_construct_function_callable<Service, Arg, Args...>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"The service construct function cannot be called. "
			"Check if the construct function is well formed, receive injected arguments first and additional parameters at the end."
		);
	}
	
	template<typename Service = T, enable_if_t<
		is_service<Service>::value &&
		has_any_construct<Service>::value &&
		!dependency_trait<is_service, Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value,
			"A dependency or one of their dependencies is not a service. Be sure to use the service definition in the list of dependencies of that service."
		);
	}
	
	template<typename Arg, typename Service = T, enable_if_t<
		is_service<Service>::value &&
		has_any_construct<Service, Arg, Args...>::value &&
		!dependency_trait<is_service, Service, Arg, Args...>::value, int> = 0>
	service_error(Arg&&) {
		static_assert(false_t<Service>::value,
			"A dependency or one of their dependencies is not a service. Be sure to use the service definition in the list of dependencies of that service."
		);
	}
	
	template<typename Service = T, enable_if_t<!is_service<Service>::value && !has_forward<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value, "The type sent to kgr::Container::service(...) is not a service.");
	}
	
	template<typename Service = T, enable_if_t<!is_service<Service>::value && has_forward<Service>::value, int> = 0>
	service_error() {
		static_assert(false_t<Service>::value, "The service type must not contain any virtual functions or virtual inheritance.");
	}
};

struct not_invokable_error {
	template<typename T = void>
	not_invokable_error(...) {
		static_assert(false_t<T>::value,
			"The function sent is not invokable. Ensure to include all services definitions "
			"you need and that received parameters are correct."
		);
	}
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_ERROR_HPP
