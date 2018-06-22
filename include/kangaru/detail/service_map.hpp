#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_MAP_HPP

#include "traits.hpp"

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

template<typename S>
struct probe {
	template<typename T, typename U = S, enable_if_t<
		std::is_same<T&, U>::value &&
		!std::is_rvalue_reference<U>::value &&
		!std::is_same<decay_t<U>, U>::value &&
		!std::is_const<T>::value &&
		std::is_constructible<T&, U>::value, int> = 0>
	operator T& ();
	
	template<typename T, typename U = S&&, enable_if_t<
		std::is_same<T&&, U>::value &&
		!std::is_lvalue_reference<U>::value &&
		std::is_constructible<T&&, U>::value, int> = 0>
	operator T&& ();
	
	template<typename T, typename U = S, enable_if_t<
		std::is_same<T const&, U>::value &&
		!std::is_rvalue_reference<U>::value &&
		!std::is_same<decay_t<U>, U>::value &&
		std::is_constructible<T const&, U>::value, int> = 0>
	operator T const& () const;
};

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

template<typename R>
using is_indirect_map = typename is_indirect_map_helper<R>::type;

/*
 * Trait that returns the return type of the service_map function
 */
template<typename, typename, typename = void>
struct map_result {};

/*
* Specialization of map_result when service_map() exist for S with the map M.
*/
template<typename S, typename M>
struct map_result<S, M, enable_if_t<!is_indirect_map<decltype(service_map(std::declval<S>(), std::declval<M>()))>::value>> {
	using type = decltype(service_map(std::declval<S>(), std::declval<M>()));
};

/*
* Specialization of map_result when service_map() exist for S with no map.
*/
template<typename S>
struct map_result<S, void, enable_if_t<!is_indirect_map<decltype(service_map(std::declval<S>()))>::value>> {
	using type = decltype(service_map(std::declval<S>()));
};

/*
* Specialization of map_result when service_map() exist for S with the map M.
*/
template<typename S, typename M>
struct map_result<S, M, enable_if_t<is_indirect_map<decltype(service_map(std::declval<probe<S>>(), std::declval<M>()))>::value>> {
private:
	template<typename Mapped, typename Service>
	struct expand {
		using type = typename Mapped::template mapped_service<Service>;
	};

public:
	using type = typename expand<decltype(service_map(std::declval<probe<S>>(), std::declval<M>())), S>::type;
};

/*
* Specialization of map_result when service_map() exist for S with no map.
*/
template<typename S>
struct map_result<S, void, enable_if_t<is_indirect_map<decltype(service_map(std::declval<probe<S>>()))>::value>> {
private:
	template<typename Mapped, typename Service>
	struct expand {
		using type = typename Mapped::template mapped_service<Service>;
	};

public:
	using type = typename expand<decltype(service_map(std::declval<probe<S>>())), S>::type;
};

/*
 * Alias for map_result::type
 */
template<typename S, typename M>
using map_result_t = typename map_result<S, M>::type;

/*
 * This is a trait that tell if the service S is mapped in M
 */
template<typename, typename, typename = void>
struct is_mapped : std::false_type {};

/*
 * Trait that check if the result of the map is a valid service that forwards something convertible to S.
 */
template<typename M, typename S>
using is_valid_entry = std::integral_constant<bool,
	std::is_convertible<typename std::remove_cv<service_type<map_result_t<S, M>>>::type&&, S&>::value ||
	std::is_convertible<typename std::remove_cv<service_type<map_result_t<S, M>>>::type&&, S&&>::value
>;

/*
 * Specialization when map M maps the service S
 */
template<typename M, typename S>
struct is_mapped<map<M>, S, enable_if_t<is_valid_entry<map_t<M>, S>::value>> : std::true_type {};

/*
 * Specialization for an empty map, equivalent for a void map
 */
template<typename S>
struct is_mapped<map<>, S, enable_if_t<is_valid_entry<map_t<>, S>::value>> : std::true_type {};

/*
 * Specialization when no map is specified.
 */
template<typename S>
struct is_mapped<void, S, enable_if_t<is_valid_entry<void, S>::value>> : std::true_type {};

/*
 * Trait that extranct the mapped service type given the parameter and a group of maps.
 */
template<typename, typename, typename = void>
struct map_entry {};

/*
 * When no service map entry has been found for First, continue with the next map
 */
template<typename First, typename... Maps, typename P>
struct map_entry<map<First, Maps...>, P, enable_if_t<!is_mapped<map<First>, P>::value>> : map_entry<map<Maps...>, P> {};

/*
 * Case when a map has been found.
 * We proceed to create an alias for the mapped definition.
 */
template<typename First, typename... Maps, typename P>
struct map_entry<map<First, Maps...>, P, enable_if_t<is_mapped<map<First>, P>::value>> {
	using mapped_service = map_result_t<P, map_t<First>>;
};

/*
 * Case when a map has been found for an empty map.
 * We proceed to create an alias for the mapped definition.
 */
template<typename P>
struct map_entry<map<>, P, enable_if_t<is_mapped<map<>, P>::value>> {
	using mapped_service = map_result_t<P, map_t<>>;
};

/*
 * Case when the empty map does not yeild any mapped service.
 * We proceed to extend an try the void map.
 */
template<typename P>
struct map_entry<map<>, P, enable_if_t<!is_mapped<map<>, P>::value>> : map_entry<void, P> {};

/*
 * Case when the only service map found has no map specified at all.
 */
template<typename P>
struct map_entry<void, P, enable_if_t<is_mapped<void, P>::value>> {
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
