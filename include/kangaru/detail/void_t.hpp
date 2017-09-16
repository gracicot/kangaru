#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VOID_T_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VOID_T_HPP 

namespace kgr {
namespace detail {

/*
 * This is the void_t implementation.
 * We chose this particular implementation because C++11 don't enforce sfinae otherwise.
 */
template<typename...>
struct voider { using type = void; };

template<typename... Ts> using void_t = typename voider<Ts...>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VOID_T_HPP
