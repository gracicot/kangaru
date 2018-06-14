#ifndef KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP

#include "container.hpp"
#include "generic.hpp"

#include "detail/service_map.hpp"
#include "detail/utils.hpp"
#include "detail/traits.hpp"
#include "detail/container_service.hpp"
#include "detail/injected.hpp"
#include "detail/autowire_traits.hpp"

namespace kgr {

template<typename, typename>
struct service;

template<typename, typename>
struct single_service;

template<typename, typename>
struct unique_service;

template<typename, typename>
struct shared_service;

namespace detail {

template<typename Type, std::size_t... S, typename... Args>
inline auto deduce_construct(seq<S...>, inject_t<container_service> cont, Args&&... args) -> inject_result<deducer_expand_t<Type, S>..., Args...> {
	auto& container = cont.forward();
	return kgr::inject((void(S), deducer<Type>{container})..., std::forward<Args>(args)...);
}

template<template<typename, typename> class service_type, typename Map, std::size_t max_dependencies>
struct autowire_tag {
	template<typename T>
	using mapped_service = service_type<T, autowire_tag<service, Map, max_dependencies>>;
	
};

} // namespace detail

template<typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_tag = detail::autowire_tag<service, Map, max_dependencies>;

template<typename Type, typename Map, std::size_t max_dependencies>
struct service<Type, autowire_tag<Map, max_dependencies>> : generic_service<Type> {
private:
	using parent = generic_service<Type>;
	
	template<typename... Args>
	using amount_deduced = detail::amount_of_deductible_service<Type, detail::meta_list<Args...>, max_dependencies>;
	
protected:
	using parent::instance;
	
public:
	using parent::parent;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> detail::enable_if_t<amount_deduced<Args...>::deductible, typename amount_deduced<Args...>::result_t>
	{
		return detail::deduce_construct<Type>(amount_deduced<Args...>::amount, std::move(cont), std::forward<Args>(args)...);
	}
	
	Type forward() {
		return std::move(instance());
	}
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, instance(), std::forward<Args>(args)...);
	}
};

template<typename Type, typename Map, std::size_t max_dependencies>
struct single_service<Type, autowire_tag<Map, max_dependencies>> : generic_service<Type>, single {
private:
	using parent = generic_service<Type>;
	
	template<typename... Args>
	using amount_deduced = detail::amount_of_deductible_service<Type, detail::meta_list<Args...>, max_dependencies>;
	
protected:
	using parent::instance;
	
public:
	using parent::parent;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> detail::enable_if_t<amount_deduced<Args...>::deductible, typename amount_deduced<Args...>::result_t>
	{
		return detail::deduce_construct<Type>(amount_deduced<Args...>::amount, std::move(cont), std::forward<Args>(args)...);
	}
	
	Type& forward() {
		return instance();
	}
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, instance(), std::forward<Args>(args)...);
	}
};

using autowire = detail::autowire_tag<service, map<>, detail::default_max_dependency>;
using autowire_single = detail::autowire_tag<single_service, map<>, detail::default_max_dependency>;

template<typename Map, std::size_t max_dependencies = detail::default_max_dependency>
using mapped_autowire = detail::autowire_tag<service, Map, max_dependencies>;

template<typename Map, std::size_t max_dependencies = detail::default_max_dependency>
using mapped_autowire_single = detail::autowire_tag<single_service, Map, max_dependencies>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_service = single_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_single_service = single_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

template<typename T, typename Map = map<>>
using autowired = mapped_service_t<T, Map>;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP
