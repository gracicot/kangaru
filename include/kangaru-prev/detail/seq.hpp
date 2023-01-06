#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SEQ_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SEQ_HPP

#include "meta_list.hpp"

#include <tuple>

namespace kgr {
namespace detail {

template<typename S1, typename S2> struct seq_concat;

template<std::size_t...>
struct seq { using type = seq; };

template<std::size_t... I1, std::size_t... I2>
struct seq_concat<seq<I1...>, seq<I2...>> : seq<I1..., (sizeof...(I1)+I2)...>{};

template<class S1, class S2>
using seq_concat_t = typename seq_concat<S1, S2>::type;

template<std::size_t n>
struct seq_gen : seq_concat_t<typename seq_gen<n / 2>::type, typename seq_gen<n - n / 2>::type> {};

template<>
struct seq_gen<0> : seq<> {};

template<>
struct seq_gen<1> : seq<0> {};

template<>
struct seq_gen<2> : seq<0, 1> {};

template<>
struct seq_gen<3> : seq<0, 1, 2> {};

template<>
struct seq_gen<4> : seq<0, 1, 2, 3> {};

template<>
struct seq_gen<5> : seq<0, 1, 2, 3, 4> {};

template<>
struct seq_gen<6> : seq<0, 1, 2, 3, 4, 5> {};

template<>
struct seq_gen<7> : seq<0, 1, 2, 3, 4, 5, 6> {};

template<>
struct seq_gen<8> : seq<0, 1, 2, 3, 4, 5, 6, 7> {};

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
