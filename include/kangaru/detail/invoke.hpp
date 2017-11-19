#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP

#include "meta_list.hpp"
#include "service_map.hpp"
#include "utils.hpp"
#include "traits.hpp"
#include "injected.hpp"

namespace kgr {
namespace detail {

/*
 * This class will the the member autocall function part of the service definition.
 * Maybe the autocall function part of the service definition should not exist, and moved to the container.
 */
template<typename T, typename F>
struct autocall_function {
private:
	/*
	 * This is used for the case of a regular autocall call.
	 */
	template<typename U, typename C>
	struct get_member_autocall {
		using type = std::integral_constant<
			decltype(exact(&U::template do_autocall<T, C, typename U::map>)),
			&U::template do_autocall<T, C, typename U::map>
		>;
	};
	
	/*
	 * This is used when a invoke autocall is used.
	 */
	template<typename U, typename C, std::size_t... S>
	struct get_invoke_autocall {
		using type = std::integral_constant<
			decltype(exact(&U::template do_autocall<T, C, detail::meta_list_element_t<S, typename C::parameters>...>)),
			&U::template do_autocall<T, C, detail::meta_list_element_t<S, typename C::parameters>...>
		>;
	};
	
	template<typename U, typename C, std::size_t... S>
	static get_invoke_autocall<U, C, S...> test_helper(seq<S...>);
	
	template<typename U, typename C, enable_if_t<is_invoke_call<C>::value, int> = 0>
	static decltype(test_helper<U, C>(tuple_seq<typename C::parameters>{})) test();
	
	template<typename U, typename C, enable_if_t<is_member_autocall<U, C>::value && !is_invoke_call<C>::value, int> = 0>
	static get_member_autocall<U, C> test();
	
	using inner_type = decltype(test<T, F>());
	
public:
	using type = typename inner_type::type;
};

/*
 * This trait extract what arguments are needed in a autocall call.
 * This is mostly used for other trait to validate an autocall function.
 */
template<typename, typename, typename = void>
struct autocall_arguments;

/*
 * This is the case for an invoke call.
 * Since all aguments are listed, we simply take those.
 */
template<typename T, typename F>
struct autocall_arguments<T, F, enable_if_t<is_invoke_call<F>::value>> {
	using type = typename F::parameters;
};

/*
 * This is the case for an autocall member function call.
 * Since the arguments are the injected service type, we must use a map and extract the definition type out of each arguments.
 */
template<typename T, typename F>
struct autocall_arguments<T, F, enable_if_t<is_member_autocall<T, F>::value && !is_invoke_call<F>::value>> {
private:
	template<typename Map>
	struct mapped_type {
		template<typename U>
		using map = service_map_t<U, Map>;
	};
	
public:
	using type = meta_list_transform_t<function_arguments_t<typename F::value_type>, mapped_type<typename T::map>::template map>;
};

/*
 * This gets the integral_constant found in autocall_function
 */
template<typename T, typename F>
using autocall_function_t = typename autocall_function<T, F>::type;

/*
 * This returns the nth autocall function type in the autocall list of a service.
 */
template<typename T, std::size_t I>
using autocall_nth_function = detail::autocall_function_t<T, detail::meta_list_element_t<I, typename T::autocall_functions>>;

/*
 * This returns the value type of autocall_nth_function
 */
template<typename T, std::size_t I>
using autocall_nth_function_t = typename detail::autocall_function_t<T, detail::meta_list_element_t<I, typename T::autocall_functions>>::value_type;

/*
 * This is an alias for the argument list of an autocall function.
 */
template<typename T, typename F>
using autocall_arguments_t = typename autocall_arguments<T, F>::type;

/*
 * This checks if an entry in the autocall list is actually a valid autocall function type.
 */
template<typename T, typename F>
using is_valid_autocall_function = std::integral_constant<bool,
	is_invoke_call<F>::value || is_member_autocall<T, F>::value
>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
