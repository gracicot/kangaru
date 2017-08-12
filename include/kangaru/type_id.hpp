#ifndef KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP

namespace kgr {
namespace detail {

template<typename T>
struct type_id_ptr {
	// Having a static data member will ensure us that it has only one address for the whole program.
	// Furthermore, the static data member having different types will ensure it won't get optimized.
	static constexpr T* id = nullptr;
};

template<typename T>
T* const type_id_ptr<T>::id;

} // namespace detail

using type_id_t = const void*;

template <typename T>
constexpr const void* type_id() {
	return &detail::type_id_ptr<T>::id;
}

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
