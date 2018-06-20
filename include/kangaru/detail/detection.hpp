#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DETECTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DETECTION_HPP

#include "meta_list.hpp"
#include "void_t.hpp"
#include "utils.hpp"

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

template<bool, typename Default, template<typename...> class, typename...>
struct instanciate_if {
	using type = Default;
};

template<typename Default, template<typename...> class Template, typename... Args>
struct instanciate_if<true, Default, Template, Args...> {
	using type = typename detector<Default, void, Template, Args...>::type;
};

} // namespace detail

struct nonesuch {
	nonesuch() = delete;
	~nonesuch() = delete;
	nonesuch(nonesuch const&) = delete;
	void operator=(nonesuch const&) = delete;
};

template <template<class...> class Op, typename... Args>
using is_detected = typename detail_detection::detector<nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, typename... Args>
using detected_t = typename detail_detection::detector<nonesuch, void, Op, Args...>::type;

template <typename Default, template<class...> class Op, typename... Args>
using detected_or = typename detail_detection::detector<Default, void, Op, Args...>::type;

template <bool b, template<class...> class Template, typename... Args>
using instanciate_if_t = typename detail_detection::instanciate_if<b, nonesuch, Template, Args...>::type;

template <bool b, typename Default, template<class...> class Template, typename... Args>
using instanciate_if_or = typename detail_detection::instanciate_if<b, Default, Template, Args...>::type;

template<typename B>
struct negation : bool_constant<!bool(B::value)> {};

template<typename...> struct conjunction : std::true_type {};
template<typename B1> struct conjunction<B1> : B1 {};
template<typename B1, typename... Bn>
struct conjunction<B1, Bn...> : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};

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

template<typename, typename, template<typename...> class, typename... Args>
struct expand_n_helper;

template<std::size_t... S, typename List, template<typename...> class Trait>
struct expand_n_helper<seq<S...>, List, Trait> {
	using type = Trait<meta_list_element_t<S, List>...>;
};

template<std::size_t N, typename List, template<typename...> class Trait>
using expand_n = typename expand_n_helper<typename seq_gen<N>::type, List, Trait>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DETECTION_HPP
