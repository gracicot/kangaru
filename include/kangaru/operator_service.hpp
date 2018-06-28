#ifndef KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP

#include "container.hpp"
#include "operator.hpp"

namespace kgr {
namespace detail {

struct operator_service_base {
	inline explicit operator_service_base(in_place_t, kgr::container& container) noexcept : _container{&container} {}
	
	inline static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
protected:
	kgr::container* _container;
};

template<typename Type>
struct operator_service : operator_service_base {
	using operator_service_base::operator_service_base;
	
	auto forward() -> Type {
		return Type{*_container};
	}
};

template<typename Predicate, typename Type>
struct forked_operator_service {
	explicit forked_operator_service(in_place_t, kgr::container& container) noexcept : _container{container.fork<Predicate>()} {}
	
	static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
	auto forward() -> Type {
		return Type{std::move(_container)};
	}
	
protected:
	kgr::container _container;
};

} // namespace detail

template<typename Predicate>
struct filtered_fork_service : detail::forked_operator_service<Predicate, container> {
	using detail::forked_operator_service<Predicate, container>::forked_operator_service;
	
	auto forward() -> container {
		return std::move(this->_container);
	}
};

template<typename Map>
using mapped_invoker_service = detail::operator_service<mapped_invoker<Map>>;

template<typename Map, typename Predicate = all>
using forked_mapped_invoker_service = detail::forked_operator_service<Predicate, forked_mapped_invoker<Map>>;

template<typename T>
using generator_service = detail::operator_service<generator<T>>;

template<typename T, typename Predicate = all>
using forked_generator_service = detail::forked_operator_service<Predicate, forked_generator<T>>;

template<typename T>
using lazy_service = detail::operator_service<lazy<T>>;

template<typename T, typename Predicate = all>
using forked_lazy_service = detail::forked_operator_service<Predicate, forked_lazy<T>>;

using fork_service = filtered_fork_service<all>;
using invoker_service = mapped_invoker_service<map<>>;
using forked_invoker_service = forked_mapped_invoker_service<map<>>;

auto service_map(container&&) -> fork_service;

template<typename T>
auto service_map(const generator<T>&) -> generator_service<T>;

template<typename T>
auto service_map(const forked_generator<T>&) -> forked_generator_service<T>;

template<typename Map>
auto service_map(const mapped_invoker<Map>&) -> mapped_invoker_service<Map>;

template<typename Map>
auto service_map(const forked_mapped_invoker<Map>&) -> forked_mapped_invoker_service<Map>;

template<typename T>
auto service_map(const lazy<T>&) -> lazy_service<T>;

template<typename T>
auto service_map(const forked_lazy<T>&) -> forked_lazy_service<T>;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP
