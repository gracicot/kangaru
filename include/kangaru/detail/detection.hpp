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

template<typename Default, bool b, typename AlwaysVoid, template<typename...> class C, typename... Args>
struct enabled_detector {
	using type = Default;
};

template<typename Default, template<typename...> class C, typename... Args>
struct enabled_detector<Default, false, void, C, Args...> {
	using type = Default;
};

} // namespace detail

struct nonesuch {
	nonesuch() = delete;
	~nonesuch() = delete;
	nonesuch(nonesuch const&) = delete;
	void operator=(nonesuch const&) = delete;
};

template <template<class...> class Op, class... Args>
using is_detected = typename detail_detection::detector<nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
using detected_t = typename detail_detection::detector<nonesuch, void, Op, Args...>::type;

template <class Default, template<class...> class Op, class... Args>
using detected_or = detail_detection::detector<Default, void, Op, Args...>;

template<typename B>
struct negation : bool_constant<!bool(B::value)> {};

template<typename...> struct conjunction : std::true_type {};
template<typename B1> struct conjunction<B1> : B1 {};
template<typename B1, typename... Bn>
struct conjunction<B1, Bn...> : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};

template<typename T, template<typename...> class, typename...>
struct all_of_traits : std::false_type {};

template<typename... Types, template<typename...> class Trait, typename... Args>
struct all_of_traits<meta_list<Types...>, Trait, Args...> : conjunction<Trait<Types, Args...>...> {};

template<typename... Types, template<typename...> class Trait, typename... Args>
struct all_of_traits<std::tuple<Types...>, Trait, Args...> : conjunction<Trait<Types, Args...>...> {};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DETECTION_HPP
