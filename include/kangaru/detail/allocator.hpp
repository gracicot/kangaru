#ifndef KANGARU5_DETAIL_ALLOCATOR_HPP
#define KANGARU5_DETAIL_ALLOCATOR_HPP

#include <type_traits>
#include <cstddef>
#include <memory>
#include <limits>

namespace kangaru {
	template<typename T>
	concept object_allocator = requires(T allocator, int* ptr) {
		{ allocator.template allocate_object<int>() } -> std::same_as<int*>;
		{ allocator.template deallocate_object<int>(ptr) } -> std::same_as<void>;
	};
	
	struct default_allocator {
		constexpr auto allocate_bytes(std::size_t size, std::size_t alignment) -> void* {
			if (std::is_constant_evaluated()) {
				// We ignore alignment in constexpr
				return std::allocator<std::byte>{}.allocate(size);
			} else {
				return ::operator new(size, static_cast<std::align_val_t>(alignment));
			}
		}
		
		constexpr auto deallocate_bytes(void* ptr, std::size_t size, std::size_t alignment) noexcept -> void {
			if (std::is_constant_evaluated()) {
				std::allocator<std::byte>{}.deallocate(static_cast<std::byte*>(ptr), size);
			} else {
				::operator delete(ptr, static_cast<std::align_val_t>(alignment));
			}
		}
		
		template<typename U>
		constexpr auto allocate_object(std::size_t n = 1) -> U* {
			if(n > std::numeric_limits<std::size_t>::max() / sizeof(U)) {
				throw std::bad_array_new_length{};
			}
			
			if (std::is_constant_evaluated()) {
				return std::allocator<U>().allocate(n);
			} else {
				return static_cast<U*>(allocate_bytes(n * sizeof(U), alignof(U)));
			}
		}
		
		template<typename U>
		constexpr auto deallocate_object(U* ptr, std::size_t n = 1) noexcept -> void {
			if (std::is_constant_evaluated()) {
				std::allocator<U>{}.deallocate(ptr, n);
			} else {
				deallocate_bytes(ptr, n * sizeof(U), alignof(U));
			}
		}
	};
}

#endif // KANGARU5_DETAIL_ALLOCATOR_HPP
