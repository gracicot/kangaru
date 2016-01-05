#pragma once

#include <memory>
#include <type_traits>

#include "function_traits.hpp"

namespace kgr {
namespace detail {

template<typename...> using void_t = void;
using type_id_t = void(*)();
template <typename T> void type_id() {}

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

template<typename T, typename = void>
struct has_invoke : std::false_type {};

template<typename T>
struct has_invoke<T, void_t<typename T::invoke>> : std::true_type {};

template<typename T, typename = void>
struct has_overrides : std::false_type {};

template<typename T>
struct has_overrides<T, void_t<typename T::Next>> : std::true_type {};

template<typename T, typename = void>
struct has_next : std::false_type {};

template<typename T>
struct has_next<T, void_t<typename T::Next>> : std::true_type {};

} // namespace detail

template<typename T>
using ServiceType = detail::function_result_t<decltype(&T::forward)>;

} // namespace kgr
