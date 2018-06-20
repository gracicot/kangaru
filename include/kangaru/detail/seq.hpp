#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SEQ_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SEQ_HPP

#include "meta_list.hpp"

#include <tuple>

namespace kgr {
namespace detail {

template<std::size_t ...>
struct seq {};

template<std::size_t n, std::size_t ...S>
struct seq_gen : seq_gen<n-1, n-1, S...> {};

template<std::size_t ...S>
struct seq_gen<0, S...> {
	using type = seq<S...>;
};

template<typename>
struct tuple_seq_gen;

template<typename... Types>
struct tuple_seq_gen<std::tuple<Types...>> : seq_gen<sizeof...(Types)> {};

template<typename... Types>
struct tuple_seq_gen<detail::meta_list<Types...>> : seq_gen<sizeof...(Types)> {};

template<typename Tuple>
using tuple_seq = typename tuple_seq_gen<Tuple>::type;

template<typename>
struct seq_drop_first;

template<std::size_t... S>
struct seq_drop_first<seq<0, S...>> {
	using type = seq<S...>;
};

inline constexpr auto safe_minus(std::size_t lhs, std::size_t rhs) -> std::size_t {
	return lhs - (lhs > rhs ? rhs : lhs);
}

template<typename S>
using seq_drop_first_t = typename seq_drop_first<S>::type;

template<typename List, std::size_t n>
using tuple_seq_minus = typename detail::seq_gen<safe_minus(meta_list_size<List>::value, n)>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SEQ_HPP
