#ifndef KGR_KANGARU_INCLUDE_KANGARU_EXPERIMENTAL_AUTOWIRING_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_EXPERIMENTAL_AUTOWIRING_HPP

#include "../detail/autowire_traits.hpp"

namespace kgr {
namespace experimental {
namespace autowiring {

/*
 * A class used for indirect mapping and tranmitting information about autowiring.
 */
template<template<typename, typename> class ServiceType, template<typename> class GetService, typename Map, std::size_t max_dependencies>
using autowire_map = detail::autowire_map<ServiceType, GetService, Map, max_dependencies>;

using default_inject_function_t = detail::default_inject_function_t;
constexpr default_inject_function_t default_inject_function{};

/*
 * A construct function usable by many service definition implementation.
 * Will send as many deducers as there are numbers in S
 */
template<typename Self, typename Map, typename I, std::size_t... S, typename... Args>
inline auto deduce_construct(detail::seq<S...> s, I inject, inject_t<container_service> cont, Args&&... args) -> detail::call_result_t<I, detail::deducer_expand_t<Self, Map, S>..., Args...> {
	return detail::deduce_construct<Self, Map>(s, std::move(inject), std::move(cont), std::forward<Args>(args)...);
}

/*
 * A shortcut for deduce_construct with the default injection function.
 */
template<typename Self, typename Map, std::size_t... S, typename... Args>
inline auto deduce_construct_default(detail::seq<S...> s, inject_t<container_service> cont, Args&&... args) -> detail::call_result_t<default_inject_function_t, detail::deducer_expand_t<Self, Map, S>..., Args...> {
	return detail::deduce_construct_default<Self, Map>(s, std::move(cont), std::forward<Args>(args)...);
}

/*
 * Alias to amount_of_deductible_service_helper to ease usage
 */
template<typename S, typename T, typename Map, std::size_t max, typename... Args>
using amount_of_deductible_service = detail::amount_of_deductible_service<S, T, Map, max, Args...>;

/*
 * Tag that replaces dependencies in a service defintion. Carries all information on how to autowire.
 */
template<typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_tag = detail::autowire_tag<Map, max_dependencies>;

} // namespace autowiring
} // namespace experimental
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_EXPERIMENTAL_AUTOWIRING_HPP
