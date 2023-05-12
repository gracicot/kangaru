#ifndef KANGARU5_DETAIL_UTILITY_HPP
#define KANGARU5_DETAIL_UTILITY_HPP

#include <type_traits>
#include <utility>

#include "define.hpp"

namespace kangaru::detail::utility {
	template<typename T, typename U> [[nodiscard]]
	constexpr auto forward_like(U&& x) noexcept -> auto&& {
		constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;
		if constexpr (std::is_lvalue_reference_v<T&&>) {
			if constexpr (is_const) {
				return std::as_const(x);
			} else {
				return static_cast<U&>(x);
			}
		} else {
			if constexpr (is_const) {
				return std::move(std::as_const(x));
			} else {
				return std::move(x);
			}
		}
	}
	
	auto decay_copy(auto&& v) -> std::decay_t<decltype(v)> {
		return KANGARU5_FWD(v);
	}
	
	template<typename T, typename U>
	using forward_like_t = decltype(forward_like<T>(std::declval<U&>()));
	
	template<typename T, std::size_t>
	using expand = T;
	
	template<typename... Functions>
	struct overload : Functions... {
		using Functions::operator()...;
	};
	
	template<typename... Functions>
	overload(Functions...) -> overload<Functions...>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_UTILITY_HPP
