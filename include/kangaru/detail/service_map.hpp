#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP

#include "traits.hpp"

namespace kgr {
namespace detail {

template<typename, typename, typename = void>
struct map_entry {};

template<typename... Maps, typename P>
struct map_entry<kgr::Map<Maps...>, P, enable_if_t<is_service<decltype(service_map(std::declval<P>(), std::declval<kgr::Map<> >()))>::value>> {
	using Service = decltype(service_map(std::declval<P>(), std::declval<kgr::Map<> >()));
};

template<typename... Maps, typename P>
struct map_entry<kgr::Map<Maps...>, P, enable_if_t<is_service<decltype(service_map(std::declval<P>()))>::value>> {
	using Service = decltype(service_map(std::declval<P>()));
};

template<typename Map, typename P>
struct AdlMap : detail::map_entry<Map, P> {};

template<typename Map, typename T>
using service_map_t = typename AdlMap<Map, T>::Service;

template<typename Map, typename T, typename = void>
struct is_complete_map : std::false_type {};

template<typename Map, typename T>
struct is_complete_map<Map, T, void_t<service_map_t<Map, T>>> : std::true_type {};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
