#ifndef KANGARU5_DETAIL_UTILITY_HPP
#define KANGARU5_DETAIL_UTILITY_HPP

#include <type_traits>
#include <utility>

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

	template<typename T, typename U>
	using forward_like_t = decltype(forward_like<T>(std::declval<U&>()));
	
	template<typename T, std::size_t>
	using expand = T;
}

#endif // KANGARU5_DETAIL_UTILITY_HPP
