#ifndef KGR_INCLUDE_KANGARU_TYPE_ID
#define KGR_INCLUDE_KANGARU_TYPE_ID

namespace kgr {

using type_id_t = void(*)();
template <typename T> void type_id() {}

} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_TYPE_ID
