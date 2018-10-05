#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DETECTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DETECTION_HPP

#include "meta_list.hpp"
#include "void_t.hpp"
#include "utils.hpp"
#include "seq.hpp"

namespace kgr {
namespace detail {

/*
 * The following namespace contains the detection idiom implementation
 */
namespace detail_detection {

template <typename Default, typename AlwaysVoid, template<typename...> class Op, typename... Args>
struct detector {
	using value_t = std::false_type;
	using type = Default;
};

template <typename Default, template<typename...> class Op, typename... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
	using value_t = std::true_type;
	using type = Op<Args...>;
};

/*
 * This higher order metafunction only instantiate a template using `detector` if a condition is satisfied.
 * Useful when the instantiation of a template is known to trigger a hard error.
 */
template<bool, typename Default, template<typename...> class, typename...>
struct instantiate_if {
	using type = Default;
};

template<typename Default, template<typename...> class Template, typename... Args>
struct instantiate_if<true, Default, Template, Args...> {
	using type = typename detector<Default, void, Template, Args...>::type;
};

} // namespace detail_detection

/*
 * Represent nothing. Used as a non-void nothing type.
 */
struct nonesuch {
	nonesuch() = delete;
	~nonesuch() = delete;
	nonesuch(nonesuch const&) = delete;
	void operator=(nonesuch const&) = delete;
};

/*
 * Alias to the detectors.
 */
template <template<class...> class Op, typename... Args>
using is_detected = typename detail_detection::detector<nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, typename... Args>
using detected_t = typename detail_detection::detector<nonesuch, void, Op, Args...>::type;

template <typename Default, template<class...> class Op, typename... Args>
using detected_or = typename detail_detection::detector<Default, void, Op, Args...>::type;

template <bool b, template<class...> class Template, typename... Args>
using instantiate_if_t = typename detail_detection::instantiate_if<b, nonesuch, Template, Args...>::type;

template <bool b, typename Default, template<class...> class Template, typename... Args>
using instantiate_if_or = typename detail_detection::instantiate_if<b, Default, Template, Args...>::type;

/*
 * Simple meta operators missing from C++11 and C++14.
 */
template<typename B>
struct negation : bool_constant<!bool(B::value)> {};

template<typename...> struct conjunction : std::true_type {};
template<typename B1> struct conjunction<B1> : B1 {};
template<typename B1, typename... Bn>
struct conjunction<B1, Bn...> : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};

template<typename T>
using such = negation<std::is_same<T, nonesuch>>;

/*
 * all_of_traits is a higher order type trait that execute a particular type trait over a list of types.
 * Work much like the STL algorithm `all_of` but for type traits.
 */
template<typename T, template<typename...> class, typename...>
struct all_of_traits : std::false_type {
	static_assert(false_t<T>::value, "Incorrect usage of all_of_traits. The first parameter must be a meta_list, a tuple or nonesuch");
};

template<template<typename...> class Trait, typename... Args>
struct all_of_traits<nonesuch, Trait, Args...> : std::false_type {};

template<typename... Types, template<typename...> class Trait, typename... Args>
struct all_of_traits<meta_list<Types...>, Trait, Args...> : conjunction<Trait<Args..., Types>...> {};

template<typename... Types, template<typename...> class Trait, typename... Args>
struct all_of_traits<std::tuple<Types...>, Trait, Args...> : conjunction<Trait<Args..., Types>...> {};

/*
 * This higher order metafunction will expand `n` elements of a meta_list into a template.
 */
template<typename, typename, template<typename...> class, typename... Args>
struct expand_n_helper;

template<std::size_t... S, typename List, template<typename...> class Trait, typename... Args>
struct expand_n_helper<seq<S...>, List, Trait, Args...> {
	using type = Trait<Args..., meta_list_element_t<S, List>...>;
};

/*
 * Shortcut to expand_n that expand all the list minus `N` elements into the template.
 */
template<std::size_t N, typename List, template<typename...> class Trait>
using expand_minus_n = typename expand_n_helper<tuple_seq_minus<List, N>, List, Trait>::type;

/*
 * Shortcut to expand_n that always expand all the list into the template.
 */
template<typename List, template<typename...> class Trait, typename... Args>
using expand_all = typename expand_n_helper<tuple_seq<List>, List, Trait, Args...>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DETECTION_HPP
