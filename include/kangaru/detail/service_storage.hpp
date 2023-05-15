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

/*
 * Pair of a pointer to a service and it's forward function
 * Used to passe around a type erase service that yield a particular type
 */
template<typename T>
struct typed_service_storage {
	void* service;
	forward_ptr<T> forward;
};

/*
 * Tag type to tell service_storage that its supposed to contain an index of an override
 */
struct override_index_t {} constexpr override_index{};

/*
 * Type erased storage for any service type or an override index
 */
struct service_storage {
private:
	using function_pointer = void*(*)(void*);
	
public:
	template<typename T>
	service_storage(typed_service_storage<T> const& storage) noexcept : _service{storage.service} {
		static_assert(sizeof(function_pointer) >= sizeof(forward_storage<T>), "The forward storage size exceed the size of a function pointer");
		static_assert(alignof(function_pointer) >= alignof(forward_storage<T>), "The forward storage alignement exceed the alignement of a function pointer");
		
		new (&forward_function) forward_storage<T>{storage.forward};
	}
	
	inline explicit service_storage(override_index_t, std::size_t index) noexcept : _service{nullptr} {
		static_assert(
			sizeof(function_pointer) >= sizeof(std::size_t) &&
			alignof(function_pointer) >= alignof(std::size_t),
			"The size and alignement of std::size_t exceed the size and alignement of a function pointer"
		);
		
		new (&forward_function) std::size_t{index};
	}
	
	inline explicit service_storage(override_index_t, void* collection) noexcept : _service{collection}, forward_function{} {}
	
	template<typename T>
	auto service() noexcept -> T& {
		return *static_cast<T*>(_service);
	}
	
	template<typename T>
	auto service() const noexcept -> T const& {
		return *static_cast<T const*>(_service);
	}
	
	template<typename T>
	auto cast() const noexcept -> typed_service_storage<T> {
		return typed_service_storage<T>{
			_service,
			static_cast<forward_storage<T> const*>(static_cast<void const*>(&forward_function))->forward
		};
	}
	
	inline auto index() const noexcept -> std::size_t {
		return *static_cast<std::size_t const*>(static_cast<void const*>(&forward_function));
	}
	
private:
	void* _service;
	alignas(alignof(function_pointer)) unsigned char forward_function[sizeof(function_pointer)];
};

/*
 * A non moveable and non copyable type wrapper for a service
 *
 * Also conveniently choose the right constructor
 */
template<typename T>
struct memory_block {
	memory_block(memory_block const&) = delete;
	memory_block(memory_block &&) = delete;
	memory_block& operator=(memory_block const&) = delete;
	memory_block& operator=(memory_block &&) = delete;
	
	template<typename... Args, enable_if_t<std::is_constructible<T, Args...>::value, int> = 0>
	explicit memory_block(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) :
		service(std::forward<Args>(args)...) {}
	
	template<typename... Args, enable_if_t<is_only_brace_constructible<T, Args...>::value, int> = 0>
	explicit memory_block(Args&&... args) noexcept(noexcept(T{std::forward<Args>(args)...})) :
		service{std::forward<Args>(args)...} {}
	
	T service;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_STORAGE_HPP
