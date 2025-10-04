#ifndef KANGARU5_DETAIL_CONCEPTS_HPP
#define KANGARU5_DETAIL_CONCEPTS_HPP

#ifndef KANGARU5_MODULES
#include <concepts>
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<typename T, typename U>
	concept different_from = not std::same_as<T, U>;
	
	KANGARU5_EXPORT template<typename Forwarded, typename T>
	concept forwarded = std::same_as<T, std::remove_cvref_t<Forwarded>>;
	
	KANGARU5_EXPORT template<typename T>
	concept object = std::is_object_v<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept forwarded_object = object<std::remove_reference_t<T>>;
	
	KANGARU5_EXPORT template<typename T>
	concept unqualified_object = object<T> and not std::is_const_v<T> and not std::is_volatile_v<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept reference = not object<T> and not unqualified_object<T> and std::is_reference_v<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept lvalue_reference = reference<T> and std::is_lvalue_reference_v<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept rvalue_reference = reference<T> and std::is_rvalue_reference_v<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept movable_object = unqualified_object<T> and std::move_constructible<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept function_object = object<T> and std::move_constructible<T>;
	
	KANGARU5_EXPORT template<typename T, typename Self>
	concept not_self = unqualified_object<T> and different_from<T, std::decay_t<Self>>;
	
	KANGARU5_EXPORT template<typename T>
	concept pointer = object<T> and std::is_pointer_v<T>;
	
	KANGARU5_EXPORT template<typename Rhs, typename Lhs>
	concept assign_into = std::assignable_from<Lhs, Rhs>;
	
	// Matches more our usage of syntax for function calling
	KANGARU5_EXPORT template<typename F, typename... Args>
	concept callable = requires(F&& f, Args&&... args) {
		KANGARU5_FWD(f)(KANGARU5_FWD(args)...);
	};
	
	KANGARU5_EXPORT template<typename F, typename R, typename... Args>
	concept callable_returns =
		    callable<F, Args...>
		and requires(F&& f, Args&&... args) {
			{ KANGARU5_FWD(f)(KANGARU5_FWD(args)...) } -> std::same_as<R>;
		};
	
	KANGARU5_EXPORT template<typename T, typename... Args>
	concept brace_constructible = requires(Args&&... args) {
		T{KANGARU5_FWD(args)...};
	};
	
	KANGARU5_EXPORT template<typename T, typename... Args>
	concept constructor_callable = std::constructible_from<T, Args...> or brace_constructible<T, Args...>;
	
	KANGARU5_EXPORT template<typename F, typename T, typename... Args>
	concept callable_template_1t = requires(F&& f, Args&&... args) {
		KANGARU5_FWD(f).template operator()<T>(KANGARU5_FWD(args)...);
	};
	
	KANGARU5_EXPORT template<typename F, typename R, typename T, typename...Args>
	concept callable_template_1t_returns =
		    callable_template_1t<F, T, Args...>
		and requires(F&& f, Args&&... args) {
			{ KANGARU5_FWD(f).template operator()<T>(KANGARU5_FWD(args)...) } -> std::same_as<R>;
		};
	
	KANGARU5_EXPORT template<typename T, typename U>
	concept allows_construction_of = std::constructible_from<U, T>;
	
	KANGARU5_EXPORT template<typename From, typename To>
	concept explicitly_castable_to = requires(From&& from) {
		static_cast<To>(KANGARU5_FWD(from));
	};
	
	KANGARU5_EXPORT template<typename From, typename To>
	concept safe_convertible_to =
		    std::convertible_to<From, To>
		and (reference<From> or unqualified_object<To>);
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONCEPTS_HPP
