#pragma once

#include "traits.hpp"

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

template<typename T, typename = void>
struct parent_type_helper {
	using ParentTypes = std::tuple<>;
};

template<typename T>
struct parent_type_helper<T, void_t<typename T::ParentTypes>> {
	using ParentTypes = typename T::ParentTypes;
};

template<typename T> using parent_types = typename parent_type_helper<T>::ParentTypes;

} // namespace detail
} // namespace kgr
