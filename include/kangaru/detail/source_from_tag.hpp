#ifndef KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP
#define KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP

#include "allocator.hpp"
#include "recursive_source.hpp"
#include "source.hpp"
#include "constructor.hpp"

#include <concepts>
#include <unordered_map>
#include <vector>

#include "define.hpp"

namespace kangaru::sources {
	template<source Source, movable_object MakeInjector = make_spread_injector_function>
	struct source_from_tag_source {
		explicit constexpr source_from_tag_source(Source source) noexcept
			requires std::default_initializable<MakeInjector> :
			source{std::move(source)} {}
		
		Source source;
		
		template<typename T>
		friend constexpr auto provide(provide_tag<T>, forwarded<source_from_tag_source> auto&& source) {
			return source.make_injector(source.source)(constructor<T>());
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<source Source>
	struct with_source_from_tag {
		explicit constexpr with_source_from_tag(Source source) noexcept : source{std::move(source)} {}
		
		template<typename T>
		friend constexpr auto provide(provide_tag<T>, forwarded<with_source_from_tag> auto&& source) -> T {
			auto source_source = source_from_tag_source{kangaru::ref(KANGARU5_FWD(source).source)};
			auto source_for_t = provide(provide_tag_v<cache_using_source_t<T>>, source_source);
			return provide(provide_tag_v<T>, source_for_t);
		}
		
		Source source;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_FROM_TAG_HPP
