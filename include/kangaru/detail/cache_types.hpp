#ifndef KANGARU5_DETAIL_CACHE_TYPES_HPP
#define KANGARU5_DETAIL_CACHE_TYPES_HPP

#include "attributes.hpp"
#include "utility.hpp"
#include "source.hpp"
#include "ctti.hpp"
#include "concepts.hpp"
#include "source_reference_wrapper.hpp"

#ifndef KANGARU5_MODULES
#include <cstddef>
#include <concepts>
#include <iterator>
#include <utility>
#include <unordered_map>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<typename T>
	concept cache_map = requires(T map) {
		{ map.begin() } -> std::forward_iterator;
		{ map.end() } -> std::forward_iterator;
		{ std::as_const(map).begin() } -> std::forward_iterator;
		{ std::as_const(map).end() } -> std::forward_iterator;
		{ map.insert(map.begin(), map.end()) };
		{ map.clear() };
		{ std::as_const(map).empty() } -> std::same_as<bool>;
		{ std::as_const(map).size() } -> std::same_as<std::size_t>;
		{ map.swap(map) };
		
		typename T::key_type;
		typename T::value_type;
		typename T::mapped_type;
		typename T::iterator;
		typename T::const_iterator;
	};
	
	KANGARU5_EXPORT template<typename T>
	concept dereferenceable_cache_map = cache_map<T> or requires {
		requires cache_map<source_reference_wrapped_type<T>>;
	};
	
	KANGARU5_EXPORT template<typename T>
	concept forwarded_dereferenceable_cache_map = cache_map<std::remove_cvref_t<T>>;
	
	static_assert(cache_map<std::unordered_map<type_id, void*>>);
	
	KANGARU5_EXPORT template<dereferenceable_cache_map Map>
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
		
		template<injectable T, allows_construction_of<T> U>
		constexpr auto insert(std::pair<static_type_id<T>, U> const& value) -> std::pair<iterator, bool> {
			insert_overrides(value.second);
			// TODO: Should we cast to std::pair<key_type, T> first?
			return map.insert(static_cast<value_type>(value));
		}
		
		template<injectable T, allows_construction_of<T> U>
		constexpr auto insert(std::pair<static_type_id<T>, U>&& value) -> std::pair<iterator, bool> {
			insert_overrides(value.second);
			// TODO: Should we cast to std::pair<key_type, T> first?
			return map.insert(static_cast<value_type>(value));
		}
		
		constexpr auto insert(const_iterator hint, value_type const& value) -> std::pair<iterator, bool> {
			return map.insert(hint, value);
		}
		
		constexpr auto insert(const_iterator hint, value_type&& value) -> std::pair<iterator, bool> {
			return map.insert(hint, std::move(value));
		}
		
		template<std::input_iterator Iterator> requires requires(Iterator it) { { *it } -> std::convertible_to<const_reference>; }
		constexpr auto insert(Iterator begin, Iterator end) -> void {
			map.insert(begin, end);
		}
		
		template<injectable T, allows_construction_of<T> U>
		constexpr auto insert_or_assign(static_type_id<T> const& k, U&& obj) {
			insert_or_assign_overrides(obj);
			return map.insert_or_assign(k, static_cast<T>(KANGARU5_FWD(obj)));
		}
		
		template<injectable T, allows_construction_of<T> U>
		constexpr auto insert_or_assign(const_iterator hint, static_type_id<T> const& k, U&& obj) {
			insert_or_assign_overrides(obj);
			return map.insert_or_assign(hint, k, static_cast<T>(KANGARU5_FWD(obj)));
		}
		
		constexpr auto erase(iterator pos) -> iterator {
			return map.erase(pos);
		}
		
		constexpr auto erase(const_iterator pos) -> iterator {
			return map.erase(pos);
		}
		
		template<injectable T>
		constexpr auto erase(static_type_id<T> id) -> size_type {
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
		
		template<typename K> requires(requires(Map& map, K const& key) { { map.find(key) } -> std::same_as<iterator>; })
		constexpr auto find(K const& key) -> iterator {
			return map.find(key);
		}
		
		template<typename K> requires(requires(Map const& map, K const& key) { { map.find(key) } -> std::same_as<const_iterator>; })
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
		constexpr auto insert_overrides(T& value) -> void {
			using overrides = overrides_types_in_cache_t<T>;
			std::apply([this, &value](auto... s) {
				[[maybe_unused]]
				auto const for_each = [this](auto i, T& value) {
					using override = std::tuple_element_t<i, overrides>;
					constexpr auto id = KANGARU5_NO_ADL(type_id_for<override>)();
					map.insert(std::pair{id, static_cast<override>(value)});
				};
				(for_each(s, value), ...);
			}, detail::sequence_tuple_for_tuple<overrides>{});
		}
		
		template<different_from<void> T>
		constexpr auto insert_or_assign_overrides(T& value) -> void {
			using overrides = overrides_types_in_cache_t<T>;
			std::apply([this, &value](auto... s) {
				auto const for_each = [this](auto i, T& value) {
					using override = std::tuple_element_t<i, overrides>;
					constexpr auto id = KANGARU5_NO_ADL(type_id_for<override>)();
					map.insert_or_assign(id, static_cast<override>(value));
				};
				(for_each(s, value), ...);
			}, detail::sequence_tuple_for_tuple<overrides>{});
		}
		
		template<different_from<void> T>
		constexpr auto remove_overrides() -> size_type {
			auto n = size_type{};
			using overrides = overrides_types_in_cache_t<T>;
			std::apply([this, &n](auto... s) {
				auto const for_each = [this, &n](auto i) {
					using override = std::tuple_element_t<i, overrides>;
					constexpr auto id = KANGARU5_NO_ADL(type_id_for<override>)();
					n += map.erase(id);
				};
				(for_each(s), ...);
			}, detail::sequence_tuple_for_tuple<overrides>{});
			return n;
		}
		
		Map map;
	};
	
	static_assert(cache_map<polymorphic_map<std::unordered_map<type_id, void*>>>);
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_TYPES_HPP
