#ifndef KANGARU5_DETAIL_TUPLE_SOURCE_HPP
#define KANGARU5_DETAIL_TUPLE_SOURCE_HPP

#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"

#include <tuple>
#include <concepts>

#include "define.hpp"

namespace kangaru::sources {
	template<source... Sources>
	struct composed_source {
		explicit constexpr composed_source(std::tuple<Sources...> sources) noexcept : sources{std::move(sources)} {}
		
		template<typename T>
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<composed_source> auto&& source) -> T
		requires (((source_of<Sources, T> ? 1 : 0) + ...) == 1) {
			/* static */ constexpr auto index = index_of<T>(std::index_sequence_for<Sources...>{});
			return kangaru::provide(provide_tag<T>, std::get<index>(KANGARU5_FWD(source).sources));
		}
		
	private:
		template<typename T, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			return ((source_of<Sources, T> ? S : 0) + ...);
		}
		
		std::tuple<Sources...> sources;
	};
	
	template<std::move_constructible... Ts>
	struct tuple_source {
		explicit constexpr tuple_source(std::tuple<Ts...> objects) noexcept : objects{std::move(objects)} {}
		
		template<typename T> requires (... or std::same_as<T, Ts>)
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<tuple_source> auto&& source) -> T {
			return std::get<T>(KANGARU5_FWD(source).objects);
		}
		
	private:
		std::tuple<Ts...> objects;
	};
	
	template<std::move_constructible T>
	struct object_source {
		explicit constexpr object_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
	};
	
	template<typename Source>
	struct ref_source {
		constexpr ref_source(Source& source) noexcept : source{std::addressof(source)} {}
		
		template<typename T> 
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<ref_source> auto&& source)
		requires source_of<Source, T> {
			return kangaru::provide(provide_tag<T>, detail::utility::forward_like<decltype(source)>(*source.source));
		}
		
	private:
		Source* source;
	};
	
	inline constexpr auto concat(auto&&... sources) requires(... and source<std::remove_cvref_t<decltype(sources)>>) {
		return composed_source{std::tuple{KANGARU5_FWD(sources)...}};
	}
	
	inline constexpr auto ref(source auto& source) {
		return ref_source{source};
	}
	
	inline constexpr auto tie(auto&&... sources) requires(... and source<std::remove_cvref_t<decltype(sources)>>) {
		return concat(ref(KANGARU5_FWD(sources))...);
	}
}

namespace kangaru {
	using namespace kangaru::sources;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_TUPLE_SOURCE_HPP
