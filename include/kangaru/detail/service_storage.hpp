#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP

#include "traits.hpp"
#include "utils.hpp"
#include "single.hpp"

namespace kgr {
namespace detail {

union ServiceStorage {
	aligned_storage_t<service_storage_size, service_storage_alignement> storage;
	void* pointer;
	
	template<typename T>
	ServiceStorage(T* ptr) noexcept : pointer{ptr} {}
	
	template<typename T, typename... Args, enable_if_t<std::is_constructible<T, Args...>::value && is_service_embeddable<T>::value, int> = 0>
	ServiceStorage(type_wrapper<T>, Args&&... args) noexcept { new (&storage) T(std::forward<Args>(args)...); }
	
	template<typename T, typename... Args, enable_if_t<is_only_brace_constructible<T, Args...>::value && is_service_embeddable<T>::value, int> = 0>
	ServiceStorage(type_wrapper<T>, Args&&... args) noexcept { new (&storage) T{std::forward<Args>(args)...}; }
	
	template<typename T, enable_if_t<is_service_embeddable<T>::value, int> = 0>
	T& get() noexcept {
		return *reinterpret_cast<T*>(&storage);
	}
	
	template<typename T, enable_if_t<!is_service_embeddable<T>::value, int> = 0>
	T& get() noexcept {
		return *static_cast<T*>(pointer);
	}
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP
