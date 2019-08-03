#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OPERATOR_SERVICE_HELPER_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OPERATOR_SERVICE_HELPER_HPP

#include <type_traits>

namespace kgr {

struct all;

namespace detail {

template<typename, typename>
struct forked_operator_service;

template<typename>
struct operator_service;

struct operator_base;
struct forked_operator_base;

template<typename>
struct select_operator_service;

template<>
struct select_operator_service<operator_base> {
	template<typename T>
	using mapped_service = operator_service<typename std::remove_cv<typename std::remove_reference<T>::type>::type>;
};

template<>
struct select_operator_service<forked_operator_base> {
	template<typename T>
	using mapped_service = forked_operator_service<all, typename std::remove_cv<typename std::remove_reference<T>::type>::type>;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OPERATOR_SERVICE_HELPER_HPP
