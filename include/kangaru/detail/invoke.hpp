#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP

#include "meta_list.hpp"
#include "service_map.hpp"
#include "utils.hpp"
#include "traits.hpp"
#include "injected.hpp"

#ifndef KGR_KANGARU_MSVC_EXACT_DECLTYPE
#if _MSC_VER
#ifndef __clang__
// MSVC has a defect that makes decltype with the address of a
// generic lambda not possible unless sending the address to a function.
#define KGR_KANGARU_MSVC_EXACT_DECLTYPE
#endif // !__clang__
#endif // _MSC_VER
#endif // KGR_KANGARU_MSVC_EXACT_DECLTYPE

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
		template<typename V, typename Function, typename Map, enable_if_t<is_map<Map>::value && !is_map<F>::value, int> = 0>
		static void autocall(inject_t<container_service> cs, V& service) {
			U::template autocall_helper<V, Map, Function>(detail::function_seq<typename Function::value_type>{}, std::move(cs), service);
		}
		
		using type = std::integral_constant<
#ifdef KGR_KANGARU_MSVC_EXACT_DECLTYPE
			decltype(&get_member_autocall::autocall<T, C, typename U::map>),
#else
			decltype(exact(&get_member_autocall::autocall<T, C, typename U::map>)),
#endif
			&get_member_autocall::autocall<T, C, typename U::map>
		>;
	};
	
	/*
	 * This is used when a invoke autocall is used.
	 */
	template<typename U, typename C, std::size_t... S>
	struct get_invoke_autocall {
		template<typename V, typename Function, typename... Ts, int_t<enable_if_t<!is_map<F>::value>, enable_if_t<!is_map<Ts>::value>...> = 0>
		static void autocall(inject_t<Ts>... others, V& service) {
			service.call(Function::value, std::forward<inject_t<Ts>>(others).forward()...);
		}
		
		using type = std::integral_constant<
#ifdef KGR_KANGARU_MSVC_EXACT_DECLTYPE
			decltype(exact(&get_invoke_autocall::autocall<T, C, detail::meta_list_element_t<S, typename C::parameters>...>)),
#else
			decltype(&get_invoke_autocall::autocall<T, C, detail::meta_list_element_t<S, typename C::parameters>...>),
#endif
			&get_invoke_autocall::autocall<T, C, detail::meta_list_element_t<S, typename C::parameters>...>
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
