#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP

#include <type_traits>

namespace kgr {
namespace detail {

constexpr std::size_t service_storage_size = sizeof(void*);
constexpr std::size_t service_storage_alignement = alignof(void*);

struct no_autocall_t {};

template<typename, typename = void>
struct ServiceTypeHelper {};

template<typename T>
struct ServiceTypeHelper<T, decltype(void(std::declval<T>().forward()))> {
	using type = decltype(std::declval<T>().forward());
};

template<typename>
struct type_wrapper {};

} // namespace detail

template<typename...>
struct Map;

template<>
struct Map<> {};

template<typename T>
struct Map<T> {
	using Service = T;
};

template<typename T>
using ServiceType = typename detail::ServiceTypeHelper<T>::type;

struct in_place_t{};

constexpr in_place_t in_place{};
constexpr detail::no_autocall_t no_autocall{};

namespace detail {

template<template<typename> class Map, typename T>
using service_map_t = typename Map<T>::Service;

template<typename T>
struct is_map : std::false_type {};

template<typename... Ts>
struct is_map<kgr::Map<Ts...>> : std::true_type {};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
