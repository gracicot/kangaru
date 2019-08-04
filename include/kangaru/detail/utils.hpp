#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP

#include <type_traits>

namespace kgr {
namespace detail {

/*
 * Dependent type false
 * Useful for static asserts
 */
template<typename...>
struct to_false {
	using type = std::false_type;
};

template<typename... Ts>
using false_t = typename to_false<Ts...>::type;

/*
 * Missing utility taken from C++17
 */
template<bool b>
using bool_constant = std::integral_constant<bool, b>;

/*
 * Simple alias to member type `value_type`
 * Used mostly for detection.
 */
template<typename T>
using value_type_t = typename T::value_type;

/*
 * Variable sent as parameter for in_place_t.
 * 
 * Used by the container to desambiguate between contructor that construct the service, and those that doesnt.
 * 
 * We defined the variable before the type because the variable is private, but the type is public.
 */
struct {} constexpr in_place{};

} // namespace detail

/*
 * This is a tag type to mark a constructor as the one the container should use to construct a definition.
 */
using in_place_t = decltype(detail::in_place);

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
