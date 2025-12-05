#ifndef KANGARU5_DETAIL_CACHE_HPP
#define KANGARU5_DETAIL_CACHE_HPP

#include "utility.hpp"
#include "source.hpp"
#include "ctti.hpp"
#include "concepts.hpp"
#include "cache_types.hpp"
#include "source_rebind.hpp"

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
		cache_map Cache = std::unordered_map<std::size_t, std::any>,
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
		
		template<object T, forwarded<with_cache_asymmetric> Self>
			requires source_of<detail::utility::forward_like_t<Self, source_type>, CacheFrom<T>>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			constexpr auto id = detail::ctti::type_id_for<T>();
			auto const it = KANGARU5_NO_ADL(maybe_unwrap)(source.cache).find(id);
			
			if (it == KANGARU5_NO_ADL(maybe_unwrap)(source.cache).end()) {
				decltype(auto) object = kangaru::provide<CacheFrom<T>>(KANGARU5_FWD(source).source);
				auto const [it, _] = KANGARU5_NO_ADL(maybe_unwrap)(source.cache).insert(
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
	
	KANGARU5_EXPORT template<template<source> typename CacheFrom, forwarded_source Source, forwarded_cache_map Cache>
	inline constexpr auto make_source_with_cache_asymmetric(Source&& source, Cache&& cache) {
		return with_cache_asymmetric<std::decay_t<Source>, std::decay_t<Cache>, CacheFrom>{KANGARU5_FWD(source), KANGARU5_FWD(cache)};
	}
	
	KANGARU5_EXPORT template<template<source> typename CacheFrom, forwarded_source Source>
	inline constexpr auto make_source_with_cache_asymmetric(Source&& source) {
		return with_cache_asymmetric<std::decay_t<Source>, std::unordered_map<std::size_t, std::any>, CacheFrom>{KANGARU5_FWD(source)};
	}
	
	// TODO: Make it readable
	KANGARU5_EXPORT template<
		source Source,
		cache_map Cache = std::unordered_map<std::size_t, std::any>
	>
	struct with_cache : with_cache_asymmetric<Source, Cache> {
	private:
		using parent_t = with_cache_asymmetric<Source, Cache>;
		
		template<source S, cache_map C>
		friend struct with_cache;
		
		explicit constexpr with_cache(parent_t&& parent) noexcept : with_cache_asymmetric<Source, Cache>{std::move(parent)} {}
		explicit constexpr with_cache(parent_t const& parent) : with_cache_asymmetric<Source, Cache>{parent} {}
		
	public:
		explicit constexpr with_cache(Source source) noexcept
		requires (
			std::default_initializable<Cache>
		) : parent_t{std::move(source)} {}
		
		constexpr with_cache(Source source, Cache cache) noexcept :
			parent_t{std::move(source), std::move(cache)} {}
		
		template<forwarded<with_cache> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> with_cache<wrapped_source_rebind_result_t<detail::utility::forward_like_t<Original, parent_t>, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Cache>&>>
		{
			return with_cache<wrapped_source_rebind_result_t<detail::utility::forward_like_t<Original, parent_t>, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Cache>&>>{
				parent_t::rebind(static_cast<detail::utility::forward_like_t<Original, parent_t>&&>(original), KANGARU5_FWD(new_leaf))
			};
		}
		
		template<object T, forwarded<with_cache> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(static_cast<detail::utility::forward_like_t<Self, parent_t>&&>(source));
		}
	};
	
	KANGARU5_EXPORT template<forwarded_source Source, forwarded_cache_map Cache>
	inline constexpr auto make_source_with_cache(Source&& source, Cache&& cache) {
		return with_cache<std::decay_t<Source>, std::decay_t<Cache>>{KANGARU5_FWD(source), KANGARU5_FWD(cache)};
	}
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_cache(Source&& source) {
		return with_cache<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	static_assert(cache_map<with_cache<none_source>>);
	static_assert(cache_map<source_reference_wrapper<with_cache<with_cache<none_source>>>>);
	static_assert(cache_map<with_cache<none_source, source_reference_wrapper<with_cache<none_source>>>>);
	
	KANGARU5_EXPORT template<template<unqualified_object> typename SourceType>
	struct cached_reference_to_source {
		template<injectable T>
		struct ttype {
			using type = SourceType<std::remove_cvref_t<T>>&;
		};
	};
	
	KANGARU5_EXPORT template<source Source, template<typename> typename SourceFor>
	struct with_cache_using_source {
		template<injectable T, forwarded<with_cache_using_source> Self>
			requires (
				    allow_runtime_caching_v<T>
				and not detail::utility::is_specialisation_of_v<SourceFor, T>
				and wrapping_source_of<Self, SourceFor<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			decltype(auto) source_for_t = kangaru::provide<SourceFor<T>>(KANGARU5_FWD(source).source);
			return kangaru::provide<T>(KANGARU5_FWD(source_for_t));
		}
		
		Source source;
	};
	
	KANGARU5_EXPORT template<template<typename> typename SourceFor>
	inline constexpr auto make_source_with_cache_using_source(forwarded_source auto&& source) {
		return with_cache_using_source<std::decay_t<decltype(source)>, SourceFor>{KANGARU5_FWD(source)};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_HPP
