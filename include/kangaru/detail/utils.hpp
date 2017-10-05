#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP

#include <type_traits>

namespace kgr {
namespace detail {

struct no_autocall_t {};

/*
 * Type trait that extract the return type of the forward function.
 */
template<typename, typename = void>
struct ServiceTypeHelper {};

/*
 * Specialization of ServiceTypeHelper when T has a forward function callable without parameter.
 */
template<typename T>
struct ServiceTypeHelper<T, typename std::enable_if<!std::is_same<decltype(std::declval<T>().forward()), void>::value>::type> {
	using type = decltype(std::declval<T>().forward());
};

/*
 * Variable sent as parameter for in_place_t.
 * 
 * Used by the container to desambiguate between contructor that construct the service, and those that doesnt.
 * 
 * We defined the variable before the type because the variable is private, but the type is public.
 */
struct {} constexpr in_place;

} // namespace detail

/**
 * This type is the return type of the forward function of the service T.
 */
template<typename T>
using ServiceType = typename detail::ServiceTypeHelper<T>::type;

/*
 * This is a tag type to mark a constructor as the one the container should use to construct a definition.
 */
using in_place_t = decltype(detail::in_place);

/*
 * This is the instance of no_autocall_t that should be used.
 */
constexpr detail::no_autocall_t no_autocall{};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
