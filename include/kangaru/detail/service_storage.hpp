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
private:
	using function_pointer = void*(*)(void*);
	
public:
	template<typename T>
	service_storage(const typed_service_storage<T>& storage) noexcept : _service{storage.service} {
		static_assert(sizeof(function_pointer) >= sizeof(forward_storage<T>), "The forward storage size exceed the size of a function pointer");
		static_assert(alignof(function_pointer) >= alignof(forward_storage<T>), "The forward storage alignement exceed the alignement of a function pointer");
		
		new (&forward_function) forward_storage<T>{storage.forward};
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
	auto cast() noexcept -> typed_service_storage<T> {
		return typed_service_storage<T>{_service, reinterpret_cast<const forward_storage<T>*>(&forward_function)->forward};
	}
	
private:
	void* _service;
	aligned_storage_t<sizeof(function_pointer), alignof(function_pointer)> forward_function;
};


template<typename Derived, typename Type, typename = void>
struct memory_block_destruction {
	~memory_block_destruction() {
		static_cast<Derived*>(this)->cast().~Type();
	}
};

template<typename Derived, typename Type>
struct memory_block_destruction<Derived, Type, enable_if_t<std::is_trivially_destructible<Type>::value>> {};

#if _MSC_VER == 1900
#ifndef __clang__
#pragma warning( push )
#pragma warning( disable : 4521 )
#endif
#endif

template<typename T>
struct memory_block : memory_block_destruction<memory_block<T>, T> {
	memory_block(memory_block const&) = delete;
	memory_block(memory_block &&) = delete;
	memory_block(memory_block&) = delete;
	memory_block(memory_block const&&) = delete;
	memory_block& operator=(memory_block const&) = delete;
	memory_block& operator=(memory_block &&) = delete;
	
	template<typename... Args, enable_if_t<std::is_constructible<T, Args...>::value, int> = 0>
	explicit memory_block(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) {
		new (&service) T(std::forward<Args>(args)...);
	}
	
	template<typename... Args, enable_if_t<
		!std::is_constructible<T, Args...>::value &&
		is_brace_constructible_helper<T, Args...>::value, int> = 0>
	explicit memory_block(Args&&... args) noexcept(noexcept(T{std::forward<Args>(args)...})) {
		new (&service) T{std::forward<Args>(args)...};
	}
	
	auto cast() noexcept -> T& {
		return *static_cast<T*>(static_cast<void*>(&service));
	}
	
	aligned_storage_t<sizeof(T), alignof(T)> service;
};

#if _MSC_VER
#ifndef __clang__
#pragma warning( pop )
#endif
#endif

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP
