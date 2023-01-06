#ifndef KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP

#include "container.hpp"
#include "operator.hpp"

namespace kgr {
namespace detail {

/*
 * Base class for all non-forking operator service definitions.
 * 
 * Holds a non-owning reference to the container as a pointer.
 */
struct operator_service_base {
	inline explicit operator_service_base(in_place_t, kgr::container& container) noexcept : _container{&container} {}
	
	inline static auto construct(inject_t<container_service> cs) -> decltype(inject(cs.forward())) {
		return inject(cs.forward());
	}
	
protected:
	kgr::container* _container;
};

/*
 * Template base class for non-forking operator service definitions.
 * 
 * Only exist to provide the forward function and not template the whole base class.
 */
template<typename Type>
struct operator_service : operator_service_base {
	using operator_service_base::operator_service_base;
	
	auto forward() -> Type {
		return Type{*_container};
	}
};

/*
 * Base class for all forking operator service definitions.
 * 
 * Holds a non-owning reference to the container as a pointer.
 */
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

/*
 * Service defintion for a forking container.
 */
template<typename Predicate>
struct filtered_fork_service : detail::forked_operator_service<Predicate, container> {
	using detail::forked_operator_service<Predicate, container>::forked_operator_service;
	
	auto forward() -> container {
		return std::move(this->_container);
	}
};

/*
 * Service defintion for the mapped_invoker.
 */
template<typename Map>
using mapped_invoker_service = detail::operator_service<mapped_invoker<Map>>;

/*
 * Service defintion for the forked_mapped_invoker.
 */
template<typename Map, typename Predicate = all>
using forked_mapped_invoker_service = detail::forked_operator_service<Predicate, forked_mapped_invoker<Map>>;

/*
 * Service defintion for the generator.
 */
template<typename T>
using generator_service = detail::operator_service<generator<T>>;

/*
 * Service defintion for the forked_generator.
 */
template<typename T, typename Predicate = all>
using forked_generator_service = detail::forked_operator_service<Predicate, forked_generator<T>>;

/*
 * Service defintion for the lazy.
 */
template<typename T>
using lazy_service = detail::operator_service<lazy<T>>;

/*
 * Service defintion for the forked_lazy.
 */
template<typename T, typename Predicate = all>
using forked_lazy_service = detail::forked_operator_service<Predicate, forked_lazy<T>>;

/*
 * Alias to the filtered_fork_service with the default predicate.
 */
using fork_service = filtered_fork_service<all>;

/*
 * Alias to the invoker service with the default map.
 */
using invoker_service = mapped_invoker_service<map<>>;

/*
 * Alias to the forked invoker service with the default map.
 * 
 * TODO: add a way to use a custom predicate
 */
using forked_invoker_service = forked_mapped_invoker_service<map<>>;


} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_SERVICE_HPP
