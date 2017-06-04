#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP

#include "traits.hpp"
#include "meta_list.hpp"

namespace kgr {

struct Single {
	Single() = default;
	~Single() = default;
	Single(const Single&) = delete;
	Single& operator=(const Single&) = delete;
	Single(Single&&) = default;
	Single& operator=(Single&&) = default;
};

struct Abstract {};
struct Virtual {};

template<typename T>
struct Default {
	using DefaultService = T;
};

template<typename... Types>
struct Overrides {
	using ParentTypes = detail::meta_list<Types...>;
};

namespace detail {

template<typename, typename = void>
struct parent_type_helper {
	using ParentTypes = meta_list<>;
};

template<typename T>
struct parent_type_helper<T, void_t<typename T::ParentTypes>> {
	using ParentTypes = typename T::ParentTypes;
};

template<typename T>
using parent_types = typename parent_type_helper<T>::ParentTypes;

template<typename, typename = void>
struct default_type_helper {
	using has_default = std::false_type;
};

template<typename T>
struct default_type_helper<T, void_t<typename T::DefaultService>> {
	using has_default = std::true_type;
	using Service = typename T::DefaultService;
};

template<typename T>
using default_type = typename default_type_helper<T>::Service;

template<typename T>
using has_default = typename default_type_helper<T>::has_default;

template<typename T>
using is_abstract_service = std::integral_constant<bool, std::is_base_of<Abstract, T>::value || std::is_abstract<T>::value>;

template<typename T>
using is_single = std::integral_constant<bool, std::is_base_of<Single, T>::value || is_abstract_service<T>::value>;

template<typename T>
using is_virtual = std::integral_constant<bool, is_abstract_service<T>::value ||std::is_base_of<Virtual, T>::value || (meta_list_size<parent_types<T>>::value > 0)>;

template<typename Service, typename Overrider>
using is_overriden_by = meta_list_contains<Service, parent_types<Overrider>>;

template<typename T>
using is_service_embeddable = std::integral_constant<bool,
	std::is_trivially_destructible<T>::value && !is_virtual<T>::value &&
	sizeof(T) <= service_storage_size && alignof(T) <= service_storage_alignement
>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SINGLE_HPP
