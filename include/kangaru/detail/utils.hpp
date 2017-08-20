#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP

#include <type_traits>

namespace kgr {
namespace detail {

struct no_autocall_t {};

template<typename, typename = void>
struct ServiceTypeHelper {};

template<typename T>
struct ServiceTypeHelper<T, decltype(void(std::declval<T>().forward()))> {
	using type = decltype(std::declval<T>().forward());
};

} // namespace detail

template<typename T>
using ServiceType = typename detail::ServiceTypeHelper<T>::type;

struct in_place_t{};

constexpr in_place_t in_place{};
constexpr detail::no_autocall_t no_autocall{};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
