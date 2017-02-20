#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP

#include "../container.hpp"
#include "lazy_storage.hpp"

namespace kgr {
namespace detail {

template<typename CRTP, typename T>
struct LazyBase {
private:
	using ref = typename std::remove_reference<ServiceType<T>>::type&;
	using rref = typename std::remove_reference<ServiceType<T>>::type&&;
	using ptr = decay_t<ServiceType<T>>*;
	
	static constexpr bool nothrow_get = 
		noexcept(std::declval<Container>().service<T>()) &&
		noexcept(std::declval<LazyStorage<ServiceType<T>>>().construct(std::declval<ServiceType<T>>()));
	
public:
	ref get() noexcept(nothrow_get) {
		if (!_service) {
			_service.construct(static_cast<CRTP*>(this)->container().template service<T>());
		}
		
		return _service.value();
	}
	
	ref operator*() & noexcept(nothrow_get) {
		return get();
	}
	
	ptr operator->() noexcept(nothrow_get) {
		return &get();
	}
	
	rref operator*() && noexcept(nothrow_get) {
		return std::move(get());
	}
	
private:
	LazyStorage<ServiceType<T>> _service;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_LAZY_BASE_HPP
