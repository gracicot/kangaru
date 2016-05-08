#pragma once

#include "container.hpp"
#include "detail/lazy_base.hpp"

namespace kgr {
namespace detail {

template<typename CRTP, template<typename> class Map>
struct InvokerBase {
	template<typename F, typename... Args>
	detail::function_result_t<typename std::decay<F>::type> operator()(F&& f, Args&&... args) {
		return static_cast<CRTP*>(this)->_container.template invoke<Map>(std::forward<F>(f), std::forward<Args>(args)...);
	}
};

template<typename CRTP, typename T>
struct GeneratorBase {
	static_assert(!std::is_base_of<Single, T>::value, "Generator only work with non-single services.");
	
	template<typename... Args>
	ServiceType<T> operator()(Args&& ...args) {
		return static_cast<CRTP*>(this)->_container.template service<T>(std::forward<Args>(args)...);
	}
};

} // namespace detail

template<template<typename> class Map>
struct Invoker : detail::InvokerBase<Invoker<Map>, Map> {
	explicit Invoker(Container& container) : _container{container} {}
	
private:
	friend struct detail::InvokerBase<Invoker<Map>, Map>;
	kgr::Container& _container;
};

template<template<typename> class Map>
struct ForkedInvoker : detail::InvokerBase<ForkedInvoker<Map>, Map> {
	explicit ForkedInvoker(Container container) : _container{std::move(container)} {}
	
private:
	friend struct detail::InvokerBase<ForkedInvoker<Map>, Map>;
	kgr::Container _container;
};

template<typename T>
struct Generator : detail::GeneratorBase<Generator<T>, T> {
	explicit Generator(Container& container) : _container{container} {}
	
private:
	friend struct detail::GeneratorBase<Generator<T>, T>;
	kgr::Container& _container;
};

template<typename T>
struct ForkedGenerator : detail::GeneratorBase<ForkedGenerator<T>, T> {
	explicit ForkedGenerator(Container container) : _container{std::move(container)} {}
	
private:
	friend struct detail::GeneratorBase<ForkedGenerator<T>, T>;
	kgr::Container _container;
};

template<typename T>
struct Lazy : detail::LazyBase<Lazy<T>, T> {
	explicit Lazy(kgr::Container& container) : _container{&container} {}
	
private:
	kgr::Container& container() {
		return *_container;
	}
	
	const kgr::Container& container() const {
		return *_container;
	}
	
	friend struct detail::LazyBase<Lazy<T>, T>;
	kgr::Container* _container;
};

template<typename T>
struct ForkedLazy : detail::LazyBase<ForkedLazy<T>, T> {
	explicit ForkedLazy(kgr::Container container) : _container{std::move(container)} {}
	
private:
	kgr::Container& container() {
		return _container;
	}
	
	const kgr::Container& container() const {
		return _container;
	}
	
	friend struct detail::LazyBase<ForkedLazy<T>, T>;
	kgr::Container _container;
};

} // namespace kgr
