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
