#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP

#include "../container.hpp"
#include "lazy_storage.hpp"

namespace kgr {
namespace detail {

/*
 * This is the base class of the Lazy and ForkedLazy class.
 * This implements all the common things a Lazy must implement.
 */
template<typename CRTP, typename T>
struct lazy_base {
private:
	using ref = typename std::remove_reference<service_type<T>>::type&;
	using rref = typename std::remove_reference<service_type<T>>::type&&;
	using ptr = typename std::add_pointer<remove_cvref_t<service_type<T>>>::type;
	
	static constexpr bool nothrow_get() {
		return noexcept(std::declval<container>().service<T>()) &&
			noexcept(std::declval<lazy_storage<service_type<T>>>().construct(std::declval<service_type<T>>()));
	}
	
public:
	ref get() noexcept(nothrow_get()) {
		if (!_service) {
			container& container = static_cast<CRTP*>(this)->container();
			_service.construct(container.service<T>());
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
	lazy_storage<service_type<T>> _service;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
