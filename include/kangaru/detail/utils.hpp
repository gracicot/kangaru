#ifndef KGR_INCLUDE_KANGARU_DETAIL_UTILS_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_UTILS_HPP

#include "function_traits.hpp"

namespace kgr {
namespace detail {

struct no_autocall_t {};

} // namespace detail

template<typename T>
struct Map {
	using Service = T;
};

template<typename T>
using ServiceType = decltype(std::declval<T>().forward());

struct in_place_t{};

constexpr in_place_t in_place{};
constexpr detail::no_autocall_t no_autocall{};

namespace detail {

template<template<typename> class Map, typename T>
using service_map_t = typename Map<T>::Service;

} // namespace detail
} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_UTILS_HPP
