#ifndef KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP

#include "container.hpp"
#include "operator.hpp"

namespace kgr {

template<typename Predicate>
struct FilteredForkService {
	explicit FilteredForkService(in_place_t, Container& container) : _container{container.fork<Predicate>()} {}
	
	inline Container forward() {
		return std::move(_container);
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container _container;
};

using ForkService = FilteredForkService<All>;

auto service_map(Container&&) -> ForkService;

template<typename Map>
struct MappedInvokerService {
	explicit MappedInvokerService(in_place_t, Container& container) : _container{container} {}
	
	MappedInvoker<Map> forward() {
		return MappedInvoker<Map>{_container};
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container& _container;
};

using InvokerService = MappedInvokerService<kgr::Map<>>;

template<typename Map>
auto service_map(const MappedInvoker<Map>&) -> MappedInvokerService<Map>;

template<typename Map, typename Predicate = All>
struct ForkedMappedInvokerService {
	explicit ForkedMappedInvokerService(in_place_t, Container& container) : _container{container.fork<Predicate>()} {}
	
	ForkedMappedInvoker<Map> forward() {
		return ForkedMappedInvoker<Map>{std::move(_container)};
	}
	
	static auto construct(Inject<ContainerService> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	Container _container;
};

using ForkedInvokerService = ForkedMappedInvokerService<kgr::Map<>>;

template<typename Map>
auto service_map(const ForkedMappedInvoker<Map>&) -> ForkedMappedInvokerService<Map>;

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

template<typename T>
auto service_map(const Generator<T>&) -> GeneratorService<T>;

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
auto service_map(const ForkedGenerator<T>&) -> ForkedGeneratorService<T>;

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

template<typename T>
auto service_map(const Lazy<T>&) -> LazyService<T>;

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

template<typename T>
auto service_map(const ForkedLazy<T>&) -> ForkedLazyService<T>;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP
