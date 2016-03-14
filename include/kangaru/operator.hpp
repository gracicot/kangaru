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

}
