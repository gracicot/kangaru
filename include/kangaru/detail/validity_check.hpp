#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VALIDITY_CHECK_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VALIDITY_CHECK_HPP

#include "invoke_function.hpp"
#include "single.hpp"
#include "service_check.hpp"
#include "autocall_traits.hpp"

namespace kgr {
namespace detail {

/*
 * Check if a call for kgr::Container::service has no arguments in the case of a single service
 */
template<typename T, typename... Args>
using is_single_no_args = std::integral_constant<bool,
	!is_single<T>::value || meta_list_empty<meta_list<Args...>>::value
>;

/*
 * Complete validity check for a particlar service and all it's properties
 */
template<typename T, typename... Args>
struct is_construction_valid : std::integral_constant<bool,
	service_check<T, Args...>::value &&
	dependency_check<T, Args...>::value &&
	is_autocall_valid<T>::value &&
	dependency_trait<is_autocall_valid, T, Args...>::value
> {};

/*
 * Complete validity check for a particlar service and all it's properties
 */
template<typename T, typename... Args>
struct is_service_valid : std::integral_constant<bool,
	is_single_no_args<T, Args...>::value &&
	is_construction_valid<T, Args...>::value
> {};

/* 
 * Trait that check if a function is invocable, and all it's injected arguments are valid.
 */
template<typename Map, typename T, typename... Args>
struct is_invoke_service_valid_helper {
private:
	template<typename U>
	using map_t = mapped_service_t<U, Map>;
	
	template<typename U, std::size_t I>
	struct expander {
		using type = std::integral_constant<bool, is_service_valid<map_t<invoke_function_argument_t<I, Map, U, Args...>>>::value>;
	};
	
	template<typename U, typename... As, std::size_t... S, int_t<map_t<invoke_function_argument_t<S, Map, U, As...>>..., enable_if_t<expander<U, S>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename... As>
	static decltype(test_helper<U, As...>(tuple_seq_minus<invoke_function_arguments_t<Map, U, As...>, sizeof...(Args)>{})) test(int);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(0));
};

/*
 * Alias for is_invoke_service_valid_helper
 */
template<typename Map, typename T, typename... Args>
struct is_invoke_service_valid : is_invoke_service_valid_helper<Map, T, Args...>::type {};

/*
 * Validity check for a invoke expression
 */
template<typename Map, typename T, typename... Args>
using is_invoke_valid = std::integral_constant<bool,
	is_invoke_service_valid<Map, T, Args...>::value &&
	is_invokable<Map, T, Args...>::value
>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_VALIDITY_CHECK_HPP
