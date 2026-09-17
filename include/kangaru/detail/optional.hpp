#ifndef KANGARU5_DETAIL_OPTIONAL_HPP
#define KANGARU5_DETAIL_OPTIONAL_HPP

#include "define.hpp"

#if KANGARU5_OPTIONAL_IMPLEMENTATION() == 1

#ifndef KANGARU5_MODULES
#include <optional>
#endif

KANGARU5_EXPORT namespace kangaru {
	template<typename T>
	using optional = std::optional<T>;
	
	using std::bad_optional_access;
	using std::nullopt_t;
	using std::in_place_t;
	
	inline constexpr auto nullopt = std::nullopt;
	inline constexpr auto in_place = std::in_place;
} // namespace kangaru

#elif KANGARU5_OPTIONAL_IMPLEMENTATION() == 2

#ifndef KANGARU5_MODULES
#include <tl/optional.hpp>
#endif

KANGARU5_EXPORT namespace kangaru {
	template<typename T>
	using optional = tl::optional<T>;
	
	using tl::bad_optional_access;
	using tl::nullopt_t;
	using tl::in_place_t;
	
	inline constexpr auto nullopt = tl::nullopt;
	inline constexpr auto in_place = tl::in_place;
} // namespace kangaru

#elif KANGARU5_OPTIONAL_IMPLEMENTATION() == 3

// Embedded implementation must be included in modules
#include "tl/optional.hpp"

KANGARU5_EXPORT namespace kangaru {
	template<typename T>
	using optional = detail::tl::optional<T>;
	
	using detail::tl::bad_optional_access;
	using detail::tl::nullopt_t;
	using detail::tl::in_place_t;
	
	inline constexpr auto nullopt = detail::tl::nullopt;
	inline constexpr auto in_place = detail::tl::in_place;
} // namespace kangaru

#endif

#include "undef.hpp"

#endif // KANGARU5_DETAIL_OPTIONAL_HPP
