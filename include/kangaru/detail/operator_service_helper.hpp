#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OPERATOR_SERVICE_HELPER_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OPERATOR_SERVICE_HELPER_HPP

#include <type_traits>

namespace kgr {

struct all;

namespace detail {

/*
 * Forward declaration of services defined in operator_service.hpp
 */
template<typename, typename>
struct forked_operator_service;

template<typename>
struct operator_service;

struct operator_base;
struct forked_operator_base;

/*
 * Indirect map that generate a service definition for an operator service
 *
 * It will choose between operator_service and forked_operator_service dependening on the operator_base type
 */
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
