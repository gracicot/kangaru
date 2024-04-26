#ifndef KANGARU5_DETAIL_HEAP_STORAGE_HPP
#define KANGARU5_DETAIL_HEAP_STORAGE_HPP

#include "source_types.hpp"
#include "source.hpp"
#include "allocator.hpp"

#include <concepts>
#include <vector>
#include <utility>

#include "define.hpp"

namespace kangaru {
	/**
	 * Simple struct that contains both a pointer to something and a deleter for that pointer.
	 * 
	 * I would have implemented the struct with RAII, but couldn't find a way to pass the allocator.
	 */
	struct runtime_dynamic_storage {
		using dynamic_deleter = auto(void* ptr, void* resource) noexcept -> void;
		
		constexpr runtime_dynamic_storage(void* ptr, dynamic_deleter* deleter) noexcept : ptr{ptr}, deleter{deleter} {}
		
		runtime_dynamic_storage(runtime_dynamic_storage const&) = delete;
		auto operator=(runtime_dynamic_storage const&) -> runtime_dynamic_storage& = delete;
		
		constexpr runtime_dynamic_storage(runtime_dynamic_storage&& other) noexcept :
			ptr{std::exchange(other.ptr, nullptr)}, deleter{std::exchange(other.deleter, nullptr)} {}
		
		constexpr auto operator=(runtime_dynamic_storage&& rhs) noexcept -> runtime_dynamic_storage& {
			using std::swap;
			swap(ptr, rhs.ptr);
			swap(deleter, rhs.deleter);
			return *this;
		}
		
		void* ptr;
		dynamic_deleter* deleter;
	};
	
	template<typename T>
	concept heap_storage_container =
		    std::move_constructible<T>
		and std::same_as<runtime_dynamic_storage, typename T::value_type>
		and requires(T container, runtime_dynamic_storage s) {
			{ container.push_back(s) };
			{ container.rbegin() } -> std::same_as<typename T::reverse_iterator>;
			{ container.rend() } -> std::same_as<typename T::reverse_iterator>;
			{ container.begin() } -> std::same_as<typename T::iterator>;
			{ container.end() } -> std::same_as<typename T::iterator>;
			{ std::as_const(container).rbegin() } -> std::same_as<typename T::const_reverse_iterator>;
			{ std::as_const(container).rend() } -> std::same_as<typename T::const_reverse_iterator>;
			{ std::as_const(container).begin() } -> std::same_as<typename T::const_iterator>;
			{ std::as_const(container).end() } -> std::same_as<typename T::const_iterator>;
			{ std::as_const(container).empty() } -> std::same_as<bool>;
			{ container.clear() } -> std::same_as<void>;
			{ container.insert(container.begin(), container.begin(), container.end()) };
			{ container.insert(container.begin(), std::as_const(container).begin(), std::as_const(container).end()) };
			{ container.erase(container.begin()) } -> std::same_as<typename T::iterator>;
			{ container.erase(std::as_const(container).begin()) } -> std::same_as<typename T::iterator>;
		};
	
	template<heap_storage_container Container, object_allocator Allocator = default_allocator>
	struct basic_heap_storage {
		constexpr basic_heap_storage() = default;
		
		constexpr basic_heap_storage(Container container) noexcept
		requires (std::default_initializable<Allocator>) :
			container{std::move(container)} {}
		
		constexpr basic_heap_storage(Container container, Allocator allocator) noexcept :
			container{std::move(container)},
			allocator{std::move(allocator)} {}
		
		basic_heap_storage(basic_heap_storage const&) = delete;
		auto operator=(basic_heap_storage const&) -> basic_heap_storage& = delete;
		
		constexpr basic_heap_storage(basic_heap_storage&& other) noexcept :
			allocator{std::move(other.allocator)}, container{std::move(other.container)} {}
		
		constexpr auto operator=(basic_heap_storage&& rhs) noexcept -> basic_heap_storage& {
			using std::swap;
			swap(allocator, rhs.allocator);
			swap(container, rhs.container);
			return *this;
		}
		
		KANGARU5_CONSTEXPR_VOIDSTAR ~basic_heap_storage() {
			for (auto it = container.rbegin(); it < container.rend(); ++it) {
				it->deleter(it->ptr, &allocator);
			}
		}
		
		template<std::copy_constructible F>
		constexpr auto emplace_from(F function) -> std::invoke_result_t<F>* {
			using object_type = std::invoke_result_t<F>;
			
			auto const ptr = allocator.template allocate_object<object_type>();
			
			auto provide_as_function_source = function_source{function};
			
			std::construct_at(
				ptr,
				strict_deducer<decltype(provide_as_function_source)&>{provide_as_function_source}
			);
			
			container.push_back(runtime_dynamic_storage{
				ptr,
				[](void* ptr, void* allocator) KANGARU5_CONSTEXPR_VOIDSTAR noexcept {
					auto const object_ptr = static_cast<object_type*>(ptr);
					std::destroy_at(object_ptr);
					static_cast<Allocator*>(allocator)->template deallocate_object<object_type>(object_ptr);
				}
			});
			
			return ptr;
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Allocator allocator;
		
		Container container;
	};
	
	using default_heap_storage = basic_heap_storage<std::vector<runtime_dynamic_storage>, default_allocator>;
	
	template<typename T>
	concept heap_storage =
		    std::move_constructible<T>
		and requires(T storage, int(*function)()) {
			{ storage.emplace_from(function) } -> std::same_as<int*>;
		};
	
	static_assert(heap_storage_container<std::vector<runtime_dynamic_storage>>);
	
	template<source Source, heap_storage Storage = default_heap_storage>
	struct with_heap_storage {
		using source_type = Source;
		
		explicit constexpr with_heap_storage(source_type source) noexcept
			requires(
				std::default_initializable<Storage>
			) : source{std::move(source)} {}
		
		constexpr with_heap_storage(source_type source, Storage storage) noexcept :
			source{std::move(source)}, storage{std::move(storage)} {}
		
		source_type source;
		
		template<std::copy_constructible F>
		constexpr auto emplace_from(F function) -> std::invoke_result_t<F>* {
			return storage.emplace_from(function);
		}
		
	private:
		template<object T> requires source_of<source_type, T>
		friend constexpr auto provide(provide_tag<T*>, forwarded<with_heap_storage> auto&& source) -> T* {
			return source.storage.emplace_from(
				[&source] {
					return provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
				}
			);
		}
		
		Storage storage = {};
	};
	
	static_assert(heap_storage<with_heap_storage<noop_source>>);
	static_assert(heap_storage<with_heap_storage<noop_source, with_heap_storage<noop_source>>>);
	
	template<source Source, heap_storage InnerStorage = default_heap_storage>
	struct with_heap_storage_reference_wrapper {
		using source_type = source_reference_wrapper_for_t<Source>;
		
		explicit constexpr with_heap_storage_reference_wrapper(with_heap_storage<Source, InnerStorage>& source) noexcept :
			source{ref(source.source)}, storage{std::addressof(source)} {}
		
		template<std::copy_constructible F>
		constexpr auto emplace_from(F function) -> std::invoke_result_t<F>* {
			return storage->emplace_from(function);
		}
		
		[[nodiscard]]
		constexpr auto unwrap() const noexcept -> with_heap_storage<Source>& {
			return *storage;
		}
		
		source_type source;
		
	private:
		template<object T> requires source_of<with_heap_storage<Source, InnerStorage>, T*>
		friend constexpr auto provide(provide_tag<T*> tag, forwarded<with_heap_storage_reference_wrapper> auto&& source) -> T* {
			return provide(tag, *source.storage);
		}
		
		with_heap_storage<Source, InnerStorage>* storage;
	};
	
	template<source Source, heap_storage Storage>
	struct source_reference_wrapper_for<with_heap_storage<Source, Storage>> {
		using type = with_heap_storage_reference_wrapper<Source, Storage>;
	};
	
	static_assert(heap_storage<with_heap_storage_reference_wrapper<with_heap_storage<noop_source>>>);
	static_assert(heap_storage<with_heap_storage<noop_source, with_heap_storage_reference_wrapper<with_heap_storage<noop_source>>>>);
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_HEAP_STORAGE_HPP
