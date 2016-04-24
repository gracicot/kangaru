#pragma once

#include "function_traits.hpp"

namespace kgr {

template<typename T>
struct Map {
	using Service = T;
};

template<typename T>
using ServiceType = decltype(std::declval<T>().forward());

struct in_place_t{};
constexpr in_place_t in_place{};

struct no_autocall_t {};
constexpr no_autocall_t no_autocall{};

} // namespace kgr
