#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOWIRE_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOWIRE_TRAITS_HPP

#include "traits.hpp"
#include "container_service.hpp"

namespace kgr {
namespace detail {

/*
 * This class is an object convertible to any mapped service.
 * Upon conversion, it calls the container to get that service.
 *
 * Primarely used for autowire services in constructors.
 */
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

/*
 * Alias that simply add a std::size_t parameter so it can be expanded using a sequence.
 */
template<typename For, typename Map, std::size_t>
using deducer_expand_t = deducer<For, Map>;

/*
 * Trait that check if a service is constructible using `n` amount of deducers.
 */
template<typename, typename, typename, std::size_t n, typename, typename = typename seq_gen<n>::type, typename = void>
struct amount_of_deductible_service_helper : std::false_type {};

/*
 * Specialization of amount_of_deductible_service_helper that exists when a callable constructor is found.
 * It also returns the injected result of the construct function (assuming a basic construct function)
 */
template<typename Service, typename T, typename Map, typename... Args, std::size_t... S, std::size_t n>
struct amount_of_deductible_service_helper<Service, T, Map, n, meta_list<Args...>, seq<S...>, enable_if_t<is_someway_constructible<T, deducer_expand_t<Service, Map, S>..., Args...>::value>> : std::true_type {
	using result_t = inject_result<deducer_expand_t<Service, Map, S>..., Args...>;
};

/*
 * Trait that tries to find the amount of deducer required.
 * Iterate from 0 to max deducers
 */
template<typename, typename, typename, typename, std::size_t, std::size_t = 0, typename = void>
struct amount_of_deductible_service {
	static constexpr bool deductible = false;
	static constexpr seq<> amount = {};
	using result_t = inject_result<>;
};

/*
 * Specialization of amount_of_deductible_service used when the service is constructible using the amount `n`
 */
template<typename S, typename T, typename Map, typename... Args, std::size_t max, std::size_t n>
struct amount_of_deductible_service<S, T, Map, meta_list<Args...>, max, n, enable_if_t<amount_of_deductible_service_helper<S, T, Map, n, meta_list<Args...>>::value>> {
	static constexpr bool deductible = true;
	static constexpr typename seq_gen<n>::type amount = {};
	using result_t = typename amount_of_deductible_service_helper<S, T, Map, n, meta_list<Args...>>::result_t;
};

/*
 * Specialization of amount_of_deductible_service used when the service is not constructible with `n` deducer
 * Tries the next iteration.
 */
template<typename S, typename T, typename Map, typename... Args, std::size_t max, std::size_t n>
struct amount_of_deductible_service<S, T, Map, meta_list<Args...>, max, n, enable_if_t<!amount_of_deductible_service_helper<S, T, Map, n, meta_list<Args...>>::value && (n < max)>> :
	amount_of_deductible_service<S, T, Map, meta_list<Args...>, max, n + 1> {};

/*
 * The default maximum amount of deducer sent to the construct function when autowireing.
 * Tells how many autowired dependencies a service can have by default.
 */
constexpr std::size_t default_max_dependency = 10;

} // namespace detail
} // namespace sbg

#endif
