#ifndef KANGARU5_DETAIL_CONCEPTS_HPP
#define KANGARU5_DETAIL_CONCEPTS_HPP

#ifndef KANGARU5_MODULES
#include <concepts>
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru::detail::concepts_private {
	template<typename Type, typename... Pack>
	consteval auto count_type_in_pack() -> std::size_t {
		return ((std::same_as<Type, Pack> ? std::size_t{1} : std::size_t{0}) + ... + std::size_t{0});
	}
}

KANGARU5_EXPORT namespace kangaru {
	template<typename T, typename U>
	concept different_from = not std::same_as<T, U>;
	
	template<typename Forwarded, typename T>
	concept forwarded = std::same_as<T, std::remove_cvref_t<Forwarded>>;
	
	template<typename T>
	concept object = std::is_object_v<T>;
	
	template<typename T>
	concept forwarded_object = object<std::remove_reference_t<T>>;
	
	template<typename T>
	concept unqualified_object = object<T> and not std::is_const_v<T> and not std::is_volatile_v<T>;
	
	template<typename T>
	concept reference = not object<T> and not unqualified_object<T> and std::is_reference_v<T>;
	
	template<typename T>
	concept lvalue_reference = reference<T> and std::is_lvalue_reference_v<T>;
	
	template<typename T>
	concept rvalue_reference = reference<T> and std::is_rvalue_reference_v<T>;
	
	template<typename T>
	concept movable_object = unqualified_object<T> and std::move_constructible<T>;
	
	template<typename T>
	concept forwarded_movable_object = movable_object<std::remove_cvref_t<T>>;
	
	template<typename T>
	concept copiable_object =
		    movable_object<T>
		and unqualified_object<T>
		and std::copy_constructible<T>;
	
	template<typename T>
	concept forwarded_copiable_object = copiable_object<std::remove_cvref_t<T>>;
	
	template<typename T>
	concept function_object = object<T> and std::move_constructible<T>;
	
	template<typename T>
	concept forwarded_function_object = function_object<std::remove_cvref_t<T>>;
	
	template<typename T, typename Self>
	concept not_self = unqualified_object<Self> and different_from<std::decay_t<T>, Self>;
	
	template<typename T>
	concept pointer = object<T> and std::is_pointer_v<T>;
	
	template<typename T>
	concept pointer_to_member = std::is_member_pointer_v<T>;
	
	template<typename T>
	concept pointer_to_member_function = pointer_to_member<T> and std::is_member_function_pointer_v<T>;
	
	template<typename T>
	concept weak_injectable = unqualified_object<T> or reference<T>;
	
	// Matches more our usage of syntax for function calling
	template<typename F, typename... Args>
	concept callable = requires(F&& f, Args&&... args) {
		KANGARU5_FWD(f)(KANGARU5_FWD(args)...);
	};
	
	template<typename R, typename F, typename... Args>
	concept callable_returns =
		    callable<F, Args...>
		and requires(F&& f, Args&&... args) {
			{ KANGARU5_FWD(f)(KANGARU5_FWD(args)...) } -> std::same_as<R>;
		};
	
	template<typename T, typename... Args>
	concept brace_constructible = requires(Args&&... args) {
		T{KANGARU5_FWD(args)...};
	};
	
	template<typename F, typename T, typename... Args>
	concept callable_template_1t = requires(F&& f, Args&&... args) {
		KANGARU5_FWD(f).template operator()<T>(KANGARU5_FWD(args)...);
	};
	
	template<typename R, typename F, typename T, typename...Args>
	concept callable_template_1t_returns =
		    callable_template_1t<F, T, Args...>
		and requires(F&& f, Args&&... args) {
			{ KANGARU5_FWD(f).template operator()<T>(KANGARU5_FWD(args)...) } -> std::same_as<R>;
		};
	
	template<typename T, typename U>
	concept allows_construction_of = std::constructible_from<U, T&&>;
	
	template<typename From, typename To>
	concept explicitly_castable_to = requires(From&& from) {
		static_cast<To>(KANGARU5_FWD(from));
	};
	
	// TODO: Actually not safe when converting to a type with ref semantics
	template<typename From, typename To>
	concept safe_convertible_to =
		    std::convertible_to<From, To>
		and (reference<From> or unqualified_object<To>);
	
	template<typename... Pack>
	concept pack_distinct = (... and (detail::concepts_private::count_type_in_pack<Pack, Pack...>() == 1));
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONCEPTS_HPP
