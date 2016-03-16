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

} // namespace kgr
