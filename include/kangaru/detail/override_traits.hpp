#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_TRAITS_HPP

#include "traits.hpp"
#include "meta_list.hpp"
#include "detection.hpp"

namespace kgr {
namespace detail {

/*
 * Trait that check if all types specified in overrides are services
 */
template<typename T>
using is_override_services = all_of_traits<parent_types<T>, is_service>;

/*
 * Trait that check if the service type returned by
 * one override's service definitions can be converted to the service type of the service T definition.
 */
template<typename Parent, typename T>
using is_one_override_convertible = is_explicitly_convertible<detected_t<service_type, T>, detected_t<service_type, Parent>>;

/*
 * Trait that check if the service type returned by
 * all overriden service definitions can be converted to the service type of the service T definition.
 */
template<typename T>
using is_override_convertible = all_of_traits<parent_types<T>, is_one_override_convertible, T>;

/*
 * Trait that check if all overriden services are polymorphic
 */
template<typename T>
using is_override_polymorphic = all_of_traits<parent_types<T>, is_polymorphic>;

/*
 * Trait that check if no overriden services are final
 */
template<typename T>
using is_override_not_final = all_of_traits<parent_types<T>, is_not_final_service>;

/*
 * Trait that check if the default service of an abstract service overrides that abstract service
 */
template<typename T>
using is_default_overrides_abstract = bool_constant<
	!has_default<T>::value || is_overriden_by<T, detected_t<default_type, T>>::value
>;

/*
 * Trait that check if the default service type is convertible to the abstract service type.
 */
template<typename T>
using is_default_convertible = bool_constant<
	!has_default<T>::value || is_explicitly_convertible<
		detected_t<service_type, detected_t<default_type, T>>,
		detected_t<service_type, T>
	>::value
>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_TRAITS_HPP
