#ifndef KANGARU5_DETAIL_RUNTIME_CACHED_SOURCE_HPP
#define KANGARU5_DETAIL_RUNTIME_CACHED_SOURCE_HPP

#include "constructor.hpp"
#include "metadata.hpp"
#include "utility.hpp"
#include "murmur.hpp"
#include "source.hpp"
#include "ctti.hpp"

#include <cstddef>
#include <type_traits>
#include <concepts>
#include <memory>
#include <unordered_map>
#include <limits>

#include "define.hpp"

namespace kangaru {
	template<typename Function>
		requires requires (Function f) { { f() } -> unqualified_object; }
	struct expr_to_rvo {
		constexpr explicit expr_to_rvo(Function function) noexcept : function{std::move(function)} {}
		
		constexpr operator auto () const {
			return function();
		}
		
	private:
		Function function;
	};
	
	struct runtime_dynamic_storage {
		using dynamic_deleter = auto(void*) noexcept -> void;
		
		template<unqualified_object T>
		explicit constexpr runtime_dynamic_storage(T* ptr) noexcept
			: ptr{ptr}, deleter{[](void* ptr) KANGARU5_CONSTEXPR_VOIDSTAR noexcept { delete static_cast<T*>(ptr); }} {}
		
		KANGARU5_CONSTEXPR_VOIDSTAR ~runtime_dynamic_storage() {
			if (ptr and deleter) {
				deleter(ptr);
			}
		}
		
		runtime_dynamic_storage(runtime_dynamic_storage const&) = delete;
		auto operator=(runtime_dynamic_storage const&) -> runtime_dynamic_storage& = delete;
		
		constexpr runtime_dynamic_storage(runtime_dynamic_storage&& other) noexcept :
			ptr{std::exchange(other.ptr, nullptr)},
			deleter{std::exchange(other.deleter, nullptr)} {}
			
		constexpr auto operator=(runtime_dynamic_storage&& other) noexcept -> runtime_dynamic_storage& {
			std::swap(ptr, other.ptr);
			std::swap(deleter, other.deleter);
			return *this;
		}
		
		template<typename T>
		KANGARU5_CONSTEXPR_VOIDSTAR auto as() noexcept -> T& {
			return *get_as<T>();
		}
		
		template<typename T>
		KANGARU5_CONSTEXPR_VOIDSTAR auto get_as() noexcept -> T* {
			return static_cast<T*>(ptr);
		}
		
		inline auto get() noexcept -> void* {
			return ptr;
		}
		
	private:
		void* ptr;
		dynamic_deleter* deleter;
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
	
	template<
		source Source,
		typename Map = std::unordered_map<std::size_t, void*>,
		typename Memory = std::vector<runtime_dynamic_storage>,
		typename Allocator = default_allocator
	>
	struct runtime_source {
		explicit constexpr runtime_source(Source source) noexcept
		requires (
			    std::default_initializable<Map>
			and std::default_initializable<Memory>
			and std::default_initializable<Allocator>
		) : source{std::move(source)} {}
		
		constexpr runtime_source(Source source, Map cache) noexcept
		requires (
			    std::default_initializable<Memory>
			and std::default_initializable<Allocator>
		) : source{std::move(source)}, cache{std::move(cache)} {}
		
		constexpr runtime_source(Source source, Map cache, Memory memory) noexcept
		requires (std::default_initializable<Allocator>) :
			source{std::move(source)}, cache{std::move(cache)}, memory{std::move(memory)} {}
		
		constexpr runtime_source(Source source, Map cache, Memory memory, Allocator allocator) noexcept :
			source{std::move(source)},
			cache{std::move(cache)},
			memory{std::move(memory)},
			allocator{std::move(allocator)} {}
		
	private:
		template<object T, forwarded<runtime_source> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag<T&> tag, Self&& source) -> T& {
			constexpr auto id = detail::ctti::type_id_for<T>();
			auto const it = source.cache.find(id);
			
			if (it == source.cache.end()) {
				auto const ptr = source.allocator.template allocate_object<T>();
				std::construct_at(
					ptr,
					expr_to_rvo{
						[&source, tag] {
							return kangaru::provide(tag, KANGARU5_FWD(source).source);
						}
					}
				);
				auto const [it, _] = source.cache.insert(std::pair<std::size_t, void*>{id, ptr});
				return *static_cast<T*>(it->second);
			} else {
				return *static_cast<T*>(it->second);
			}
		}
		
		template<object T, forwarded<runtime_source> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T&>
		friend constexpr auto provide(provide_tag<T&> tag, Self&& source) -> T& {
			constexpr auto id = detail::ctti::type_id_for<T>();
			auto const it = source.cache.find(id);
			
			if (it == source.cache.end()) {
				auto& ref = kangaru::provide(tag, KANGARU5_FWD(source).source);
				auto const [it, _] = source.cache.insert(std::pair<std::size_t, void*>{id, std::addressof(ref)});
				return *static_cast<T*>(it->second);
			} else {
				return *static_cast<T*>(it->second);
			}
		}
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Source source;
		
		Map cache = {};
		Memory memory = {};
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Allocator allocator = {};
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RUNTIME_CACHED_SOURCE_HPP
