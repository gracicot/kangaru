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

template<typename F, typename... Args>
struct is_construct_invokable_helper {
private:
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename C, typename... As, std::size_t... S, int_t<
		decltype(std::declval<C>()(std::declval<function_argument_t<S, C>>()..., std::declval<As>()...)),
		enable_if_t<is_service<injected_argument_t<S, C>>::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename C, typename... As>
	static decltype(test_helper<C, As...>(tuple_seq_minus<function_arguments_t<C>, sizeof...(As)>{})) test(int);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<F, Args...>(0));
};

template<typename F, typename... Args>
using is_construct_invokable = typename is_construct_invokable_helper<F, Args...>::type;

template<typename, typename, typename, typename = void>
struct has_callable_template_construct : std::false_type {};

template<typename T, typename... TArgs, typename... Args>
struct has_callable_template_construct<T, meta_list<TArgs...>, meta_list<Args...>, enable_if_t<is_construct_invokable<decltype(&T::template construct<TArgs...>), Args...>::value>> : std::true_type {};

template<typename, typename, typename = void>
struct has_callable_construct : std::false_type {};

template<typename T, typename... Args>
struct has_callable_construct<T, meta_list<Args...>, enable_if_t<is_construct_invokable<decltype(&T::construct), Args...>::value>> : std::true_type {};

template<typename, typename, typename, typename = void>
struct get_template_construct_helper;

template<typename T, typename... Args>
struct get_template_construct_helper<T, meta_list<>, meta_list<Args...>, enable_if_t<!has_callable_template_construct<T, meta_list<>, meta_list<Args...>>::value>> {};

template<typename T, typename Head, typename... Tail, typename... Args>
struct get_template_construct_helper<T, meta_list<Head, Tail...>, meta_list<Args...>, enable_if_t<!has_callable_template_construct<T, meta_list<Head, Tail...>, meta_list<Args...>>::value>> : get_template_construct_helper<T, meta_list<Tail...>, meta_list<Args...>> {};

template<typename T, typename... TArgs, typename... Args>
struct get_template_construct_helper<T, meta_list<TArgs...>, meta_list<Args...>, enable_if_t<has_callable_template_construct<T, meta_list<TArgs...>, meta_list<Args...>>::value>> : std::integral_constant<decltype(&T::template construct<TArgs...>), &T::template construct<TArgs...>> {};

template<typename T, typename... Args>
using get_template_construct = get_template_construct_helper<T, meta_list<Args...>, meta_list<Args...>>;

template<typename, typename, typename = void>
struct has_template_construct_helper : std::false_type {};

template<typename T, typename... Args>
struct has_template_construct_helper<T, meta_list<Args...>, void_t<typename get_template_construct<T, Args...>::value_type>> : std::true_type {};

template<typename T, typename... Args>
using has_template_construct = has_template_construct_helper<T, meta_list<Args...>>;

template<bool, typename T, typename... Args>
struct construct_function_helper {
private:
	template<typename U>
	struct get_construct {
		using type = std::integral_constant<decltype(&U::construct), &U::construct>;
	};
	
	template<typename U, typename... As, enable_if_t<
		has_construct<U>::value &&
		!has_template_construct<U, As...>::value, int> = 0>
	static get_construct<U> test();
	
	template<typename U, typename... As, enable_if_t<has_template_construct<U, As...>::value, int> = 0>
	static get_template_construct<U, As...> test();
	
	using inner_type = decltype(test<T, Args...>());
	
public:
	using type = typename inner_type::type;
};

template<typename T, typename... Args>
struct construct_function_helper<false, T, Args...> {};

template<typename T, typename... Args>
using has_any_construct = std::integral_constant<bool, has_template_construct<T, Args...>::value || has_construct<T>::value>;

template<typename T, typename... Args>
using construct_function = typename construct_function_helper<has_any_construct<T, Args...>::value, T, Args...>::type;

template<typename T, typename... Args>
using construct_function_t = typename construct_function<T, Args...>::value_type;

template<typename T, typename... Args>
using construct_result_seq = tuple_seq<function_result_t<construct_function_t<T, Args...>>>;

template<typename, typename, typename = void>
struct is_construct_function_callable_helper : std::false_type {};

template<typename T, typename... Args>
struct is_construct_function_callable_helper<T, meta_list<Args...>, enable_if_t<is_construct_invokable<construct_function_t<T, Args...>, Args...>::value>> : std::true_type {};

template<typename T, typename... Args>
struct is_construct_function_callable_helper<T, meta_list<Args...>, enable_if_t<is_container_service<T>::value || std::is_abstract<T>::value>> : std::true_type {};

template<typename T, typename... Args>
using is_construct_function_callable = is_construct_function_callable_helper<T, meta_list<Args...>>;

template<template<typename...> class Trait, typename T, typename... Args>
struct dependency_trait_helper {
	static std::true_type sink(...);
	
	template<typename U, typename... As, std::size_t... S>
	static decltype(sink(
		std::declval<enable_if_t<dependency_trait_helper<Trait, injected_argument_t<S, construct_function_t<U, As...>>>::type::value, int>>()...,
		std::declval<enable_if_t<Trait<injected_argument_t<S, construct_function_t<U, As...>>>::value, int>>()...
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

template<template<typename...> class Trait, typename T, typename... Args>
using dependency_trait = typename dependency_trait_helper<Trait, T, Args...>::type;

template<typename T, typename... Args>
struct is_service_constructible_helper {
private:
	template<typename U, typename... As, std::size_t F, std::size_t... S>
	static is_service_instantiable<T,
		tuple_element_t<F, function_result_t<construct_function_t<U, As...>>>,
		tuple_element_t<S, function_result_t<construct_function_t<U, As...>>>...> test(seq<F, S...>);

	// This overload is needed for msvc.
	// Or else it will try to call the one just above with a 0 as S for strange reason.
	template<typename U, typename... As>
	static std::true_type test(seq<>);
	
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
	// This is a workaround for msvc. Expansion in very complex expression
	// leaves the compiler without clues about what's going on.
	template<std::size_t I, typename U>
	struct expander {
		using type = is_explicitly_convertible<ServiceType<U>, ServiceType<meta_list_element_t<I, parent_types<U>>>>;
	};

	template<typename...>
	static std::false_type test(...);

	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<expander<S, U>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<is_service<meta_list_element_t<S, parent_types<U>>>::value>...> = 0>
	static decltype(test_helper<U>(seq<S...>{})) test(seq<S...>);
	
public:
	using type = decltype(test<T>(tuple_seq<parent_types<T>>{}));
};

template<typename T>
using is_override_convertible = typename is_override_convertible_helper<T>::type;

template<typename T>
struct is_override_services_helper {
private:
	// This is a workaround for msvc. Expansion in very complex expression
	// leaves the compiler without clues about what's going on.
	template<std::size_t I, typename U>
	struct expander {
		using type = is_service<meta_list_element_t<I, parent_types<U>>>;
	};
	template<typename...>
	static std::false_type test(...);

	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<expander<S, U>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<is_service<meta_list_element_t<S, parent_types<U>>>::value>...> = 0>
	static decltype(test_helper<U>(seq<S...>{})) test(seq<S...>);
	
public:
	using type = decltype(test<T>(tuple_seq<parent_types<T>>{}));
};

template<typename T>
using is_override_services = typename is_override_services_helper<T>::type;

template<typename T, typename... Args>
using is_single_no_args = std::integral_constant<bool,
	!is_single<T>::value || meta_list_empty<meta_list<Args...>>::value
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
struct is_service_valid : std::integral_constant<bool,
	is_single_no_args<T, Args...>::value &&
	is_service<T>::value &&
	dependency_trait<is_service, T, Args...>::value &&
	is_service_constructible<T, Args...>::value &&
	dependency_trait<is_service_constructible, T, Args...>::value &&
	is_construct_function_callable<T, Args...>::value &&
	dependency_trait<is_construct_function_callable, T, Args...>::value &&
	is_default_service_valid<T>::value &&
	dependency_trait<is_default_service_valid, T, Args...>::value &&
	is_override_convertible<T>::value &&
	dependency_trait<is_override_convertible, T, Args...>::value &&
	is_override_services<T>::value &&
	dependency_trait<is_override_services, T, Args...>::value
> {};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP
