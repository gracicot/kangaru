#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP

#include "traits.hpp"
#include "single.hpp"
#include "injected.hpp"
#include "container_service.hpp"

namespace kgr {
namespace detail {

template<typename T>
using is_container_service = std::is_base_of<ContainerServiceTag, T>;

template<typename>
struct original;

template<typename T>
struct original<BaseInjected<T>&> {
	using type = T;
};

template<typename T>
struct original<Injected<T>&&> {
	using type = T;
};

template<typename T>
using original_t = typename original<T>::type;

template<std::size_t n, typename F>
using injected_argument_t = original_t<function_argument_t<n, F>>;

template<typename T, typename... Args>
struct is_dependencies_services_helper {
private:
	static std::true_type sink(...);
	
	template<typename U, typename... As, std::size_t... S>
	static decltype(sink(
		std::declval<enable_if_t<is_dependencies_services_helper<injected_argument_t<S, construct_function_t<U, As...>>>::type::value, int>>()...,
		std::declval<enable_if_t<is_service<injected_argument_t<S, construct_function_t<U, As...>>>::value, int>>()...
	)) test(seq<S...>);
	
	template<typename U, typename...>
	static enable_if_t<is_container_service<U>::value || std::is_abstract<U>::value, std::true_type> test_helper(int);
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename... As>
	static decltype(test<U, As...>(tuple_seq_minus<function_arguments_t<construct_function_t<U, As...>>, sizeof...(As)>{})) test_helper(int);
	
public:
	using type = decltype(test_helper<T, Args...>(0));
};

template<typename T, typename... Args>
using is_dependencies_services = typename is_dependencies_services_helper<T, Args...>::type;

template<typename T, typename... Args>
struct is_service_constructible_helper {
private:
	static std::true_type sink(...);
	
	template<typename U, typename... As, std::size_t... S>
	static decltype(sink(
		std::declval<enable_if_t<is_service_instantiable<T, tuple_element_t<S, function_result_t<construct_function_t<U, As...>>>...>::value, int>>()
	)) test(seq<S...>);
	
	template<typename U, typename...>
	static enable_if_t<is_container_service<U>::value || std::is_abstract<U>::value, std::true_type> test_helper(int);
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename... As>
	static decltype(test<U, As...>(tuple_seq<function_result_t<construct_function_t<U, As...>>>{})) test_helper(int);
	
public:
	using type = decltype(test_helper<T, Args...>(0));
};

template<typename T, typename... Args>
using is_service_constructible = typename is_service_constructible_helper<T, Args...>::type;

template<typename T, typename... Args>
struct is_dependencies_constructible_helper {
private:
	static std::true_type sink(...);
	
	template<typename U, typename... As, std::size_t... S>
	static decltype(sink(
		std::declval<enable_if_t<is_dependencies_constructible_helper<injected_argument_t<S, construct_function_t<U, As...>>>::type::value, int>>()...,
		std::declval<enable_if_t<is_service_constructible<injected_argument_t<S, construct_function_t<U, As...>>>::type::value, int>>()...
	)) test(seq<S...>);
	
	template<typename U, typename...>
	static enable_if_t<is_container_service<U>::value || std::is_abstract<U>::value, std::true_type> test_helper(int);
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename... As>
	static decltype(test<U, As...>(tuple_seq_minus<function_arguments_t<construct_function_t<U, As...>>, sizeof...(As)>{})) test_helper(int);
	
public:
	using type = decltype(test_helper<T, Args...>(0));
};

template<typename T, typename... Args>
using is_dependencies_constructible = typename is_dependencies_constructible_helper<T, Args...>::type;

template<typename T>
struct is_override_service_helper {
private:
	template<typename...>
	static std::false_type test(...);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<is_service<meta_list_element_t<S, parent_types<U>>>::value>...> = 0>
	static std::true_type test(seq<S...>);
	
public:
	using type = decltype(test<T>(tuple_seq<parent_types<T>>{}));
};

template<typename T>
using is_override_service = typename is_override_service_helper<T>::type;

template<typename T>
struct is_override_convertible_helper {
private:
	template<typename...>
	static std::false_type test(...);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<is_explicitly_convertible<ServiceType<U>, ServiceType<meta_list_element_t<S, parent_types<U>>>>::value>...> = 0>
	static std::true_type test(seq<S...>);
	
public:
	using type = decltype(test<T>(tuple_seq<parent_types<T>>{}));
};

template<typename T>
using is_override_convertible = typename is_override_convertible_helper<T>::type;

template<typename T, typename... Args>
struct is_dependencies_override_convertible_helper {
private:
	static std::true_type sink(...);
	
	template<typename U, typename... As, std::size_t... S>
	static decltype(sink(
		std::declval<enable_if_t<is_dependencies_override_convertible_helper<injected_argument_t<S, construct_function_t<U, As...>>>::type::value, int>>()...,
		std::declval<enable_if_t<is_override_convertible<injected_argument_t<S, construct_function_t<U, As...>>>::type::value, int>>()...
	)) test(seq<S...>);
	
	template<typename U, typename...>
	static enable_if_t<is_container_service<U>::value || std::is_abstract<U>::value, std::true_type> test_helper(int);
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename... As>
	static decltype(test<U, As...>(tuple_seq_minus<function_arguments_t<construct_function_t<U, As...>>, sizeof...(As)>{})) test_helper(int);
	
public:
	using type = decltype(test_helper<T, Args...>(0));
};

template<typename T, typename... Args>
using is_dependencies_override_convertible = typename is_dependencies_override_convertible_helper<T, Args...>::type;


template<typename T, typename... Args>
using is_single_no_args = std::integral_constant<bool,
	!is_single<T>::value || sizeof...(Args) == 0
>;

template<typename T>
struct is_default_overrides_abstract_helper {
private:
	template<typename>
	static std::false_type test(...);
	
	template<typename U, enable_if_t<has_default<U>::value && std::is_abstract<U>::value, int> = 0>
	static is_overriden_by<U, default_type<U>> test(int);
	
	template<typename U, enable_if_t<!has_default<U>::value, int> = 0>
	static std::true_type test(int);
	
public:
	using type = decltype(test<T>(0));
};

template<typename T>
using is_default_overrides_abstract = typename is_default_overrides_abstract_helper<T>::type;

template<typename T>
struct is_default_convertible_helper {
private:
	template<typename>
	static std::false_type test(...);
	
	template<typename U, enable_if_t<has_default<U>::value, int> = 0>
	static is_explicitly_convertible<ServiceType<default_type<U>>, ServiceType<U>> test(int);
	
	template<typename U, enable_if_t<!has_default<U>::value, int> = 0>
	static std::true_type test(int);
	
public:
	using type = decltype(test<T>(0));
};

template<typename T>
using is_default_convertible = typename is_default_convertible_helper<T>::type;

template<typename T, typename... Args>
using is_default_service_valid = std::integral_constant<bool,
	((has_default<T>::value && std::is_abstract<T>::value) ||
	!(has_default<T>::value || std::is_abstract<T>::value)) &&
	is_default_overrides_abstract<T>::value &&
	is_default_convertible<T>::value
>;

template<typename T, typename... Args>
using is_service_valid = std::integral_constant<bool, 
	is_service<T>::value &&
	is_single_no_args<T, Args...>::value &&
	is_dependencies_services<T, Args...>::value &&
	is_service_constructible<T, Args...>::value &&
	is_dependencies_constructible<T, Args...>::value &&
	is_default_service_valid<T>::value &&
	is_override_convertible<T>::value &&
	is_dependencies_override_convertible<T, Args...>::value
>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP
