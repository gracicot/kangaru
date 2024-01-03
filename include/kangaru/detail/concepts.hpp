#ifndef KANGARU5_DETAIL_CONCEPTS_HPP
#define KANGARU5_DETAIL_CONCEPTS_HPP

#include <concepts>
#include <type_traits>

#include "define.hpp"

namespace kangaru {
	template<typename T, typename U>
	concept different_from = not std::same_as<T, U>;
	
	template<typename Forwarded, typename T>
	concept forwarded = std::same_as<T, std::remove_cvref_t<Forwarded>>;
	
	template<typename T>
	concept object = std::is_object_v<T>;
	
	template<typename T>
	concept unqualified_object = object<T> and not std::is_const_v<T> and not std::is_volatile_v<T>;
	
	template<typename T>
	concept reference = not object<T> and not unqualified_object<T> and std::is_reference_v<T>;
	
	template<typename T, typename Self>
	concept not_self = unqualified_object<T> and different_from<T, std::decay_t<Self>>;
	
	template<typename T>
	concept pointer = object<T> and std::is_pointer_v<T>;
	
	template<typename Rhs, typename Lhs>
	concept assign_into = std::assignable_from<Lhs, Rhs>;
	
	// Matches more our usage of syntax for function calling
	template<typename F, typename... Args>
	concept callable = requires(F&& f, Args&&... args) {
		KANGARU5_FWD(f)(KANGARU5_FWD(args)...);
	};
	
	template<typename T, typename... Args>
	concept brace_constructible = requires(Args&&... args) {
		::new T{KANGARU5_FWD(args)...};
	};
	
	template<typename F, typename T, typename... Args>
	concept callable_template1 = requires(F&& f, Args&&... args) {
		KANGARU5_FWD(f).template operator()<T>(KANGARU5_FWD(args)...);
	};
	
	template<typename T, typename U>
	concept allows_construction_of = std::constructible_from<U, T>;
	
	template<typename T>
	concept movable_object = unqualified_object<T> and std::move_constructible<T>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONCEPTS_HPP
