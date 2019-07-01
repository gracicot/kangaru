#ifndef KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP

#include <cstdint>

namespace kgr {
namespace detail {

template<typename>
struct index_storage;

struct override_storage_service;

template<typename>
struct is_index_storage : std::false_type {};

template<typename T>
struct is_index_storage<index_storage<T>> : std::true_type {};

struct type_id_data {
	enum struct kind_t : std::uint8_t { normal, override_storage, index_storage } kind;
	
	template<typename T>
	static constexpr auto kind_for() noexcept -> type_id_data {
		return type_id_data{
			std::is_same<T, override_storage_service>::value
			? kind_t::override_storage
			: is_index_storage<T>::value ? kind_t::index_storage : kind_t::normal
		};
	}
};

/*
 * Template class that hold the declaration of the id.
 * 
 * We use the pointer of this id as type id.
 */
template<typename T>
struct type_id_ptr {
	struct id_t {
		type_id_data data;
	};

	// Having a static data member will ensure us that it has only one address for the whole program.
	// Furthermore, the static data member having different types will ensure it won't get optimized.
	static constexpr id_t id = id_t{type_id_data::kind_for<T>()};
};

/*
 * Definition of the id.
 * 
 * Before that, we used the pointer to a function as type_id.
 * 
 * However, on some platform, the rule that a pointer to a function
 * always yeild to the same value, no matter the dll or TU was not always respected.
 * 
 * Using the pointer of a static data member is more stable.
 */
template<typename T>
constexpr typename type_id_ptr<T>::id_t const type_id_ptr<T>::id;

inline constexpr auto type_id_kind(void const* id) -> type_id_data::kind_t {
	return static_cast<type_id_data const*>(id)->kind;
}

} // namespace detail

/*
 * The type of a type id.
 */
using type_id_t = void const*;

/*
 * The function that returns the type id.
 * 
 * It uses the pointer to the static data member of a class template to achieve this.
 * Altough the value is not predictible, it's stable.
 */
template <typename T>
constexpr auto type_id() -> type_id_t {
	return &detail::type_id_ptr<T>::id;
}

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
