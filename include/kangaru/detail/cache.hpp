#ifndef KANGARU5_DETAIL_CACHE_HPP
#define KANGARU5_DETAIL_CACHE_HPP

#include "utility.hpp"
#include "source.hpp"
#include "ctti.hpp"
#include "concepts.hpp"
#include "cache_types.hpp"

#include <type_traits>
#include <concepts>
#include <utility>
#include <any>

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
	
	template<
		source Source,
		cache_map Cache = std::unordered_map<std::size_t, std::any>
	>
	struct with_cache {
		using source_type = Source;
		using cache_type = Cache;
	
	private:
		using unwrapped_cache_type = maybe_wrapped_t<cache_type>;
	
	public:
		explicit constexpr with_cache(source_type source) noexcept
		requires (
			std::default_initializable<cache_type>
		) : source{std::move(source)}, cache{} {}
		
		constexpr with_cache(source_type source, cache_type cache) noexcept :
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
		
		constexpr auto swap(with_cache& other) noexcept -> void {
			std::ranges::swap(source, other.source);
			std::ranges::swap(cache, other.cache);
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
		
		template<object T, forwarded<with_cache> Self>
			requires source_of<detail::utility::forward_like_t<Self, source_type>, T>
		friend constexpr auto provide(Self&& source) -> T {
			constexpr auto id = detail::ctti::type_id_for<T>();
			auto const it = KANGARU5_NO_ADL(maybe_unwrap)(source.cache).find(id);
			
			if (it == KANGARU5_NO_ADL(maybe_unwrap)(source.cache).end()) {
				auto object = kangaru::provide<T>(KANGARU5_FWD(source).source);
				auto const [it, _] = KANGARU5_NO_ADL(maybe_unwrap)(source.cache).insert(std::pair{id, std::move(object)});
				return cast<T>(it->second);
			} else {
				return cast<T>(it->second);
			}
		}
		
		cache_type cache;
	};
	
	template<forwarded_source Source, forwarded_cache_map Cache>
	constexpr auto make_source_with_cache(Source&& source, Cache&& cache) {
		return with_cache<std::decay_t<Source>, std::decay_t<Cache>>{KANGARU5_FWD(source), KANGARU5_FWD(cache)};
	}
	
	template<forwarded_source Source>
	constexpr auto make_source_with_cache(Source&& source) {
		return with_cache<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	static_assert(cache_map<with_cache<none_source>>);
	static_assert(cache_map<source_reference_wrapper<with_cache<with_cache<none_source>>>>);
	static_assert(cache_map<with_cache<none_source, source_reference_wrapper<with_cache<none_source>>>>);
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_HPP
