#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP

#include "traits.hpp"

namespace kgr {

template<typename...>
struct Map {};

template<typename = void>
struct map_t {};

namespace detail {

template<typename T>
struct is_map : std::false_type {};

template<typename... Ts>
struct is_map<Map<Ts...>> : std::true_type {};

template<typename, typename, typename = void>
struct is_mapped : std::false_type {};

template<typename M, typename S>
struct is_mapped<Map<M>, S, enable_if_t<is_service<decltype(service_map(std::declval<S>(), std::declval<map_t<M> >()))>::value>> : std::true_type {};

template<typename S>
struct is_mapped<void, S, enable_if_t<is_service<decltype(service_map(std::declval<S>()))>::value>> : std::true_type {};

template<typename, typename, typename = void>
struct map_entry {};

template<typename First, typename... Maps, typename P>
struct map_entry<Map<First, Maps...>, P, enable_if_t<!is_mapped<Map<First>, P>::value>> : map_entry<Map<Maps...>, P> {};

template<typename First, typename... Maps, typename P>
struct map_entry<Map<First, Maps...>, P, enable_if_t<is_mapped<Map<First>, P>::value>> {
	using Service = decltype(service_map(std::declval<P>(), std::declval<map_t<First> >()));
};

template<typename P>
struct map_entry<Map<>, P, enable_if_t<is_mapped<Map<>, P>::value>> {
	using Service = decltype(service_map(std::declval<P>(), std::declval<map_t<> >()));
};

template<typename P>
struct map_entry<Map<>, P, enable_if_t<is_mapped<void, P>::value>> {
	using Service = decltype(service_map(std::declval<P>()));
};

template<typename Map, typename T>
using service_map_t = typename map_entry<Map, T>::Service;

template<typename Map, typename T, typename = void>
struct is_complete_map : std::false_type {};

template<typename Map, typename T>
struct is_complete_map<Map, T, void_t<service_map_t<Map, T>>> : std::true_type {};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
