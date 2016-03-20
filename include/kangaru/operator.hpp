#pragma once

#include "container.hpp"

namespace kgr {

template<template<typename> class Map>
struct Invoker {
	explicit Invoker(Container& container) : _container{container} {}
	
	template<typename F, typename... Args>
	detail::function_result_t<typename std::decay<F>::type> operator()(F&& f, Args&&... args) {
		return _container.invoke<Map>(std::forward<F>(f), std::forward<Args>(args)...);
	}
	
private:
	kgr::Container& _container;
};

template<template<typename> class Map>
struct ForkedInvoker {
	explicit ForkedInvoker(Container container) : _container{std::move(container)} {}
	
	template<typename F, typename... Args>
	detail::function_result_t<typename std::decay<F>::type> operator()(F&& f, Args&&... args) {
		return _container.invoke<Map>(std::forward<F>(f), std::forward<Args>(args)...);
	}
	
private:
	kgr::Container _container;
};

namespace detail {

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
			emplace(std::move(other.get()));
		}
	}
	
	LazyBase& operator=(LazyBase&& other) {
		if (other._initialized) {
			emplace(std::move(other.get()));
		}
		return *this;
	}
	
	LazyBase(const LazyBase& other) {
		if (other._initialized) {
			emplace(other.get());
		}
	}
	
	LazyBase& operator=(const LazyBase& other) {
		if (other._initialized) {
			emplace(other.get());
		}
		return *this;
	}
	
	~LazyBase() {
		if (_initialized) {
			get().~ServiceType<T>();
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
		
		return this->value(*reinterpret_cast<type*>(&_service));
	}
	
private:
	template<typename... Args>
	void emplace(Args&&... args) {
		if (_initialized) {
			get().~ServiceType<T>();
		}
		
		_initialized = true;
		new (&_service) type{std::forward<Args>(args)...};
	}
	
	bool _initialized = false;
	typename std::aligned_storage<sizeof(type), alignof(type)>::type _service;
};

} // namespace detail

template<typename T>
struct ForkedLazy : detail::LazyBase<ForkedLazy<T>, T> {
	explicit ForkedLazy(kgr::Container container) : _container{std::move(container)} {}
	
private:
	friend struct detail::LazyBase<ForkedLazy<T>, T>;
	kgr::Container _container;
};

template<typename T>
struct Lazy : detail::LazyBase<Lazy<T>, T> {
	explicit Lazy(kgr::Container& container) : _container{container} {}
	
private:
	friend struct detail::LazyBase<Lazy<T>, T>;
	kgr::Container& _container;
};

}
