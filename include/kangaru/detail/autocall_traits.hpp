#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_TRAITS_HPP

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
template<typename, typename, typename = void>
struct autocall_function_helper;

template<typename T, typename F>
struct autocall_function_helper<T, F, enable_if_t<is_member_autocall<T, F>::value && !is_invoke_call<F>::value>> {
private:
	static void autocall(inject_t<container_service> cs, T& service) {
		T::template autocall_helper<T, typename T::map, F>(
			detail::function_seq<typename F::value_type>{},
			std::move(cs),
			service
		);
	}
	
public:
	using type = std::integral_constant<
		decltype(&autocall_function_helper::autocall),
		&autocall_function_helper::autocall
	>;
};

template<typename T, typename F>
struct autocall_function_helper<T, F, enable_if_t<is_invoke_call<F>::value>> {
private:
	template<typename... Args>
	struct function {
		static void autocall(inject_t<Args>... others, T& service) {
			service.call(F::value, std::forward<inject_t<Args>>(others).forward()...);
		}
	};
	
	template<std::size_t... S>
	using function_constant = std::integral_constant<
		decltype(&function<detail::meta_list_element_t<S, typename F::parameters>...>::autocall),
		&function<detail::meta_list_element_t<S, typename F::parameters>...>::autocall
	>;
	
	template<std::size_t... S>
	static auto get_function(seq<S...>) -> function_constant<S...>;
	
public:
	using type = decltype(get_function(tuple_seq<typename F::parameters>{}));
};

template<typename T, typename F>
using autocall_function = typename autocall_function_helper<T, F>::type;

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
		using map = mapped_service_t<U, Map>;
	};
	
public:
	using type = meta_list_transform_t<function_arguments_t<typename F::value_type>, mapped_type<typename T::map>::template map>;
};

/*
 * This gets the integral_constant found in autocall_function
 */
template<typename T, typename F>
using autocall_function_t = typename autocall_function<T, F>::value_type;

/*
 * This returns the nth autocall function type in the autocall list of a service.
 */
template<typename T, std::size_t I>
using autocall_nth_function = detail::autocall_function<T, detail::meta_list_element_t<I, typename T::autocall_functions>>;

/*
 * This returns the value type of autocall_nth_function
 */
template<typename T, std::size_t I>
using autocall_nth_function_t = typename detail::autocall_function_t<T, detail::meta_list_element_t<I, typename T::autocall_functions>>;

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

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_TRAITS_HPP
