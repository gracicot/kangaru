#ifndef KGR_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP

#include "traits.hpp"
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

template<typename T, typename... Args>
struct is_dependencies_services_helper {
private:
	static std::true_type sink(...);
	
	template<typename U, typename... As, std::size_t... S>
	static decltype(sink(
		enable_if_t<is_dependencies_services_helper<original_t<function_argument_t<S, construct_function_t<U, As...>>>>::type::value, int>{}...,
		std::declval<ServiceType<original_t<function_argument_t<S, construct_function_t<U, As...>>>>>()...
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

} // namespace detail
} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP
