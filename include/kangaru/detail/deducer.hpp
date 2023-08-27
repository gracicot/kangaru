#ifndef KANGARU_DETAIL_DEDUCER_HPP
#define KANGARU_DETAIL_DEDUCER_HPP

#include "concepts.hpp"
#include "source.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

#include <utility>
#include <type_traits>

#include "define.hpp"

namespace kangaru {
	struct kangaru_deducer_tag {};
	
	template<typename Deducer>
	concept deducer =
		    object<Deducer>
		and requires {
			requires std::same_as<typename Deducer::is_deducer, kangaru_deducer_tag>;
		};
	
	template<typename Deducer, typename T>
	concept deducer_for = deducer<Deducer> and requires(Deducer deducer) {
		{ deducer.operator T() } -> std::same_as<T>;
	};
	
	template<typename T>
	concept deducible = unqualified_object<T> and not deducer<T>;
	
	template<typename T, typename Source>
	concept deducible_lvalue = deducible<T> and source_of<Source, T&>;
	
	template<typename T, typename Source>
	concept deducible_const_lvalue = deducible<T> and (
		   source_of<Source, T const&>
		or source_of<Source, T&>
		or source_of<Source, T const&&>
		or source_of<Source, T&&>
	);
	
	template<typename T, typename Source>
	concept deducible_prvalue = deducible<T> and (source_of<Source, T> or deducible_const_lvalue<T, Source>);
	
	template<typename T, typename Source>
	concept deducible_rvalue = deducible<T> and source_of<Source, T&&>;
	
	template<typename T, typename Source>
	concept deducible_const_rvalue = deducible<T> and (source_of<Source, T const&&> or source_of<Source, T&&>);
	
	template<typename T, typename Source>
	concept deducible_strict_prvalue = deducible<T> and source_of<Source, T>;
	
	template<typename T, typename Source>
	concept deducible_strict_lvalue = deducible<T> and source_of<Source, T&>;
	
	template<typename T, typename Source>
	concept deducible_strict_const_lvalue = deducible<T> and source_of<Source, T const&>;
	
	template<typename T, typename Source>
	concept deducible_strict_rvalue = deducible<T> and source_of<Source, T&&>;
	
	template<typename T, typename Source>
	concept deducible_strict_const_rvalue = deducible<T> and source_of<Source, T const&&>;
	
	struct ambiguous_prvalue_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		template<deducible T>
		operator T () const;
		
		template<deducible T>
		operator T& () const;
		
		#if KANGARU5_LVALUE_CONST_AMBIGUOUS == 1
		template<deducible T>
		operator T const& () const;
		#endif
		
		#if KANGARU5_RVALUE_AMBIGUOUS == 1
		template<typename T>
		operator T&& () const;
		#endif
	};
	
	struct ambiguous_overloaded_reference_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		template<deducible T>
		operator T () const;
		
		template<deducible T>
		operator T& ();
		
		#if KANGARU5_LVALUE_CONST_AMBIGUOUS == 1
		template<deducible T>
		operator T const& ();
		#endif
		
		#if KANGARU5_RVALUE_AMBIGUOUS == 1
		template<typename T>
		operator T&& ();
		#endif
	};
	
	struct placeholder_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		template<deducible T>
		operator T ();
		
		template<deducible T>
		operator T& () const;
		
		template<deducible T>
		operator T const& () const;
		
		template<deducible T>
		operator T&& () const;
		
		template<deducible T>
		operator T const&& () const;
	};
	
	template<source_ref Source>
	struct basic_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr basic_deducer(forwarded<std::remove_cvref_t<Source>> auto const&& source)
		noexcept requires std::is_const_v<std::remove_reference_t<Source>> :
			source{std::addressof(source)} {}
		
		explicit constexpr basic_deducer(forwarded<std::remove_cvref_t<Source>> auto&& source) noexcept :
			source{std::addressof(source)} {}
		
		template<deducible_prvalue<Source> T>
		constexpr operator T() {
			if constexpr (source_of<Source, T>) {
				return provide(provide_tag_v<T>, static_cast<Source&&>(*source));
			} else if constexpr (source_of<Source, T const&>) {
				return provide(provide_tag_v<T const&>, static_cast<Source&&>(*source));
			} else if constexpr (source_of<Source, T&>) {
				return std::as_const(provide(provide_tag_v<T&>, static_cast<Source&&>(*source)));
			} else if constexpr (source_of<Source, T const&&>) {
				return static_cast<T const&>(provide(provide_tag_v<T const&&>, static_cast<Source&&>(*source)));
			} else {
				return static_cast<T const&>(provide(provide_tag_v<T&&>, static_cast<Source&&>(*source)));
			}
		}
		
		template<deducible_lvalue<Source> T>
		constexpr operator T&() const {
			return provide(provide_tag_v<T&>, static_cast<Source&&>(*source));
		}
		
		template<deducible_const_lvalue<Source> T>
		constexpr operator T const&() const {
			if constexpr (source_of<Source, T const&>) {
				return provide(provide_tag_v<T const&>, static_cast<Source&&>(*source));
			} else if constexpr (source_of<Source, T&>) {
				return std::as_const(provide(provide_tag_v<T&>, static_cast<Source&&>(*source)));
			} else if constexpr (source_of<Source, T const&&>) {
				return static_cast<T const&>(provide(provide_tag_v<T const&&>, static_cast<Source&&>(*source)));
			} else {
				return static_cast<T const&>(provide(provide_tag_v<T&&>, static_cast<Source&&>(*source)));
			}
		}
		
		template<deducible_rvalue<Source> T>
		constexpr operator T&&() const {
			return provide(provide_tag_v<T&&>, static_cast<Source&&>(*source));
		}
		
		template<deducible_const_rvalue<Source> T>
		constexpr operator T const&&() const {
			if constexpr (source_of<Source, T const&&>) {
				return provide(provide_tag_v<T const&&>, static_cast<Source&&>(*source));
			} else {
				return provide(provide_tag_v<T&&>, static_cast<Source&&>(*source));
			}
		}
		
	private:
		std::remove_reference_t<Source>* source;
	};
	
	template<source_ref Source>
	struct strict_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr strict_deducer(forwarded<std::remove_cvref_t<Source>> auto const&& source)
		noexcept requires std::is_const_v<std::remove_reference_t<Source>> :
			source{std::addressof(source)} {}
		
		explicit constexpr strict_deducer(forwarded<std::remove_cvref_t<Source>> auto&& source) noexcept :
			source{std::addressof(source)} {}
		
		template<deducible_strict_prvalue<Source> T>
		constexpr operator T() {
			return provide(provide_tag_v<T>, static_cast<Source&&>(*source));
		}
		
		template<deducible_strict_lvalue<Source> T>
		constexpr operator T&() const {
			return provide(provide_tag_v<T&>, static_cast<Source&&>(*source));
		}
		
		template<deducible_strict_const_lvalue<Source> T>
		constexpr operator T const&() const {
			return provide(provide_tag_v<T const&>, static_cast<Source&&>(*source));
		}
		
		template<deducible_strict_rvalue<Source> T>
		constexpr operator T&&() const {
			return provide(provide_tag_v<T&&>, static_cast<Source&&>(*source));
		}
		
		template<deducible_strict_const_rvalue<Source> T>
		constexpr operator T const&&() const {
			return provide(provide_tag_v<T const&&>, static_cast<Source&&>(*source));
		}
		
	private:
		std::remove_reference_t<Source>* source;
	};
	
	template<typename Exclude, deducer Deducer>
	struct exclude_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr exclude_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
			
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer, T> and not deducer_for<Deducer const, T>)
		constexpr operator T() {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T&> and deducer_for<Deducer, T&> and not deducer_for<Deducer const, T&>)
		constexpr operator T&() {
			return deducer.operator T&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T const&> and deducer_for<Deducer, T const&> and not deducer_for<Deducer const, T const&>)
		constexpr operator T const&() {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T&&> and deducer_for<Deducer, T&&> and not deducer_for<Deducer const, T&&>)
		constexpr operator T&&() {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T const&&> and deducer_for<Deducer, T const&&> and not deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() {
			return deducer.operator T const&&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T>)
		constexpr operator T() const {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T&> and deducer_for<Deducer const, T&>)
		constexpr operator T&() const {
			return deducer.operator T&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T const&> and deducer_for<Deducer const, T const&>)
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T&&> and deducer_for<Deducer const, T&&>)
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T const&&> and deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() const {
			return deducer.operator T const&&();
		}
		
	private:
		Deducer deducer;
	};
	
	template<typename T>
	inline constexpr auto exclude_deduction(deducer auto deducer) {
		return exclude_deducer<T, decltype(deducer)>{deducer};
	}
	
	template<typename Exclude, deducer Deducer>
	struct exclude_special_constructors_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr exclude_special_constructors_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer, T> and not deducer_for<Deducer const, T>)
		constexpr operator T() {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer, T&> and not deducer_for<Deducer const, T&>)
		constexpr operator T&() {
			return deducer.operator T&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer, T const&> and not deducer_for<Deducer const, T const&>)
		constexpr operator T const&() {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer, T&&> and not deducer_for<Deducer const, T&&>)
		constexpr operator T&&() {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer, T const&&> and not deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() {
			return deducer.operator T const&&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T>)
		constexpr operator T() const {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T&>)
		constexpr operator T&() const {
			return deducer.operator T&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T const&>)
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T&&>)
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() const {
			return deducer.operator T const&&();
		}
		
	private:
		Deducer deducer;
	};
	
	enum struct reference_kind : std::uint8_t {
		none = 0,
		lvalue_reference = 0b0001u,
		lvalue_const_reference = 0b0010u,
		rvalue_reference = 0b0100u,
		rvalue_const_reference = 0b1000u,
		lvalue_reference_and_lvalue_const_reference = 0b0011u,
		lvalue_reference_and_rvalue_reference = 0b0101u,
		lvalue_reference_and_rvalue_const_reference = 0b1001u,
		lvalue_const_reference_and_rvalue_reference = 0b0110u,
		lvalue_const_reference_and_rvalue_const_reference = 0b1010u,
		rvalue_reference_and_rvalue_const_reference = 0b1100u,
		all_except_lvalue_reference = 0b1110u,
		all_except_lvalue_const_reference = 0b1101u,
		all_except_rvalue_reference = 0b1011u,
		all_except_rvalue_const_reference = 0b0111u,
		all_reference_kind = 0b1111u
	};
	
	inline constexpr auto operator|(reference_kind lhs, reference_kind rhs) noexcept -> reference_kind {
		return static_cast<reference_kind>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
	}
	
	inline constexpr auto operator&(reference_kind lhs, reference_kind rhs) noexcept -> reference_kind {
		return static_cast<reference_kind>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
	}
	
	inline constexpr auto operator^(reference_kind lhs, reference_kind rhs) noexcept -> reference_kind {
		return static_cast<reference_kind>(static_cast<std::uint8_t>(lhs) ^ static_cast<std::uint8_t>(rhs));
	}
	
	inline constexpr auto operator~(reference_kind q) noexcept -> reference_kind {
		return static_cast<reference_kind>(~static_cast<std::uint8_t>(q) & 0b1111);
	}
	
	template<typename Deducer, reference_kind kind>
	struct filtered_value_category_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr filtered_value_category_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
			
		template<deducible T>
			requires (kind == reference_kind::none and deducer_for<Deducer, T> and not deducer_for<Deducer const, T>)
		constexpr operator T() {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires ((kind & reference_kind::lvalue_reference) != reference_kind::none and deducer_for<Deducer, T&> and not deducer_for<Deducer const, T&>)
		constexpr operator T&() {
			return deducer.operator T&();
		}
		
		template<deducible T>
			requires ((kind & reference_kind::lvalue_const_reference) != reference_kind::none and deducer_for<Deducer, T const&> and not deducer_for<Deducer const, T const&>)
		constexpr operator T const&() {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires ((kind & reference_kind::rvalue_reference) != reference_kind::none and deducer_for<Deducer, T&&> and not deducer_for<Deducer const, T&&>)
		constexpr operator T&&() {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires ((kind & reference_kind::rvalue_const_reference) != reference_kind::none and deducer_for<Deducer, T const&&> and not deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() {
			return deducer.operator T const&&();
		}
		
		template<deducible T>
			requires (kind == reference_kind::none and deducer_for<Deducer const, T>)
		constexpr operator T() const {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires ((kind & reference_kind::lvalue_reference) != reference_kind::none and deducer_for<Deducer const, T&>)
		constexpr operator T&() const {
			return deducer.operator T&();
		}
		
		template<deducible T>
			requires ((kind & reference_kind::lvalue_const_reference) != reference_kind::none and deducer_for<Deducer const, T const&>)
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires ((kind & reference_kind::rvalue_reference) != reference_kind::none and deducer_for<Deducer const, T&&>)
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires ((kind & reference_kind::rvalue_const_reference) != reference_kind::none and deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() const {
			return deducer.operator T const&&();
		}
		
	private:
		Deducer deducer;
	};
	
	template<typename Deducer>
	using exclude_prvalue_deducer = filtered_value_category_deducer<Deducer, reference_kind::all_reference_kind>;
	
	template<typename Deducer>
	using exclude_references_deducer = filtered_value_category_deducer<Deducer, reference_kind::none>;
	
	template<deducible T>
	inline constexpr auto exclude_special_constructors_for(deducer auto deducer) {
		return exclude_special_constructors_deducer<T, decltype(deducer)>{deducer};
	}
	
	namespace detail::deducer {
		// These traits and function are only required for compilers that always prefer prvalue
		template<typename T, typename F, std::size_t... before, std::size_t... after>
		inline constexpr auto callbale_with_nth_parameter_being_expand(std::index_sequence<before...>, std::index_sequence<after...>) -> bool {
			return callable<
				F,
				detail::utility::expand<placeholder_deducer, before>...,
				T,
				detail::utility::expand<placeholder_deducer, after>...
			>;
		}
		
		template<typename T, typename F, std::size_t nth, std::size_t max>
		inline constexpr auto callbale_with_nth_parameter_being() -> bool {
			return callbale_with_nth_parameter_being_expand<T, F>(std::make_index_sequence<nth>{}, std::make_index_sequence<max - nth - 1>{});
		}
		
		template<typename F, std::size_t nth, std::size_t max>
		inline constexpr auto is_nth_parameter_prvalue() -> bool {
			return (
				    not callbale_with_nth_parameter_being<ambiguous_prvalue_deducer, F, nth, max>()
				and callbale_with_nth_parameter_being<ambiguous_overloaded_reference_deducer, F, nth, max>()
			);
		}
		
		template<typename T, typename F, std::size_t nth, std::size_t max>
		inline constexpr auto reference_kind_for_nth_parameter() -> reference_kind {
			if constexpr (is_nth_parameter_prvalue<F, nth, max>()) {
				return reference_kind::none;
			} else {
				constexpr auto callbale_with_lvalue_const_ref = callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::lvalue_const_reference>, F, nth, max>();
				
				constexpr auto callbale_with_rvalue_const_ref = callbale_with_lvalue_const_ref
					? not callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::lvalue_const_reference_and_rvalue_const_reference>, F, nth, max>()
					: (
						callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::rvalue_const_reference>, F, nth, max>()
						or (
							callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::rvalue_reference>, F, nth, max>()
							and not callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::rvalue_reference_and_rvalue_const_reference>, F, nth, max>()
						)
					);
				
				constexpr auto callbale_with_lvalue_ref = callbale_with_lvalue_const_ref
					? not callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::lvalue_reference_and_lvalue_const_reference>, F, nth, max>()
					: callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::lvalue_reference>, F, nth, max>();
				
				constexpr auto callbale_with_rvalue_ref = callbale_with_rvalue_const_ref
					? not callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::rvalue_reference_and_rvalue_const_reference>, F, nth, max>()
					: callbale_with_lvalue_const_ref
						? not callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::lvalue_const_reference_and_rvalue_reference>, F, nth, max>()
						: callbale_with_nth_parameter_being<filtered_value_category_deducer<T, reference_kind::rvalue_reference>, F, nth, max>();
				
				return
					(callbale_with_lvalue_ref ? reference_kind::lvalue_reference : reference_kind::none)
					| (callbale_with_lvalue_const_ref ? reference_kind::lvalue_const_reference : reference_kind::none)
					| (callbale_with_rvalue_ref ? reference_kind::rvalue_reference : reference_kind::none)
					| (callbale_with_rvalue_const_ref ? reference_kind::rvalue_const_reference : reference_kind::none);
			}
		}
		
		template<typename F, typename Deducer, std::size_t nth, std::size_t arity>
		using filtered_value_category_deducer_for = filtered_value_category_deducer<
			Deducer,
			reference_kind_for_nth_parameter<placeholder_deducer, F, nth, arity>()
		>;
		
		template<typename F, typename Deducer, std::size_t nth, std::size_t arity>
		using prvalue_filtered_deducer_for = detail::type_traits::conditional_t<
			    not is_nth_parameter_prvalue<F, nth, arity>()
			and callbale_with_nth_parameter_being<exclude_prvalue_deducer<Deducer>, F, nth, arity>(),
			exclude_prvalue_deducer<Deducer>,
			exclude_references_deducer<Deducer>
		>;
		
		template<typename F, kangaru::deducer... Deducers, std::size_t... S>
		inline constexpr auto invoke_with_deducers_prvalue_filter(
			F&& function,
			std::index_sequence<S...>,
			Deducers... deduce
		) -> decltype(auto) requires callable<F, prvalue_filtered_deducer_for<F, Deducers, S, sizeof...(S)>...> {
			return KANGARU5_FWD(function)(prvalue_filtered_deducer_for<F, Deducers, S, sizeof...(S)>{deduce}...);
		}
		
		template<typename F, kangaru::deducer... Deducers, std::size_t... S>
		inline constexpr auto invoke_with_deducers_strict_reference(
			F&& function,
			std::index_sequence<S...>,
			Deducers... deduce
		)  -> decltype(auto) requires callable<F, filtered_value_category_deducer_for<F, Deducers, S, sizeof...(S)>...> {
			return KANGARU5_FWD(function)(filtered_value_category_deducer_for<F, Deducers, S, sizeof...(S)>{deduce}...);
		}
		
		template<std::size_t... S>
		inline constexpr auto invoke_with_deducer_sequence(std::index_sequence<S...>, auto&& function, kangaru::deducer auto deduce) -> decltype(invoke_with_deducers_prvalue_filter(KANGARU5_FWD(function), std::index_sequence<S...>{}, (void(S), deduce)...)) {
			return invoke_with_deducers_prvalue_filter(KANGARU5_FWD(function), std::index_sequence<S...>{}, (void(S), deduce)...);
		}
		
		template<std::size_t... S>
		inline constexpr auto invoke_with_strict_deducer_sequence(std::index_sequence<S...>, auto&& function, kangaru::deducer auto deduce) -> decltype(invoke_with_deducers_strict_reference(KANGARU5_FWD(function), std::index_sequence<S...>{}, (void(S), deduce)...)) {
			return invoke_with_deducers_strict_reference(KANGARU5_FWD(function), std::index_sequence<S...>{}, (void(S), deduce)...);
		}
	}
	
	template<deducer... Deducers>
	inline constexpr auto invoke_with_deducers(callable<Deducers...> auto&& function, Deducers... deduce) -> decltype(auto) {
		return detail::deducer::invoke_with_deducers_prvalue_filter(KANGARU5_FWD(function), std::index_sequence_for<Deducers...>{}, deduce...);
	}
	
	template<deducer... Deducers>
	inline constexpr auto invoke_with_strict_deducers(auto&& function, Deducers... deduce) -> decltype(detail::deducer::invoke_with_deducers_strict_reference(KANGARU5_FWD(function), std::index_sequence_for<Deducers...>{}, deduce...)) {
		return detail::deducer::invoke_with_deducers_strict_reference(KANGARU5_FWD(function), std::index_sequence_for<Deducers...>{}, deduce...);
	}
	
	template<typename F, typename... Deducers>
	concept callable_with_deducers =
		    (... and deducer<Deducers>)
		and callable<F, Deducers...>;
	
	template<typename F, typename... Deducers>
	concept callable_with_strict_deducers =
		    (... and deducer<Deducers>)
		and requires(F&& f, Deducers... deduce) {
			invoke_with_strict_deducers(KANGARU5_FWD(f), deduce...);
		};

} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU_DETAIL_DEDUCER_HPP
