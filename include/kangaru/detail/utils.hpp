#pragma once

#include "function_traits.hpp"

namespace kgr {

template<typename T>
struct Map {
	using Service = T;
};

template<typename T>
using ServiceType = detail::function_result_t<decltype(&T::forward)>;

struct in_place_t{};
constexpr in_place_t in_place{};

} // namespace kgr
