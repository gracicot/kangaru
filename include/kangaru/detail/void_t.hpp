#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VOID_T_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VOID_T_HPP 

namespace kgr {
namespace detail {

// void_t implementation
template<typename...>
struct voider { using type = void; };

template<typename... Ts> using void_t = typename voider<Ts...>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VOID_T_HPP
