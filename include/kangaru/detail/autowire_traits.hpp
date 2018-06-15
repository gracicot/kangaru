#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOWIRE_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOWIRE_TRAITS_HPP

#include "traits.hpp"
#include "container_service.hpp"

namespace kgr {
namespace detail {

template<typename For, typename Map>
struct deducer {
	explicit deducer(container& c) noexcept : _container{&c} {}
	
	template<typename T, enable_if_t<
		!std::is_same<mapped_service_t<T, Map>, For>::value &&
		!std::is_reference<service_type<mapped_service_t<T, Map>>>::value, int> = 0>
	operator T () {
		return _container->service<mapped_service_t<T>>();
	}
	
	template<typename T, enable_if_t<
		!std::is_same<mapped_service_t<T&, Map>, For>::value &&
		std::is_lvalue_reference<service_type<mapped_service_t<T&, Map>>>::value, int> = 0>
	operator T& () const {
		return _container->service<mapped_service_t<T&, Map>>();
	}
	
	template<typename T, enable_if_t<
		!std::is_same<mapped_service_t<T&&, Map>, For>::value &&
		std::is_rvalue_reference<service_type<mapped_service_t<T&&, Map>>>::value, int> = 0>
	operator T&& () const {
		return _container->service<mapped_service_t<T&&, Map>>();
	}
	
private:
	container* _container;
};

template<typename For, typename Map, std::size_t>
using deducer_expand_t = deducer<For, Map>;

template<typename, typename, typename, std::size_t n, typename, typename = typename seq_gen<n>::type, typename = void>
struct amount_of_deductible_service_helper : std::false_type {};

template<typename Service, typename T, typename Map, typename... Args, std::size_t... S, std::size_t n>
struct amount_of_deductible_service_helper<Service, T, Map, n, meta_list<Args...>, seq<S...>, enable_if_t<is_someway_constructible<T, deducer_expand_t<Service, Map, S>..., Args...>::value>> : std::true_type {
	using result_t = inject_result<deducer_expand_t<Service, Map, S>..., Args...>;
};

template<typename, typename, typename, typename, std::size_t, std::size_t = 0, typename = void>
struct amount_of_deductible_service {
	static constexpr bool deductible = false;
	static constexpr seq<> amount = {};
	using result_t = inject_result<>;
};

template<typename S, typename T, typename Map, typename... Args, std::size_t max, std::size_t n>
struct amount_of_deductible_service<S, T, Map, meta_list<Args...>, max, n, enable_if_t<amount_of_deductible_service_helper<S, T, Map, n, meta_list<Args...>>::value>> {
	static constexpr bool deductible = true;
	static constexpr typename seq_gen<n>::type amount = {};
	using result_t = typename amount_of_deductible_service_helper<S, T, Map, n, meta_list<Args...>>::result_t;
};

template<typename S, typename T, typename Map, typename... Args, std::size_t max, std::size_t n>
struct amount_of_deductible_service<S, T, Map, meta_list<Args...>, max, n, enable_if_t<!amount_of_deductible_service_helper<S, T, Map, n, meta_list<Args...>>::value && (n < max)>> :
	amount_of_deductible_service<S, T, Map, meta_list<Args...>, max, n + 1> {};

constexpr std::size_t default_max_dependency = 10;

} // namespace detail
} // namespace sbg

#endif
