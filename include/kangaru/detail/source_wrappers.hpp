#ifndef KANGARU5_DETAIL_SOURCE_WRAPPERS_HPP
#define KANGARU5_DETAIL_SOURCE_WRAPPERS_HPP

#include "source.hpp"
#include "deducer.hpp"
#include "source_reference_wrapper.hpp"
#include "source_traits.hpp"
#include "two_step_init.hpp"

#ifndef KANGARU5_MODULES
#include <tuple>
#include <concepts>
#include <type_traits>
#include <cstddef>
#include <utility>
#endif

#include "define.hpp"

namespace kangaru {
	template<source Source, source Alternative>
	struct with_alternative {
		Source source;
		Alternative alternative;
		
		template<injectable T, forwarded<with_alternative> Self> requires (not wrapping_source_of<Self, T> and source_of<detail::forward_like_t<Self, Alternative&&>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).alternative);
		}
		
		template<injectable T, forwarded<with_alternative> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<forwarded<with_alternative> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_alternative<deduced_source_type<NewSource>, fwd_ref_result_t<detail::forward_like_t<Original, Alternative>>>
		{
			return with_alternative<deduced_source_type<NewSource>, fwd_ref_result_t<detail::forward_like_t<Original, Alternative>>>{
				KANGARU5_FWD(new_source),
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(original).alternative),
			};
		}
	};
	
	template<typename Source, typename Alternative>
		requires(not deducer<std::remove_cvref_t<Source>> and not deducer<std::remove_cvref_t<Alternative>>)
	with_alternative(Source&&, Alternative&&) -> with_alternative<deduced_source_type<Source>, deduced_source_type<Alternative>>;
	
	template<forwarded_source Wrapped, forwarded_source Alternative>
	inline constexpr auto make_source_with_alternative(Wrapped&& wrapped, Alternative&& alternative) {
		return with_alternative<deduced_source_type<Wrapped>, deduced_source_type<Alternative>>{KANGARU5_FWD(wrapped), KANGARU5_FWD(alternative)};
	}
	
	template<source Source, injectable... Types>
	struct with_filter {
		template<injectable T, forwarded<with_filter> Self>
			requires((... and different_from<Types, T>) and wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}

		template<forwarded<with_filter> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_filter<deduced_source_type<NewSource>, Types...>
		{
			return with_filter<deduced_source_type<NewSource>, Types...>{
				KANGARU5_FWD(new_source),
			};
		}
		
		Source source;
	};
	
	template<source Source, type_predicate Filter>
	struct with_filter_if {
		template<allows_construction_of<Source> S>
		explicit constexpr with_filter_if(S&& source) : source(KANGARU5_FWD(source)) {}
		
		template<allows_construction_of<Source> S>
		constexpr with_filter_if(S&& source, Filter) : source(KANGARU5_FWD(source)) {}
		
		template<injectable T, forwarded<with_filter_if> Self>
			requires(Filter{}.template operator()<T>() and source_of<detail::forward_like_t<Self, Source>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<typename Source, typename Filter>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_filter_if(Source&&, Filter const&) -> with_filter_if<deduced_source_type<Source>, Filter>;
	
	template<injectable... Ts, forwarded_source Source>
	inline constexpr auto make_source_with_filter(Source&& source) {
		return with_filter<deduced_source_type<Source>, Ts...>{KANGARU5_FWD(source)};
	}
	
	template<forwarded_source Source, std::default_initializable Filter>
	inline constexpr auto make_source_with_filter_if(Source&& source, [[maybe_unused]] Filter filter) {
		return with_filter_if<deduced_source_type<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
	template<std::size_t level, source Source>
	struct with_passthrough {
		Source source;
		
		template<forwarded<with_passthrough> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_passthrough<level, deduced_source_type<NewSource>>
		{
			return with_passthrough<level, deduced_source_type<NewSource>>{
				KANGARU5_FWD(new_source),
			};
		}
		
		template<injectable T, forwarded<with_passthrough> Self>
			requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<with_passthrough> Self>
			requires(
				    not wrapping_source_of<Self, T>
				and source_of<get_nested_wrapped_source_t<level, forwarded_wrapped_source_t<Self>>, T>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_NO_ADL(get_nested_wrapped_source<level>)(KANGARU5_FWD(source).source));
		}
	};
	
	template<std::size_t level, forwarded_source Source>
	inline constexpr auto make_source_with_passthrough(Source&& source) {
		return with_passthrough<level, deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_dereference {
		Source source;
		
		template<reference T, forwarded<with_dereference> Self> requires wrapping_source_of<Self, std::remove_reference_t<T>*>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return *kangaru::provide<std::remove_reference_t<T>*>(KANGARU5_FWD(source).source);
		}
		
		template<object T, forwarded<with_dereference> Self> requires (not pointer<T> and wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<with_dereference> auto&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	with_dereference(T&&) -> with_dereference<deduced_source_type<T>>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_dereference(Source&& source) {
		return with_dereference<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source, injectable... From>
	struct with_cast_from {
		Source source;
		
		template<injectable T, forwarded<with_cast_from> Self> requires(wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<with_cast_from> Self>
			requires(
				not wrapping_source_of<Self, T> and
				((
					    different_from<T, From>
					and safe_convertible_to<From, T>
					and wrapping_source_of<Self, From> ? 1 : 0
				) + ... + 0) == 1
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			constexpr auto index = index_of<T>(std::index_sequence_for<From...>{});
			using F = std::tuple_element_t<index, std::tuple<From...>>;
			return static_cast<T>(kangaru::provide<F>(KANGARU5_FWD(source).source));
		}
		
	private:
		template<typename T, std::size_t... S>
		static constexpr auto index_of(std::index_sequence<S...>) {
			return ((different_from<T, From> and safe_convertible_to<From, T> ? S : 0) + ... + 0);
		}
		
		template<kangaru::source S, injectable F>
		friend auto attribute(overrides_types_in_cache<with_cast_from<S, F>>) -> overrides_types_in_cache<S>;
		
		template<kangaru::source S, injectable F>
		friend auto attribute(second_step_init<with_cast_from<S, F>>) -> call_second_step_from_attribute_on_wrapped_source;
	};
	
	template<injectable From>
	inline constexpr auto make_source_with_cast_from(forwarded_source auto&& source) -> with_cast_from<deduced_source_type<decltype(source)>, From> {
		return with_cast_from<deduced_source_type<decltype(source)>, From>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_source_wrapping {
		Source source;
		
		template<wrapping_source T, forwarded<with_source_wrapping> Self>
			requires(
				    wrapping_source_of<Self, wrapped_source_t<T>>
				and std::constructible_from<T, wrapped_source_t<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return T(kangaru::provide<wrapped_source_t<T>>(KANGARU5_FWD(source).source));
		}
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	with_source_wrapping(T&&) -> with_source_wrapping<deduced_source_type<T>>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_source_wrapping(Source&& source) {
		return with_source_wrapping<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source, template<typename> typename SourceFor>
	struct with_provide_using_source {
		template<injectable T, forwarded<with_provide_using_source> Self>
			requires (
				// We prevent instanciation of this function with T as a SourceFor<...> to prevent
				// recursive constaint evaluation
				    not detail::is_specialisation_of_v<SourceFor, T>
				and requires { typename SourceFor<T>; }
				and source_of<SourceFor<T>, T>
				and wrapping_source_of<Self, SourceFor<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			decltype(auto) source_for_t = kangaru::provide<SourceFor<T>>(KANGARU5_FWD(source).source);
			return kangaru::provide<T>(KANGARU5_FWD(source_for_t));
		}
		
		Source source;
	};
	
	template<template<typename> typename SourceFor>
	inline constexpr auto make_source_with_provide_using_source(forwarded_source auto&& source) {
		return with_provide_using_source<deduced_source_type<decltype(source)>, SourceFor>{KANGARU5_FWD(source)};
	}
	
	template<source Source, function_object Transform>
	struct with_transformed_source {
		template<allows_construction_of<Source> S>
			requires(std::default_initializable<Transform>)
		explicit constexpr with_transformed_source(S&& source) : source(KANGARU5_FWD(source)), transform{} {}
		
		template<allows_construction_of<Source> S>
		constexpr with_transformed_source(S&& source, Transform transform) : source(KANGARU5_FWD(source)), transform(std::move(transform)) {}
		
		template<injectable T, forwarded<with_transformed_source> Self>
			requires(source_of<detail::call_result_t<Transform const&, forwarded_wrapped_source_t<Self>>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(std::as_const(source.transform)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_transformed_source> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_transformed_source<deduced_source_type<NewSource>, Transform>
		{
			return with_transformed_source<deduced_source_type<NewSource>, Transform>{
				KANGARU5_FWD(new_source),
				KANGARU5_FWD(original).transform,
			};
		}
		
		Source source;
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Transform transform;
	};
	
	template<typename T, typename Transform> requires(not deducer<std::remove_cvref_t<T>>)
	with_transformed_source(T&&, Transform const&) -> with_transformed_source<deduced_source_type<T>, Transform>;
	
	template<forwarded_source Source, forwarded_function_object Transform>
	inline constexpr auto make_source_with_transformed_source(Source&& source, Transform&& transform) {
		return with_transformed_source<deduced_source_type<Source>, std::decay_t<Transform>>{KANGARU5_FWD(source), KANGARU5_FWD(transform)};
	}
	
	template<source Source>
	struct sealed_source {
		template<allows_construction_of<Source> S>
		explicit constexpr sealed_source(S&& source) : source(KANGARU5_FWD(source)) {}
		
		template<injectable T, forwarded<sealed_source> Self>
			requires(source_of<detail::forward_like_t<Self, Source&&>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		constexpr auto wrapped_source() & -> Source& {
			return source;
		}
		
		constexpr auto wrapped_source() && -> Source&& {
			return std::move(source);
		}
		
		constexpr auto wrapped_source() const& -> Source const& {
			return source;
		}
		
		constexpr auto wrapped_source() const&& -> Source const&& {
			return std::move(source);
		}
		
	private:
		Source source;
	};
	
	template<typename Source>
		requires(not deducer<std::remove_cvref_t<Source>>)
	sealed_source(Source&&) -> sealed_source<deduced_source_type<Source>>;
	
	template<forwarded_source Source>
	inline constexpr auto seal_source(Source&& source) {
		return sealed_source<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct basic_wrapping_source {
		template<injectable T, forwarded<basic_wrapping_source> Self>
			requires(wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	basic_wrapping_source(T&&) -> basic_wrapping_source<deduced_source_type<T>>;
	
	template<forwarded_source Source>
	inline constexpr auto wrap_source(Source&& source) {
		return basic_wrapping_source<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_WRAPPERS_HPP
