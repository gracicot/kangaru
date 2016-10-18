#ifndef KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP

namespace kgr {

using type_id_t = void(*)();
template <typename T> void type_id() {}

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
