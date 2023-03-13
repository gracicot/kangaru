#ifndef KANGARU5_DETAIL_CONCEPTS_HPP
#define KANGARU5_DETAIL_CONCEPTS_HPP

#include <concepts>
#include <type_traits>

#include "define.hpp"

namespace kangaru::detail::concepts {
	template<typename T, typename U>
	concept different_from = not std::same_as<T, U>;
	
	template<typename Forwarded, typename T>
	concept forwarded = std::same_as<T, std::remove_cvref_t<Forwarded>>;
	
	template<typename T>
	concept object = std::is_object_v<T>;
	
	template<typename T>
	concept prvalue = object<T> and not std::is_const_v<T> and not std::is_volatile_v<T>;
	
	// Matches more our usage of syntax for function calling
	template<typename F, typename... Args>
	concept callable = requires(F&& f, Args&&... args) {
		KANGARU5_FWD(f)(KANGARU5_FWD(args)...);
	};

	template<typename T, typename... Args>
	concept brace_constructible = requires(Args&&... args) {
		::new T{KANGARU5_FWD(args)...};
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONCEPTS_HPP
