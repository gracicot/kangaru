#ifndef KANGARU5_DETAIL_RUNTIME_CACHED_SOURCE_HPP
#define KANGARU5_DETAIL_RUNTIME_CACHED_SOURCE_HPP

#include "constructor.hpp"
#include "tag.hpp"
#include "utility.hpp"
#include "murmur.hpp"
#include "source.hpp"
#include "ctti.hpp"
#include "concepts.hpp"

#include <cstddef>
#include <type_traits>
#include <concepts>
#include <memory>
#include <unordered_map>
#include <limits>
#include <iterator>

#include "define.hpp"

namespace kangaru {
	template<typename T>
	concept cached_source_map = requires(T map, detail::ctti::type_id_for_result<T> id, void* ptr) {
		{ map.begin() } -> std::forward_iterator;
		{ map.end() } -> std::forward_iterator;
		{ *map.begin() } -> std::same_as<std::pair<std::size_t const, void*>&>;
		{ std::as_const(map).begin() } -> std::forward_iterator;
		{ std::as_const(map).end() } -> std::forward_iterator;
		{ *std::as_const(map).begin() } -> std::same_as<std::pair<std::size_t const, void*> const&>;
		{ map.insert(std::pair{id, ptr}) };
		{ map.insert(std::pair{id, &map}) };
		{ map.insert(map.begin(), map.end()) };
		{ map.insert(std::initializer_list<std::pair<std::size_t const, void*>>{}) };
		{ map.insert_or_assign(std::size_t{}, ptr) };
		{ map.insert_or_assign(id, &map) };
		{ map.find(id) } -> std::same_as<decltype(map.begin())>;
		{ std::as_const(map).find(id) } -> std::same_as<decltype(std::as_const(map).begin())>;
		{ std::as_const(map).contains(id) } -> std::same_as<bool>;
		{ map.erase(id) };
		{ map.clear() };
		{ std::as_const(map).empty() } -> std::same_as<bool>;
		{ std::as_const(map).size() } -> std::same_as<std::size_t>;
	};
	
	static_assert(cached_source_map<std::unordered_map<std::size_t, void*>>);
	
	template<typename T>
	concept cached_source_allocator = requires(T allocator, int* ptr) {
		{ allocator.template allocate_object<int>() } -> std::same_as<int*>;
		{ allocator.template deallocate_object<int>(ptr) } -> std::same_as<void>;
	};
	
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
	
	/**
	 * Simple struct that contains both a pointer to something and a deleter for that pointer.
	 * 
	 * I would have implemented the struct with RAII, but couldn't find a way to pass the allocator.
	 */
	struct runtime_dynamic_storage {
		using dynamic_deleter = auto(void* ptr, void* resource) noexcept -> void;
		
		void* ptr;
		dynamic_deleter* deleter;
	};
	
	template<cached_source_map Map>
	struct polymorphic_map {
		using key_type = typename Map::key_type;
		using mapped_type = typename Map::mapped_type;
		using value_type = typename Map::value_type;
		using size_type = typename Map::size_type;
		using difference_type = typename Map::difference_type;
		using hasher = typename Map::hasher;
		using key_equal = typename Map::key_equal;
		using allocator_type = typename Map::allocator_type;
		using reference = typename Map::reference;
		using const_reference = typename Map::const_reference;
		using pointer = typename Map::pointer;
		using const_pointer = typename Map::const_pointer;
		using iterator = typename Map::iterator;
		using const_iterator = typename Map::const_iterator;
		
		constexpr auto begin() noexcept -> iterator {
			return map.begin();
		}
		
		constexpr auto begin() const noexcept -> const_iterator {
			return map.begin();
		}
		
		constexpr auto end() noexcept -> iterator {
			return map.end();
		}
		
		constexpr auto end() const noexcept -> const_iterator {
			return map.end();
		}
		
		[[nodiscard]]
		constexpr auto empty() const noexcept -> bool {
			return map.empty();
		}
		
		constexpr auto size() const noexcept -> size_type {
			return map.size();
		}
		
		constexpr auto clear() noexcept -> void {
			map.clear();
		}
		
		constexpr auto insert(value_type const& value) -> std::pair<iterator, bool> {
			return map.insert(value);
		}
		
		constexpr auto insert(value_type&& value) -> std::pair<iterator, bool> {
			return map.insert(std::move(value));
		}
		
		constexpr auto insert(allows_construction_of<value_type> auto&& value) -> std::pair<iterator, bool> {
			if constexpr (requires{ { detail::utility::decay_copy(std::get<1>(value)) } -> kangaru::pointer; }) {
				insert_overrides(std::get<1>(value));
			}
			return map.insert(KANGARU5_FWD(value));
		}
		
		template<different_from<void> T>
		constexpr auto insert(std::pair<detail::ctti::type_id_for_result<T>, T*> const& value) -> std::pair<iterator, bool> {
			insert_overrides(value.second);
			return map.insert(value);
		}
		
		template<different_from<void> T>
		constexpr auto insert(std::pair<detail::ctti::type_id_for_result<T>, T*>&& value) -> std::pair<iterator, bool> {
			insert_overrides(value.second);
			return map.insert(value);
		}
		
		constexpr auto insert(const_iterator hint, value_type const& value) -> std::pair<iterator, bool> {
			return map.insert(hint, value);
		}
		
		constexpr auto insert(const_iterator hint, value_type&& value) -> std::pair<iterator, bool> {
			return map.insert(hint, std::move(value));
		}
		
		constexpr auto insert(const_iterator hint, allows_construction_of<value_type> auto&& value) -> std::pair<iterator, bool> {
			if constexpr (requires{ { detail::utility::decay_copy(std::get<1>(value)) } -> kangaru::pointer; }) {
				insert_overrides(std::get<1>(value));
			}
			
			return map.insert(hint, KANGARU5_FWD(value));
		}
		
		template<std::input_iterator Iterator> requires requires(Iterator it) { { *it } -> std::convertible_to<const_reference>; }
		constexpr auto insert(Iterator begin, Iterator end) -> void {
			map.insert(begin, end);
		}
		
		constexpr auto insert(std::initializer_list<value_type> values) -> void {
			map.insert(values);
		}
		
		constexpr auto insert_or_assign(key_type const& k, assign_into<mapped_type&> auto&& obj) {
			map.insert_or_assign(k, KANGARU5_FWD(obj));
		}
		
		constexpr auto insert_or_assign(key_type&& k, assign_into<mapped_type&> auto&& obj) {
			map.insert_or_assign(std::move(k), KANGARU5_FWD(obj));
		}
		
		template<different_from<void> T>
		constexpr auto insert_or_assign(detail::ctti::type_id_for_result<T> const& k, assign_into<value_type> auto&& obj) {
			insert_or_assign_overrides(static_cast<T*>(std::get<1>(KANGARU5_FWD(obj))));
			map.insert_or_assign(k, KANGARU5_FWD(obj));
		}
		
		template<different_from<void> T>
		constexpr auto insert_or_assign(detail::ctti::type_id_for_result<T>&& k, assign_into<value_type> auto&& obj) {
			insert_or_assign_overrides(static_cast<T*>(std::get<1>(KANGARU5_FWD(obj))));
			map.insert_or_assign(std::move(k), KANGARU5_FWD(obj));
		}
		
		constexpr auto insert_or_assign(const_iterator hint, key_type const& k, assign_into<value_type> auto&& obj) {
			map.insert_or_assign(hint, k, KANGARU5_FWD(obj));
		}
		
		constexpr auto insert_or_assign(const_iterator hint, key_type&& k, assign_into<value_type> auto&& obj) {
			map.insert_or_assign(hint, std::move(k), KANGARU5_FWD(obj));
		}
		
		template<different_from<void> T>
		constexpr auto insert_or_assign(const_iterator hint, detail::ctti::type_id_for_result<T> const& k, assign_into<value_type> auto&& obj) {
			insert_or_assign_overrides(static_cast<T*>(std::get<1>(KANGARU5_FWD(obj))));
			map.insert_or_assign(hint, k, KANGARU5_FWD(obj));
		}
		
		template<different_from<void> T>
		constexpr auto insert_or_assign(const_iterator hint, detail::ctti::type_id_for_result<T>&& k, assign_into<value_type> auto&& obj) {
			insert_or_assign_overrides(static_cast<T*>(std::get<1>(KANGARU5_FWD(obj))));
			map.insert_or_assign(hint, std::move(k), KANGARU5_FWD(obj));
		}
		
		constexpr auto erase(iterator pos) -> iterator {
			return map.erase(pos);
		}
		
		constexpr auto erase(const_iterator pos) -> iterator {
			return map.erase(pos);
		}
		
		template<typename T>
		constexpr auto erase(detail::ctti::type_id_for_result<T> id) -> size_type {
			auto const overrides = remove_overrides<T>();
			return overrides + map.erase(id);
		}
		
		constexpr auto swap(polymorphic_map& other) noexcept(noexcept(map.swap(other.map))) -> void{
			map.swap(other.map);
		}
		
		friend constexpr auto swap(polymorphic_map& left, polymorphic_map& right) {
			left.map.swap(right.map);
		}
		
		constexpr auto find(key_type const& key) -> iterator {
			return map.find(key);
		}
		
		constexpr auto find(key_type const& key) const -> const_iterator {
			return map.find(key);
		}
		
		template<typename K> requires(
			    not std::same_as<key_type, K>
			and requires{
				typename hasher::is_transparent;
				typename key_equal::is_transparent;
			}
		)
		constexpr auto find(K const& key) -> iterator {
			return map.find(key);
		}
		
		template<typename K> requires(
			    not std::same_as<key_type, K>
			and requires{
				typename hasher::is_transparent;
				typename key_equal::is_transparent;
			}
		)
		constexpr auto find(K const& key) const -> const_iterator {
			return map.find(key);
		}
		
		constexpr auto contains(key_type const& key) const -> bool {
			return map.contains(key);
		}
		
		template<typename K> requires(
			    not std::same_as<key_type, K>
			and requires{
				typename hasher::is_transparent;
				typename key_equal::is_transparent;
			}
		)
		constexpr auto contains(K const& key) const -> bool {
			return map.contains(key);
		}
		
	private:
		template<different_from<void> T>
		constexpr auto insert_overrides(T* ptr) -> void {
			using overrides = overrides_types_in_cache_t<T>;
			std::apply([this, ptr](auto... s) {
				auto const for_each = [this](auto i, T* ptr) {
					using override = std::tuple_element_t<i, overrides>;
					constexpr auto id = detail::ctti::type_id_for<override>();
					map.insert(std::pair{id, static_cast<override*>(ptr)});
				};
				(for_each(s, ptr), ...);
			}, detail::utility::sequence_tuple_for_tuple<overrides>{});
		}
		
		template<different_from<void> T>
		constexpr auto insert_or_assign_overrides(T* ptr) -> void {
			using overrides = overrides_types_in_cache_t<T>;
			std::apply([this, ptr](auto... s) {
				auto const for_each = [this](auto i, T* ptr) {
					using override = std::tuple_element_t<i, overrides>;
					constexpr auto id = detail::ctti::type_id_for<override>();
					map.insert_or_assign(std::pair{id, static_cast<override*>(ptr)});
				};
				(for_each(s, ptr), ...);
			}, detail::utility::sequence_tuple_for_tuple<overrides>{});
		}
		
		template<different_from<void> T>
		constexpr auto remove_overrides() -> size_type {
			auto n = size_type{};
			using overrides = overrides_types_in_cache_t<T>;
			std::apply([this, &n](auto... s) {
				auto const for_each = [this, &n](auto i) {
					using override = std::tuple_element_t<i, overrides>;
					constexpr auto id = detail::ctti::type_id_for<override>();
					n += map.erase(id);
				};
				(for_each(s), ...);
			}, detail::utility::sequence_tuple_for_tuple<overrides>{});
			return n;
		}
		
		Map map;
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
} // namespace kangaru

namespace kangaru::sources {
	template<
		source Source,
		cached_source_map Map = std::unordered_map<std::size_t, void*>,
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
		
		runtime_source(runtime_source const&) = delete;
		auto operator=(runtime_source const&) -> runtime_source& = delete;
		
		constexpr runtime_source(runtime_source&& other) noexcept :
			source{std::move(other.source)},
			cache{std::move(other.cache)},
			memory{std::move(other.memory)},
			allocator{std::move(other.allocator)} {}
		
		constexpr auto operator=(runtime_source&& rhs) noexcept -> runtime_source& {
			using std::swap;
			swap(source, rhs.source);
			swap(cache, rhs.cache);
			swap(memory, rhs.memory);
			swap(allocator, rhs.allocator);
			return *this;
		}
		
		KANGARU5_CONSTEXPR_VOIDSTAR ~runtime_source() {
			for (auto it = memory.rbegin(); it < memory.rend(); ++it) {
				it->deleter(it->ptr, &allocator);
			}
		}
		
	private:
		template<object T, forwarded<runtime_source> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag<T&>, Self&& source) -> T& {
			constexpr auto id = detail::ctti::type_id_for<T>();
			auto const it = source.cache.find(id);
			
			if (it == source.cache.end()) {
				auto const ptr = source.allocator.template allocate_object<T>();
				
				std::construct_at(
					ptr,
					expr_to_rvo{
						[&source] {
							return kangaru::provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
						}
					}
				);
				
				source.memory.push_back(runtime_dynamic_storage{
					.ptr = ptr,
					.deleter = [](void* ptr, void* allocator) noexcept KANGARU5_CONSTEXPR_VOIDSTAR {
						auto const object_ptr = static_cast<T*>(ptr);
						std::destroy_at(object_ptr);
						static_cast<Allocator*>(allocator)->template deallocate_object<T>(object_ptr);
					}
				});
				
				source.cache.insert(std::pair{id, ptr});
				return *static_cast<T*>(ptr);
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
				source.cache.insert(std::pair{id, std::addressof(ref)});
				return *static_cast<T*>(ref);
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
