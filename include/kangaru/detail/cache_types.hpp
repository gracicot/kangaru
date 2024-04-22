#ifndef KANGARU5_DETAIL_CACHE_TYPES_HPP
#define KANGARU5_DETAIL_CACHE_TYPES_HPP

#include "constructor.hpp"
#include "tag.hpp"
#include "utility.hpp"
#include "source.hpp"
#include "ctti.hpp"
#include "concepts.hpp"
#include "allocator.hpp"

#include <cstddef>
#include <type_traits>
#include <concepts>
#include <memory>
#include <unordered_map>
#include <iterator>
#include <utility>

#include "define.hpp"

namespace kangaru {
	template<typename T>
	concept cache_map = requires(T map, detail::ctti::type_id_for_result<T> id) {
		{ map.begin() } -> std::forward_iterator;
		{ map.end() } -> std::forward_iterator;
		{ std::as_const(map).begin() } -> std::forward_iterator;
		{ std::as_const(map).end() } -> std::forward_iterator;
		{ map.insert(map.begin(), map.end()) };
		{ map.find(id) } -> std::same_as<decltype(map.begin())>;
		{ std::as_const(map).find(id) } -> std::same_as<decltype(std::as_const(map).begin())>;
		{ std::as_const(map).contains(id) } -> std::same_as<bool>;
		{ map.erase(id) };
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
	
	static_assert(cache_map<std::unordered_map<std::size_t, void*>>);
	
	template<cache_map Map>
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
				[[maybe_unused]]
				auto const for_each = [this](auto i, T* ptr) {
					using override = std::tuple_element_t<i, overrides>;
					constexpr auto id = detail::ctti::type_id_for<override*>();
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
					constexpr auto id = detail::ctti::type_id_for<override*>();
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
					constexpr auto id = detail::ctti::type_id_for<override*>();
					n += map.erase(id);
				};
				(for_each(s), ...);
			}, detail::utility::sequence_tuple_for_tuple<overrides>{});
			return n;
		}
		
		Map map;
	};
	
	static_assert(cache_map<polymorphic_map<std::unordered_map<std::size_t, void*>>>);
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_TYPES_HPP
