#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP

#include "traits.hpp"
#include "detection.hpp"

namespace kgr {

/*
 * This class is the type when referring to a group of maps.
 * This can be used with kgr::autocall and mapped operator services.
 */
template<typename...>
struct map {};

/*
 * This is the type you receive in a service_map function.
 * This was previously kgr::Map in kangaru 3.x.y
 */
template<typename = void>
struct map_t {};

namespace detail {

inline auto service_map() -> void;

/*
 * Alias to the expression of a normal named service map
 */
template<typename S, typename M>
using normal_map_result_mapped = decltype(service_map(std::declval<S>(), std::declval<M>()));

/*
 * Alias to the expression of a normal non named service map
 */
template<typename S>
using normal_map_result_void = decltype(service_map(std::declval<S>()));

#if (defined(__clang__) && __clang_major__ < 7) || (defined(_MSC_VER) && !defined(__clang__)) || defined(__APPLE__)
template<typename S>
struct probe {
	template<typename T, enable_if_t<
		std::is_same<T&, S>::value &&
		!std::is_const<T>::value, int> = 0>
	operator T& ();
	
	template<typename T, enable_if_t<
		std::is_same<T&&, S&&>::value &&
		!std::is_const<T>::value, int> = 0>
	operator T&& ();
	
	template<typename T, enable_if_t<
		std::is_same<T const&, S>::value, int> = 0>
	operator T const& () const;
	
	template<typename T, enable_if_t<
		std::is_same<T const&&, S&&>::value, int> = 0>
	operator T const&& () const;
};

/*
 * Alias to the expression of a normal mapped service map
 */
template<typename S, typename M>
using probed_map_result_mapped = normal_map_result_mapped<probe<S>, M>;

/*
 * Alias to the expression of a normal mapped service map
 */
template<typename S>
using probed_map_result_void = normal_map_result_void<probe<S>>;
#else
template<typename S>
struct probe {
	template<typename T, enable_if_t<std::is_same<T&, S>::value, int> = 0>
	operator T& ();
	
	template<typename T, enable_if_t<std::is_same<T&&, S&&>::value, int> = 0>
	operator T&& ();
};

/*
 * Alias to the expression of a normal mapped service map
 */
template<typename S, typename M, typename Result = detected_or<detected_t<normal_map_result_mapped, probe<constify_t<S>>, M>, normal_map_result_mapped, probe<S>, M>>
using probed_map_result_mapped = enable_if_t<such<Result>::value, Result>;

/*
 * Alias to the expression of a normal mapped service map
 */
template<typename S, typename Result = detected_or<detected_t<normal_map_result_void, probe<constify_t<S>>>, normal_map_result_void, probe<S>>>
using probed_map_result_void = enable_if_t<such<Result>::value, Result>;
#endif

/*
 * Trait that determines if a type is a map type.
 */
template<typename T>
struct is_map : std::false_type {};

/*
 * This is the specialization when T is a kgr::map.
 */
template<typename... Ts>
struct is_map<map<Ts...>> : std::true_type {};

/*
 * Type trait that check if a particular type is an indirect map.
 */
template<typename R>
struct is_indirect_map_helper {
private:
	template<template<typename> class>
	static auto test_helper() -> std::true_type;
	
	template<typename T>
	static auto test(int) -> decltype(test_helper<T::template mapped_service>());
	
	template<typename>
	static auto test(void*) -> std::false_type;
	
public:
	using type = decltype(test<R>(0));
};

/*
 * Alias to the is_indirect_map_helper trait.
 */
template<typename R>
using is_indirect_map = typename is_indirect_map_helper<R>::type;

/*
 * helper implementation for the indirect map.
 */
template<typename Indirect, typename Service>
struct indirect_map {
	using type = typename Indirect::template mapped_service<Service>;
};

/*
 * Trait that returns the return type of the service_map function
 */
template<typename, typename, typename = void>
struct map_result {};

//TODO: In kangaru 5, disable implicit conversion in service maps.
/*
 * Specialization of map_result when service_map() exist for S with the map M.
 */
template<typename S, typename M>
struct map_result<S, M, enable_if_t<!is_indirect_map<normal_map_result_mapped<S, M>>::value>> {
	using type = normal_map_result_mapped<S, M>;
};

/*
 * Specialization of map_result when service_map() exist for S with no map.
 */
template<typename S>
struct map_result<S, void, enable_if_t<!is_indirect_map<normal_map_result_void<S>>::value>> {
	using type = normal_map_result_void<S>;
};

/*
 * Specialization of map_result when service_map() exist for S with the map M.
 */
template<typename S, typename M>
struct map_result<S, M, enable_if_t<is_detected<probed_map_result_mapped, S, M>::value && is_indirect_map<normal_map_result_mapped<S, M>>::value>> :
	indirect_map<normal_map_result_mapped<S, M>, S> {};

/*
 * Specialization of map_result when service_map() exist for S with no map.
 */
template<typename S>
struct map_result<S, void, enable_if_t<is_detected<probed_map_result_void, S>::value && is_indirect_map<normal_map_result_void<S>>::value>> :
	indirect_map<normal_map_result_void<S>, S> {};

/*
 * Alias for map_result::type
 */
template<typename S, typename M>
using map_result_t = typename map_result<S, M>::type;

/*
 * Trait that check if the result of the map is a valid service that forwards something convertible to S.
 */
template<typename M, typename S, typename Result = typename std::remove_cv<service_type<map_result_t<S, M>>>::type&&>
using is_valid_entry = std::integral_constant<bool,
	std::is_convertible<Result, S&>::value || std::is_convertible<Result, S&&>::value
>;

/*
 * Trait that extranct the mapped service type given the parameter and a group of maps.
 */
template<typename, typename, typename = void>
struct map_entry {};

/*
 * When no service map entry has been found for First, continue with the next map
 */
template<typename First, typename... Maps, typename P>
struct map_entry<map<First, Maps...>, P, enable_if_t<!detected_or<std::false_type, is_valid_entry, map_t<First>, P>::value>> : map_entry<map<Maps...>, P> {};

/*
 * Case when a map has been found.
 * We proceed to create an alias for the mapped definition.
 */
template<typename First, typename... Maps, typename P>
struct map_entry<map<First, Maps...>, P> {
	using mapped_service = map_result_t<P, map_t<First>>;
};

/*
 * Case when a map has been found for an empty map.
 * We proceed to create an alias for the mapped definition.
 */
template<typename P>
struct map_entry<map<>, P, enable_if_t<is_valid_entry<map_t<>, P>::value>> {
	using mapped_service = map_result_t<P, map_t<>>;
};

/*
 * Case when the empty map does not yeild any mapped service.
 * We proceed to extend an try the void map.
 */
template<typename P>
struct map_entry<map<>, P, enable_if_t<!detected_or<std::false_type, is_valid_entry, map_t<>, P>::value>> : map_entry<void, P> {};

/*
 * Case when the only service map found has no map specified at all.
 */
template<typename P>
struct map_entry<void, P, enable_if_t<is_valid_entry<void, P>::value>> {
	using mapped_service = map_result_t<P, void>;
};

/*
 * Trait to tell if a parameter is mapped using the map group Map.
 */
template<typename Map, typename T, typename = void>
struct is_complete_map : std::false_type {};

/*
 * Specialization of is_complete_map when the service T is mapped in Map
 */
template<typename... Maps, typename T>
struct is_complete_map<map<Maps...>, T, void_t<typename map_entry<map<Maps...>, T>::mapped_service>> : std::true_type {};

} // namespace detail

/*
 * This is an alias for the mapped definition in a service map.
 */
template<typename T, typename Map = map<>>
using mapped_service_t = typename detail::map_entry<Map, T>::mapped_service;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
