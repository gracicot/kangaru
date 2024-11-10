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
		
		template<injectable T>
		friend constexpr auto provide(forwarded<composed_source> auto&& source) -> T
		requires (
			((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> ? 1 : 0) + ...) == 1
		) {
			constexpr auto index = index_of<T, decltype(source)>(std::index_sequence_for<Sources...>{});
			return provide<T>(std::get<index>(KANGARU5_FWD(source).sources));
		}
		
	private:
		template<typename T, typename Self, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			return ((source_of<detail::utility::forward_like_t<Self, Sources>, T> ? S : 0) + ...);
		}
		
		std::tuple<Sources...> sources;
	};
	
	inline constexpr auto concat(forwarded_source auto&&... sources) {
		return composed_source{KANGARU5_FWD(sources)...};
	}
	
	template<source... Sources>
	struct select_first_source {
		explicit constexpr select_first_source(std::tuple<Sources...> sources) noexcept : sources{std::move(sources)} {}
		
		template<typename T>
		friend constexpr auto provide(forwarded<select_first_source> auto&& source) -> T
		requires ((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> or ...)) {
			constexpr auto index = index_of<T, decltype(source)>(std::index_sequence_for<Sources...>{});
			return provide<T>(std::get<index>(KANGARU5_FWD(source).sources));
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
		
		template<injectable T> requires (... or std::same_as<T, Ts>)
		friend constexpr auto provide(forwarded<tuple_source> auto&& source) -> T {
			return std::get<T>(KANGARU5_FWD(source).objects);
		}
		
	private:
		std::tuple<Ts...> objects;
	};
	
	template<std::invocable F> requires (unqualified_object<F> and std::move_constructible<F> and injectable<std::invoke_result_t<F>>)
	struct function_source {
		explicit constexpr function_source(F function) noexcept : function{std::move(function)} {}
	
	private:
		using T = decltype(std::declval<F>()());
		
		friend constexpr auto provide(forwarded<function_source> auto&& source) -> T {
			return KANGARU5_FWD(source).function();
		}
		
		F function;
	};
	
	template<unqualified_object T>
	struct object_source {
		explicit constexpr object_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(forwarded<object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_object_source {
		template<deducer Deducer1, deducer... Deducer>
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, Deducer1>,
				Deducer&...
			>
		explicit(sizeof...(Deducer) == 0) injectable_object_source(Deducer1 deduce1, Deducer... deduce) :
			object(KANGARU5_NO_ADL(constructor<T>)()(KANGARU5_NO_ADL(exclude_special_constructors_for<T>)(deduce1), deduce...)) {}
		
		injectable_object_source() requires std::default_initializable<T> : object{} {}
		
		friend constexpr auto provide(forwarded<injectable_object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_reference_source {
		template<deducer Deducer1, deducer... Deducer>
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, Deducer1>,
				Deducer&...
			>
		explicit(sizeof...(Deducer) == 0) injectable_reference_source(Deducer1 deduce1, Deducer... deduce) :
			object(KANGARU5_NO_ADL(constructor<T>)()(KANGARU5_NO_ADL(exclude_special_constructors_for<T>)(deduce1), deduce...)) {}
		
		injectable_reference_source() requires std::default_initializable<T> : object{} {}
		
		friend constexpr auto provide(injectable_reference_source& source) -> T& {
			return source.object;
		}
		
		friend constexpr auto provide(injectable_reference_source&& source) -> T& {
			return source.object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_rvalue_source {
		template<deducer Deducer1, deducer... Deducer>
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, Deducer1>,
				Deducer&...
			>
		explicit(sizeof...(Deducer) == 0) injectable_rvalue_source(Deducer1 deduce1, Deducer... deduce) :
			object(KANGARU5_NO_ADL(constructor<T>)()(KANGARU5_NO_ADL(exclude_special_constructors_for<T>)(deduce1), deduce...)) {}
		
		injectable_rvalue_source() requires std::default_initializable<T> : object{} {}
		
		friend constexpr auto provide(injectable_rvalue_source& source) -> T&& {
			return std::move(source).object;
		}
		
		friend constexpr auto provide(injectable_rvalue_source&& source) -> T&& {
			return std::move(source).object;
		}
		
	private:
		T object;
	};
	
	template<object T>
	struct rvalue_source {
		explicit constexpr rvalue_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(rvalue_source& source) -> T&& {
			return std::move(source.object);
		}
		
		friend constexpr auto provide(rvalue_source&& source) -> T&& {
			return std::move(source.object);
		}
		
	private:
		T object;
	};
	
	template<object T>
	struct external_rvalue_source {
		explicit constexpr external_rvalue_source(T&& reference) noexcept : reference{std::addressof(reference)} {}
		
		friend constexpr auto provide(external_rvalue_source const& source) -> T&& {
			return std::move(*source.reference);
		}
		
	private:
		T* reference;
	};
	
	template<object T>
	struct external_reference_source {
		explicit constexpr external_reference_source(T& reference) noexcept : reference{std::addressof(reference)} {}
		
		friend constexpr auto provide(external_reference_source const& source) -> T& {
			return *source.reference;
		}
		
	private:
		T* reference;
	};
	
	template<object T>
	struct reference_source {
		explicit constexpr reference_source(T object) noexcept : object{std::move(object)} {}
		
		constexpr auto provide() & -> T& {
			return object;
		}
		
		constexpr auto provide() && -> T& {
			return object;
		}
		
	private:
		T object;
	};
	
	template<source Source, source Alternative>
	struct with_alternative {
		constexpr with_alternative(Source source, Alternative alternative) noexcept : source{std::move(source)}, alternative{std::move(alternative)} {}
		
		Source source;
		
		template<injectable T, forwarded<with_alternative> Self> requires (source_of<detail::utility::forward_like_t<Self, Alternative&&>, T> and not wrapping_source_of<Self, T>)
		friend constexpr auto provide(Self&& source) -> T {
			return provide<T>(KANGARU5_FWD(source).alternative);
		}
		
		template<injectable T, forwarded<with_alternative> Self> requires wrapping_source_of<Self, T>
		friend constexpr auto provide(Self&& source) -> T {
			return provide<T>(KANGARU5_FWD(source).source);
		}
		
	private:
		Alternative alternative;
	};
	
	template<forwarded_source Wrapped, forwarded_source Concat>
	inline constexpr auto make_source_with_alternative(Wrapped&& wrapped, Concat&& concat) {
		return with_alternative<std::decay_t<Wrapped>, std::decay_t<Concat>>{KANGARU5_FWD(wrapped), KANGARU5_FWD(concat)};
	}
	
	template<wrapping_source Source, injectable Filtered>
	struct with_filter_passthrough {
		explicit constexpr with_filter_passthrough(Source source) noexcept : source{std::move(source)} {}
		
		template<different_from<Filtered> T, forwarded<with_filter_passthrough> Self> requires source_of<wrapped_source_t<Self>, T>
		friend constexpr auto provide(Self&& source) -> T {
			return provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<forwarded<with_filter_passthrough> Self> requires wrapping_source_of<detail::utility::forward_like_t<Self, Source>, Filtered>
		friend constexpr auto provide(Self&& source) -> Filtered {
			return provide<Filtered>(KANGARU5_FWD(source).source.source);
		}
		
		Source source;
	};
	
	template<injectable T, forwarded_source Source>
	constexpr auto make_source_with_filter_passthrough(Source&& source) {
		return with_filter_passthrough<std::decay_t<Source>, T>{KANGARU5_FWD(source)};
	}
	
	template<source Source, injectable Type>
	struct filter_source {
		constexpr filter_source(Source source) noexcept : source{std::move(source)} {}
		
	private:
		template<different_from<Type> T> requires injectable<T>
		friend constexpr auto provide(forwarded<filter_source> auto const& source) -> T {
			return provide<T>(source.source);
		}
		
		Source source;
	};
	
	template<source Source, std::default_initializable Filter>
	struct filter_if_source {
		constexpr filter_if_source(Source source) noexcept : source{std::move(source)} {}
		
	private:
		template<injectable T> requires(Filter{}.template operator()<T>())
		friend constexpr auto provide(forwarded<filter_if_source> auto const& source) -> T {
			return provide<T>(source.source);
		}
		
		Source source;
	};
	
	template<injectable T, forwarded_source Source>
	inline constexpr auto filter(Source&& source) {
		return filter_source<std::decay_t<Source>, T>{KANGARU5_FWD(source)};
	}
	
	template<forwarded_source Source, std::default_initializable Filter>
	inline constexpr auto filter_if(Source&& source, [[maybe_unused]] Filter filter) {
		return filter_if_source<std::decay_t<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_reference_passthrough {
		explicit constexpr with_reference_passthrough(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<object T, forwarded<with_reference_passthrough> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(Self&& source) -> T {
			return provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<reference T, forwarded<with_reference_passthrough> Self>
			requires wrapping_source_of<Self, T>
		friend constexpr auto provide(Self&& source) -> T {
			return provide<T>(KANGARU5_FWD(source).source.source);
		}
	};
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_reference_passthrough(Source&& source) noexcept {
		return with_reference_passthrough<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_dereference {
		explicit constexpr with_dereference(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<reference T> requires source_of<Source, std::remove_reference_t<T>*>
		friend constexpr auto provide(forwarded<with_dereference> auto&& source) -> T {
			return *provide<std::remove_reference_t<T>*>(KANGARU5_FWD(source).source);
		}
		
		template<object T> requires (not std::is_pointer_v<T> and source_of<Source, T>)
		friend constexpr auto provide(forwarded<with_dereference> auto&& source) -> T {
			return provide<T>(KANGARU5_FWD(source).source);
		}
	};
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_dereference(Source&& source) {
		return with_dereference<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct basic_wrapping_source {
		template<injectable T, forwarded<basic_wrapping_source> Self> requires source_of<wrapped_source_t<Self>, T>
		friend constexpr auto provide(Self&& source) -> T {
			return provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<forwarded_source Source>
	inline constexpr auto wrap_source(Source&& source) {
		return basic_wrapping_source<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	inline constexpr auto tie(source auto&... sources) {
		return kangaru::concat(kangaru::ref(sources)...);
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TYPES_HPP
