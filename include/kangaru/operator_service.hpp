#pragma once

#include "container.hpp"
#include "operator.hpp"

namespace kgr {

template<typename Predicate = All>
struct ForkService {
	explicit ForkService(in_place_t, Container& container) : _container{container.fork<Predicate>()} {}
	
	inline Container forward() {
		return std::move(_container);
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container _container;
};

template<template<typename> class Map>
struct InvokerService {
	explicit InvokerService(in_place_t, Container& container) : _container{container} {}
	
	Invoker<Map> forward() {
		return Invoker<Map>{_container};
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container& _container;
};

template<template<typename> class Map, typename Predicate = All>
struct ForkedInvokerService {
	explicit ForkedInvokerService(in_place_t, Container& container) : _container{container.fork<Predicate>()} {}
	
	ForkedInvoker<Map> forward() {
		return ForkedInvoker<Map>{std::move(_container)};
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container _container;
};

template<typename T>
struct GeneratorService {
	explicit GeneratorService(in_place_t, Container& container) : _container{container} {}
	
	Generator<T> forward() {
		return Generator<T>{_container};
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container& _container;
};

template<typename T, typename Predicate = All>
struct ForkedGeneratorService {
	explicit ForkedGeneratorService(in_place_t, Container& container) : _container{container.fork<Predicate>()} {}
	
	ForkedGenerator<T> forward() {
		return ForkedGenerator<T>{std::move(_container)};
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container _container;
};

template<typename T>
struct LazyService {
	explicit LazyService(in_place_t, Container& container) : _container{container} {}
	
	Lazy<T> forward() {
		return Lazy<T>{_container};
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container& _container;
};

template<typename T, typename Predicate = All>
struct ForkedLazyService {
	explicit ForkedLazyService(in_place_t, Container& container) : _container{container.fork<Predicate>()} {}
	
	ForkedLazy<T> forward() {
		return ForkedLazy<T>{std::move(_container)};
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container _container;
};

}
