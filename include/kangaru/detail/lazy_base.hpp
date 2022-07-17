#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP

#include "operator_service_helper.hpp"
#include "../optional.hpp"
#include "../container.hpp"

namespace kgr {
namespace detail {

/*
 * This is the base class of the Lazy and ForkedLazy class.
 * This implements all the common things a Lazy must implement.
 */
template<typename Base, typename T>
struct lazy_base : protected Base {
private:
	using ref = typename std::remove_reference<service_type<T>>::type&;
	using rref = typename std::remove_reference<service_type<T>>::type&&;
	using ptr = typename std::add_pointer<remove_cvref_t<service_type<T>>>::type;
	
public:
	using Base::Base;

	ref get() {
		if (!_service) {
			kgr::container& c = this->container();
			_service.construct(c.service<T>());
		}
		
		return _service.value();
	}
	
	ref operator*() & {
		return get();
	}
	
	ptr operator->() {
		return &get();
	}
	
	rref operator*() && {
		return std::move(get());
	}
	
private:
	optional<service_type<T>> _service;
	
	inline friend auto service_map(lazy_base const&) -> select_operator_service<Base> { return {}; }
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
