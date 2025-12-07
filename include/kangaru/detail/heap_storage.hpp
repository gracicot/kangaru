#ifndef KANGARU5_DETAIL_HEAP_STORAGE_HPP
#define KANGARU5_DETAIL_HEAP_STORAGE_HPP

#include "kangaru/detail/type_traits.hpp"
#include "source_types.hpp"
#include "source.hpp"
#include "allocator.hpp"
#include "source_rebind.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#include <vector>
#include <utility>
#endif

#include "define.hpp"

namespace kangaru {
	/**
	 * Simple struct that contains both a pointer to something and a deleter for that pointer.
	 * 
	 * I would have implemented the struct with RAII, but couldn't find a way to pass the allocator.
	 */
	KANGARU5_EXPORT struct runtime_dynamic_storage {
		using dynamic_deleter = auto(void* ptr, void* resource) noexcept -> void;
		
		KANGARU5_UNSAFE constexpr runtime_dynamic_storage(void* ptr, dynamic_deleter* deleter) noexcept : ptr{ptr}, deleter{deleter} {}
		
		runtime_dynamic_storage(runtime_dynamic_storage const&) = delete;
		auto operator=(runtime_dynamic_storage const&) -> runtime_dynamic_storage& = delete;
		
		constexpr runtime_dynamic_storage(runtime_dynamic_storage&& other) noexcept :
			ptr{std::exchange(other.ptr, nullptr)}, deleter{std::exchange(other.deleter, nullptr)} {}
		
		constexpr auto operator=(runtime_dynamic_storage&& rhs) noexcept -> runtime_dynamic_storage& {
			std::ranges::swap(ptr, rhs.ptr);
			std::ranges::swap(deleter, rhs.deleter);
			return *this;
		}
		
		void* ptr;
		dynamic_deleter KANGARU5_UNSAFE* deleter;
	};
	
	KANGARU5_EXPORT template<typename T>
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
			{ container.insert(container.begin(), std::make_move_iterator(container.begin()), std::make_move_iterator(container.end())) };
			{ container.erase(container.begin()) } -> std::same_as<typename T::iterator>;
			{ container.erase(std::as_const(container).begin()) } -> std::same_as<typename T::iterator>;
		};
	
	namespace detail::heap_storage {
		template<object_allocator Allocator>
		struct basic_heap_storage_base {
			template<typename ObjectType>
			static constexpr auto destroyer() {
				return [](void* ptr, void* allocator) KANGARU5_UNSAFE KANGARU5_CONSTEXPR_VOIDSTAR noexcept {
					auto const object_ptr = static_cast<ObjectType*>(ptr);
					std::destroy_at(object_ptr);
					static_cast<Allocator*>(allocator)->template deallocate_object<ObjectType>(object_ptr);
				};
			}
			
			template<std::copy_constructible F> KANGARU5_UNSAFE
			constexpr auto construct(Allocator& allocator, F function) -> type_traits::call_result_t<F>* {
				using object_type = type_traits::call_result_t<F>;
				
				auto const ptr = allocator.template allocate_object<object_type>();
				
				auto provide_as_function_source = function_source{function};
				
				std::construct_at(
					ptr,
					strict_deducer<decltype(provide_as_function_source)&>{provide_as_function_source}
				);
				
				return ptr;
			}
		};
	} // namespace detail::heap_storage
	
	KANGARU5_EXPORT template<heap_storage_container Container, object_allocator Allocator = default_allocator>
	struct basic_heap_storage : private detail::heap_storage::basic_heap_storage_base<Allocator> {
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
			std::ranges::swap(allocator, rhs.allocator);
			std::ranges::swap(container, rhs.container);
			return *this;
		}
		
		KANGARU5_CONSTEXPR_VOIDSTAR ~basic_heap_storage() {
			KANGARU5_UNSAFE_BLOCK {
				for (auto it = container.rbegin(); it < container.rend(); ++it) {
					it->deleter(it->ptr, &allocator);
				}
			}
		}
		
		template<std::copy_constructible F>
		constexpr auto emplace_from(F function) -> detail::type_traits::call_result_t<F>* {
			KANGARU5_UNSAFE_BLOCK {
				auto const ptr = basic_heap_storage::construct(allocator, std::move(function));
				
				using object_type = detail::type_traits::call_result_t<F>;
				container.push_back(runtime_dynamic_storage{
					ptr,
					basic_heap_storage::template destroyer<object_type>(),
				});
				
				return ptr;
			}
		}
		
	private:
		Container container;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Allocator allocator;
	};
	
	KANGARU5_EXPORT using default_heap_storage = basic_heap_storage<std::vector<runtime_dynamic_storage>, default_allocator>;
	
	KANGARU5_EXPORT template<typename T>
	concept heap_storage =
		    std::move_constructible<T>
		and requires(T storage, detail::utility::function_pointer_t<auto() -> int> function) {
			{ storage.emplace_from(function) } -> std::same_as<int*>;
		};
	
	KANGARU5_EXPORT template<typename T>
	concept dereferenceable_heap_storage = heap_storage<T> or requires {
		requires heap_storage<source_reference_wrapped_type<T>>;
	};
	
	KANGARU5_EXPORT template<typename T>
	concept forwarded_heap_storage = dereferenceable_heap_storage<std::remove_cvref_t<T>>;
	
	static_assert(heap_storage_container<std::vector<runtime_dynamic_storage>>);
	
	KANGARU5_EXPORT template<source Source, dereferenceable_heap_storage Storage = default_heap_storage>
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
		constexpr auto emplace_from(F function) -> detail::type_traits::call_result_t<F>* {
			return KANGARU5_NO_ADL(maybe_unwrap)(storage).emplace_from(function);
		}
		
		template<forwarded<with_heap_storage> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_leaf) noexcept -> with_heap_storage<wrapped_source_rebind_result_t<Original, NewSource>, ref_result_t<detail::utility::forward_like_t<Original, Storage>&>> {
			return with_heap_storage<wrapped_source_rebind_result_t<Original, NewSource>, ref_result_t<detail::utility::forward_like_t<Original, Storage>&>>{
				kangaru::rebind(KANGARU5_FWD(original).source, new_leaf),
				KANGARU5_NO_ADL(ref)(original.storage)
			};
		}
		
		template<pointer T> requires source_of<source_type, std::remove_pointer_t<T>>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<with_heap_storage> auto&& source) -> T {
			return KANGARU5_NO_ADL(maybe_unwrap)(source.storage).emplace_from(
				[&source] {
					return kangaru::provide<std::remove_pointer_t<T>>(KANGARU5_FWD(source).source);
				}
			);
		}
		
	private:
		Storage storage = {};
	};
	
	KANGARU5_EXPORT template<forwarded_source Source, forwarded_heap_storage Storage>
	inline constexpr auto make_source_with_heap_storage(Source&& source, Storage&& storage) {
		return with_heap_storage<std::decay_t<Source>, std::decay_t<Storage>>{
			KANGARU5_FWD(source), KANGARU5_FWD(storage)
		};
	}
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_heap_storage(Source&& source) {
		return with_heap_storage<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	static_assert(heap_storage<with_heap_storage<none_source>>);
	static_assert(heap_storage<with_heap_storage<none_source, with_heap_storage<none_source>>>);
	static_assert(heap_storage<with_heap_storage<none_source, source_reference_wrapper<with_heap_storage<none_source>>>>);
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_HEAP_STORAGE_HPP
