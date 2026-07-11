#ifndef KANGARU5_DETAIL_CACHE_HPP
#define KANGARU5_DETAIL_CACHE_HPP

#include "type_traits.hpp"
#include "utility.hpp"
#include "source.hpp"
#include "type_id.hpp"
#include "concepts.hpp"
#include "cache_types.hpp"
#include "source_rebind.hpp"
#include "two_step_init.hpp"
#include "recursive_source.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <concepts>
#include <utility>
#include <any>
#endif

#include "define.hpp"

namespace kangaru::detail::cache_private {
	template<typename To>
	auto any_cast(struct poison) -> To requires false;
	
	template<typename From, typename To>
	concept adl_castable_to = requires (From&& from) {
		{ any_cast<To>(KANGARU5_FWD(from)) } -> std::same_as<To>;
	};
}

KANGARU5_EXPORT namespace kangaru {
	template<
		source Source,
		dereferenceable_cache_map Cache = std::unordered_map<type_id, std::any>,
		template<typename> typename CacheFrom = detail::type_identity
	>
	struct with_cache_asymmetric {
		using source_type = Source;
		using cache_type = Cache;
	
	private:
		using unwrapped_cache_type = std::remove_reference_t<maybe_unwrap_result_t<cache_type>>;
	
	public:
		template<allows_construction_of<Source> S>
			requires(std::default_initializable<cache_type>)
		explicit constexpr with_cache_asymmetric(S&& source) noexcept :
			source(KANGARU5_FWD(source)), cache{} {}
		
		template<allows_construction_of<Source> S>
		constexpr with_cache_asymmetric(S&& source, cache_type cache) noexcept :
			source(KANGARU5_FWD(source)), cache{std::move(cache)} {}
		
		using key_type = typename unwrapped_cache_type::key_type;
		using value_type = typename unwrapped_cache_type::value_type;
		using reference = typename unwrapped_cache_type::reference;
		using const_reference = typename unwrapped_cache_type::const_reference;
		using mapped_type = typename unwrapped_cache_type::mapped_type;
		using iterator = typename unwrapped_cache_type::iterator;
		using const_iterator = typename unwrapped_cache_type::const_iterator;
		using size_type = typename unwrapped_cache_type::size_type;
		
		source_type source;
		
		template<typename Key, typename Value>
			requires(
				    std::constructible_from<key_type, Key&&>
				and std::constructible_from<mapped_type, Value&&>
				and requires(unwrapped_cache_type c, Key&& key, Value&& value) { c.insert_or_assign(KANGARU5_FWD(key), KANGARU5_FWD(value)); }
			)
		constexpr auto insert_or_assign(Key&& key, Value&& value) -> std::pair<iterator, bool> {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).insert_or_assign(KANGARU5_FWD(key), KANGARU5_FWD(value));
		}
		
		constexpr auto insert(auto&& value) -> decltype(std::declval<unwrapped_cache_type>().insert(KANGARU5_FWD(value))) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).insert(KANGARU5_FWD(value));
		}
		
		template<typename It>
		constexpr auto insert(It begin, It end) -> void requires(
			requires(unwrapped_cache_type c) { { c.insert(begin, end) } -> std::same_as<void>; }
		) {
			KANGARU5_NO_ADL(maybe_unwrap)(cache).insert(begin, end);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) -> iterator requires(
			requires(unwrapped_cache_type c) { { c.find(key) } -> std::same_as<iterator>; }
		) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).find(key);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) const -> const_iterator requires(
			requires(unwrapped_cache_type const c) { { c.find(key) } -> std::same_as<const_iterator>; }
		) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).find(key);
		}
		
		[[nodiscard]]
		constexpr auto at(auto const& key) -> mapped_type& requires(
			requires(unwrapped_cache_type c) { { c.at(key) } -> std::same_as<mapped_type&>; }
		) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).at(key);
		}
		
		[[nodiscard]]
		constexpr auto at(auto const& key) const -> mapped_type const& requires(
			requires(unwrapped_cache_type const c) { { c.at(key) } -> std::same_as<mapped_type const&>; }
		) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).at(key);
		}
		
		[[nodiscard]]
		constexpr auto contains(auto const& key) const -> bool requires(
			requires(unwrapped_cache_type c) { c.contains(key); }
		) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).contains(key);
		}
		
		[[nodiscard]]
		constexpr auto begin() noexcept -> iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).begin();
		}
		
		[[nodiscard]]
		constexpr auto end() noexcept -> iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).end();
		}
		
		[[nodiscard]]
		constexpr auto begin() const noexcept -> const_iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).begin();
		}
		
		[[nodiscard]]
		constexpr auto end() const noexcept -> const_iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).end();
		}
		
		[[nodiscard]]
		constexpr auto cbegin() const noexcept -> const_iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).cbegin();
		}
		
		[[nodiscard]]
		constexpr auto cend() const noexcept -> const_iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).cend();
		}
		
		[[nodiscard]]
		constexpr auto empty() const noexcept -> bool {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).empty();
		}
		
		constexpr auto clear() noexcept -> void {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).clear();
		}
		
		[[nodiscard]]
		constexpr auto size() const noexcept -> std::size_t {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).size();
		}
		
		constexpr auto erase(auto const& key) -> decltype(std::declval<unwrapped_cache_type>().erase(key)) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).erase(key);
		}
		
		template<typename It>
		constexpr auto erase(It begin, It end) -> decltype(std::declval<unwrapped_cache_type>().erase(begin, end)) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).erase(begin, end);
		}
		
		constexpr auto swap(with_cache_asymmetric& other) noexcept(
			noexcept(std::declval<unwrapped_cache_type&>().swap(std::declval<unwrapped_cache_type&>()))
		) -> void {
			std::ranges::swap(source, other.source);
			KANGARU5_NO_ADL(maybe_unwrap)(cache).swap(KANGARU5_NO_ADL(maybe_unwrap)(other.cache));
		}
		
		template<forwarded<with_cache_asymmetric> Original, forwarded_source NewSource>
			requires(not std::is_const_v<std::remove_reference_t<Original>>)
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_cache_asymmetric<deduced_source_type<NewSource>, ref_result_t<detail::forward_like_t<Original, Cache>&>, CacheFrom>
		{
			return with_cache_asymmetric<deduced_source_type<NewSource>, ref_result_t<detail::forward_like_t<Original, Cache>&>, CacheFrom>{
				KANGARU5_FWD(new_source),
				KANGARU5_NO_ADL(ref)(original.cache),
			};
		}
		
		template<injectable T, typename Self>
			requires(
				    std::derived_from<std::remove_cvref_t<Self>, with_cache_asymmetric>
				and not std::is_const_v<std::remove_reference_t<Self>>
				and cache_map_stores<std::remove_cvref_t<Self>, T>
				and requires{ typename CacheFrom<T>; }
				and (
					   std::same_as<mapped_type, CacheFrom<T>>
					or std::constructible_from<mapped_type, CacheFrom<T>>
				)
				and wrapping_source_of<Self, CacheFrom<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<T>)();
			auto const it = source.find(id);
			
			if (it == source.end()) {
				decltype(auto) object = kangaru::provide<CacheFrom<T>>(KANGARU5_FWD(source).source);
				auto const [it, _] = source.insert(
					std::pair<std::remove_const_t<decltype(id)>, decltype(object)>{id, KANGARU5_FWD(object)}
				);
				return cast<T>(it->second);
			} else {
				return cast<T>(it->second);
			}
		}
		
	private:
		template<typename To>
		static constexpr auto cast(detail::cache_private::adl_castable_to<To> auto&& any) -> To {
			return any_cast<To>(KANGARU5_FWD(any));
		}
		
		template<typename To>
		static constexpr auto cast(explicitly_castable_to<To> auto&& any) -> To {
			return static_cast<To>(KANGARU5_FWD(any));
		}
		
		cache_type cache;
	};
	
	template<template<typename> typename CacheFrom, forwarded_source Source, forwarded_dereferenceable_cache_map Cache>
	inline constexpr auto make_source_with_cache_asymmetric(Source&& source, Cache&& cache) {
		return with_cache_asymmetric<deduced_source_type<Source>, std::decay_t<Cache>, CacheFrom>{KANGARU5_FWD(source), KANGARU5_FWD(cache)};
	}
	
	template<template<typename> typename CacheFrom, forwarded_source Source>
	inline constexpr auto make_source_with_cache_asymmetric(Source&& source) {
		return with_cache_asymmetric<deduced_source_type<Source>, std::unordered_map<type_id, std::any>, CacheFrom>{KANGARU5_FWD(source)};
	}
	
	template<
		source Source,
		dereferenceable_cache_map Cache = std::unordered_map<type_id, std::any>
	>
	struct with_cache : with_cache_asymmetric<Source, Cache> {
	private:
		using parent = with_cache_asymmetric<Source, Cache>;
		
		template<source S, dereferenceable_cache_map C>
		friend struct with_cache;
		
		explicit constexpr with_cache(parent&& parent) noexcept : with_cache_asymmetric<Source, Cache>{std::move(parent)} {}
		explicit constexpr with_cache(parent const& parent) : with_cache_asymmetric<Source, Cache>{parent} {}
		
	public:
		using parent::parent;
		
		template<forwarded<with_cache> Original, forwarded_source NewSource>
			requires(stateful_rebindable_wrapping_source<detail::forward_like_t<Original, parent>&&>)
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_cache<deduced_source_type<NewSource>, ref_result_t<detail::forward_like_t<Original, Cache>&>>
		{
			return with_cache<deduced_source_type<NewSource>, ref_result_t<detail::forward_like_t<Original, Cache>&>>{
				parent::rebind(static_cast<detail::forward_like_t<Original, parent>&&>(original), KANGARU5_FWD(new_source)),
			};
		}
	};
	
	template<typename Source, typename Cache>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_cache(Source&&, Cache const&) -> with_cache<deduced_source_type<Source>, Cache>;
	
	template<typename Source>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_cache(Source&&) -> with_cache<deduced_source_type<Source>>;
	
	template<forwarded_source Source, forwarded_dereferenceable_cache_map Cache>
	inline constexpr auto make_source_with_cache(Source&& source, Cache&& cache) {
		return with_cache<deduced_source_type<Source>, std::decay_t<Cache>>{KANGARU5_FWD(source), KANGARU5_FWD(cache)};
	}
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_cache(Source&& source) {
		return with_cache<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<
		cache_map Cache,
		second_step_function SecondStep
	> requires(source<Cache>)
	struct cache_with_two_step_init final : Cache {
	private:
		SecondStep second_step;
		
		template<injectable T, allows_construction_of<T> Value>
		constexpr auto run_second_step(Value&& value) -> void {
			using injected_type = detail::conditional_t<reference<T>,
				detail::forward_like_t<T, Value>&&,
				std::remove_reference_t<Value>
			>;
			void(std::as_const(second_step).template operator()<injected_type>(value, Cache::source));
		}
		
	public:
		template<allows_construction_of<Cache> C>
			requires(std::default_initializable<SecondStep>)
		explicit constexpr cache_with_two_step_init(C&& cache) : Cache{KANGARU5_FWD(cache)} {}
		
		template<allows_construction_of<Cache> C>
		constexpr cache_with_two_step_init(C&& cache, SecondStep second_step) : Cache{KANGARU5_FWD(cache)}, second_step{std::move(second_step)} {}
		
		template<forwarded<cache_with_two_step_init> Original, forwarded_source NewSource>
			requires(stateful_rebindable_wrapping_source<detail::forward_like_t<Original, Cache>&&>)
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> cache_with_two_step_init<rebind_result_t<detail::forward_like_t<Original, Cache>&&, NewSource>, SecondStep>
		{
			return cache_with_two_step_init<rebind_result_t<detail::forward_like_t<Original, Cache>&&, NewSource>, SecondStep>{
				Cache::rebind(static_cast<detail::forward_like_t<Original, Cache>&&>(original), KANGARU5_FWD(new_source)),
				std::as_const(original).second_step,
			};
		}
		
		template<injectable T, allows_construction_of<T> Value>
		constexpr auto insert(std::pair<static_type_id<T>, Value>&& value) -> decltype(Cache::insert(std::as_const(value))) requires(requires(Cache parent) { parent.insert(std::as_const(value)); }) {
			auto it = Cache::insert(std::as_const(value));
			run_second_step<T>(value.second); // TODO: Oh no, what if the value is actually a value type, instead of a reference type?
			return it;
		}
		
		template<injectable T, allows_construction_of<T> Value>
		constexpr auto insert(std::pair<static_type_id<T>, Value> const& value) -> decltype(Cache::insert(value))  requires(requires(Cache parent) { parent.insert(value); }) {
			auto it = Cache::insert(value);
			run_second_step<T>(value.second);
			return it;
		}
		
		template<typename It>
		constexpr auto insert(It begin, It end) -> void requires(
			requires(Cache c) { { c.insert(begin, end) } -> std::same_as<void>; }
		) {
			Cache::insert(begin, end);
		}
		
		template<injectable T, allows_construction_of<T> Value>
		constexpr auto insert_or_assign(static_type_id<T> const& key, Value&& value) -> decltype(Cache::insert_or_assign(key, value)) requires(requires(Cache parent) { parent.insert_or_assign(KANGARU5_FWD(key), value); }) {
			auto it = Cache::insert_or_assign(key, value);
			run_second_step<T>(value);
			return it;
		}
	};
	
	template<typename Cache, typename SecondStep>
		requires(not deducer<std::remove_cvref_t<Cache>>)
	cache_with_two_step_init(Cache&&, SecondStep const&) -> cache_with_two_step_init<deduced_source_type<Cache>, SecondStep>;
	
	// NOTE: Implement a cache with dynamic callbacks to fill it using the source. But the source is a template parameter? Is it possible to
	//       even have dynamic callbacks for it? I think it is technically possible, because we could compute the type in advance for a given
	//       source. How? Technically, we know it might be used in a context of a container or a polymorphic container. Given that knowledge,
	//       we can target a specific rebind for the callback parameter.
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_HPP
