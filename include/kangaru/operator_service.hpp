#pragma once

#include "container.hpp"
#include "operator.hpp"

namespace kgr {

template<typename Predicate = All>
struct ForkService : detail::ContainerServiceTag {
	explicit ForkService(Container& container) : _container{container.fork<Predicate>()} {}
	
	inline Container forward() {
		return std::move(_container);
	}
	
private:
	Container _container;
};

template<template<typename> class Map>
struct InvokerService : detail::ContainerServiceTag {
	explicit InvokerService(Container& container) : _container{container} {}
	
	Invoker<Map> forward() {
		return Invoker<Map>{_container};
	}
	
private:
	Container& _container;
};

template<template<typename> class Map, typename Predicate = All>
struct ForkedInvokerService : detail::ContainerServiceTag {
	explicit ForkedInvokerService(Container& container) : _container{container.fork<Predicate>()} {}
	
	ForkedInvoker<Map> forward() {
		return ForkedInvoker<Map>{std::move(_container)};
	}
	
private:
	Container _container;
};

template<typename T>
struct GeneratorService : detail::ContainerServiceTag {
	GeneratorService(Container& container) : _container{container} {}
	
	Generator<T> forward() {
		return Generator<T>{_container};
	}
	
private:
	Container& _container;
};

template<typename T>
struct LazyService : detail::ContainerServiceTag {
	explicit LazyService(Container& container) : _container{container} {}
	
	Lazy<T> forward() {
		return Lazy<T>{_container};
	}
	
private:
	Container& _container;
};

template<typename T, typename Predicate = All>
struct ForkedLazyService : detail::ContainerServiceTag {
	explicit ForkedLazyService(Container& container) : _container{container.fork<Predicate>()} {}
	
	ForkedLazy<T> forward() {
		return ForkedLazy<T>{std::move(_container)};
	}
	
private:
	Container _container;
};

}
