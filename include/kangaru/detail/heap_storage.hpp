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
			if (this == &rhs) return *this;
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
	
	namespace detail::heap_storage_private {
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
			
			template<unqualified_object T, typename... Args> requires(constructor_callable<T, Args...>) KANGARU5_UNSAFE
			constexpr auto construct(Allocator& allocator, Args&&... args) -> T* {
				auto ptr = std::unique_ptr<T, deallocate_uninitialized>{
					allocator.template allocate_object<T>(),
					deallocate_uninitialized{std::addressof(allocator)}
				};
				
				std::construct_at(ptr.get(), in_place_construct{[&] {
					return constructor<T>(KANGARU5_FWD(args)...);
				}});
				
				return ptr.release();
			}
			
		private:
			struct deallocate_uninitialized {
				Allocator* allocator;
				
				template<typename ObjectType>
				constexpr auto operator()(ObjectType* ptr) const {
					allocator->template deallocate_object<ObjectType>(ptr);
				}
			};
		};
	} // namespace detail::heap_storage_private
	
	KANGARU5_EXPORT template<heap_storage_container Container, object_allocator Allocator = default_allocator>
	struct basic_heap_storage : private detail::heap_storage_private::basic_heap_storage_base<Allocator> {
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
			if (this == &rhs) return *this;
			std::ranges::swap(allocator, rhs.allocator);
			std::ranges::swap(container, rhs.container);
			return *this;
		}
		
		KANGARU5_CONSTEXPR_VOIDSTAR ~basic_heap_storage() {
			KANGARU5_UNSAFE_BLOCK {
				for (auto it = container.rbegin(); it < container.rend(); ++it) {
					it->deleter(it->ptr, std::addressof(allocator));
				}
			}
		}
		
		template<unqualified_object T, typename... Args> requires(constructor_callable<T, Args&&...>)
		constexpr auto emplace(Args&&... args) {
			KANGARU5_UNSAFE_BLOCK {
				auto ptr = std::unique_ptr<T, destroy>{
					basic_heap_storage::template construct<T>(allocator, KANGARU5_FWD(args)...),
					destroy{std::addressof(allocator)},
				};
				
				container.push_back(runtime_dynamic_storage{
					ptr.get(),
					basic_heap_storage::template destroyer<T>(),
				});
				
				return ptr.release();
			}
		}
		
	private:
		struct destroy {
			Allocator* allocator;
			
			template<typename ObjectType>
			constexpr auto operator()(ObjectType* ptr) const noexcept -> void {
				basic_heap_storage::template destroyer<ObjectType>()(ptr, allocator);
			}
		};
		Container container;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Allocator allocator;
	};
	
	KANGARU5_EXPORT using default_heap_storage = basic_heap_storage<std::vector<runtime_dynamic_storage>, default_allocator>;
	
	KANGARU5_EXPORT template<typename T>
	concept heap_storage =
		    std::move_constructible<T>
		and requires(T storage, int value) {
			{ storage.template emplace<int>(value) } -> std::same_as<int*>;
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
		template<allows_construction_of<Source> S>
		explicit constexpr with_heap_storage(S&& source) noexcept
			requires(
				std::default_initializable<Storage>
			) : source(KANGARU5_FWD(source)) {}
		
		template<allows_construction_of<Source> S>
		constexpr with_heap_storage(S&& source, Storage storage) noexcept :
			source(KANGARU5_FWD(source)), storage{std::move(storage)} {}
		
		Source source;
		
		template<unqualified_object T, typename... Args> requires(constructor_callable<T, Args...>)
		constexpr auto emplace(Args&&... args) -> T* {
			return KANGARU5_NO_ADL(maybe_unwrap)(storage).template emplace<T>(KANGARU5_FWD(args)...);
		}
		
		template<forwarded<with_heap_storage> Original, forwarded_source NewSource>
			requires(not std::is_const_v<std::remove_reference_t<Original>>)
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_heap_storage<deduced_source_type<NewSource>, ref_result_t<detail::forward_like_t<Original, Storage>&>>
		{
			return with_heap_storage<deduced_source_type<NewSource>, ref_result_t<detail::forward_like_t<Original, Storage>&>>{
				KANGARU5_FWD(new_source),
				KANGARU5_NO_ADL(ref)(original.storage),
			};
		}
		
		template<pointer T, forwarded<with_heap_storage> Self> requires wrapping_source_of<Self, std::remove_pointer_t<T>>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return KANGARU5_NO_ADL(maybe_unwrap)(source.storage).template emplace<std::remove_pointer_t<T>>(
				in_place_construct{[&source] {
					return kangaru::provide<std::remove_pointer_t<T>>(KANGARU5_FWD(source).source);
				}}
			);
		}
		
	private:
		Storage storage = {};
	};
	
	template<typename Source, typename Storage>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_heap_storage(Source&&, Storage const&) -> with_heap_storage<deduced_source_type<Source>, Storage>;
	
	template<typename Source>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_heap_storage(Source&&) -> with_heap_storage<deduced_source_type<Source>>;
	
	KANGARU5_EXPORT template<forwarded_source Source, forwarded_heap_storage Storage>
	inline constexpr auto make_source_with_heap_storage(Source&& source, Storage&& storage) {
		return with_heap_storage<deduced_source_type<Source>, std::decay_t<Storage>>{
			KANGARU5_FWD(source), KANGARU5_FWD(storage)
		};
	}
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_heap_storage(Source&& source) {
		return with_heap_storage<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	static_assert(heap_storage<with_heap_storage<none_source>>);
	static_assert(heap_storage<with_heap_storage<none_source, with_heap_storage<none_source>>>);
	static_assert(heap_storage<with_heap_storage<none_source, source_reference_wrapper<with_heap_storage<none_source>>>>);
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_HEAP_STORAGE_HPP
