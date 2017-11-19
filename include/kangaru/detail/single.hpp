#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP

#include "traits.hpp"
#include "meta_list.hpp"

namespace kgr {

struct single {
	single() = default;
	~single() = default;
	single(const single&) = delete;
	single& operator=(const single&) = delete;
	single(single&&) = default;
	single& operator=(single&&) = default;
};

struct polymorphic {};
struct final {};
struct supplied : single {};
struct abstract : polymorphic, single {};

template<typename T>
struct defaults_to {
	using default_service = T;
};

template<typename... Types>
struct overrides {
	using parent_types = detail::meta_list<Types...>;
};

namespace detail {

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
using is_virtual = std::integral_constant<bool, std::is_base_of<polymorphic, T>::value || !meta_list_empty<parent_types<T>>::value>;

template<typename Service, typename Overrider>
using is_overriden_by = meta_list_contains<Service, parent_types<Overrider>>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP
