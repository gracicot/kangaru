#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "concepts.hpp"
#include "utility.hpp"
#include "constructor.hpp"
#include "injector.hpp"
#include "attributes.hpp"
#include "source.hpp"
#include "source_types.hpp"
#include "source_wrappers.hpp"
#include "source_reference_wrapper.hpp"
#include "source_rebind.hpp"
#include "type_traits.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <utility>
#include <concepts>
#endif

#include "define.hpp"

// TODO: Consider no unique address

namespace kangaru::detail::recursive_source_private {
	// TODO: Can we break the circular dependency by having an artificial check that takes the current deducing type?
	
	template<source Source, function_object... Functions>
	struct with_function_call_experiment {
		explicit(sizeof...(Functions) == 0) constexpr with_function_call_experiment(Source source, Functions... functions) :
			source(std::move(source)), functions{std::move(functions)...} {}
		
		template<kangaru::source T, forwarded<with_function_call_experiment> Self> requires((... + (callable_returns<T, Functions&, ref_result_t<Source&>> ? 1 : 0)) == 1)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			constexpr auto index = index_for<T>(std::index_sequence_for<Functions...>{});
			auto& function = std::get<index>(source.functions);
			return function(KANGARU5_NO_ADL(ref)(source.source));
		}
		
		template<kangaru::source T, forwarded<with_function_call_experiment> Self> requires("Ambiguous source resolution: One or more callable returns source T",
			(... + (callable_returns<T, Functions&, ref_result_t<Source&>> ? 1 : 0)) > 1
		)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T = delete;
		
		template<forwarded<with_function_call_experiment> Original, forwarded_source NewSource>
			requires(... and std::copy_constructible<Functions>)
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_function_call_experiment<deduced_source_type<NewSource>, Functions...>
		{
			return std::apply(
				[&](Functions const&... functions) {
					return with_function_call_experiment<deduced_source_type<NewSource>, Functions...> {
						KANGARU5_FWD(new_source),
						functions...,
					};
				},
				original.functions
			);
		}
		
		Source source;
		
	private:
		template<kangaru::source T, std::size_t... S>
		static constexpr auto index_for(std::index_sequence<S...>) -> std::size_t {
			return (... + (callable_returns<T, Functions&, ref_result_t<Source&>> ? S : 0));
		}
		
		std::tuple<Functions...> functions;
	};
	
	template<source Alternative>
	struct leaf_as_alternative {
		Alternative alternative;
		
		template<forwarded_source Leaf>
		constexpr auto operator()(Leaf&& leaf) const noexcept -> with_alternative<fwd_ref_result_t<Leaf&&>, Alternative> {
			return KANGARU5_NO_ADL(make_source_with_alternative)(
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(leaf)),
				std::move(alternative)
			);
		}
	};
}

KANGARU5_EXPORT namespace kangaru {
	template<typename F>
	concept construction = copiable_object<F> and function_object<F>;
	
	template<make_injector MakeInjector>
	struct basic_non_empty_construction {
		constexpr basic_non_empty_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_non_empty_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<detail::call_result_t<MakeInjector const&, Source&&>, non_default_constructor_function<T>>
		constexpr auto operator()(Source&& source) const -> T {
			return make_injector(KANGARU5_FWD(source))(non_default_constructor_function<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<construction Construction, construction Alternative>
	struct construct_with_alternative {
		template<unqualified_object T, forwarded_source Source>
			requires(callable_template_1t_returns<T, Construction const&, T, Source&&>)
		constexpr auto operator()(Source&& source) const -> T {
			return construction.template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<unqualified_object T, forwarded_source Source>
			requires(
				    not callable_template_1t_returns<T, Construction const&, T, Source&&>
				and callable_template_1t_returns<T, Alternative const&, T, Source&&>
			)
		constexpr auto operator()(Source&& source) const -> T {
			return alternative.template operator()<T>(KANGARU5_FWD(source));
		}
		
		Construction construction;
		Alternative alternative;
	};
	
	template<function_object... Functions>
	struct select_function_for_type {
		explicit(sizeof...(Functions) == 1) constexpr select_function_for_type(Functions... funcs) noexcept : functions{std::move(funcs)...} {}
		
		template<injectable T, forwarded_source Source>
			requires(((callable_template_1t_returns<T, Functions&, T, Source&&> ? 1 : 0) + ... + 0) == 1)
		constexpr auto operator()(Source&& source) & -> T {
			constexpr auto index = index_of<T, select_function_for_type&, decltype(source)>(std::index_sequence_for<Functions...>{});
			return std::get<index>(functions).template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source>
			requires(((callable_template_1t_returns<T, Functions const&, T, Source&&> ? 1 : 0) + ... + 0) == 1)
		constexpr auto operator()(Source&& source) const& -> T {
			constexpr auto index = index_of<T, select_function_for_type const&, decltype(source)>(std::index_sequence_for<Functions...>{});
			return std::get<index>(functions).template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source>
			requires(((callable_template_1t_returns<T, Functions&&, T, Source&&> ? 1 : 0) + ... + 0) == 1)
		constexpr auto operator()(Source&& source) && -> T {
			constexpr auto index = index_of<T, select_function_for_type&&, decltype(source)>(std::index_sequence_for<Functions...>{});
			return std::get<index>(std::move(functions)).template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source>
			requires(((callable_template_1t_returns<T, Functions const&&, T, Source&&> ? 1 : 0) + ... + 0) == 1)
		constexpr auto operator()(Source&& source) const&& -> T {
			constexpr auto index = index_of<T, select_function_for_type const&&, decltype(source)>(std::index_sequence_for<Functions...>{});
			return std::get<index>(std::move(functions)).template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source> requires(
			"Ambiguous resolution, multiple callable functions can return type T",
			((callable_template_1t_returns<T, Functions&, T, Source&&> ? 1 : 0) + ... + 0) > 1
		)
		constexpr auto operator()(Source&& source) & -> T = delete;
		
		template<injectable T, forwarded_source Source> requires(
			"Ambiguous resolution, multiple callable functions can return type T",
			((callable_template_1t_returns<T, Functions const&, T, Source&&> ? 1 : 0) + ... + 0) > 1
		)
		constexpr auto operator()(Source&& source) const& -> T = delete;
		
		template<injectable T, forwarded_source Source> requires(
			"Ambiguous resolution, multiple callable functions can return type T",
			((callable_template_1t_returns<T, Functions&&, T, Source&&> ? 1 : 0) + ... + 0) > 1
		)
		constexpr auto operator()(Source&& source) && -> T = delete;
		
		template<injectable T, forwarded_source Source> requires(
			"Ambiguous resolution, multiple callable functions can return type T",
			((callable_template_1t_returns<T, Functions const&&, T, Source&&> ? 1 : 0) + ... + 0) > 1
		)
		constexpr auto operator()(Source&& source) const&& -> T = delete;
		
	private:
		template<typename T, typename Self, typename Source, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			return ((callable_template_1t_returns<T, detail::forward_like_t<Self, Functions>, T, Source&&> ? S : 0) + ... + 0);
		}
		
		std::tuple<Functions...> functions;
	};
	
	using non_empty_construction = basic_non_empty_construction<make_spread_injector_function>;
	
	template<make_injector MakeInjector>
	struct basic_unsafe_exhaustive_construction {
		constexpr basic_unsafe_exhaustive_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_unsafe_exhaustive_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<detail::call_result_t<MakeInjector const&, Source&&>, constructor_function<T>>
		constexpr auto operator()(Source&& source) const -> T {
			return make_injector(KANGARU5_FWD(source))(constructor_function<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using unsafe_exhaustive_construction = basic_unsafe_exhaustive_construction<make_spread_injector_function>;
	using unsafe_exhaustive_strict_construction = basic_unsafe_exhaustive_construction<make_strict_spread_injector_function>;
	
	template<construction Construction, second_step_function SecondStep>
	struct construction_with_two_step_init {
		constexpr construction_with_two_step_init()
			requires(std::default_initializable<Construction> and std::default_initializable<SecondStep>) = default;
		
		explicit constexpr construction_with_two_step_init(Construction construction) noexcept
			requires(std::default_initializable<SecondStep>) :
			construction{std::move(construction)} {}
		
		constexpr construction_with_two_step_init(Construction construction, SecondStep second_step) noexcept :
			construction{std::move(construction)},
			second_step{std::move(second_step)} {}
		
		template<injectable T, forwarded_source Source>
			requires(
				    callable_template_1t_returns<T, Construction, T, Source&&>
				and callable_template_1t<SecondStep, T, T&, Source&&>
			)
		constexpr auto operator()(Source&& source) const -> T {
			decltype(auto) result = construction.template operator()<T>(KANGARU5_FWD(source));
			void(second_step.template operator()<T>(result, KANGARU5_FWD(source)));
			return result;
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		SecondStep second_step;
	};
	
	template<construction Construction, second_step_function SecondStep>
	inline constexpr auto make_construction_with_two_step_init(Construction const& construction, SecondStep const& second_step) {
		return construction_with_two_step_init<Construction, SecondStep>{construction, second_step};
	}
	
	template<construction Construction, second_step_function SecondStep, type_predicate SecondStepIf>
	struct construction_with_two_step_init_if {
		constexpr construction_with_two_step_init_if()
			requires(std::default_initializable<Construction> and std::default_initializable<SecondStep>) = default;
		
		explicit constexpr construction_with_two_step_init_if(Construction construction) noexcept
			requires(std::default_initializable<SecondStep>) :
			construction{std::move(construction)} {}
		
		constexpr construction_with_two_step_init_if(Construction construction, SecondStep second_step) noexcept :
			construction{std::move(construction)},
			second_step{std::move(second_step)} {}
		
		template<injectable T, forwarded_source Source>
			requires(
				    callable_template_1t_returns<T, Construction, T, Source&&>
				and (
					   (SecondStepIf{}.template operator()<T>() and callable_template_1t<SecondStep, T, T&, Source&&>)
					or not SecondStepIf{}.template operator()<T>()
				)
			)
		constexpr auto operator()(Source&& source) const -> T {
			if constexpr (SecondStepIf{}.template operator()<T>()) {
				static_assert(callable_template_1t<SecondStep, T, T&, Source&&>);
				// NOTE: This function requires the move constructor of T to be declared but not defined.
				//       Is there a way around that and guarantee RVO?
				decltype(auto) result = construction.template operator()<T>(KANGARU5_FWD(source));
				void(second_step.template operator()<T>(result, KANGARU5_FWD(source)));
				return result;
			} else {
				return construction.template operator()<T>(KANGARU5_FWD(source));
			}
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		SecondStep second_step;
	};
	
	template<type_predicate SecondStepIf, construction Construction, second_step_function SecondStep>
	inline constexpr auto make_construction_with_two_step_init_if(Construction const& construction, SecondStep const& second_step) {
		return construction_with_two_step_init_if<Construction, SecondStep, SecondStepIf>{construction, second_step};
	}
	
	template<construction Construction, second_step_function SecondStep, type_predicate SecondStepIf>
	inline constexpr auto make_construction_with_two_step_init_if(Construction const& construction, SecondStep const& second_step, SecondStepIf const&) {
		return construction_with_two_step_init_if<Construction, SecondStep, SecondStepIf>{construction, second_step};
	}
	
	template<construction Construction>
	struct construction_with_unique_ptr {
	private:
		template<typename>
		struct tag {};
		
		template<injectable T, forwarded_source Source>
			requires(
				callable_template_1t_returns<T, Construction, T, Source&&>
			)
		auto construct(tag<T>, Source&& source) const -> T {
			return construction.template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<object T, forwarded_source Source>
			requires(
				    in_place_constructible<std::remove_const_t<T>>
				and not callable_template_1t_returns<std::unique_ptr<T>, Construction, std::unique_ptr<T>, Source&&>
				and callable_template_1t_returns<std::remove_const_t<T>, Construction, std::remove_const_t<T>, Source&&>
			)
		auto construct(tag<std::unique_ptr<T>>, Source&& source) const -> std::unique_ptr<T> {
			return std::make_unique<T>(in_place_construct{[&]{
				return construction.template operator()<std::remove_const_t<T>>(KANGARU5_FWD(source));
			}});
		}
		
	public:
		constexpr construction_with_unique_ptr()
			requires(std::default_initializable<Construction>) = default;
		
		explicit constexpr construction_with_unique_ptr(Construction construction) noexcept :
			construction{std::move(construction)} {}
		
		template<injectable T, forwarded_source Source>
		constexpr auto operator()(Source&& source) const -> decltype(construct(tag<T>{}, KANGARU5_FWD(source))) {
			return construct(tag<T>{}, KANGARU5_FWD(source));
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
	};
	
	template<construction Construction>
	inline constexpr auto make_construction_with_unique_ptr(Construction const& construction) {
		return construction_with_unique_ptr<Construction>{construction};
	}
	
	template<make_injector MakeInjector>
	struct basic_exhaustive_construction {
	private:
		template<unqualified_object T>
		using constructor_type = detail::conditional_t<
			allow_empty_injection_v<T>,
			constructor_function<T>,
			non_default_constructor_function<T>
		>;
		
	public:
		constexpr basic_exhaustive_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_exhaustive_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<detail::call_result_t<MakeInjector const&, Source&&>, constructor_type<T>>
		constexpr auto operator()(Source&& source) const -> T {
			return make_injector(KANGARU5_FWD(source))(constructor_type<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using exhaustive_construction = basic_exhaustive_construction<make_spread_injector_function>;
	using exhaustive_strict_construction = basic_exhaustive_construction<make_strict_spread_injector_function>;
	
	template<injectable Type, make_injector MakeInjector>
	struct basic_placeholder_except_construction {
		KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_except_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_except_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<injectable T, forwarded_source Source>
			requires(different_from<T, Type> and callable<detail::call_result_t<MakeInjector const&, Source&&>, constructor_function<std::decay_t<T>>>)
		auto operator()(Source&& source) const -> T;
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<injectable Type>
	using placeholder_except_construction = basic_placeholder_except_construction<Type, make_spread_injector_function>;
	
	template<injectable Type>
	using placeholder_except_strict_construction = basic_placeholder_except_construction<Type, make_strict_spread_injector_function>;
	
	template<make_injector MakeInjector>
	struct basic_placeholder_construction {
		KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<injectable T, forwarded_source Source>
			requires callable<detail::call_result_t<MakeInjector const&, Source&&>, constructor_function<std::decay_t<T>>>
		auto operator()(Source&& source) const -> T;
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using placeholder_construction = basic_placeholder_construction<make_spread_injector_function>;
	using placeholder_strict_construction = basic_placeholder_construction<make_strict_spread_injector_function>;
	
	template<source Source, function_object... Functions>
	struct with_function_call {
	private:
		using overloaded_function = select_function_for_type<Functions...>;
		
	public:
		template<allows_construction_of<Source> S>
		explicit constexpr with_function_call(S&& source) noexcept requires(
			sizeof...(Functions) > 0 and (true and ... and std::default_initializable<Functions>)
		) :
			source(KANGARU5_FWD(source)) {}
		
		template<allows_construction_of<Source> S>
		constexpr with_function_call(S&& source, Functions... functions) noexcept :
			source(KANGARU5_FWD(source)),
			functions{std::move(functions)...} {}
		
		template<injectable T, forwarded<with_function_call> Self>
			requires(callable_template_1t_returns<T, overloaded_function const&, T, fwd_ref_result_t<forwarded_wrapped_source_t<Self>>>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return std::as_const(source.functions).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_function_call> Original, forwarded_source NewSource>
			requires(std::copy_constructible<overloaded_function>)
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_function_call<deduced_source_type<NewSource>, Functions...>
		{
			return with_function_call<deduced_source_type<NewSource>, Functions...>{
				KANGARU5_FWD(new_source),
				std::as_const(original).functions,
			};
		}
		
		Source source;
		
	private:
		template<kangaru::source S, function_object... F>
		friend struct with_function_call;
		
		template<allows_construction_of<Source> S>
		constexpr with_function_call(S&& source, overloaded_function functions) noexcept :
			source(KANGARU5_FWD(source)),
			functions{std::move(functions)} {}
		
		overloaded_function functions;
	};
	
	template<typename Source, typename... Functions>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_function_call(Source&&, Functions const&...) -> with_function_call<deduced_source_type<Source>, Functions...>;
	
	template<forwarded_source Source, forwarded_function_object... Functions>
	inline constexpr auto make_source_with_function_call(Source&& source, Functions&&... functions) {
		return with_function_call<deduced_source_type<Source>, std::decay_t<Functions>...>{KANGARU5_FWD(source), KANGARU5_FWD(functions)...};
	}
	
	template<source Source, source Passthrough, construction Construction>
	struct with_construction_original_passthrough {
		template<allows_construction_of<Source> S>
		explicit constexpr with_construction_original_passthrough(S&& source) noexcept
				requires std::default_initializable<Construction> :
			source(KANGARU5_FWD(source)),
			passthrough{},
			construction{} {}
		
		template<allows_construction_of<Source> S>
		constexpr with_construction_original_passthrough(S&& source, Passthrough passthrough) noexcept
				requires(std::default_initializable<Passthrough> and std::default_initializable<Construction>) :
			source(KANGARU5_FWD(source)),
			passthrough{std::move(passthrough)},
			construction{std::move(construction)} {}
		
		template<allows_construction_of<Source> S>
		constexpr with_construction_original_passthrough(S&& source, Passthrough passthrough, Construction construction) noexcept :
			source(KANGARU5_FWD(source)),
			passthrough{std::move(passthrough)},
			construction{std::move(construction)} {}
		
		template<injectable T, forwarded<with_construction_original_passthrough> Self>
			requires(
				source_of<detail::forward_like_t<Self, Passthrough&&>, T>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).passthrough);
		}
		
		template<unqualified_object T, forwarded<with_construction_original_passthrough> Self>
			requires(
				    not source_of<detail::forward_like_t<Self, Passthrough&&>, T>
				and callable_template_1t<Construction const&, T, fwd_ref_result_t<forwarded_wrapped_source_t<Self>>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return std::as_const(source.construction).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_construction_original_passthrough> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_construction_original_passthrough<deduced_source_type<NewSource>, fwd_ref_result_t<detail::forward_like_t<Original, Passthrough>&&>, Construction>
		{
			return with_construction_original_passthrough<deduced_source_type<NewSource>, fwd_ref_result_t<detail::forward_like_t<Original, Passthrough>&&>, Construction>{
				KANGARU5_FWD(new_source),
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(original).passthrough),
				std::as_const(original).construction,
			};
		}
		
		Source source;
		
	private:
		Passthrough passthrough;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
	};
	
	template<typename Source, typename Passthrough, typename Construction>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_construction_original_passthrough(Source&&, Passthrough const&, Construction const&) ->
		with_construction_original_passthrough<deduced_source_type<Source>, Passthrough, Construction>;
	
	template<source Source, construction Construction>
	struct with_construction {
		template<allows_construction_of<Source> S>
		explicit constexpr with_construction(S&& source) noexcept
			requires std::default_initializable<Construction> :
			source(KANGARU5_FWD(source)) {}
		
		template<allows_construction_of<Source> S>
		constexpr with_construction(S&& source, Construction construction) noexcept :
			source(KANGARU5_FWD(source)),
			construction{std::move(construction)} {}
		
		template<injectable T, forwarded<with_construction> Self>
			requires(
				wrapping_source_of<Self, T>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<unqualified_object T, forwarded<with_construction> Self>
			requires(not wrapping_source_of<Self, T> and callable_template_1t<Construction const&, T, fwd_ref_result_t<forwarded_wrapped_source_t<Self>>>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return std::as_const(source.construction).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_construction> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_construction_original_passthrough<deduced_source_type<NewSource>, fwd_ref_result_t<detail::forward_like_t<Original, Source>&&>, Construction>
		{
			return with_construction_original_passthrough<deduced_source_type<NewSource>, fwd_ref_result_t<detail::forward_like_t<Original, Source>&&>, Construction>{
				KANGARU5_FWD(new_source),
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(original).source),
				std::as_const(original).construction,
			};
		}
		
		Source source;
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
	};
	
	template<typename Source, typename Construction>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_construction(Source&&, Construction const&) ->
		with_construction<deduced_source_type<Source>, Construction>;
	
	template<forwarded_source Source, construction Construction>
	inline constexpr auto make_source_with_construction(Source&& source, Construction construction) {
		return with_construction<deduced_source_type<Source>, Construction>{KANGARU5_FWD(source), std::move(construction)};
	}
	
	template<source Source>
	using with_non_empty_construction = with_construction<Source, non_empty_construction>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_non_empty_construction(Source&& source) {
		return with_construction<deduced_source_type<Source>, non_empty_construction>{KANGARU5_FWD(source), non_empty_construction{}};
	}
	
	template<source Source>
	using with_exhaustive_construction = with_construction<Source, exhaustive_construction>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_exhaustive_construction(Source&& source) {
		return with_construction<deduced_source_type<Source>, exhaustive_construction>{KANGARU5_FWD(source), exhaustive_construction{}};
	}
	
	template<source Source>
	using with_unsafe_exhaustive_construction = with_construction<Source, unsafe_exhaustive_construction>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_unsafe_exhaustive_construction(Source&& source) {
		return with_construction<deduced_source_type<Source>, unsafe_exhaustive_construction>{KANGARU5_FWD(source), unsafe_exhaustive_construction{}};
	}
	
	template<source Source>
	using with_two_step_construction = with_construction<
		Source,
		construction_with_two_step_init<non_empty_construction, second_step_from_attribute>
	>;
	
	template<forwarded_source Source, construction Construction>
	inline constexpr auto make_source_with_two_step_construction(Source&& source, Construction construction) {
		return with_construction<
			deduced_source_type<Source>,
			construction_with_two_step_init<Construction, second_step_from_attribute>
		>{
			KANGARU5_FWD(source),
			construction_with_two_step_init{std::move(construction), second_step_from_attribute{}}
		};
	}
	
	template<source Source>
	using with_non_empty_two_step_construction = with_construction<
		Source,
		construction_with_two_step_init<non_empty_construction, second_step_from_attribute>
	>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_non_empty_two_step_construction(Source&& source) {
		return with_construction<
			deduced_source_type<Source>,
			construction_with_two_step_init<non_empty_construction, second_step_from_attribute>
		>{
			KANGARU5_FWD(source),
			construction_with_two_step_init{non_empty_construction{}, second_step_from_attribute{}}
		};
	}
	
	template<source Source>
	using with_exhaustive_two_step_construction = with_construction<
		Source,
		construction_with_two_step_init<exhaustive_construction, second_step_from_attribute>
	>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_exhaustive_two_step_construction(Source&& source) {
		return with_construction<
			deduced_source_type<Source>,
			construction_with_two_step_init<exhaustive_construction, second_step_from_attribute>
		>{
			KANGARU5_FWD(source),
			construction_with_two_step_init{exhaustive_construction{}, second_step_from_attribute{}}
		};
	}
	
	template<source Source>
	using with_unsafe_exhaustive_two_step_construction = with_construction<
		Source,
		construction_with_two_step_init<unsafe_exhaustive_construction, second_step_from_attribute>
	>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_unsafe_exhaustive_two_step_construction(Source&& source) {
		return with_construction<
			deduced_source_type<Source>,
			construction_with_two_step_init<unsafe_exhaustive_construction, second_step_from_attribute>
		>{
			KANGARU5_FWD(source),
			construction_with_two_step_init{unsafe_exhaustive_construction{}, second_step_from_attribute{}}
		};
	}
	
	template<rebindable_source Source>
	struct with_recursion {
		Source source;
		
		template<injectable T, forwarded<with_recursion> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T requires(
			   wrapping_source_of<Self, T>
			or source_of<
				wrapped_source_rebind_result_t<
					Self&&,
					detail::recursive_source_private::leaf_as_alternative<with_recursion<fwd_ref_result_t<forwarded_wrapped_source_t<Self>&&>>>
				>,
				T
			>
		) {
			return kangaru::provide<T>(
				kangaru::rebind(
					KANGARU5_FWD(source).source,
					detail::recursive_source_private::leaf_as_alternative<with_recursion<fwd_ref_result_t<forwarded_wrapped_source_t<Self>&&>>>{
						KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source)
					}
				)
			);
		}
	};
	
	template<typename Source>
		requires(not deducer<std::remove_cvref_t<Source>>)
	with_recursion(Source&& source) -> with_recursion<deduced_source_type<Source>>;
	
	template<forwarded_rebindable_source Source>
	inline constexpr auto make_source_with_recursion(Source&& source) {
		return with_recursion<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<typename Tree, typename Type>
	concept construction_tree_needs = injectable<Tree> and injectable<Type> and not source_of<
		with_recursion<
			with_function_call<
				none_source,
				placeholder_except_strict_construction<Type>
			>
		>,
		Tree
	>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
