#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOWIRE_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOWIRE_TRAITS_HPP

#include "traits.hpp"
#include "container_service.hpp"
#include "validity_check.hpp"
#include "service_map.hpp"
#include "injected.hpp"

#include "../container.hpp"

#include "define.hpp"

namespace kgr {

template<typename, typename>
struct service;

namespace detail {

template<typename Service1, typename Service2>
using is_different_service = bool_constant<
	!std::is_base_of<Service1, Service2>::value &&
	!std::is_base_of<Service2, Service1>::value
>;

/*
 * This class is an object convertible to any mapped service.
 * Upon conversion, it calls the container to get that service.
 *
 * Primarely used for autowire services in constructors.
 */
template<typename For, typename Map>
struct deducer {
	explicit deducer(container& c) noexcept : _container{&c} {}
	
	template<typename T, typename Mapped = mapped_service_t<T, Map>, enable_if_t<
#ifndef KGR_KANGARU_MSVC_DISABLE_VALIDATION_AUTOWIRE
		instantiate_if_or<
			is_different_service<For, Mapped>::value && !std::is_reference<service_type<Mapped>>::value,
			std::false_type, is_service_valid, Mapped>::value
#else
		is_different_service<For, Mapped>::value && !std::is_reference<service_type<Mapped>>::value
#endif
		, int> = 0>
	operator T () {
		return _container->service<Mapped>();
	}
	
	template<typename T, typename Mapped = mapped_service_t<T&, Map>, enable_if_t<
	
#ifndef KGR_KANGARU_MSVC_DISABLE_VALIDATION_AUTOWIRE
		instantiate_if_or<
			is_different_service<For, Mapped>::value && std::is_lvalue_reference<service_type<Mapped>>::value,
			std::false_type, is_service_valid, Mapped>::value
#else
		is_different_service<For, Mapped>::value && std::is_lvalue_reference<service_type<Mapped>>::value
#endif
		, int> = 0>
	operator T& () const {
		return _container->service<Mapped>();
	}
	
	template<typename T, typename Mapped = mapped_service_t<T&&, Map>, enable_if_t<
#ifndef KGR_KANGARU_MSVC_DISABLE_VALIDATION_AUTOWIRE
		instantiate_if_or<
			is_different_service<For, Mapped>::value && std::is_rvalue_reference<service_type<Mapped>>::value,
			std::false_type, is_service_valid, Mapped>::value
#else
		is_different_service<For, Mapped>::value && std::is_rvalue_reference<service_type<Mapped>>::value
#endif
		, int> = 0>
	operator T&& () const {
		return _container->service<Mapped>();
	}
	
private:
	container* _container;
};

template<typename For, typename Map>
struct weak_deducer {
	template<typename T, enable_if_t<
		is_different_service<For, mapped_service_t<T, Map>>::value &&
		!std::is_reference<service_type<mapped_service_t<T, Map>>>::value, int> = 0>
	operator T ();

	template<typename T, enable_if_t<
		is_different_service<For, mapped_service_t<T&, Map>>::value &&
		std::is_lvalue_reference<service_type<mapped_service_t<T&, Map>>>::value, int> = 0>
	operator T& () const;

	template<typename T, enable_if_t<
		is_different_service<For, mapped_service_t<T&&, Map>>::value &&
		std::is_rvalue_reference<service_type<mapped_service_t<T&&, Map>>>::value, int> = 0>
	operator T&& () const;
};

/*
 * Alias that simply add a std::size_t parameter so it can be expanded using a sequence.
 */
template<typename For, typename Map, std::size_t>
using deducer_expand_t = deducer<For, Map>;

/*
 * Alias that simply add a std::size_t parameter so it can be expanded using a sequence.
 */
template<typename For, typename Map, std::size_t>
using weak_deducer_expand_t = weak_deducer<For, Map>;

/*
 * Trait that check if a service is constructible using `n` amount of deducers.
 */
template<typename, typename, typename, std::size_t n, typename, typename = typename seq_gen<n>::type, typename = void>
struct is_deductible_from_amount_helper : std::false_type {};

/*
 * Specialization of amount_of_deductible_service_helper that exists when a callable constructor is found.
 * It also returns the injected result of the construct function (assuming a basic construct function)
 */
template<typename Service, typename T, typename Map, typename... Args, std::size_t... S, std::size_t n>
struct is_deductible_from_amount_helper<Service, T, Map, n, meta_list<Args...>, seq<S...>, enable_if_t<is_someway_constructible<T, weak_deducer_expand_t<Service, Map, S>..., Args...>::value>> : std::true_type {
	using default_result_t = inject_result<deducer_expand_t<Service, Map, S>..., Args...>;
};

/*
 * Trait that tries to find the amount of deducer required.
 * Iterate from 0 to max deducers
 */
template<typename, typename, typename, typename, std::size_t, typename = void>
struct amount_of_deductible_service_helper {
	static constexpr bool deductible = false;
	static constexpr seq<> amount = {};
	using default_result_t = inject_result<>;
};

/*
 * Specialization of amount_of_deductible_service used when the service is constructible using the amount `n`
 */
template<typename S, typename T, typename Map, typename... Args, std::size_t n>
struct amount_of_deductible_service_helper<S, T, Map, meta_list<Args...>, n, enable_if_t<is_deductible_from_amount_helper<S, T, Map, n, meta_list<Args...>>::value>> {
	static constexpr bool deductible = true;
	static constexpr typename seq_gen<n>::type amount = {};
	using default_result_t = typename is_deductible_from_amount_helper<S, T, Map, n, meta_list<Args...>>::default_result_t;
};

/*
 * Specialization of amount_of_deductible_service_helper used when the service is not constructible with `n` deducer
 * Tries the next iteration.
 */
template<typename S, typename T, typename Map, typename... Args, std::size_t n>
struct amount_of_deductible_service_helper<S, T, Map, meta_list<Args...>, n, enable_if_t<!is_deductible_from_amount_helper<S, T, Map, n, meta_list<Args...>>::value && (n != 0)>> :
	amount_of_deductible_service_helper<S, T, Map, meta_list<Args...>, n - 1> {};

/*
 * The default maximum amount of deducer sent to the construct function when autowiring.
 * Tells how many autowired dependencies a service can have by default.
 */
constexpr std::size_t default_max_dependency = 8;

/*
 * Alias to amount_of_deductible_service_helper to ease usage
 */
template<typename S, typename T, typename Map, std::size_t max, typename... Args>
using amount_of_deductible_service = detail::amount_of_deductible_service_helper<S, T, Map, detail::meta_list<Args...>, max>;

/*
 * A class used for indirect mapping and tranmitting information about autowiring.
 */
template<template<typename, typename> class, template<typename> class, typename, std::size_t>
struct autowire_map;

template<template<typename, typename> class service_type, template<typename> class get_service, typename... Maps, std::size_t max_dependencies>
struct autowire_map<service_type, get_service, kgr::map<Maps...>, max_dependencies> {
	template<typename T>
	using mapped_service = service_type<detected_t<get_service, T>, autowire_map<service, decay_t, kgr::map<Maps...>, max_dependencies>>;
};

/*
 * The default injection function. Will call kgr::inject with all arguments.
 */
struct default_inject_function_t {
	template<typename... Args>
	constexpr auto operator()(Args&&... args) const -> inject_result<Args...> {
		return inject(std::forward<Args>(args)...);
	}
}
constexpr default_inject_function{};

/*
 * A construct function usable by many service definition implementation.
 * Will send as many deducers as there are numbers in S
 */
template<typename Self, typename Map, typename I, std::size_t... S, typename... Args, typename Deducer = detail::deducer<Self, Map>>
inline auto deduce_construct(detail::seq<S...>, I inject, inject_t<container_service> cont, Args&&... args) -> decltype(inject((void(S), std::declval<Deducer>())..., std::declval<Args>()...)) {
	auto& container = cont.forward();
	
	// The expansion of the inject call may be empty. This will silence the warning.
	static_cast<void>(container);
	
	return inject((void(S), Deducer{container})..., std::forward<Args>(args)...);
}

/*
 * A shortcut for deduce_construct with the default injection function.
 */
template<typename Self, typename Map, std::size_t... S, typename... Args>
inline auto deduce_construct_default(detail::seq<S...> s, inject_t<container_service> cont, Args&&... args) -> detail::call_result_t<default_inject_function_t, detail::deducer_expand_t<Self, Map, S>..., Args...> {
	return deduce_construct<Self, Map>(s, default_inject_function, std::move(cont), std::forward<Args>(args)...);
}

/*
 * Tag that replaces dependencies in a service defintion. Carries all information on how to autowire.
 */
template<typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_tag = detail::autowire_map<service, decay_t, Map, max_dependencies>;

} // namespace detail
} // namespace sbg

#include "undef.hpp"

#endif
