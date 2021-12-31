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
	
	static constexpr bool nothrow_get() {
		return noexcept(std::declval<kgr::container>().service<T>()) &&
			noexcept(std::declval<optional<service_type<T>>>().construct(std::declval<service_type<T>>()));
	}
	
public:
	using Base::Base;

	ref get() noexcept(nothrow_get()) {
		if (!_service) {
			kgr::container& c = this->container();
			_service.construct(c.service<T>());
		}
		
		return _service.value();
	}
	
	ref operator*() & noexcept(nothrow_get()) {
		return get();
	}
	
	ptr operator->() noexcept(nothrow_get()) {
		return &get();
	}
	
	rref operator*() && noexcept(nothrow_get()) {
		return std::move(get());
	}
	
private:
	optional<service_type<T>> _service;
	
	inline friend auto service_map(lazy_base const&) -> select_operator_service<Base> { return {}; }
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
