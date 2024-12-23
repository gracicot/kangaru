#ifndef KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP
#define KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP

#include "concepts.hpp"
#include "tag.hpp"
#include "source.hpp"

#include "define.hpp"

namespace kangaru {
	template<source Source>
	struct with_source_from_tag {
		template<injectable T, forwarded<with_source_from_tag> Self> requires (source_of<wrapped_source_t<Self>, cache_using_source_t<T>*>)
		friend constexpr auto provide(Self&& source) -> T {
			// TODO: Is there a way to do this without adding pointer or reference?
			// It depends if the reference is a need of the logic of this class
			// Or is it the need of the caching, and we need to know that something comes from a cache?
			// Asking for a value would result in copying the user type.
			// Can we really end up here without a cache?
			// Without a cache, it would just transform a source to another
			// Maybe we need a source transform? But transformed how?
			// We delegate getting the source by requesting another, then getting the type from that one.
			// Can we implement a generic algorithm like transform?
			// Can we actually just skip that source?? No. We need the polymorphic storage
			// We need to change this type so that it doesn't know about the caching. This is the solution. Generic, simple, composable.
			decltype(auto) source_for_t = kangaru::provide<cache_using_source_t<T>*>(KANGARU5_FWD(source).source);
			return kangaru::provide<T>(*KANGARU5_FWD(source_for_t));
		}
		
		Source source;
	};
	
	template<source Source>
	with_source_from_tag(Source) -> with_source_from_tag<Source>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_source_from_tag(Source&& source) {
		return with_source_from_tag<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source, template<typename> typename SourceFor>
	struct with_cache_using {
	private:
		static constexpr auto is_wrapped(auto&&) -> std::false_type;
		
		template<typename T>
		static auto is_wrapped(SourceFor<T>&&) -> std::true_type;
		
		template<typename T>
		static constexpr auto is_wrapped_v = decltype(is_wrapped(std::declval<T>()))::value;
		
	public:
		template<injectable T, forwarded<with_cache_using> Self>
			requires (
				    std::is_lvalue_reference_v<T>
				and is_cachable_v<T>
				and not is_wrapped_v<T>
				and wrapping_source_of<Self, SourceFor<std::remove_reference_t<T>>*>
			)
		friend constexpr auto provide(Self&& source) -> T {
			decltype(auto) source_for_t = kangaru::provide<SourceFor<std::remove_reference_t<T>>*>(KANGARU5_FWD(source).source);
			return kangaru::provide<T>(*KANGARU5_FWD(source_for_t));
		}
		
		template<forwarded<with_cache_using> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source) -> with_cache_using<std::decay_t<NewSource>, SourceFor> {
			return with_cache_using<std::decay_t<NewSource>, SourceFor>{
				KANGARU5_FWD(new_source),
			};
		}
		
		Source source;
	};
	
	template<template<typename> typename SourceFor>
	inline constexpr auto make_source_with_cache_using(forwarded_source auto&& source) {
		return with_cache_using<std::decay_t<decltype(source)>, SourceFor>{KANGARU5_FWD(source)};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP
