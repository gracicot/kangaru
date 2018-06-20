#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_META_LIST_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_META_LIST_HPP

#include <type_traits>
#include <tuple>

namespace kgr {
namespace detail {

/*
 * This class is a meta list of types.
 * It is used instead of tuples as a lighter alternative to simply hold a list of types.
 */
template<typename... Types>
struct meta_list {};

/*
 * This class is a trait that returns the nth element of a type.
 */
template<std::size_t, typename>
struct meta_list_element;

/*
 * This class is the specialization case where the meta list os empty, and I is at 0.
 * We don't define any alias in that case.
 */
template<std::size_t I>
struct meta_list_element<I, meta_list<>> {};

/*
 * This is the case where I is at 0. We find Head to be the nth element and define an alias to it.
 */
template<typename Head, typename... Tail>
struct meta_list_element<0, meta_list<Head, Tail...>> {
	using type = Head;
};

/*
 * This is the case where I is larger than 0, yet types are still in the list.
 * In that case, we call itself with I - 1 and the Head removed from the list.
 */
template<std::size_t I, typename Head, typename... Tail>
struct meta_list_element<I, meta_list<Head, Tail...>> : meta_list_element<I - 1, meta_list<Tail...>> {};

/*
 * This is an alias for the typedef in meta_list_element_t.
 */
template<std::size_t I, typename List>
using meta_list_element_t = typename meta_list_element<I, List>::type;

/*
 * This class is a trait that tells if a particular type is contained in the list.
 */
template <typename, typename>
struct meta_list_contains;

/*
 * This is the case where the type isn't found.
 * We extends false_type to return the result.
 */
template <typename T>
struct meta_list_contains<T, meta_list<>> : std::false_type {};

/*
 * This is the case where T is different from Head.
 * In this case, we continue to iterate by removeing the Head and comparing the Head again.
 */
template <typename T, typename Head, typename... Tail>
struct meta_list_contains<T, meta_list<Head, Tail...>> : meta_list_contains<T, meta_list<Tail...>> {};

/*
 * This is the case where T is the same as the head. In that case, return true by extending std::true_type.
 */
template <typename T, typename... Tail>
struct meta_list_contains<T, meta_list<T, Tail...>> : std::true_type {};

/*
 * This trait return if a type is in the list at most one time.
 * If the type isn't in the list, the returned value is false.
 * If the type is there multiple times, the return value is false too.
 */
template <typename, typename>
struct meta_list_unique;

/*
* This trait is used to convert a std::tuple into a meta_list
*/
template <typename>
struct to_meta_list;

template <typename... Types>
struct to_meta_list<std::tuple<Types...>> {
	using type = meta_list<Types...>;
};

template<typename T>
using to_meta_list_t = typename to_meta_list<T>::type;

/*
 * Case where we didn't find the type in the list.
 */
template <typename T>
struct meta_list_unique<T, meta_list<>> : std::false_type {};

/*
 * Case where we are still searching the type in the list.
 * We iterate by removing the head and do the comparison again.
 */
template <typename T, typename Head, typename... Tail>
struct meta_list_unique<T, meta_list<Head, Tail...>> : meta_list_unique<T, meta_list<Tail...>> {};

/*
 * This is the case where we found the type T for the first time.
 * In that case, we extend the inverse value returned by meta_list_contains for the rest of the list.
 */
template <typename T, typename... Tail>
struct meta_list_unique<T, meta_list<T, Tail...>> : std::integral_constant<bool, !meta_list_contains<T, meta_list<Tail...>>::value> {};

/*
* This trait simply return the size of the list.
*/
template <typename>
struct meta_list_size;

template <typename... Types>
struct meta_list_size<meta_list<Types...>> {
	static constexpr std::size_t value = sizeof...(Types);
};

/*
* This trait remove the first element of the list.
*/
template <typename>
struct meta_list_pop_front;

template <typename First, typename... Types>
struct meta_list_pop_front<meta_list<First, Types...>> {
	using type = meta_list<Types...>;
};

template <typename List>
using meta_list_pop_front_t = typename meta_list_pop_front<List>::type;

/*
* This trait add an element in the front of the list.
*/
template <typename, typename>
struct meta_list_push_front;

template <typename E, typename... Types>
struct meta_list_push_front<E, meta_list<Types...>> {
	using type = meta_list<E, Types...>;
};

template <typename E, typename List>
using meta_list_push_front_t = typename meta_list_push_front<E, List>::type;

/*
 * This trait apply a metafunction on each element in the list, and return the transformed list.
 */
template<typename, template<typename> class>
struct meta_list_transform;

template<typename... Types, template<typename> class F>
struct meta_list_transform<meta_list<Types...>, F> {
	using type = meta_list<F<Types>...>;
};

/*
 * This is an alias for the transformed list made by meta_list_transform.
 */
template<typename List, template<typename> class F>
using meta_list_transform_t = typename meta_list_transform<List, F>::type;

/*
 * This trait returns if the list is empty.
 */
template<typename>
struct meta_list_empty;

template<typename... Types>
struct meta_list_empty<meta_list<Types...>> : std::integral_constant<bool, sizeof...(Types) == 0> {};


} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_META_LIST_HPP
