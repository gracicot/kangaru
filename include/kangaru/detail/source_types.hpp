#ifndef KANGARU5_DETAIL_SOURCE_TYPES_HPP
#define KANGARU5_DETAIL_SOURCE_TYPES_HPP

#include "deducer.hpp"
#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"
#include "constructor.hpp"
#include "source_reference_wrapper.hpp"
#include "attributes.hpp"
#include "source_rebind.hpp"

#ifndef KANGARU5_MODULES
#include <tuple>
#include <concepts>
#include <algorithm>
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<source... Sources>
	struct composed_source {
		explicit constexpr composed_source(Sources... sources) noexcept : sources{std::move(sources)...} {}
		
		template<injectable T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<composed_source> auto&& source) -> T
		requires (
			((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> ? 1 : 0) + ... + 0) == 1
		) {
			constexpr auto index = index_of<T, decltype(source)>(std::index_sequence_for<Sources...>{});
			return kangaru::provide<T>(std::get<index>(KANGARU5_FWD(source).sources));
		}
		
		template<injectable T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<composed_source> auto&& source) -> T
		requires ("Ambiguous source resolution: One or more source can provide type T",
			((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> ? 1 : 0) + ... + 0) > 1
		) = delete;
		
	private:
		template<typename T, typename Self, std::size_t... S> requires(sizeof...(Sources) > 0)
		consteval static auto index_of(std::index_sequence<S...>) {
			return ((source_of<detail::utility::forward_like_t<Self, Sources>, T> ? S : 0) + ...);
		}
		
		std::tuple<Sources...> sources;
	};
	
	KANGARU5_EXPORT inline constexpr auto compose(forwarded_source auto&&... sources) {
		return composed_source<std::decay_t<decltype(sources)>...>{KANGARU5_FWD(sources)...};
	}
	
	KANGARU5_EXPORT template<source... Sources>
	struct select_first_source {
		explicit constexpr select_first_source(std::tuple<Sources...> sources) noexcept : sources{std::move(sources)} {}
		
		template<injectable T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<select_first_source> auto&& source) -> T
		requires ((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> or ...)) {
			constexpr auto index = index_of<T, decltype(source)>(std::index_sequence_for<Sources...>{});
			return kangaru::provide<T>(std::get<index>(KANGARU5_FWD(source).sources));
		}
		
	private:
		template<typename T, typename Self, std::size_t... S>
		consteval static auto index_of(std::index_sequence<S...>) {
			auto const source_handles = std::array{std::pair{S, source_of<detail::utility::forward_like_t<Self, Sources>, T>}...};
			
			auto const it = std::find_if(source_handles.begin(), source_handles.end(), [](std::pair<std::size_t, bool> source) {
				return source.second;
			});
			
			return std::distance(source_handles.begin(), it);
		}
		
		std::tuple<Sources...> sources;
	};
	
	KANGARU5_EXPORT template<object... Ts>
	struct tuple_source {
		explicit constexpr tuple_source(std::tuple<Ts...> objects) noexcept : objects{std::move(objects)} {}
		
		template<injectable T> requires (... or std::same_as<T, Ts>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<tuple_source> auto&& source) -> T {
			return std::get<T>(KANGARU5_FWD(source).objects);
		}
		
	private:
		std::tuple<Ts...> objects;
	};
	
	KANGARU5_EXPORT template<callable F> requires (unqualified_object<F> and std::move_constructible<F> and injectable<detail::type_traits::call_result_t<F>>)
	struct function_source {
		explicit constexpr function_source(F function) noexcept : function{std::move(function)} {}
		
		template<forwarded<function_source> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> detail::type_traits::call_result_t<detail::utility::forward_like_t<Self, F>> {
			return KANGARU5_FWD(source).function();
		}
		
	private:
		F function;
	};
	
	KANGARU5_EXPORT template<unqualified_object T>
	struct object_source {
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr object_source(From&& object) noexcept : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires constructor_callable<T, Args...>
		constexpr object_source(Args... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
		
		template<unqualified_object U>
		friend auto attribute(allow_empty_injection<object_source<U>>) -> std::true_type;
	};
	
	KANGARU5_EXPORT template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	object_source(T&&) -> object_source<std::decay_t<T>>;
	
	KANGARU5_EXPORT template<object T>
	struct rvalue_source {
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr rvalue_source(From&& object) noexcept : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires constructor_callable<T, Args...>
		constexpr rvalue_source(Args... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr auto provide() & -> T&& {
			return std::move(object);
		}
		
		constexpr auto provide() && -> T&& {
			return std::move(object);
		}
		
	private:
		T object;
		
		template<kangaru::object U>
		friend auto attribute(allow_empty_injection<rvalue_source<U>>) -> std::true_type;
	};
	
	KANGARU5_EXPORT template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	rvalue_source(T&&) -> rvalue_source<std::decay_t<T>>;
	
	KANGARU5_EXPORT template<object T>
	struct reference_source {
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr reference_source(From&& object) noexcept : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires constructor_callable<T, Args...>
		constexpr reference_source(Args... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr auto provide() & -> T& {
			return object;
		}
		
		constexpr auto provide() && -> T& {
			return object;
		}
		
	private:
		T object;
		
		template<kangaru::object U>
		friend auto attribute(allow_empty_injection<reference_source<U>>) -> std::true_type;
	};
	
	KANGARU5_EXPORT template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	reference_source(T&&) -> reference_source<std::decay_t<T>>;
	
	KANGARU5_EXPORT template<object T>
	struct external_rvalue_source {
		explicit constexpr external_rvalue_source(T&& reference) noexcept : reference{std::addressof(reference)} {}
		
		constexpr auto provide() const& -> T&& {
			return std::move(*reference);
		}
		
	private:
		T* reference;
	};
	
	KANGARU5_EXPORT template<object T>
	struct external_reference_source {
		explicit constexpr external_reference_source(T& reference) noexcept : reference{std::addressof(reference)} {}
		
		constexpr auto provide() const& -> T& {
			return *reference;
		}
		
	private:
		T* reference;
	};
	
	KANGARU5_EXPORT template<source Source, source Alternative>
	struct with_alternative {
		constexpr with_alternative(Source source, Alternative alternative) noexcept : source{std::move(source)}, alternative{std::move(alternative)} {}
		
		Source source;
		
		template<injectable T, forwarded<with_alternative> Self> requires (not wrapping_source_of<Self, T> and source_of<detail::utility::forward_like_t<Self, Alternative&&>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).alternative);
		}
		
		template<injectable T, forwarded<with_alternative> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<forwarded<with_alternative> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept -> with_alternative<wrapped_source_rebind_result_t<Original, NewLeaf>, fwd_ref_result_t<detail::utility::forward_like_t<Original, Alternative>>> {
			return with_alternative<wrapped_source_rebind_result_t<Original, NewLeaf>, fwd_ref_result_t<detail::utility::forward_like_t<Original, Alternative>>>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_NO_ADL(fwd_ref)(maybe_unwrap(KANGARU5_FWD(original).alternative))
			};
		}
		
		Alternative alternative;
	};
	
	KANGARU5_EXPORT template<forwarded_source Wrapped, forwarded_source Alternative>
	inline constexpr auto make_source_with_alternative(Wrapped&& wrapped, Alternative&& alternative) {
		return with_alternative<std::decay_t<Wrapped>, std::decay_t<Alternative>>{KANGARU5_FWD(wrapped), KANGARU5_FWD(alternative)};
	}
	
	KANGARU5_EXPORT template<source Source, injectable Type>
	struct filter_source {
		constexpr filter_source(Source source) noexcept : source{std::move(source)} {}
		
	private:
		template<different_from<Type> T> requires injectable<T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<filter_source> auto const& source) -> T {
			return kangaru::provide<T>(source.source);
		}
		
		Source source;
	};
	
	KANGARU5_EXPORT template<source Source, std::default_initializable Filter>
	struct filter_if_source {
		explicit constexpr filter_if_source(Source source) noexcept : source{std::move(source)} {}
		constexpr filter_if_source(Source source, Filter) noexcept : source{std::move(source)} {}
		
	private:
		template<injectable T> requires(Filter{}.template operator()<T>())
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<filter_if_source> auto const& source) -> T {
			return kangaru::provide<T>(source.source);
		}
		
		Source source;
	};
	
	KANGARU5_EXPORT template<std::default_initializable Filter, forwarded_source Source>
	inline constexpr auto filter(Source&& source) {
		return filter_source<std::decay_t<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto filter(Source&& source, std::default_initializable auto filter) {
		return filter_source<std::decay_t<Source>, decltype(filter)>{KANGARU5_FWD(source), filter};
	}
	
	KANGARU5_EXPORT template<forwarded_source Source, std::default_initializable Filter>
	inline constexpr auto filter_if(Source&& source, [[maybe_unused]] Filter filter) {
		return filter_if_source<std::decay_t<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<source Source>
	struct with_passthrough {
		explicit constexpr with_passthrough(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<injectable T, forwarded<with_passthrough> Self>
			requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<with_passthrough> Self>
			requires(not wrapping_source_of<Self, T> and wrapping_source_of<wrapped_source_t<Self>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source.source);
		}
	};
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_passthrough(Source&& source) noexcept {
		return with_passthrough<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<source Source>
	struct with_dereference {
		explicit constexpr with_dereference(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<reference T> requires source_of<Source, std::remove_reference_t<T>*>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<with_dereference> auto&& source) -> T {
			return *kangaru::provide<std::remove_reference_t<T>*>(KANGARU5_FWD(source).source);
		}
		
		template<object T> requires (not std::is_pointer_v<T> and source_of<Source, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<with_dereference> auto&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
	};
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_dereference(Source&& source) {
		return with_dereference<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	// TODO: Create singular source to deduce the from?
	KANGARU5_EXPORT template<source Source, injectable From> requires source_of<Source, From>
	struct with_cast_from {
		Source source;
		
		template<injectable T, forwarded<with_cast_from> Self> requires(wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<different_from<From> T, forwarded<with_cast_from> Self> requires(injectable<T> and not wrapping_source_of<Self, T> and safe_convertible_to<From, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			decltype(auto) result = kangaru::provide<From>(KANGARU5_FWD(source).source);
			return static_cast<T>(result);
		}
		
		template<kangaru::source S, injectable F>
		friend auto attribute(overrides_types_in_cache<with_cast_from<S, F>>) -> overrides_types_in_cache<S>;
	};
	
	KANGARU5_EXPORT template<injectable From>
	inline constexpr auto make_source_with_cast_from(forwarded_source auto&& source) noexcept -> with_cast_from<std::decay_t<decltype(source)>, From> {
		return with_cast_from<std::decay_t<decltype(source)>, From>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<source Source>
	struct with_source_wrapping {
		Source source;
		
		template<wrapping_source T, forwarded<with_source_wrapping> Self> requires wrapping_source_of<Self, wrapped_source_t<T>>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return T{kangaru::provide<wrapped_source_t<T>>(KANGARU5_FWD(source).source)};
		}
	};
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_source_wrapping(Source&& source) {
		return with_source_wrapping<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<source Source>
	struct basic_wrapping_source {
		template<injectable T, forwarded<basic_wrapping_source> Self> requires source_of<wrapped_source_t<Self>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto wrap_source(Source&& source) {
		return basic_wrapping_source<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT inline constexpr auto tie(source auto&... sources) {
		return KANGARU5_NO_ADL(compose)(KANGARU5_NO_ADL(ref)(sources)...);
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TYPES_HPP
