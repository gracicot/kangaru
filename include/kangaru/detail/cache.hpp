#ifndef KANGARU5_DETAIL_CACHE_HPP
#define KANGARU5_DETAIL_CACHE_HPP

#include "utility.hpp"
#include "source.hpp"
#include "ctti.hpp"
#include "concepts.hpp"
#include "allocator.hpp"
#include "cache_types.hpp"
#include "source_types.hpp"
#include "heap_storage.hpp"

#include <type_traits>
#include <concepts>
#include <memory>
#include <utility>

#include "define.hpp"

namespace kangaru {
	template<
		source Source,
		cache_map Cache = std::unordered_map<std::size_t, void*>
	>
	struct with_cache {
		using source_type = Source;
		using cache_type = Cache;
		
		explicit constexpr with_cache(source_type source) noexcept
		requires (
			std::default_initializable<cache_type>
		) : source{std::move(source)} {}
		
		constexpr with_cache(source_type source, cache_type cache) noexcept
			: source{std::move(source)}, cache{std::move(cache)} {}
		
		using key_type = typename cache_type::key_type;
		using value_type = typename cache_type::value_type;
		using mapped_type = typename cache_type::mapped_type;
		using iterator = typename cache_type::iterator;
		using const_iterator = typename cache_type::const_iterator;
		
		source_type source;
		
		constexpr auto insert(auto&& value) requires requires(cache_type c) { c.insert(KANGARU5_FWD(value)); } {
			return cache.insert(KANGARU5_FWD(value));
		}
		
		template<typename It>
		constexpr auto insert(It begin, It end) requires requires(cache_type c) { c.insert(begin, end); } {
			return cache.insert(begin, end);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) requires requires(cache_type c) { c.find(key); } {
			return cache.find(key);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) const requires requires(cache_type const c) { c.find(key); } {
			return cache.find(key);
		}
		
		[[nodiscard]]
		constexpr auto contains(auto const& key) const requires requires(cache_type c) { c.contains(key); } {
			return cache.contains(key);
		}
		
		[[nodiscard]]
		constexpr auto begin() -> typename cache_type::iterator {
			return cache.begin();
		}
		
		[[nodiscard]]
		constexpr auto end() -> typename cache_type::iterator {
			return cache.end();
		}
		
		[[nodiscard]]
		constexpr auto begin() const {
			return cache.begin();
		}
		
		[[nodiscard]]
		constexpr auto end() const {
			return cache.end();
		}
		
		[[nodiscard]]
		constexpr auto cbegin() const -> typename cache_type::const_iterator {
			return cache.cbegin();
		}
		
		[[nodiscard]]
		constexpr auto cend() const -> typename cache_type::const_iterator {
			return cache.cend();
		}
		
		[[nodiscard]]
		constexpr auto empty() const -> bool {
			return cache.empty();
		}
		
		constexpr auto clear() -> void {
			return cache.clear();
		}
		
		[[nodiscard]]
		constexpr auto size() const {
			return cache.size();
		}
		
		[[nodiscard]]
		constexpr auto erase(auto const& key) requires requires(cache_type c) { c.erase(key); } {
			return cache.erase(key);
		}
		
		constexpr auto swap(with_cache other) noexcept -> void {
			using std::swap;
			swap(source, other.source);
			swap(cache, other.cache);
		}
		
	private:
		template<object T, forwarded<with_cache> Self>
			requires source_of<detail::utility::forward_like_t<Self, source_type>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			constexpr auto id = detail::ctti::type_id_for<T>();
			auto const it = source.cache.find(id);
			
			if (it == source.cache.end()) {
				auto object = provide(tag, KANGARU5_FWD(source).source);
				auto const [it, _] = source.cache.insert(std::pair{id, std::move(object)});
				return static_cast<T>(it->second);
			} else {
				return static_cast<T>(it->second);
			}
		}
		
		cache_type cache = {};
	};

	static_assert(cache_map<with_cache<noop_source>>);
	
	template<source Source, cache_map InnerCache = std::unordered_map<std::size_t, void*>>
	struct with_cache_reference_wrapper {
		using source_type = source_reference_wrapper_for_t<Source>;
		using cache_type = with_cache<Source>;
		
		explicit constexpr with_cache_reference_wrapper(with_cache<Source, InnerCache>& source) noexcept :
			source{ref(source.source)}, cache{std::addressof(source)} {}
		
		using key_type = typename cache_type::key_type;
		using value_type = typename cache_type::value_type;
		using mapped_type = typename cache_type::mapped_type;
		using iterator = typename cache_type::iterator;
		using const_iterator = typename cache_type::const_iterator;
		
		source_type source;
		
		constexpr auto insert(auto&& value) const requires requires(cache_type c) { c.insert(KANGARU5_FWD(value)); } {
			return cache->insert(KANGARU5_FWD(value));
		}
		
		template<typename It>
		constexpr auto insert(It begin, It end) const requires requires(cache_type c) { c.insert(begin, end); } {
			return cache->insert(begin, end);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) const requires requires(cache_type const c) { c.find(key); } {
			return cache->find(key);
		}
		
		[[nodiscard]]
		constexpr auto contains(auto const& key) const requires requires(cache_type c) { c.contains(key); } {
			return cache->contains(key);
		}
		
		[[nodiscard]]
		constexpr auto begin() const -> typename cache_type::iterator {
			return cache->begin();
		}
		
		[[nodiscard]]
		constexpr auto end() const -> typename cache_type::iterator {
			return cache->end();
		}
		
		[[nodiscard]]
		constexpr auto cbegin() const -> typename cache_type::const_iterator {
			return cache->cbegin();
		}
		
		[[nodiscard]]
		constexpr auto cend() const -> typename cache_type::const_iterator {
			return cache->cend();
		}
		
		[[nodiscard]]
		constexpr auto empty() const -> bool {
			return cache->empty();
		}
		
		constexpr auto clear() const -> void {
			return cache->clear();
		}
		
		[[nodiscard]]
		constexpr auto size() const {
			return cache->size();
		}
		
		[[nodiscard]]
		constexpr auto erase(auto const& key) const requires requires(cache_type c) { c.erase(key); } {
			return cache->erase(key);
		}
		
		constexpr auto swap(with_cache_reference_wrapper other) noexcept -> void {
			using std::swap;
			swap(source, other.source);
			swap(cache, other.cache);
		}
		
		[[nodiscard]]
		constexpr auto unwrap() -> with_cache<Source, InnerCache>& {
			return *cache;
		}
		
	private:
		template<object T, forwarded<with_cache_reference_wrapper> Self>
			requires source_of<detail::utility::forward_like_t<Self, source_type>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, *source.cache);
		}
		
		cache_type* cache = {};
	};
	
	template<source Source, cache_map Cache>
	struct source_reference_wrapper_for<with_cache<Source, Cache>> {
		using type = with_cache_reference_wrapper<Source, Cache>;
	};
	
	static_assert(cache_map<with_cache_reference_wrapper<with_cache<noop_source>>>);
	static_assert(cache_map<with_cache<noop_source, with_cache_reference_wrapper<with_cache<noop_source>>>>);
	
	template<source Source>
	struct with_reference_passthrough {
		explicit constexpr with_reference_passthrough(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<object T, forwarded<with_reference_passthrough> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, KANGARU5_FWD(source).source);
		}
		
		template<reference T, forwarded<with_reference_passthrough> Self>
			requires source_of<detail::utility::forward_like_t<Self, decltype(std::declval<Source>().source)>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, KANGARU5_FWD(source).source.source);
		}
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_HPP
