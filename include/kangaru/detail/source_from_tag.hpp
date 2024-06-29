#ifndef KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP
#define KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP

#include "allocator.hpp"
#include "concepts.hpp"
#include "recursive_source.hpp"
#include "source.hpp"
#include "constructor.hpp"

#include <concepts>
#include <unordered_map>
#include <vector>

#include "define.hpp"

namespace kangaru {
	template<source Source, construction Construct>
	struct with_construction {
		explicit constexpr with_construction(Source source) noexcept
			requires std::default_initializable<Construct> :
			source{std::move(source)} {}
		
		constexpr with_construction(Source source, Construct construct) noexcept :
			source{std::move(source)},
			construct{std::move(construct)} {}
		
		template<typename T, forwarded<with_construction> Self> requires wrapping_source_of<Self, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
		template<typename T, forwarded<with_construction> Self> requires (callable_template1<Construct const&, T, wrapped_source_t<Self>>)
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return source.construct.template operator()<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
		
	private:
		Construct construct;
	};
	
	template<typename Source, typename Construct> requires (source<std::remove_cvref_t<Source>> and movable_object<std::remove_cvref_t<Construct>>)
	inline constexpr auto make_source_with_construction(Source&& source, Construct&& construct) {
		return with_construction<std::remove_cvref_t<Source>, std::remove_cvref_t<Construct>>{KANGARU5_FWD(source), KANGARU5_FWD(construct)};
	}
	
	template<source Source>
	struct with_source_from_tag {
		explicit constexpr with_source_from_tag(Source source) noexcept :
			source{std::move(source)} {}
		
		template<typename T, forwarded<with_source_from_tag> Self> requires (source_of<wrapped_source_t<Self>, cache_using_source_t<T>*>)
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			auto source_for_t = provide(provide_tag_v<cache_using_source_t<T>*>, KANGARU5_FWD(source).source);
			return provide(provide_tag_v<T>, *std::move(source_for_t));
		}
		
		Source source;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP
