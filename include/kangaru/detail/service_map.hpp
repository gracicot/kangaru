#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP

#include "traits.hpp"

namespace kgr {

/*
 * This class is the type when referring to a group of maps.
 * This can be used with kgr::AutoCall and mapped operator services.
 */
template<typename...>
struct Map {};

/*
 * This is the type you receive in a service_map function.
 * This was previously kgr::Map in kangaru 3.x.y
 */
template<typename = void>
struct map_t {};

namespace detail {

/*
 * Trait that determines if a type is a map type.
 */
template<typename T>
struct is_map : std::false_type {};

/*
 * This is the specialization when T is a kgr::Map.
 */
template<typename... Ts>
struct is_map<Map<Ts...>> : std::true_type {};

/*
 * This is a trait that tell if the service S is mapped in M
 */
template<typename, typename, typename = void>
struct is_mapped : std::false_type {};

template<typename M, typename S>
using is_valid_entry = std::is_convertible<ServiceType<decltype(service_map(std::declval<S>(), std::declval<M>()))>, S>;

template<typename S>
using is_valid_void_entry = std::is_convertible<ServiceType<decltype(service_map(std::declval<S>()))>, S>;

/*
 * Specialization when map M maps the service S
 */
template<typename M, typename S>
struct is_mapped<Map<M>, S, enable_if_t<is_valid_entry<map_t<M>, S>::value>> : std::true_type {};

/*
 * Specialization for an empty map, equivalent for a void map
 */
template<typename S>
struct is_mapped<Map<>, S, enable_if_t<is_valid_entry<map_t<>, S>::value>> : std::true_type {};

/*
 * Specialization when no map is specified.
 */
template<typename S>
struct is_mapped<void, S, enable_if_t<is_valid_void_entry<S>::value>> : std::true_type {};

/*
 * Trait that extranct the mapped service type given the parameter and a group of maps.
 */
template<typename, typename, typename = void>
struct map_entry {};

/*
 * When no service map entry has been found for First, continue with the next map
 */
template<typename First, typename... Maps, typename P>
struct map_entry<Map<First, Maps...>, P, enable_if_t<!is_mapped<Map<First>, P>::value>> : map_entry<Map<Maps...>, P> {};

/*
 * Case when a map has been found.
 * We proceed to create an alias for the mapped definition.
 */
template<typename First, typename... Maps, typename P>
struct map_entry<Map<First, Maps...>, P, enable_if_t<is_mapped<Map<First>, P>::value>> {
	using Service = decltype(service_map(std::declval<P>(), std::declval<map_t<First> >()));
};

/*
 * Case when a map has been found for an empty map.
 * We proceed to create an alias for the mapped definition.
 */
template<typename P>
struct map_entry<Map<>, P, enable_if_t<is_mapped<Map<>, P>::value>> {
	using Service = decltype(service_map(std::declval<P>(), std::declval<map_t<> >()));
};

/*
 * Case when the empty map does not yeild any mapped service.
 * We proceed to extend an try the void map.
 */
template<typename P>
struct map_entry<Map<>, P, enable_if_t<!is_mapped<Map<>, P>::value>> : map_entry<void, P> {};

/*
 * Case when the only service map found has no map specified at all.
 */
template<typename P>
struct map_entry<void, P, enable_if_t<is_mapped<void, P>::value>> {
	using Service = decltype(service_map(std::declval<P>()));
};

/*
 * Trait to tell if a parameter is mapped using the map group Map.
 */
template<typename Map, typename T, typename = void>
struct is_complete_map : std::false_type {};

template<typename Map, typename T>
struct is_complete_map<Map, T, void_t<typename detail::map_entry<Map, T>::Service>> : std::true_type {};

} // namespace detail

/*
 * This is an alias for the mapped definition in a service map.
 */
template<typename T, typename Map = kgr::Map<>>
using service_map_t = typename detail::map_entry<Map, T>::Service;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
