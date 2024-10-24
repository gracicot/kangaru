#ifndef KANGARU5_DETAIL_SOURCE_TYPES_HPP
#define KANGARU5_DETAIL_SOURCE_TYPES_HPP

#include "deducer.hpp"
#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"
#include "constructor.hpp"
#include "source_reference_wrapper.hpp"

#include <tuple>
#include <concepts>
#include <algorithm>
#include <type_traits>

#include "define.hpp"

namespace kangaru {
	template<source... Sources> requires (sizeof...(Sources) > 0)
	struct composed_source {
		explicit constexpr composed_source(Sources... sources) noexcept : sources{std::move(sources)...} {}
		
		template<typename T>
		friend constexpr auto provide(provide_tag<T>, forwarded<composed_source> auto&& source) -> T
		requires (((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> ? 1 : 0) + ...) == 1) {
			constexpr auto index = index_of<T, decltype(source)>(std::index_sequence_for<Sources...>{});
			return provide(provide_tag_v<T>, std::get<index>(KANGARU5_FWD(source).sources));
		}
		
	private:
		template<typename T, typename Self, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			return ((source_of<detail::utility::forward_like_t<Self, Sources>, T> ? S : 0) + ...);
		}
		
		std::tuple<Sources...> sources;
	};
	
	inline constexpr auto concat(auto&&... sources) requires(... and source<std::remove_reference_t<decltype(sources)>>) {
		return composed_source{KANGARU5_FWD(sources)...};
	}
	
	template<source... Sources>
	struct select_first_source {
		explicit constexpr select_first_source(std::tuple<Sources...> sources) noexcept : sources{std::move(sources)} {}
		
		template<typename T>
		friend constexpr auto provide(provide_tag<T>, forwarded<select_first_source> auto&& source) -> T
		requires ((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> or ...)) {
			constexpr auto index = index_of<T, decltype(source)>(std::index_sequence_for<Sources...>{});
			return provide(provide_tag_v<T>, std::get<index>(KANGARU5_FWD(source).sources));
		}
		
	private:
		template<typename T, typename Self, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			auto const source_handles = std::array{std::pair{S, source_of<detail::utility::forward_like_t<Self, Sources>, T>}...};
			
			auto const it = std::find_if(source_handles.begin(), source_handles.end(), [](std::pair<std::size_t, bool> source) {
				return source.second;
			});
			
			return std::distance(source_handles.begin(), it);
		}
		
		std::tuple<Sources...> sources;
	};
	
	template<object... Ts>
	struct tuple_source {
		explicit constexpr tuple_source(std::tuple<Ts...> objects) noexcept : objects{std::move(objects)} {}
		
		template<typename T> requires (... or std::same_as<T, Ts>)
		friend constexpr auto provide(provide_tag<T>, forwarded<tuple_source> auto&& source) -> T {
			return std::get<T>(KANGARU5_FWD(source).objects);
		}
		
	private:
		std::tuple<Ts...> objects;
	};
	
	template<unqualified_object F> requires std::invocable<F>
	struct function_source {
		explicit constexpr function_source(F function) noexcept : function{std::move(function)} {}
	
	private:
		using T = decltype(std::declval<F>()());
		
		friend constexpr auto provide(provide_tag<T>, forwarded<function_source> auto&& source) -> T {
			return KANGARU5_FWD(source).function();
		}
		
		F function;
	};
	
	template<unqualified_object T>
	struct object_source {
		explicit constexpr object_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(provide_tag<T>, forwarded<object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_object_source {
		template<deducer Deducer1, deducer... Deducer>
			requires callable<
				KANGARU5_CONSTRUCTOR_T(T),
				exclude_special_constructors_deducer<T, Deducer1>,
				Deducer&...
			>
		explicit(sizeof...(Deducer) == 0) injectable_object_source(Deducer1 deduce1, Deducer... deduce) :
			object(kangaru::constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), deduce...)) {}
		
		injectable_object_source() requires std::default_initializable<T> : object{} {}
		
		friend constexpr auto provide(provide_tag<T>, forwarded<injectable_object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_reference_source {
		template<deducer Deducer1, deducer... Deducer>
			requires callable<
				KANGARU5_CONSTRUCTOR_T(T),
				exclude_special_constructors_deducer<T, Deducer1>,
				Deducer&...
			>
		explicit(sizeof...(Deducer) == 0) injectable_reference_source(Deducer1 deduce1, Deducer... deduce) :
			object(kangaru::constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), deduce...)) {}
		
		injectable_reference_source() requires std::default_initializable<T> : object{} {}
		
		friend constexpr auto provide(provide_tag<T&>, injectable_reference_source& source) -> T& {
			return source.object;
		}
		
		friend constexpr auto provide(provide_tag<T&>, injectable_reference_source&& source) -> T& {
			return source.object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_rvalue_source {
		template<deducer Deducer1, deducer... Deducer>
			requires callable<
				KANGARU5_CONSTRUCTOR_T(T),
				exclude_special_constructors_deducer<T, Deducer1>,
				Deducer&...
			>
		explicit(sizeof...(Deducer) == 0) injectable_rvalue_source(Deducer1 deduce1, Deducer... deduce) :
			object(kangaru::constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), deduce...)) {}
		
		injectable_rvalue_source() requires std::default_initializable<T> : object{} {}
		
		friend constexpr auto provide(provide_tag<T&&>, injectable_rvalue_source& source) -> T&& {
			return std::move(source).object;
		}
		
		friend constexpr auto provide(provide_tag<T&&>, injectable_rvalue_source&& source) -> T&& {
			return std::move(source).object;
		}
		
	private:
		T object;
	};
	
	template<object T>
	struct rvalue_source {
		explicit constexpr rvalue_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(provide_tag<T&&>, rvalue_source& source) -> T&& {
			return std::move(source.object);
		}
		
		friend constexpr auto provide(provide_tag<T&&>, rvalue_source&& source) -> T&& {
			return std::move(source.object);
		}
		
	private:
		T object;
	};
	
	template<object T>
	struct external_rvalue_source {
		explicit constexpr external_rvalue_source(T&& reference) noexcept : reference{std::addressof(reference)} {}
		
		friend constexpr auto provide(provide_tag<T&&>, external_rvalue_source const& source) -> T&& {
			return std::move(*source.reference);
		}
		
	private:
		T* reference;
	};
	
	template<object T>
	struct external_reference_source {
		explicit constexpr external_reference_source(T& reference) noexcept : reference{std::addressof(reference)} {}
		
		friend constexpr auto provide(provide_tag<T&>, external_reference_source const& source) -> T& {
			return *source.reference;
		}
		
	private:
		T* reference;
	};
	
	template<object T>
	struct reference_source {
		explicit constexpr reference_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(provide_tag<T&>, reference_source& source) -> T& {
			return source.object;
		}
		
		friend constexpr auto provide(provide_tag<T&>, reference_source&& source) -> T& {
			return source.object;
		}
		
	private:
		T object;
	};
	
	template<source Source, source Alternative>
	struct with_alternative {
		constexpr with_alternative(Source source, Alternative alternative) noexcept : source{std::move(source)}, alternative{std::move(alternative)} {}
		
		Source source;
		
		template<typename T, forwarded<with_alternative> Self> requires (source_of<detail::utility::forward_like_t<Self, Alternative&&>, T> and not wrapping_source_of<Self, T>)
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return provide(provide_tag_v<T>, KANGARU5_FWD(source).alternative);
		}
		
		template<typename T, forwarded<with_alternative> Self> requires wrapping_source_of<Self, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
	private:
		Alternative alternative;
	};
	
	inline constexpr auto make_source_with_alternative(auto&& wrapped, auto&& concat) requires(source<std::remove_reference_t<decltype(wrapped)>> and source<std::remove_reference_t<decltype(concat)>>) {
		return with_alternative<std::remove_cvref_t<decltype(wrapped)>, std::remove_cvref_t<decltype(concat)>>{KANGARU5_FWD(wrapped), KANGARU5_FWD(concat)};
	}
	
	template<wrapping_source Source, typename Filtered>
	struct with_filter_passthrough {
		explicit constexpr with_filter_passthrough(Source source) noexcept : source{std::move(source)} {}
		
		template<different_from<Filtered> T, forwarded<with_filter_passthrough> Self> requires source_of<wrapped_source_t<Self>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, KANGARU5_FWD(source).source);
		}
		
		template<forwarded<with_filter_passthrough> Self> requires wrapping_source_of<detail::utility::forward_like_t<Self, Source>, Filtered>
		friend constexpr auto provide(provide_tag<Filtered> tag, Self&& source) -> Filtered {
			return provide(tag, KANGARU5_FWD(source).source.source);
		}
		
		Source source;
	};
	
	template<typename T, typename Source> requires source<std::remove_reference_t<Source>>
	constexpr auto make_source_with_filter_passthrough(Source&& source) {
		return with_filter_passthrough<std::remove_cvref_t<Source>, T>{KANGARU5_FWD(source)};
	}
	
	template<source Source, typename Type>
	struct filter_source {
		constexpr filter_source(Source source) noexcept : source{std::move(source)} {}
		
	private:
		template<different_from<Type> T>
		friend constexpr auto provide(provide_tag<T>, forwarded<filter_source> auto const& source) -> T {
			return provide(provide_tag_v<T>, source.source);
		}
		
		Source source;
	};
	
	template<source Source, std::default_initializable Filter>
	struct filter_if_source {
		constexpr filter_if_source(Source source) noexcept : source{std::move(source)} {}
		
	private:
		template<typename T> requires(requires { requires Filter{}.template operator()<T>(); })
		friend constexpr auto provide(provide_tag<T>, forwarded<filter_if_source> auto const& source) -> T {
			return provide(provide_tag_v<T>, source.source);
		}
		
		Source source;
	};
	
	template<typename T, typename Source>
	inline constexpr auto filter(Source&& source) -> filter_source<std::decay_t<Source>, T> {
		return filter_source<std::decay_t<Source>, T>{KANGARU5_FWD(source)};
	}
	
	template<typename Source, typename Filter>
	inline constexpr auto filter_if(Source&& source, [[maybe_unused]] Filter filter) -> filter_if_source<std::decay_t<Source>, Filter> {
		return filter_if_source<std::decay_t<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
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
			requires wrapping_source_of<Self, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, KANGARU5_FWD(source).source.source);
		}
	};
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	inline constexpr auto make_source_with_reference_passthrough(Source&& source) noexcept {
		return with_reference_passthrough<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_dereference {
		explicit constexpr with_dereference(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<typename T> requires source_of<Source, T*>
		friend constexpr auto provide(provide_tag<T&>, forwarded<with_dereference> auto&& source) -> T& {
			return *provide(provide_tag_v<T*>, KANGARU5_FWD(source).source);
		}
		
		template<typename T> requires (not std::is_pointer_v<T> and source_of<Source, T>)
		friend constexpr auto provide(provide_tag<T>, forwarded<with_dereference> auto&& source) -> T {
			return provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
	};
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	inline constexpr auto make_source_with_dereference(Source&& source) {
		return with_dereference<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct basic_wrapping_source {
		template<typename T, forwarded<basic_wrapping_source> Self> requires source_of<wrapped_source_t<Self>, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	inline constexpr auto wrap_source(Source&& source) {
		return basic_wrapping_source<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	inline constexpr auto tie(source auto&... sources) {
		return kangaru::concat(kangaru::ref(sources)...);
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TYPES_HPP
