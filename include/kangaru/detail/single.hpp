#pragma once

#include <tuple>

namespace kgr {

struct Single {
	Single() = default;
	~Single() = default;
	Single(const Single&) = delete;
	Single& operator=(const Single&) = delete;
	Single(Single&&) = default;
	Single& operator=(Single&&) = default;
};

template<typename... Types>
struct Overrides {
	using ParentTypes = std::tuple<Types...>;
};

namespace detail {

template<typename T> using is_single = std::is_base_of<Single, T>;
template<typename T> using parent_types = typename T::ParentTypes;

} // namespace detail
} // namespace kgr
