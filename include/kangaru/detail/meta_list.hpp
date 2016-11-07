#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_META_LIST_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_META_LIST_HPP

#include <type_traits>

namespace kgr {
namespace detail {

template<typename... Types>
struct meta_list {};

template<std::size_t, typename>
struct meta_list_element;

template<std::size_t I>
struct meta_list_element<I, meta_list<>> {};

template<typename Head, typename... Tail>
struct meta_list_element<0, meta_list<Head, Tail...>> {
	using type = Head;
};

template<std::size_t I, typename Head, typename... Tail>
struct meta_list_element<I, meta_list<Head, Tail...>> : meta_list_element<I - 1, meta_list<Tail...>> {};

template <typename, typename>
struct meta_list_contains;

template <typename T>
struct meta_list_contains<T, meta_list<>> : std::false_type {};

template <typename T, typename Head, typename... Tail>
struct meta_list_contains<T, meta_list<Head, Tail...>> : meta_list_contains<T, meta_list<Tail...>> {};

template <typename T, typename... Tail>
struct meta_list_contains<T, meta_list<T, Tail...>> : std::true_type {};

template<std::size_t I, typename List>
using meta_list_element_t = typename meta_list_element<I, List>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_META_LIST_HPP
