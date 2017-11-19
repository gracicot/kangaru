#ifndef KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP

#include "container.hpp"
#include "operator.hpp"

namespace kgr {

template<typename Predicate>
struct filtered_fork_service {
	explicit filtered_fork_service(in_place_t, container& container) : _container{container.fork<Predicate>()} {}
	
	container forward() {
		return std::move(_container);
	}
	
	static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	container _container;
};

using fork_service = filtered_fork_service<all>;

auto service_map(container&&) -> fork_service;

template<typename Map>
struct mapped_invoker_service {
	explicit mapped_invoker_service(in_place_t, container& container) : _container{container} {}
	
	mapped_invoker<Map> forward() {
		return mapped_invoker<Map>{_container};
	}
	
	static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	container& _container;
};

using invoker_service = mapped_invoker_service<map<>>;

template<typename Map>
auto service_map(const mapped_invoker<Map>&) -> mapped_invoker_service<Map>;

template<typename Map, typename Predicate = all>
struct forked_mapped_invoker_service {
	explicit forked_mapped_invoker_service(in_place_t, container& container) : _container{container.fork<Predicate>()} {}
	
	forked_mapped_invoker<Map> forward() {
		return forked_mapped_invoker<Map>{std::move(_container)};
	}
	
	static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	container _container;
};

using forked_invoker_service = forked_mapped_invoker_service<map<>>;

template<typename Map>
auto service_map(const forked_mapped_invoker<Map>&) -> forked_mapped_invoker_service<Map>;

template<typename T>
struct generator_service {
	explicit generator_service(in_place_t, container& container) : _container{container} {}
	
	generator<T> forward() {
		return generator<T>{_container};
	}
	
	static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	container& _container;
};

template<typename T>
auto service_map(const generator<T>&) -> generator_service<T>;

template<typename T, typename Predicate = all>
struct forked_generator_service {
	explicit forked_generator_service(in_place_t, container& container) : _container{container.fork<Predicate>()} {}
	
	forked_generator<T> forward() {
		return forked_generator<T>{std::move(_container)};
	}
	
	static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	container _container;
};

template<typename T>
auto service_map(const forked_generator<T>&) -> forked_generator_service<T>;

template<typename T>
struct lazy_service {
	explicit lazy_service(in_place_t, container& container) : _container{container} {}
	
	lazy<T> forward() {
		return lazy<T>{_container};
	}
	
	static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	container& _container;
};

template<typename T>
auto service_map(const lazy<T>&) -> lazy_service<T>;

template<typename T, typename Predicate = all>
struct forked_lazy_service {
	explicit forked_lazy_service(in_place_t, container& container) : _container{container.fork<Predicate>()} {}
	
	forked_lazy<T> forward() {
		return forked_lazy<T>{std::move(_container)};
	}
	
	static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
private:
	container _container;
};

template<typename T>
auto service_map(const forked_lazy<T>&) -> forked_lazy_service<T>;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP
