#ifndef KANGARU_DETAIL_DEDUCER_HPP
#define KANGARU_DETAIL_DEDUCER_HPP

#include "concepts.hpp"
#include "source.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

#include <utility>
#include <type_traits>

#include "define.hpp"

struct Scene;
struct Camera;

namespace kangaru {
	struct kangaru_deducer_tag {};
	
	template<typename Deducer>
	concept deducer =
		    detail::concepts::object<Deducer>
		and requires {
			requires std::same_as<typename Deducer::is_deducer, kangaru_deducer_tag>;
		};
	
	template<typename Deducer, typename T>
	concept deducer_for = deducer<Deducer> and requires(Deducer deducer) {
		{ deducer.operator T() } -> std::same_as<T>;
	};

	template<typename T>
	concept deducible = detail::concepts::prvalue<T> and not deducer<T>;
	
	template<typename T, typename Source>
	concept deducible_prvalue = deducible<T> and source_of<Source, T>;
	
	template<typename T, typename Source>
	concept deducible_lvalue = deducible<T> and source_of<Source, T&>;
	
	template<typename T, typename Source>
	concept deducible_const_lvalue =
		    deducible<T>
		and (
			   source_of<Source, T const&>
			or source_of<Source, T&>
		);

	template<typename T, typename Source>
	concept deducible_rvalue = deducible<T> and source_of<Source, T&&>;
	
	template<typename T, typename Source>
	concept deducible_const_rvalue =
		    deducible<T>
		and (
			   source_of<Source, T const&&>
			or source_of<Source, T&&>
		);
	
	struct ambiguous_prvalue_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		template<deducible T>
		operator T () const;
		
		template<deducible T>
		operator T& () const;
		
		template<deducible T>
		operator T const& () const;
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
		
		explicit constexpr basic_deducer(detail::concepts::forwarded<std::remove_reference_t<Source>> auto&& source) noexcept :
			source{std::addressof(source)} {}
		
		template<deducible_prvalue<Source> T>
		constexpr operator T() {
			return provide(provide_tag<T>, static_cast<Source&&>(*source));
		}

		template<deducible_lvalue<Source> T>
		constexpr operator T&() const {
			return provide(provide_tag<T&>, static_cast<Source&&>(*source));
		}

		template<deducible_const_lvalue<Source> T>
		constexpr operator T const&() const {
			if constexpr (source_of<Source, T const&>) {
				return provide(provide_tag<T const&>, static_cast<Source&&>(*source));
			} else {
				return std::as_const(provide(provide_tag<T&>, static_cast<Source&&>(*source)));
			}
		}

		template<deducible_rvalue<Source> T>
		constexpr operator T&&() const {
			return provide(provide_tag<T&&>, static_cast<Source&&>(*source));
		}
		
		template<deducible_const_rvalue<Source> T>
		constexpr operator T const&&() const {
			if constexpr (source_of<Source, T const&&>) {
				return provide(provide_tag<T const&&>, static_cast<Source&&>(*source));
			} else {
				return provide(provide_tag<T&&>, static_cast<Source&&>(*source));
			}
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
			requires (detail::concepts::different_from<Exclude, T> and deducer_for<Deducer, T>)
		constexpr operator T() {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires (detail::concepts::different_from<Exclude, T&> and deducer_for<Deducer const, T&>)
		constexpr operator T&() const {
			return deducer.operator T&();
		}
		
		template<deducible T>
			requires (detail::concepts::different_from<Exclude, T const&> and deducer_for<Deducer const, T const&>)
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires (detail::concepts::different_from<Exclude, T&&> and deducer_for<Deducer const, T&&>)
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires (detail::concepts::different_from<Exclude, T const&&> and deducer_for<Deducer const, T const&&>)
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
			requires (detail::concepts::different_from<Exclude, T> and deducer_for<Deducer, T>)
		constexpr operator T() {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires (detail::concepts::different_from<Exclude, T> and deducer_for<Deducer const, T&>)
		constexpr operator T&() const {
			return deducer.operator T&();
		}

		template<deducible T>
			requires (detail::concepts::different_from<Exclude, T> and deducer_for<Deducer const, T const&>)
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires (detail::concepts::different_from<Exclude, T> and deducer_for<Deducer const, T&&>)
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires (detail::concepts::different_from<Exclude, T> and deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() const {
			return deducer.operator T const&&();
		}
		
	private:
		Deducer deducer;
	};
	
	template<deducer Deducer>
	struct exclude_prvalue_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr exclude_prvalue_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<deducible T>
			requires deducer_for<Deducer const, T&>
		constexpr operator T&() const {
			return deducer.operator T&();
		}

		template<deducible T>
			requires deducer_for<Deducer const, T const&>
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires deducer_for<Deducer const, T&&>
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires deducer_for<Deducer const, T const&&>
		constexpr operator T const&&() const {
			return deducer.operator T const&&();
		}
		
	private:
		Deducer deducer;
	};
	
	template<deducer Deducer>
	struct exclude_references_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr exclude_references_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<deducible T>
			requires deducer_for<Deducer, T>
		constexpr operator T() {
			return deducer.operator T();
		}
		
	private:
		Deducer deducer;
	};
	
	template<deducible T>
	inline constexpr auto exclude_special_constructors_for(deducer auto deducer) {
		return exclude_special_constructors_deducer<T, decltype(deducer)>{deducer};
	}
	
	namespace detail::deducer {
		// These traits and function are only required for compilers that always prefer prvalue
		template<typename, typename, typename, typename>
		inline constexpr bool callbale_with_nth_parameter_being_expand = false;
		
		template<typename S, typename F, std::size_t... before, std::size_t... after>
		inline constexpr bool callbale_with_nth_parameter_being_expand<
			S, F,
			std::index_sequence<before...>,
			std::index_sequence<after...>
		> = detail::concepts::callable<
			F,
			detail::utility::expand<placeholder_deducer, before>...,
			S,
			detail::utility::expand<placeholder_deducer, after>...
		>;
		
		template<typename S, typename F, std::size_t nth, std::size_t max>
		concept callbale_with_nth_parameter_being = callbale_with_nth_parameter_being_expand<
			S, F,
			std::make_index_sequence<nth>,
			std::make_index_sequence<max - nth - 1>
		>;
		
		template<typename F, kangaru::deducer... Deducers, std::size_t... S>
		inline constexpr auto invoke_with_deducers_prvalue_filter(F&& function, std::index_sequence<S...>, Deducers... deduce) -> decltype(auto) requires detail::concepts::callable<F, Deducers...> {
			return KANGARU5_FWD(function)(
				detail::type_traits::conditional_t<
					    callbale_with_nth_parameter_being<ambiguous_prvalue_deducer, F, S, sizeof...(S)>
					and callbale_with_nth_parameter_being<exclude_prvalue_deducer<Deducers>, F, S, sizeof...(S)>,
					exclude_prvalue_deducer<Deducers>,
					exclude_references_deducer<Deducers>
				>{deduce}...
			);
		}
	}
	
	template<typename F, deducer... Deducers>
	inline constexpr auto invoke_with_deducers(F&& function, Deducers... deduce) -> decltype(auto) requires detail::concepts::callable<F, Deducers...> {
		#if NEEDS_PRVALUE_PREPASS == 1
			return detail::deducer::invoke_with_deducers_prvalue_filter(KANGARU5_FWD(function), std::index_sequence_for<Deducers...>{}, deduce...);
		#else
			return KANGARU5_FWD(function)(deduce...);
		#endif
	}
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU_DETAIL_DEDUCER_HPP
