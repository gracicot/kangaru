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

namespace detail {

template<template<typename> class Map, typename T, typename = void>
struct service_map {
	static_assert(!std::is_same<T, T>::value, "The service sent to the service map is imcomplete. Have you forgot to include your service definition?");
};

template<template<typename> class Map, typename T>
struct service_map<Map, T, void_t<typename Map<T>::Service>> {
	using Service = typename Map<T>::Service;
};

template<template<typename> class Map, typename T>
using service_map_t = typename service_map<Map, T>::Service;

} // namespace detail

using type_id_t = void(*)();
template <typename T> void type_id() {}

} // namespace kgr
