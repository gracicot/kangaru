#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_CHECK_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_CHECK_HPP

#include "meta_list.hpp"
#include "traits.hpp"
#include "service_map.hpp"
#include "autocall_traits.hpp"
#include "service_check.hpp"

namespace kgr {
namespace detail {

/*
 * Meta trait to apply a trait over all autocall entry in a autocall list
 */
template<template<typename...> class Trait, typename T>
using autocall_trait = all_of_traits<detected_or<meta_list<>, autocall_functions_t, T>, Trait, T>;

/*
 * Trait that check if every injected arguments of a particular autocall entry are valid services.
 * 
 * However, we don't validate autocall checks for these injected services.
 */
template<typename T, typename F>
using is_autocall_entry_valid = all_of_traits<detected_t<autocall_services, T, F>, service_check>;

/*
 * Validity check for autocall entries in a service T
 */
template<typename T>
using is_autocall_valid = autocall_trait<is_autocall_entry_valid, T>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_CHECK_HPP
