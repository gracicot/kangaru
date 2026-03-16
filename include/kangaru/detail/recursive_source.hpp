#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "utility.hpp"
#include "constructor.hpp"
#include "injector.hpp"
#include "attributes.hpp"
#include "source.hpp"
#include "source_types.hpp"
#include "source_reference_wrapper.hpp"
#include "source_rebind.hpp"
#include "type_traits.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <utility>
#include <concepts>
#endif

#include "define.hpp"

namespace kangaru {
	namespace detail::recursive_source::file_private {
		// TODO: Can we break the circular dependency by having an artificial check that takes the current deducing type?
		template<movable_object... Functions>
		struct select_function {
			explicit(sizeof...(Functions) == 1) constexpr select_function(Functions... funcs) noexcept : functions{std::move(funcs)...} {}
			
			template<injectable T, forwarded_source Source>
				requires (((callable_template_1t_returns<T, Functions&, T, Source&&> ? 1 : 0) + ... + 0) == 1)
			constexpr auto operator()(Source&& source) & -> T {
				constexpr auto index = index_of<T, select_function&, decltype(source)>(std::index_sequence_for<Functions...>{});
				return std::get<index>(functions).template operator()<T>(KANGARU5_FWD(source));
			}
			
			template<injectable T, forwarded_source Source>
				requires (((callable_template_1t_returns<T, Functions const&, T, Source&&> ? 1 : 0) + ... + 0) == 1)
			constexpr auto operator()(Source&& source) const& -> T {
				constexpr auto index = index_of<T, select_function const&, decltype(source)>(std::index_sequence_for<Functions...>{});
				return std::get<index>(functions).template operator()<T>(KANGARU5_FWD(source));
			}
			
			template<injectable T, forwarded_source Source>
				requires (((callable_template_1t_returns<T, Functions&&, T, Source&&> ? 1 : 0) + ... + 0) == 1)
			constexpr auto operator()(Source&& source) && -> T {
				constexpr auto index = index_of<T, select_function&&, decltype(source)>(std::index_sequence_for<Functions...>{});
				return std::get<index>(std::move(functions)).template operator()<T>(KANGARU5_FWD(source));
			}
			
			template<injectable T, forwarded_source Source>
				requires (((callable_template_1t_returns<T, Functions const&&, T, Source&&> ? 1 : 0) + ... + 0) == 1)
			constexpr auto operator()(Source&& source) const&& -> T {
				constexpr auto index = index_of<T, select_function const&&, decltype(source)>(std::index_sequence_for<Functions...>{});
				return std::get<index>(std::move(functions)).template operator()<T>(KANGARU5_FWD(source));
			}
			
			template<injectable T, forwarded_source Source> requires (
				"Ambiguous resolution, multiple callable functions can return type T",
				((callable_template_1t_returns<T, Functions&, T, Source&&> ? 1 : 0) + ... + 0) > 1
			)
			constexpr auto operator()(Source&& source) & -> T = delete;
			
			template<injectable T, forwarded_source Source> requires (
				"Ambiguous resolution, multiple callable functions can return type T",
				((callable_template_1t_returns<T, Functions const&, T, Source&&> ? 1 : 0) + ... + 0) > 1
			)
			constexpr auto operator()(Source&& source) const& -> T = delete;
			
			template<injectable T, forwarded_source Source> requires (
				"Ambiguous resolution, multiple callable functions can return type T",
				((callable_template_1t_returns<T, Functions&&, T, Source&&> ? 1 : 0) + ... + 0) > 1
			)
			constexpr auto operator()(Source&& source) && -> T = delete;
			
			template<injectable T, forwarded_source Source> requires (
				"Ambiguous resolution, multiple callable functions can return type T",
				((callable_template_1t_returns<T, Functions const&&, T, Source&&> ? 1 : 0) + ... + 0) > 1
			)
			constexpr auto operator()(Source&& source) const&& -> T = delete;
			
		private:
			template<typename T, typename Self, typename Source, std::size_t... S>
			constexpr static auto index_of(std::index_sequence<S...>) {
				return ((callable_template_1t_returns<T, detail::utility::forward_like_t<Self, Functions>, T, Source&&> ? S : 0) + ... + 0);
			}
			
			std::tuple<Functions...> functions;
		};
		
		template<kangaru::source Source, kangaru::function_object... Functions>
		struct with_function_call_experiment {
			explicit(sizeof...(Functions) == 0) constexpr with_function_call_experiment(Source source, Functions... functions) :
				source{std::move(source)}, functions{std::move(functions)...} {}
			
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
			
			template<forwarded<with_function_call_experiment> Original, forwarded_source NewLeaf>
				requires(
					std::constructible_from<
						std::tuple<Functions...>,
						detail::utility::forward_like_t<Original, std::tuple<Functions...>>
					>
				)
			static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept ->
				with_function_call_experiment<wrapped_source_rebind_result_t<Original, NewLeaf>, Functions...>
			{
				return std::apply(
					[&](auto&&... functions) {
						return with_function_call_experiment<wrapped_source_rebind_result_t<Original, NewLeaf>, Functions...> {
							kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
							KANGARU5_FWD(functions)...
						};
					},
					KANGARU5_FWD(original).functions
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
	}
	
	KANGARU5_EXPORT template<copiable_object MakeInjector>
	struct basic_non_empty_construction {
		constexpr basic_non_empty_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_non_empty_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<detail::type_traits::call_result_t<MakeInjector const&, Source>, non_default_constructor_function<T>>
		constexpr auto operator()(Source&& source) const -> T {
			return make_injector(KANGARU5_FWD(source))(non_default_constructor_function<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	KANGARU5_EXPORT using non_empty_construction = basic_non_empty_construction<make_spread_injector_function>;
	
	KANGARU5_EXPORT template<copiable_object MakeInjector>
	struct basic_unsafe_exhaustive_construction {
		constexpr basic_unsafe_exhaustive_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_unsafe_exhaustive_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<detail::type_traits::call_result_t<MakeInjector const&, Source>, constructor_function<T>>
		constexpr auto operator()(Source&& source) const -> T {
			return make_injector(KANGARU5_FWD(source))(constructor_function<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	KANGARU5_EXPORT using unsafe_exhaustive_construction = basic_unsafe_exhaustive_construction<make_spread_injector_function>;
	
	KANGARU5_EXPORT template<copiable_object Construction, second_step_function SecondStep>
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
		Construction construction;
		SecondStep second_step;
	};
	
	KANGARU5_EXPORT template<copiable_object Construction, second_step_function SecondStep = noop_second_step>
	inline constexpr auto make_construction_with_two_step_init(Construction const& construction, SecondStep const& second_step) {
		return construction_with_two_step_init<Construction, SecondStep>{KANGARU5_FWD(construction), second_step};
	}
	
	KANGARU5_EXPORT template<copiable_object MakeInjector>
	struct basic_exhaustive_construction {
	private:
		template<unqualified_object T>
		using constructor_type = detail::type_traits::conditional_t<
			allow_empty_injection_v<T>,
			constructor_function<T>,
			non_default_constructor_function<T>
		>;
		
	public:
		constexpr basic_exhaustive_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_exhaustive_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<detail::type_traits::call_result_t<MakeInjector const&, Source&&>, constructor_type<T>>
		constexpr auto operator()(Source&& source) const -> T {
			return make_injector(KANGARU5_FWD(source))(constructor_type<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	KANGARU5_EXPORT using exhaustive_construction = basic_exhaustive_construction<make_spread_injector_function>;
	
	KANGARU5_EXPORT template<injectable Type, copiable_object MakeInjector>
	struct basic_placeholder_except_construction {
		KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_except_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_except_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<injectable T, forwarded_source Source>
			requires(different_from<T, Type> and callable<detail::type_traits::call_result_t<MakeInjector const&, Source>, constructor_function<std::decay_t<T>>>)
		auto operator()(Source&& source) const -> T;
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	KANGARU5_EXPORT template<injectable Type>
	using placeholder_except_construction = basic_placeholder_except_construction<Type, make_spread_injector_function>;
	
	KANGARU5_EXPORT template<copiable_object MakeInjector>
	struct basic_placeholder_construction {
		KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<injectable T, forwarded_source Source>
			requires callable<detail::type_traits::call_result_t<MakeInjector const&, Source>, constructor_function<std::decay_t<T>>>
		auto operator()(Source&& source) const -> T;
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	KANGARU5_EXPORT using placeholder_construction = basic_placeholder_construction<make_spread_injector_function>;
	
	KANGARU5_EXPORT template<source Source, function_object... Functions>
	struct with_function_call {
	private:
		using overloaded_function = detail::recursive_source::file_private::select_function<Functions...>;
		
	public:
		explicit constexpr with_function_call(Source source) noexcept requires(
			sizeof...(Functions) > 0 and (true and ... and std::default_initializable<Functions>)
		) :
			source{std::move(source)} {}
		
		constexpr with_function_call(Source source, Functions... functions) noexcept :
			source{std::move(source)},
			functions{std::move(functions)...} {}
		
		template<injectable T, forwarded<with_function_call> Self>
			requires (callable_template_1t_returns<T, overloaded_function const&, T, wrapped_source_t<Self>>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return std::as_const(source.functions).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_function_call> Original, forwarded_source NewLeaf>
			requires std::constructible_from<overloaded_function, detail::utility::forward_like_t<Original, overloaded_function>>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> with_function_call<wrapped_source_rebind_result_t<Original, NewLeaf>, Functions...>
		{
			return with_function_call<wrapped_source_rebind_result_t<Original, NewLeaf>, Functions...>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_FWD(original).functions,
			};
		}
		
		Source source;
		
	private:
		template<kangaru::source S, function_object... F>
		friend struct with_function_call;
		
		constexpr with_function_call(Source source, overloaded_function functions) noexcept :
			source{std::move(source)},
			functions{std::move(functions)} {}
		
		overloaded_function functions;
	};
	
	KANGARU5_EXPORT template<forwarded_source Source, forwarded_function_object... Functions>
	inline constexpr auto make_source_with_function_call(Source&& source, Functions&&... functions) {
		return with_function_call<std::decay_t<Source>, std::decay_t<Functions>...>{KANGARU5_FWD(source), KANGARU5_FWD(functions)...};
	}
	
	KANGARU5_EXPORT template<source Source, source Passthrough, copiable_object Constructor>
	struct with_construction_original_passthrough {
		explicit constexpr with_construction_original_passthrough(Source source) noexcept
				requires std::default_initializable<Constructor> :
			source{std::move(source)},
			passthrough{},
			construction{} {}
		
		constexpr with_construction_original_passthrough(Source source, Passthrough passthrough) noexcept
				requires (std::default_initializable<Passthrough> and std::default_initializable<Constructor>) :
			source{std::move(source)},
			passthrough{std::move(passthrough)},
			construction{std::move(construction)} {}
		
		constexpr with_construction_original_passthrough(Source source, Passthrough passthrough, Constructor construction) noexcept :
			source{std::move(source)},
			passthrough{std::move(passthrough)},
			construction{std::move(construction)} {}
		
		template<injectable T, forwarded<with_construction_original_passthrough> Self>
			requires (
				source_of<detail::utility::forward_like_t<Self, Passthrough&&>, T>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).passthrough);
		}
		
		template<unqualified_object T, forwarded<with_construction_original_passthrough> Self>
			requires (
				    not source_of<detail::utility::forward_like_t<Self, Passthrough&&>, T>
				and callable_template_1t<Constructor const&, T, wrapped_source_t<Self>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return std::as_const(source.construction).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_construction_original_passthrough> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> with_construction_original_passthrough<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Passthrough>&>, Constructor>
		{
			return with_construction_original_passthrough<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Passthrough>&>, Constructor>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_NO_ADL(ref)(original.passthrough),
				KANGARU5_FWD(original).construction
			};
		}
		
		Source source;
		
	private:
		Passthrough passthrough;
		Constructor construction;
	};
	
	KANGARU5_EXPORT template<source Source, copiable_object Constructor>
	struct with_construction {
		explicit constexpr with_construction(Source source) noexcept
			requires std::default_initializable<Constructor> :
			source{std::move(source)} {}
		
		constexpr with_construction(Source source, Constructor construction) noexcept :
			source{std::move(source)},
			construction{std::move(construction)} {}
		
		template<injectable T, forwarded<with_construction> Self>
			requires (
				wrapping_source_of<Self, T>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<unqualified_object T, forwarded<with_construction> Self>
			requires (callable_template_1t<Constructor const&, T, wrapped_source_t<Self>> and not wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return std::as_const(source.construction).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_construction> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> with_construction_original_passthrough<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Source>&>, Constructor>
		{
			return with_construction_original_passthrough<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<detail::utility::forward_like_t<Original, Source>&>, Constructor>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_NO_ADL(ref)(original.source),
				KANGARU5_FWD(original).construction
			};
		}
		
		Source source;
		
	private:
		Constructor construction;
	};
	
	KANGARU5_EXPORT template<source Source>
	using with_non_empty_construction = with_construction<Source, non_empty_construction>;
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_non_empty_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, non_empty_construction>{KANGARU5_FWD(source), non_empty_construction{}};
	}
	
	KANGARU5_EXPORT template<source Source>
	using with_exhaustive_construction = with_construction<Source, exhaustive_construction>;
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_exhaustive_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, exhaustive_construction>{KANGARU5_FWD(source), exhaustive_construction{}};
	}
	
	KANGARU5_EXPORT template<source Source>
	using with_unsafe_exhaustive_construction = with_construction<Source, unsafe_exhaustive_construction>;
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_unsafe_exhaustive_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, unsafe_exhaustive_construction>{KANGARU5_FWD(source), unsafe_exhaustive_construction{}};
	}

	KANGARU5_EXPORT template<source Source>
	using with_non_empty_two_step_construction = with_construction<Source, construction_with_two_step_init<non_empty_construction, second_step_from_attribute>>;
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_non_empty_two_step_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, construction_with_two_step_init<non_empty_construction, second_step_from_attribute>>{KANGARU5_FWD(source), construction_with_two_step_init{non_empty_construction{}, second_step_from_attribute{}}};
	}
	
	KANGARU5_EXPORT template<source Source>
	using with_exhaustive_two_step_construction = with_construction<Source, construction_with_two_step_init<exhaustive_construction, second_step_from_attribute>>;
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_exhaustive_two_step_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, construction_with_two_step_init<exhaustive_construction, second_step_from_attribute>>{KANGARU5_FWD(source), construction_with_two_step_init{exhaustive_construction{}, second_step_from_attribute{}}};
	}
	
	KANGARU5_EXPORT template<source Source>
	using with_unsafe_exhaustive_two_step_construction = with_construction<Source, construction_with_two_step_init<unsafe_exhaustive_construction, second_step_from_attribute>>;
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_unsafe_exhaustive_two_step_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, construction_with_two_step_init<unsafe_exhaustive_construction, second_step_from_attribute>>{KANGARU5_FWD(source), construction_with_two_step_init{unsafe_exhaustive_construction{}, second_step_from_attribute{}}};
	}
	
	namespace detail::recursive_source {
		template<kangaru::source Alternative>
		struct leaf_as_alternative {
			Alternative alternative;
			
			constexpr auto operator()(forwarded_source auto&& leaf) const noexcept {
				return KANGARU5_NO_ADL(make_source_with_alternative)(
					KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(leaf)),
					alternative
				);
			}
		};
	}
	
	KANGARU5_EXPORT template<rebindable_source Source>
	struct with_recursion {
		Source source;
		
		template<injectable T, forwarded<with_recursion> Self> requires (not wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T requires(
			source_of<
				wrapped_source_rebind_result_t<
					Self&&,
					detail::recursive_source::leaf_as_alternative<with_recursion<fwd_ref_result_t<forwarded_wrapped_source_t<Self>&&>>>
				>,
				T
			>
		) {
			return kangaru::provide<T>(
				kangaru::rebind(
					source,
					detail::recursive_source::leaf_as_alternative<with_recursion<fwd_ref_result_t<forwarded_wrapped_source_t<Self>&&>>>{
						KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source)
					}
				).source
			);
		}
		
		template<injectable T, forwarded<with_recursion> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
	};
	
	// This deduction guide is required for clang 16 to work
	KANGARU5_EXPORT template<typename Source>
	with_recursion(Source const& source) -> with_recursion<Source>;
	
	KANGARU5_EXPORT template<forwarded_source Source>
	inline constexpr auto make_source_with_recursion(Source&& source) {
		return with_recursion<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<typename Tree, typename Type>
	concept construction_tree_needs = injectable<Tree> and injectable<Type> and not source_of<
		with_recursion<
			with_function_call<
				none_source,
				basic_placeholder_except_construction<Type, make_strict_spread_injector_function>
			>
		>,
		Tree
	>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
