#pragma once

#include "function_traits.hpp"

namespace kgr {

template<typename T>
struct Map {
	using Service = T;
};

template<typename T>
using ServiceType = detail::function_result_t<decltype(&T::forward)>;

} // namespace kgr
