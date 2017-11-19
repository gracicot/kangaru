#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP

#include "traits.hpp"
#include "utils.hpp"

namespace kgr {
namespace detail {

/*
 * This defines the pointer to a forward function.
 * This is simply a shortcut for not writing the function pointer type everywhere.
 */
template<typename T>
using forward_ptr = service_type<T>(*)(void*);

template<typename T>
struct forward_storage {
	forward_ptr<T> forward;
};

template<typename T>
struct typed_service_storage {
	void* service;
	forward_ptr<T> forward;
};

struct service_storage {
	template<typename T>
	service_storage(const typed_service_storage<T>& storage) noexcept : service{storage.service} {
		new (&forward_function) forward_storage<T>{storage.forward};
	}
	
	void* service;
	
	template<typename T>
	auto cast() const -> typed_service_storage<T> {
		return typed_service_storage<T>{service, reinterpret_cast<const forward_storage<T>*>(&forward_function)->forward};
	}
	
private:
	using function_pointer = void*(*)(void*);
	aligned_storage_t<sizeof(function_pointer), alignof(function_pointer)> forward_function;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP
