#ifndef KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP

#include <cstdint>
#include <type_traits>

#include "detail/string_view.hpp"
#include "detail/hash.hpp"

#include "detail/define.hpp"

namespace kgr {

/*
 * The type of a type id.
 */
using type_id_t = std::uint64_t;

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
 * We need two bits to encode all kind of services
 */
constexpr auto kind_significant_bits = std::uint64_t{2};

/**
 * Returns which kind of service a given type is
 */
template<typename T>
inline constexpr auto kind_for() noexcept -> service_kind_t {
	return std::is_same<T, override_storage_service>::value
		? service_kind_t::override_storage
		: is_index_storage<T>::value ? service_kind_t::index_storage : service_kind_t::normal;
}

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

} // namespace detail

/*
 * The function that returns the type id.
 * 
 * It uses the pointer to the static data member of a class template to achieve this.
 * Altough the value is not predictible, it's stable.
 */
template <typename T>
inline constexpr auto type_id() -> type_id_t {
	return (
		(detail::hash_64_fnv1a(detail::type_name<T>()) & detail::hash_mask) |
		(static_cast<std::uint64_t>(detail::kind_for<T>()) << (64 - detail::kind_significant_bits))
	);
}

} // namespace kgr

#include "detail/undef.hpp"

#endif // KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
