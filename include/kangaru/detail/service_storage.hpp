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

template<typename T>
struct constant_typed_service_storage {
	void const* service;
	forward_ptr<T> forward;
};

struct service_storage {
private:
	using function_pointer = auto(*)(void*) -> void*;
	
public:
	template<typename T>
	explicit service_storage(const typed_service_storage<T>& storage) noexcept : _service{storage.service} {
		static_assert(sizeof(function_pointer) >= sizeof(forward_storage<T>), "The forward storage size exceed the size of a function pointer");
		static_assert(alignof(function_pointer) >= alignof(forward_storage<T>), "The forward storage alignement exceed the alignement of a function pointer");
		
		new (&forward_function) forward_storage<T>{storage.forward};
	}
	
	template<typename T>
	auto operator=(const typed_service_storage<T>& storage) -> service_storage& {
		static_assert(sizeof(function_pointer) >= sizeof(forward_storage<T>), "The forward storage size exceed the size of a function pointer");
		static_assert(alignof(function_pointer) >= alignof(forward_storage<T>), "The forward storage alignement exceed the alignement of a function pointer");
		
		_service = storage.service;
		new (&forward_function) forward_storage<T>{storage.forward};
		return *this;
	}
	
	template<typename T>
	auto service() noexcept -> T& {
		return *static_cast<T*>(_service);
	}
	
	template<typename T>
	auto service() const noexcept -> T const& {
		return *static_cast<T const*>(_service);
	}
	
	template<typename T>
	auto cast() const noexcept -> constant_typed_service_storage<T> {
		return constant_typed_service_storage<T>{_service, reinterpret_cast<const forward_storage<T>*>(&forward_function)->forward};
	}
	
	template<typename T>
	auto cast() noexcept -> typed_service_storage<T> {
		return typed_service_storage<T>{_service, reinterpret_cast<const forward_storage<T>*>(&forward_function)->forward};
	}
	
private:
	void* _service;
	aligned_storage_t<sizeof(function_pointer), alignof(function_pointer)> forward_function;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP
