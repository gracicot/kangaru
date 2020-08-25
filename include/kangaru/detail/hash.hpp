#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_HASH_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_HASH_HPP

#include "string_view.hpp"

#include <cstdint>

namespace kgr {
namespace detail {

// C++11 compile type hash function for strings
// Implementation taken from TheLartians/StaticTypeInfo
// Link: https://github.com/TheLartians/StaticTypeInfo/blob/master/include/static_type_info/hash.h
// License: MIT

constexpr auto val_64_const = std::uint64_t{0xCBF29CE484222325};
constexpr auto prime_64_const = std::uint64_t{0x100000001B3};

inline constexpr auto hash_64_fnv1a(string_view const str, std::uint64_t const value = val_64_const) noexcept -> std::uint64_t {
	return (str.size() == 0) ? value : hash_64_fnv1a(str.substr(1), (value ^ static_cast<std::uint64_t>(str[0])) * prime_64_const);
}

}
}

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_HASH_HPP
