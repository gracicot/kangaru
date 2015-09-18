#pragma once

#include <memory>
#include <type_traits>

namespace kgr {
namespace detail {

using type_id_t = void(*)();
template <typename ...T> void type_id() {}

enum class enabler {};

constexpr enabler null = {};

template <bool b, typename T>
using enable_if_t = typename std::enable_if<b, T>::type;

template<int ...>
struct seq {};

template<int n, int ...S>
struct seq_gen : seq_gen<n-1, n-1, S...> {};

template<int ...S>
struct seq_gen<0, S...> {
	using type = seq<S...>;
};

} // namespace detail
} // namespace kgr
