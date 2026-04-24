#ifndef KANGARU5_DETAIL_CACHE_TYPES_HPP
#define KANGARU5_DETAIL_CACHE_TYPES_HPP

#include "attributes.hpp"
#include "utility.hpp"
#include "source.hpp"
#include "type_id.hpp"
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

KANGARU5_EXPORT namespace kangaru {
	template<typename T>
	concept cache_map = requires(T map) {
		typename T::key_type;
		typename T::value_type;
		typename T::reference;
		typename T::const_reference;
		typename T::mapped_type;
		typename T::iterator;
		typename T::const_iterator;
		typename T::size_type;
		
		{ map.begin() } noexcept -> std::forward_iterator;
		{ map.end() } noexcept -> std::forward_iterator;
		{ std::as_const(map).begin() } noexcept -> std::forward_iterator;
		{ std::as_const(map).end() } noexcept -> std::forward_iterator;
		{ map.insert(map.begin(), map.end()) } -> std::same_as<void>;
		{ map.clear() } noexcept;
		{ std::as_const(map).empty() } noexcept -> std::same_as<bool>;
		{ std::as_const(map).size() } noexcept -> std::same_as<typename T::size_type>;
		{ map.swap(map) };
	};
	
	template<typename Map, typename T>
	concept cache_map_stores = cache_map<Map> and requires(Map map, static_type_id<T> id, std::pair<static_type_id<T>, T> element, T value) {
		{ map.at(id) } -> std::same_as<typename Map::mapped_type&>;
		{ std::as_const(map).at(id) } -> std::same_as<typename Map::mapped_type const&>;
		{ map.find(id) } -> std::same_as<typename Map::iterator>;
		{ std::as_const(map).find(id) } -> std::same_as<typename Map::const_iterator>;
		{ std::as_const(map).contains(id) } -> std::same_as<bool>;
		{ map.erase(id) };
		{ map.insert(std::move(element)) } -> std::same_as<std::pair<typename Map::iterator, bool>>;
		{ map.insert_or_assign(id, std::move(value)) } -> std::same_as<std::pair<typename Map::iterator, bool>>;
	};
	
	template<typename T>
	concept dereferenceable_cache_map = cache_map<T> or requires {
		requires cache_map<source_reference_wrapped_type<T>>;
	};
	
	template<typename T>
	concept forwarded_dereferenceable_cache_map = cache_map<std::remove_cvref_t<T>>;
	
	static_assert(cache_map<std::unordered_map<type_id, void*>>);
	
	template<dereferenceable_cache_map Map>
	struct polymorphic_map {
		using key_type = typename Map::key_type;
		using mapped_type = typename Map::mapped_type;
		using reference = typename Map::reference;
		using const_reference = typename Map::const_reference;
		using iterator = typename Map::iterator;
		using const_iterator = typename Map::const_iterator;
		using value_type = typename Map::value_type;
		using size_type = typename Map::size_type;
		
		constexpr auto insert(value_type const& value) -> std::pair<iterator, bool> {
			return map.insert(value);
		}
		
		constexpr auto insert(value_type&& value) -> std::pair<iterator, bool> {
			return map.insert(std::move(value));
		}
		
		template<injectable T, allows_construction_of<T> U>
		constexpr auto insert(std::pair<static_type_id<T>, U> const& value) -> std::pair<iterator, bool> {
			insert_overrides(value.second);
			return map.insert(value_type{value.first, static_cast<T>(value.second)});
		}
		
		template<injectable T, allows_construction_of<T> U>
		constexpr auto insert(std::pair<static_type_id<T>, U>&& value) -> std::pair<iterator, bool> {
			insert_overrides(value.second);
			return map.insert(value_type{value.first, static_cast<T>(std::move(value).second)});
		}
		
		constexpr auto insert(const_iterator hint, value_type const& value) -> std::pair<iterator, bool> {
			return map.insert(hint, value);
		}
		
		constexpr auto insert(const_iterator hint, value_type&& value) -> std::pair<iterator, bool> {
			return map.insert(hint, std::move(value));
		}
		
		template<std::input_iterator Iterator> requires(
			requires(Iterator it) { { *it } -> std::convertible_to<const_reference>; }
		)
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
		
		constexpr auto swap(polymorphic_map& other) noexcept(noexcept(map.swap(other.map))) -> void {
			map.swap(other.map);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) -> iterator requires(
			requires(Map c) { { c.find(key) } -> std::same_as<iterator>; }
		) {
			return map.find(key);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) const -> const_iterator requires(
			requires(Map const c) { { c.find(key) } -> std::same_as<const_iterator>; }
		) {
			return map.find(key);
		}
		
		[[nodiscard]]
		constexpr auto at(auto const& key) -> mapped_type& requires(
			requires(Map c) { { c.at(key) } -> std::same_as<mapped_type&>; }
		) {
			return map.at(key);
		}
		
		[[nodiscard]]
		constexpr auto at(auto const& key) const -> mapped_type const& requires(
			requires(Map const c) { { c.at(key) } -> std::same_as<mapped_type const&>; }
		) {
			return map.at(key);
		}
		
		[[nodiscard]]
		constexpr auto contains(auto const& key) const -> bool requires(
			requires(Map const c) { c.contains(key); }
		) {
			return map.contains(key);
		}
		
		[[nodiscard]]
		constexpr auto begin() noexcept -> iterator {
			return map.begin();
		}
		
		[[nodiscard]]
		constexpr auto end() noexcept -> iterator {
			return map.end();
		}
		
		[[nodiscard]]
		constexpr auto begin() const noexcept -> const_iterator {
			return map.begin();
		}
		
		[[nodiscard]]
		constexpr auto end() const noexcept -> const_iterator {
			return map.end();
		}
		
		[[nodiscard]]
		constexpr auto cbegin() const noexcept -> const_iterator {
			return map.cbegin();
		}
		
		[[nodiscard]]
		constexpr auto cend() const noexcept -> const_iterator {
			return map.cend();
		}
		
		[[nodiscard]]
		constexpr auto empty() const noexcept -> bool {
			return map.empty();
		}
		
		constexpr auto clear() noexcept -> void {
			return map.clear();
		}
		
		[[nodiscard]]
		constexpr auto size() const noexcept -> size_type {
			return map.size();
		}
		
		template<injectable T>
		constexpr auto erase(static_type_id<T> id) -> size_type {
			auto const overrides = remove_overrides<T>();
			return overrides + map.erase(id);
		}
		
		template<typename It>
		constexpr auto erase(It begin, It end) -> decltype(std::declval<Map>().erase(begin, end)) {
			return map.erase(begin, end);
		}
		
		constexpr auto erase(iterator pos) -> iterator {
			return map.erase(pos);
		}
		
		constexpr auto erase(const_iterator pos) -> iterator {
			return map.erase(pos);
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
	
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_TYPES_HPP
