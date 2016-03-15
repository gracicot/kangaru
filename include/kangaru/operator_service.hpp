#pragma once

#include "container.hpp"
#include "operator.hpp"

namespace kgr {

struct ForkService : detail::ContainerServiceTag {
	ForkService(Container& container) : _container{container.fork()} {}
	
	inline Container forward() {
		return std::move(_container);
	}
	
private:
	Container _container;
};

template<template<typename> class Map>
struct InvokerService : detail::ContainerServiceTag {
	InvokerService(Container& container) : _container{container} {}
	
	Invoker<Map> forward() {
		return Invoker<Map>{_container};
	}
	
private:
	Container& _container;
};

template<template<typename> class Map>
struct ForkedInvokerService : detail::ContainerServiceTag {
	ForkedInvokerService(Container& container) : _container{container.fork()} {}
	
	ForkedInvoker<Map> forward() {
		return ForkedInvoker<Map>{std::move(_container)};
	}
	
private:
	Container _container;
};

}
