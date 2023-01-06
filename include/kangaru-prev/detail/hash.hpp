#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_HASH_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_HASH_HPP

#include "string_view.hpp"

#include <cstdint>

#include "define.hpp"

namespace kgr {
namespace detail {

// C++11 compile type hash function for strings
// Implementation taken from TheLartians/StaticTypeInfo
// Link: https://github.com/TheLartians/StaticTypeInfo/blob/master/include/static_type_info/hash.h
// License: MIT

// Implementation also inspired from ruby0x1/hash_fnv1a.h
// Link: https://gist.github.com/ruby0x1/81308642d0325fd386237cfa3b44785c
// License: public domain


constexpr auto val_64_const = std::uint64_t{0xcbf29ce484222325};
constexpr auto prime_64_const = std::uint64_t{0x100000001b3};

inline constexpr auto hash_64_fnv1a(string_view const str, std::uint64_t const value = val_64_const) noexcept -> std::uint64_t {
#ifdef KGR_KANGARU_HASH_EXTENDED_CONSTEXPR
	auto hash = value;
	auto const len = str.size();
	for(std::size_t i = 0; i < len; ++i) {
		uint8_t value = static_cast<uint8_t>(str[i]);
		hash = hash ^ value;
		hash *= prime_64_const;
	}

	return hash;
#else
	return (str.size() == 0) ? value : hash_64_fnv1a(str.substr(1), (value ^ static_cast<std::uint64_t>(str[0])) * prime_64_const);
#endif
}

}
}

#include "undef.hpp"

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_HASH_HPP
