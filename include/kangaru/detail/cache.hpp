#ifndef KANGARU5_DETAIL_CACHE_HPP
#define KANGARU5_DETAIL_CACHE_HPP

#include "type_traits.hpp"
#include "utility.hpp"
#include "source.hpp"
#include "ctti.hpp"
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

namespace kangaru {
	namespace detail::cache {
		template<typename To>
		auto any_cast(struct poison) -> To requires false;
		
		template<typename From, typename To>
		concept adl_castable_to = requires (From&& from) {
			{ any_cast<To>(KANGARU5_FWD(from)) } -> std::same_as<To>;
		};
	}
	
	KANGARU5_EXPORT
	template<
		source Source,
		dereferenceable_cache_map Cache = std::unordered_map<std::size_t, std::any>,
		template<typename> typename CacheFrom = detail::utility::type_identity
	>
	struct with_cache_asymmetric {
		using source_type = Source;
		using cache_type = Cache;
	
	private:
		using unwrapped_cache_type = std::remove_reference_t<maybe_unwrap_result_t<cache_type>>;
	
	public:
		explicit constexpr with_cache_asymmetric(source_type source) noexcept
		requires (
			std::default_initializable<cache_type>
		) : source{std::move(source)}, cache{} {}
		
		constexpr with_cache_asymmetric(source_type source, cache_type cache) noexcept :
			source{std::move(source)}, cache{std::move(cache)} {}
		
		using key_type = typename unwrapped_cache_type::key_type;
		using value_type = typename unwrapped_cache_type::value_type;
		using mapped_type = typename unwrapped_cache_type::mapped_type;
		using iterator = typename unwrapped_cache_type::iterator;
		using const_iterator = typename unwrapped_cache_type::const_iterator;
		
		source_type source;
		
		constexpr auto insert(auto&& value) requires requires(unwrapped_cache_type c) { c.insert(KANGARU5_FWD(value)); } {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).insert(KANGARU5_FWD(value));
		}
		
		template<typename It>
		constexpr auto insert(It begin, It end) requires requires(unwrapped_cache_type c) { c.insert(begin, end); } {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).insert(begin, end);
		}
		
		template<typename Key, typename Value>
			requires(
				    std::constructible_from<key_type, Key&&>
				and std::constructible_from<mapped_type, Value&&>
				and requires(unwrapped_cache_type c, Key&& key, Value&& value) { c.insert_or_assign(KANGARU5_FWD(key), KANGARU5_FWD(value)); }
			)
		constexpr auto insert_or_assign(Key&& key, Value&& value) {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).insert_or_assign(KANGARU5_FWD(key), KANGARU5_FWD(value));
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) requires requires(unwrapped_cache_type c) { c.find(key); } {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).find(key);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) const requires requires(unwrapped_cache_type const c) { c.find(key); } {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).find(key);
		}
		
		[[nodiscard]]
		constexpr auto contains(auto const& key) const requires requires(unwrapped_cache_type c) { c.contains(key); } {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).contains(key);
		}
		
		[[nodiscard]]
		constexpr auto begin() -> typename unwrapped_cache_type::iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).begin();
		}
		
		[[nodiscard]]
		constexpr auto end() -> typename unwrapped_cache_type::iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).end();
		}
		
		[[nodiscard]]
		constexpr auto begin() const {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).begin();
		}
		
		[[nodiscard]]
		constexpr auto end() const {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).end();
		}
		
		[[nodiscard]]
		constexpr auto cbegin() const -> typename unwrapped_cache_type::const_iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).cbegin();
		}
		
		[[nodiscard]]
		constexpr auto cend() const -> typename unwrapped_cache_type::const_iterator {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).cend();
		}
		
		[[nodiscard]]
		constexpr auto empty() const -> bool {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).empty();
		}
		
		constexpr auto clear() -> void {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).clear();
		}
		
		[[nodiscard]]
		constexpr auto size() const {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).size();
		}
		
		[[nodiscard]]
		constexpr auto erase(auto const& key) requires requires(unwrapped_cache_type c) { c.erase(key); } {
			return KANGARU5_NO_ADL(maybe_unwrap)(cache).erase(key);
		}
		
		constexpr auto swap(with_cache_asymmetric& other) noexcept -> void {
			std::ranges::swap(source, other.source);
			std::ranges::swap(cache, other.cache);
		}
		
		template<forwarded<with_cache_asymmetric> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept -> with_cache_asymmetric<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Cache>&>, CacheFrom> {
			return with_cache_asymmetric<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Cache>&>, CacheFrom>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_NO_ADL(ref)(original.cache)
			};
		}
		
		template<injectable T, typename Self>
			requires(
				    std::derived_from<std::remove_cvref_t<Self>, with_cache_asymmetric>
				and cache_map<std::remove_cvref_t<Self>>
				and not std::is_const_v<std::remove_reference_t<Self>>
				and requires{ typename CacheFrom<T>; }
				and std::constructible_from<mapped_type, CacheFrom<T>>
				and source_of<detail::utility::forward_like_t<Self, source_type>, CacheFrom<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			constexpr auto id = detail::ctti::type_id_for<T>();
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
		static constexpr auto cast(detail::cache::adl_castable_to<To> auto&& any) -> To {
			return any_cast<To>(KANGARU5_FWD(any));
		}
		
		template<typename To>
		static constexpr auto cast(explicitly_castable_to<To> auto&& any) -> To {
			return static_cast<To>(KANGARU5_FWD(any));
		}
		
		cache_type cache;
	};
	
	KANGARU5_EXPORT template<template<typename> typename CacheFrom, forwarded_source Source, forwarded_dereferenceable_cache_map Cache>
	inline constexpr auto make_source_with_cache_asymmetric(Source&& source, Cache&& cache) {
		return with_cache_asymmetric<std::decay_t<Source>, std::decay_t<Cache>, CacheFrom>{KANGARU5_FWD(source), KANGARU5_FWD(cache)};
	}
	
	KANGARU5_EXPORT template<template<typename> typename CacheFrom, forwarded_source Source>
	inline constexpr auto make_source_with_cache_asymmetric(Source&& source) {
		return with_cache_asymmetric<std::decay_t<Source>, std::unordered_map<std::size_t, std::any>, CacheFrom>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<
		source Source,
		dereferenceable_cache_map Cache = std::unordered_map<std::size_t, std::any>
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
		
		// TODO: Can we automate calling rebind from parent?
		template<forwarded<with_cache> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> with_cache<wrapped_source_rebind_result_t<detail::utility::forward_like_t<Original, parent>, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Cache>&>>
		{
			return with_cache<wrapped_source_rebind_result_t<detail::utility::forward_like_t<Original, parent>, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Cache>&>>{
				parent::rebind(static_cast<detail::utility::forward_like_t<Original, parent>&&>(original), KANGARU5_FWD(new_leaf))
			};
		}
	};
	
	template<typename Source, typename Cache>
	with_cache(Source const&, Cache const&) -> with_cache<Source, Cache>;
	
	template<typename Source>
	with_cache(Source const&) -> with_cache<Source>;
	
	KANGARU5_EXPORT template<forwarded_source Source, forwarded_dereferenceable_cache_map Cache>
	inline constexpr auto make_source_with_cache(Source&& source, Cache&& cache) {
		return with_cache<std::decay_t<Source>, std::decay_t<Cache>>{KANGARU5_FWD(source), KANGARU5_FWD(cache)};
	}
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_cache(Source&& source) {
		return with_cache<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<
		cache_map Cache,
		function_object SecondStep
	> requires(source<Cache>)
	struct cache_with_two_step_init_on_insert final : Cache {
	private:
		SecondStep second_step;
		
		template<injectable T, allows_construction_of<T> Value>
		constexpr auto run_second_step(Value&& value) -> void {
			using injected_type = detail::type_traits::conditional_t<reference<T>,
				detail::utility::forward_like_t<T, Value>&&,
				Value
			>;
			void(std::as_const(second_step).template operator()<injected_type>(value, Cache::source));
		}
		
	public:
		explicit constexpr cache_with_two_step_init_on_insert(Cache cache) requires(std::default_initializable<SecondStep>) : Cache{std::move(cache)} {}
		constexpr cache_with_two_step_init_on_insert(Cache cache, SecondStep second_step) : Cache{std::move(cache)}, second_step{std::move(second_step)} {}
		
		// TODO: Can we automate calling rebind from parent?
		// TODO: Big todo: change forwarded_source to function_object
		template<forwarded<cache_with_two_step_init_on_insert> Original, forwarded_source NewLeaf>
			requires(std::constructible_from<SecondStep, detail::utility::forward_like_t<Original, SecondStep>> and not std::is_const_v<std::remove_reference_t<Original>>)
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> cache_with_two_step_init_on_insert<rebind_result_t<Cache, NewLeaf>, SecondStep>
		{
			return cache_with_two_step_init_on_insert<rebind_result_t<Cache, NewLeaf>, SecondStep>{
				Cache::rebind(static_cast<detail::utility::forward_like_t<Original, Cache>&&>(original), KANGARU5_FWD(new_leaf)),
				KANGARU5_FWD(original).second_step,
			};
		}
		
		// TODO: properly constraints. To do so, we must test circular dependencies.
		//       If circular dependency cannot be checked in the signature, we must do static_assert
		template<injectable T, allows_construction_of<T> Value>
		constexpr auto insert(std::pair<detail::ctti::type_id_for_result<T>, Value>&& value) -> decltype(auto) requires(requires(Cache parent) { parent.insert(KANGARU5_FWD(value)); }) {
			run_second_step<T>(value.second);
			return Cache::insert(std::move(value));
		}
		
		template<injectable T, allows_construction_of<T> Value>
		constexpr auto insert(std::pair<detail::ctti::type_id_for_result<T>, Value> const& value) -> decltype(auto) requires(requires(Cache parent) { parent.insert(KANGARU5_FWD(value)); }) {
			run_second_step<T>(value.second);
			return Cache::insert(value);
		}
		
		template<typename It>
		constexpr auto insert(It begin, It end) requires requires(Cache c) { c.insert(begin, end); } {
			return Cache::insert(begin, end);
		}
		
		template<injectable T, allows_construction_of<T> Value>
		constexpr auto insert_or_assign(detail::ctti::type_id_for_result<T> const& key, Value&& value) -> decltype(auto) requires(requires(Cache parent) { parent.insert_or_assign(KANGARU5_FWD(key), KANGARU5_FWD(value)); }) {
			run_second_step<T>(value);
			return Cache::insert_or_assign(KANGARU5_FWD(key), KANGARU5_FWD(value));
		}
	};
	
	static_assert(cache_map<with_cache<none_source>>);
	static_assert(dereferenceable_cache_map<source_reference_wrapper<with_cache<with_cache<none_source>>>>);
	static_assert(cache_map<with_cache<none_source, source_reference_wrapper<with_cache<none_source>>>>);
	static_assert(cache_map<cache_with_two_step_init_on_insert<with_cache_asymmetric<none_source>, noop_second_step>>);
	static_assert(rebindable_source<cache_with_two_step_init_on_insert<with_cache_asymmetric<none_source>, noop_second_step>>);
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_HPP
