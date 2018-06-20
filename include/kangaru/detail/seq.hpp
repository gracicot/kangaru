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

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SEQ_HPP
