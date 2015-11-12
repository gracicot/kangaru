#pragma once

namespace kgr {

template<typename T, T t>
struct Method {
	using Type = T;
	constexpr static T method = t;
};

template<typename T, T t>
constexpr const T Method<T, t>::method;

template<typename...>
struct Invoke;

template<>
struct Invoke<> {};

template<typename Method, typename... Others>
struct Invoke<Method, Others...> {
	using Next = Invoke<Others...>;
	constexpr static typename Method::Type method = Method::method;
};

template<typename Method, typename... Others>
constexpr const typename Method::Type Invoke<Method, Others...>::method;

}
