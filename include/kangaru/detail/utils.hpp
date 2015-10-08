#pragma once

#include <memory>
#include <type_traits>

#include "function_traits.hpp"

namespace kgr {
namespace detail {

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

template<typename T>
struct has_invoke {
private:
	template<typename C> static std::true_type test(decltype(C::invoke)*);
	template<typename C> static std::false_type test(...);
	
public:
	constexpr static bool value = decltype(test<T>(nullptr))::value;
};

template<typename T>
struct has_overrides {
private:
	template<typename C> static std::true_type test(typename C::ParentTypes*);
	template<typename C> static std::false_type test(...);
	
public:
	constexpr static bool value = decltype(test<T>(nullptr))::value;
};

} // namespace detail

template<typename T>
using ServiceType = detail::function_result_t<decltype(&T::forward)>;

} // namespace kgr
