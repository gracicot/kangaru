#ifndef KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP

#include <cstdint>
#include <type_traits>

#include "kangaru/detail/config.hpp"

#ifdef KGR_KANGARU_HASH_TYPE_ID
#include "detail/string_view.hpp"
#include "detail/hash.hpp"
#endif

#include "detail/define.hpp"

namespace kgr {
namespace detail {

/*
 * Declaration of built-in private service types.
 */
template<typename>
struct index_storage;

struct override_storage_service;

/*
 * Trait that identify if some type is an index_storage
 */
template<typename>
struct is_index_storage : std::false_type {};

template<typename T>
struct is_index_storage<index_storage<T>> : std::true_type {};

/*
 * We need to have a small amount of static data in order to
 * get it's pointer. We reuse that space to store meta information.
 */
enum struct service_kind_t : std::uint8_t { normal, override_storage, index_storage };

/**
 * Returns which kind of service a given type is
 */
template<typename T>
inline constexpr auto kind_for() noexcept -> service_kind_t {
	return std::is_same<T, override_storage_service>::value
		? service_kind_t::override_storage
		: is_index_storage<T>::value ? service_kind_t::index_storage : service_kind_t::normal;
}

#ifdef KGR_KANGARU_HASH_TYPE_ID

/**
 * The type of a generated type id.
 */
using type_id_t = std::uint64_t;

/**
 * We need two bits to encode all kind of services
 */
constexpr auto kind_significant_bits = std::uint64_t{2};

/**
 * Returns the kind of service from a type id
 */
inline constexpr auto type_id_kind(type_id_t const id) -> service_kind_t {
	return static_cast<service_kind_t>((id >> (64 - kind_significant_bits)) & 0x2);
}

// Extraction of type names from a function signature
// Implementation taken from TheLartians/StaticTypeInfo
// Link: https://github.com/TheLartians/StaticTypeInfo/blob/master/include/static_type_info/type_name.h
// License: MIT

template<typename T>
inline constexpr auto typed_signature() -> string_view {
	return KGR_KANGARU_FUNCTION_SIGNATURE;
}

constexpr auto signature_prefix_length = std::size_t{typed_signature<int>().find("int")};
constexpr auto signature_postfix_length = std::size_t{typed_signature<int>().size() - signature_prefix_length - string_view{"int"}.size()};

static_assert(signature_prefix_length != string_view::npos, "Cannot find the type name in the function signature");

template<typename T>
inline constexpr auto type_name() -> string_view {
	return typed_signature<T>().substr(
		signature_prefix_length,
		typed_signature<T>().size() - signature_prefix_length - signature_postfix_length
	);
}

/**
 * We need to drop two if the upper bits to encode our metadata about the services
 * This mask will clear the two uppermost bits of a 64 bit number when used with '&'
 */
constexpr auto hash_mask = std::uint64_t{0x4FFFFFFFFFFFFFFF};

template<typename T>
inline constexpr auto type_id_impl() -> type_id_t {
	return (
		(hash_64_fnv1a(type_name<T>()) & hash_mask) |
		(static_cast<std::uint64_t>(kind_for<T>()) << (64 - kind_significant_bits))
	);
}

#else

using type_id_t = void const*;

struct type_id_data {
	service_kind_t const kind;
};

template<typename T>
struct type_id_ptr {
#ifdef KGR_KANGARU_NONCONST_TYPEID
	// On visual studio, we must have the id as non const since it may nonetheless be optimised away
	// It is strongly recommended to use hash based type id on Visual Studio compiler
	static type_id_data id;
#else
	// Having a static data member will ensure us that it has only one address for the whole program.
	static constexpr type_id_data id = type_id_data{kind_for<T>()};
#endif
};

#ifdef KGR_KANGARU_NONCONST_TYPEID
template<typename T>
type_id_data type_id_ptr<T>::id = type_id_data{kind_for<T>()};
#else
template<typename T>
constexpr type_id_data const type_id_ptr<T>::id;
#endif

inline constexpr auto type_id_kind(void const* id) -> service_kind_t {
	return static_cast<type_id_data const*>(id)->kind;
}

template <typename T>
constexpr auto type_id_impl() -> type_id_t {
	return &detail::type_id_ptr<T>::id;
}

#endif // KGR_KANGARU_HASH_TYPE_ID

} // namespace detail

using detail::type_id_t;

/*
 * The function that returns the type id.
 * 
 * It uses the pointer to the static data member of a class template to achieve this.
 * Altough the value is not predictible, it's stable.
 */
template <typename T>
inline constexpr auto type_id() -> type_id_t {
	return detail::type_id_impl<T>();
}

} // namespace kgr

#include "detail/undef.hpp"

#endif // KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
