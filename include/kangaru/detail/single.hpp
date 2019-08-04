#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP

#include "traits.hpp"
#include "meta_list.hpp"
#include "detection.hpp"

namespace kgr {

/*
 * Tag base class to identify a single service
 *
 * Also disable copy construction
 */
struct single {
	single() = default;
	~single() = default;
	single(const single&) = delete;
	single& operator=(const single&) = delete;
	single(single&&) = default;
	single& operator=(single&&) = default;
};

/*
 * Bunch of other tag types for various features
 */
struct polymorphic {};
struct final {};
struct supplied {};
struct abstract : polymorphic, single {};

/*
 * Mixin for abstract service to set the default implementation
 */
template<typename T>
struct defaults_to {
	using default_service = T;
};

/*
 * Used to list all types a service should override when constructing
 */
template<typename... Types>
struct overrides {
	using parent_types = detail::meta_list<Types...>;
};

namespace detail {

/*
 * Type trait to either get the specified overrides or an empty list
 */
template<typename, typename = void>
struct parent_type_helper {
	using parent_types = meta_list<>;
};

template<typename T>
struct parent_type_helper<T, void_t<typename T::parent_types>> {
	using parent_types = typename T::parent_types;
};

template<typename T>
using parent_types = typename parent_type_helper<T>::parent_types;

/*
 * Type trait to either get the default implementation type of an abstract serivce
 *
 * Also tell if there is a default or not
 */
template<typename, typename = void>
struct default_type_helper {
	using has_default = std::false_type;
};

template<typename T>
struct default_type_helper<T, void_t<typename T::default_service>> {
	using has_default = std::true_type;
	using service = typename T::default_service;
};

template<typename T>
using default_type = typename default_type_helper<T>::service;

template<typename T>
using has_default = typename default_type_helper<T>::has_default;

template<typename T>
using is_abstract_service = std::is_base_of<abstract, T>;

template<typename T>
using is_single = std::is_base_of<single, T>;

template<typename T>
using is_supplied_service = std::is_base_of<supplied, T>;

template<typename T>
using is_final_service = std::is_base_of<final, T>;

template<typename T>
using is_not_final_service = negation<is_final_service<T>>;

template<typename T>
using is_polymorphic = std::integral_constant<bool, std::is_base_of<polymorphic, T>::value || !meta_list_empty<parent_types<T>>::value>;

template<typename Service, typename Overrider>
using is_overriden_by = meta_list_contains<Service, parent_types<Overrider>>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP
