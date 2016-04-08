#pragma once

#include "container.hpp"

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

template<typename T>
struct LazyHelper {
protected:
	using type = T;
	using ref = T&;
	using ptr = T*;
	
	T&& assign(T&& service) {
		return std::move(service);
	}
	
	T& value(T& service) {
		return service;
	}
};

template<typename T>
struct LazyHelper<T&> {
protected:
	using type = T*;
	using ref = T&;
	using ptr = T*;
	
	T* assign(T& service) {
		return &service;
	}
	
	T& value(T* service) {
		return *service;
	}
};

template<typename CRTP, typename T>
struct LazyBase : LazyHelper<ServiceType<T>> {
private:
	using typename detail::LazyHelper<ServiceType<T>>::type;
	using typename detail::LazyHelper<ServiceType<T>>::ref;
	using typename detail::LazyHelper<ServiceType<T>>::ptr;
	
public:
	LazyBase() = default;
	LazyBase(LazyBase&& other) {
		if (other._initialized) {
			emplace(std::move(other.data()));
		}
	}
	
	LazyBase& operator=(LazyBase&& other) {
		if (other._initialized) {
			emplace(std::move(other.data()));
		}
		return *this;
	}
	
	LazyBase(const LazyBase& other) {
		if (other._initialized) {
			emplace(other.data());
		}
	}
	
	LazyBase& operator=(const LazyBase& other) {
		if (other._initialized) {
			emplace(other.data());
		}
		return *this;
	}
	
	~LazyBase() {
		if (_initialized) {
			data().~type();
		}
	}
	
	ref operator*() {
		return get();
	}
	
	ptr operator->() {
		return &get();
	}
	
	ref get() {
		if (!_initialized) {
			emplace(this->assign(static_cast<CRTP*>(this)->_container.template service<T>()));
		}
		
		return this->value(data());
	}
	
private:
	type& data() {
		return *reinterpret_cast<type*>(&_service);
	}
	
	template<typename... Args>
	void emplace(Args&&... args) {
		if (_initialized) {
			data().~type();
		}
		
		_initialized = true;
		new (&_service) type(std::forward<Args>(args)...);
	}
	
	bool _initialized = false;
	typename std::aligned_storage<sizeof(type), alignof(type)>::type _service;
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
	explicit Lazy(kgr::Container& container) : _container{container} {}
	
private:
	friend struct detail::LazyBase<Lazy<T>, T>;
	kgr::Container& _container;
};

template<typename T>
struct ForkedLazy : detail::LazyBase<ForkedLazy<T>, T> {
	explicit ForkedLazy(kgr::Container container) : _container{std::move(container)} {}
	
private:
	friend struct detail::LazyBase<ForkedLazy<T>, T>;
	kgr::Container _container;
};

} // namespace kgr
