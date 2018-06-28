#ifndef KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP

#include "container.hpp"
#include "detail/lazy_base.hpp"

namespace kgr {
namespace detail {

/*
 * Base class for invokers. Implements the call operator that invoke the function.
 */
template<typename CRTP, typename Map>
struct invoker_base {
	template<typename F, typename... Args, enable_if_t<is_invoke_valid<Map, decay_t<F>, Args...>::value, int> = 0>
	invoke_function_result_t<Map, decay_t<F>, Args...> operator()(F&& f, Args&&... args) {
		container& c = static_cast<CRTP*>(this)->container();
		return c.invoke<Map>(std::forward<F>(f), std::forward<Args>(args)...);
	}
	
	sink operator()(not_invokable_error = {}, ...) = delete;
};

/*
 * Base class for generators. Implements the call operator that create services.
 */
template<typename CRTP, typename T>
struct generator_base {
	static_assert(!is_single<T>::value, "Generator only work with non-single services.");
	
	template<typename... Args, enable_if_t<is_service_valid<T, Args...>::value, int> = 0>
	service_type<T> operator()(Args&&... args) {
		container& c = static_cast<CRTP*>(this)->container();
		return c.service<T>(std::forward<Args>(args)...);
	}
	
	template<typename U = T, enable_if_t<std::is_default_constructible<service_error<U>>::value, int> = 0>
	sink operator()(service_error<U> = {}) = delete;
	
	template<typename... Args>
	sink operator()(service_error<T, identity_t<Args>...>, Args&&...) = delete;
};

/*
 * Base class for any non forking operators.
 * 
 * Hold a non-owning reference to the container as a pointer.
 */
struct operator_base {
	explicit operator_base(kgr::container& c) noexcept : _container{&c} {}
	
	inline auto container() noexcept -> kgr::container& {
		return *_container;
	}
	
	inline auto container() const noexcept -> kgr::container const& {
		return *_container;
	}
	
	kgr::container* _container;
};

/*
 * Base class for any forking operators.
 * 
 * Hold a fork of the container as a member value.
 */
struct forked_operator_base {
	explicit forked_operator_base(kgr::container&& c) noexcept : _container{std::move(c)} {}
	
	inline auto container() noexcept -> kgr::container& {
		return _container;
	}
	
	inline auto container() const noexcept -> kgr::container const& {
		return _container;
	}
	
	kgr::container _container;
};

/*
 * Helper class to setup the base operator class and the operator generic implementation.
 */
template<template<typename, typename> class Operator, typename Param, typename Base = operator_base>
struct basic_operator : Operator<basic_operator<Operator, Param, Base>, Param>, private Base {
	using Base::Base;
	
protected:
	friend struct Operator<basic_operator<Operator, Param, Base>, Param>;
	using Base::container;
};

/*
 * Alias to the helper class with the forking operator base.
 */
template<template<typename, typename> class Operator, typename Param>
using basic_forked_operator = basic_operator<Operator, Param, forked_operator_base>;

} // namespace detail

/**
 * Function object that calls container::invoke.
 * 
 * Useful when dealing with higer order functions or to convey intent.
 */
template<typename Map>
struct mapped_invoker : detail::basic_operator<detail::invoker_base, Map> {
	using detail::basic_operator<detail::invoker_base, Map>::basic_operator;
	
	template<typename M>
	mapped_invoker(const mapped_invoker<M>& other) :
		detail::basic_operator<detail::invoker_base, Map>{other.container()} {}
};

/**
 * Function object that calls container::invoke.
 * 
 * This version forks the container.
 * 
 * Useful when dealing with higer order functions or to convey intent.
 */
template<typename Map>
struct forked_mapped_invoker : detail::basic_forked_operator<detail::invoker_base, Map> {
	using detail::basic_forked_operator<detail::invoker_base, Map>::basic_forked_operator;
	
	template<typename M>
	forked_mapped_invoker(forked_mapped_invoker<M>&& other) :
		detail::basic_forked_operator<detail::invoker_base, Map>{std::move(other.container())} {}
};

/**
 * Function object that calls creates a service.
 * Basically a factory function for a particular service.
 * 
 * Useful to convey intent or contraining usage of the container.
 */
template<typename T>
using generator = detail::basic_operator<detail::generator_base, T>;

/**
 * Function object that calls creates a service.
 * Basically a factory function for a particular service.
 * 
 * This version forks the container.
 * 
 * Useful to convey intent or contraining usage of the container.
 */
template<typename T>
using forked_generator = detail::basic_forked_operator<detail::generator_base, T>;

/**
 * A proxy class that delays the construction of a service until usage.
 */
template<typename T>
using lazy = detail::basic_operator<detail::lazy_base, T>;

/**
 * A proxy class that delays the construction of a service until usage.
 * 
 * This will fork the container when the proxy is created.
 */
template<typename T>
using forked_lazy = detail::basic_forked_operator<detail::lazy_base, T>;

/**
 * Alias to the default invoker
 */
using invoker = mapped_invoker<map<>>;

/**
 * Alias to the default forked invoker
 */
using forked_invoker = forked_mapped_invoker<map<>>;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP
