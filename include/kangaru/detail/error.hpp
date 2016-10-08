#ifndef KGR_INCLUDE_KANGARU_DETAIL_ERROR_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_ERROR_HPP

namespace kgr {
namespace detail {

template<typename...>
struct false_t { constexpr static bool value = false; };

struct NotInvokableError {
	template<typename T = void>
	NotInvokableError(...) {
		static_assert(false_t<T>::value, "The function sent is not invokable. Ensure to include all services definitions you need and that received parameters are correct.");
	};
};

struct NotAbstracPolymorphicError {
	template<typename T = void>
	NotAbstracPolymorphicError(...) {
		static_assert(false_t<T>::value, "The service T must not be polymorphic or must be abstract.");
	};
};

struct NotServiceError {
	template<typename T = void>
	NotServiceError(...) {
		static_assert(false_t<T>::value, "T is not a service.");
	};
};

struct DependenciesNotServiceError {
	template<typename T = void>
	DependenciesNotServiceError(...) {
		static_assert(false_t<T>::value, "One dependency of the service T is not an actual service.");
	};
};

struct ServiceNotConstructibleError {
	template<typename T = void>
	ServiceNotConstructibleError(...) {
		static_assert(false_t<T>::value, "The service T is not constructible given it's dependencies and given arguments.");
	};
};

struct DependencyNotConstructibleError {
	template<typename T = void>
	DependencyNotConstructibleError(...) {
		static_assert(false_t<T>::value, "One dependency is not constructible with it's dependencies as constructor argument. Please revise that dependency's constructor and dependencies.");
	};
};

} // namespace detail
} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_ERROR_HPP
