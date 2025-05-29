#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "utility.hpp"
#include "constructor.hpp"
#include "injector.hpp"
#include "tag.hpp"
#include "source.hpp"
#include "source_types.hpp"
#include "source_reference_wrapper.hpp"
#include "source_rebind.hpp"

#include <type_traits>
#include <cstdlib>

#include "define.hpp"

namespace kangaru {
	template<movable_object MakeInjector>
	struct basic_non_empty_constructor {
		constexpr basic_non_empty_constructor() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_non_empty_constructor(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return KANGARU5_NO_ADL(constructor<T>)()(
					KANGARU5_NO_ADL(exclude_special_constructors_for<T>)(deduce1),
					KANGARU5_NO_ADL(exclude_deduction<T>)(deduce)...
				);
			}
		};
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using non_empty_constructor = basic_non_empty_constructor<make_spread_injector_function>;
	
	template<movable_object MakeInjector>
	struct basic_unsafe_exhaustive_constructor {
		constexpr basic_unsafe_exhaustive_constructor() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_unsafe_exhaustive_constructor(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return KANGARU5_NO_ADL(constructor<T>)()(
					KANGARU5_NO_ADL(exclude_special_constructors_for<T>)(deduce1),
					KANGARU5_NO_ADL(exclude_deduction<T>)(deduce)...
				);
			}
			
			constexpr auto operator()() const -> T requires constructor_callable<T> {
				return KANGARU5_NO_ADL(constructor<T>)()();
			}
		};
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}
	
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using unsafe_exhaustive_constructor = basic_unsafe_exhaustive_constructor<make_spread_injector_function>;
	
	template<movable_object MakeInjector>
	struct basic_exhaustive_constructor {
		constexpr basic_exhaustive_constructor() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_exhaustive_constructor(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return KANGARU5_NO_ADL(constructor<T>)()(
					KANGARU5_NO_ADL(exclude_special_constructors_for<T>)(deduce1),
					KANGARU5_NO_ADL(exclude_deduction<T>)(deduce)...
				);
			}
			
			constexpr auto operator()() const -> T requires(
				constructor_callable<T> and is_empty_injection_constructible_v<T>
			) {
				return KANGARU5_NO_ADL(constructor<T>)()();
			}
		};
		
		template<unqualified_object T, forwarded_source Source>
			requires callable<std::invoke_result_t<MakeInjector const&, Source&&>, construct<T>>
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using exhaustive_constructor = basic_exhaustive_constructor<make_spread_injector_function>;
	
	template<injectable Type, movable_object MakeInjector>
	struct basic_placeholder_constructor_except {
		KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_constructor_except() requires std::default_initializable<MakeInjector> = default;
		
		explicit KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_constructor_except(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<injectable T>
		struct construct {
			[[noreturn]]
			auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires (different_from<T, Type> and constructor_callable<
				std::decay_t<T>,
				exclude_special_constructors_deducer<std::decay_t<T>, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			>);
			
			[[noreturn]]
			auto operator()() const -> T requires (different_from<T, Type> and constructor_callable<std::decay_t<T>>);
		};
		
		template<injectable T, forwarded_source Source>
			requires callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>
		auto operator()(Source&& source) const -> T;
	
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<injectable Type>
	using placeholder_constructor_except = basic_placeholder_constructor_except<Type, make_spread_injector_function>;
	
	template<movable_object MakeInjector>
	struct basic_placeholder_construct {
		KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construct() requires std::default_initializable<MakeInjector> = default;
		
		explicit KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construct(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<injectable T>
		struct construct {
			[[noreturn]]
			auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires (not unqualified_object<T> and constructor_callable<
				std::decay_t<T>,
				exclude_special_constructors_deducer<std::decay_t<T>, decltype(deduce1)>,
				exclude_deducer<std::decay_t<T>, decltype(deduce)>...
			>);
			
			[[noreturn]]
			auto operator()() const -> T requires constructor_callable<T>;
		};
		
		template<injectable T, forwarded_source Source>
			requires callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>
		auto operator()(Source&& source) const -> T;
	
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using placeholder_construct = basic_placeholder_construct<make_spread_injector_function>;
	
	template<source Source, movable_object Function>
	struct with_function_call {
	private:
		using construction_type = std::remove_reference_t<maybe_unwrap_result_t<Function>>;
		
	public:
		explicit constexpr with_function_call(Source source) noexcept
			requires std::default_initializable<Function> :
			source{std::move(source)} {}
		
		constexpr with_function_call(Source source, Function construction) noexcept :
			source{std::move(source)},
			construction{std::move(construction)} {}
		
		template<injectable T, forwarded<with_function_call> Self>
			requires (callable_template_1t<construction_type const&, T, wrapped_source_t<Self>>)
		constexpr KANGARU5_PROVIDE_FUNCTION_DECL(Self&& source) -> T {
			return std::as_const(source.construction).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_function_call> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> with_function_call<wrapped_source_rebind_result_t<Original, NewLeaf>, Function>
		{
			return with_function_call<wrapped_source_rebind_result_t<Original, NewLeaf>, Function>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_FWD(original).construction
			};
		}
		
		Source source;
		
	private:
		Function construction;
	};
	
	template<movable_object... Functions>
	struct overload {
		explicit(sizeof...(Functions) < 2) overload(Functions... funcs) noexcept : functions{std::move(funcs)...} {}
		
		template<injectable T, forwarded_source Source>
			requires (((callable_template_1t_returns<Functions&, T, T, Source&&> ? 1 : 0) + ...) == 1)
		constexpr auto operator()(Source&& source) & -> T {
			constexpr auto index = index_of<T, overload&, decltype(source)>(std::index_sequence_for<Functions...>{});
			return std::get<index>(functions).template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source>
			requires (((callable_template_1t_returns<Functions const&, T, T, Source&&> ? 1 : 0) + ...) == 1)
		constexpr auto operator()(Source&& source) const& -> T {
			constexpr auto index = index_of<T, overload const&, decltype(source)>(std::index_sequence_for<Functions...>{});
			return std::get<index>(functions).template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source>
			requires (((callable_template_1t_returns<Functions&&, T, T, Source&&> ? 1 : 0) + ...) == 1)
		constexpr auto operator()(Source&& source) && -> T {
			constexpr auto index = index_of<T, overload&&, decltype(source)>(std::index_sequence_for<Functions...>{});
			return std::get<index>(std::move(functions)).template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source>
			requires (((callable_template_1t_returns<Functions const&&, T, T, Source&&> ? 1 : 0) + ...) == 1)
		constexpr auto operator()(Source&& source) const&& -> T {
			constexpr auto index = index_of<T, overload const&&, decltype(source)>(std::index_sequence_for<Functions...>{});
			return std::get<index>(std::move(functions)).template operator()<T>(KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source> requires (
			"Ambiguous resolution, multiple callable functions can return type T",
			((callable_template_1t_returns<Functions&, T, T, Source&&> ? 1 : 0) + ...) > 1
		)
		constexpr auto operator()(Source&& source) const& -> T = delete;
		
	private:
		template<typename T, typename Self, typename Source, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			return ((callable_template_1t_returns<detail::utility::forward_like_t<Self, Functions>, T, T, Source&&> ? S : 0) + ...);
		}
		
		std::tuple<Functions...> functions;
	};
	
	template<movable_object Function, movable_object MakeInjector>
	struct function {
	private:
		template<injectable T>
		struct call {
			Function const& func;
			constexpr auto operator()(deducer auto... deduce) const -> T
			requires callable_returns<
				Function const&,
				T,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return func(KANGARU5_NO_ADL(exclude_deduction<T>)(deduce)...);
			}
		};
		
	public:
		explicit constexpr function(Function func) noexcept
			requires std::default_initializable<MakeInjector> : func{std::move(func)} {}
		
		constexpr function(Function func, MakeInjector make_injector) noexcept :
			func{std::move(func)},
			make_injector{std::move(make_injector)} {}
		
		template<injectable T, forwarded_source Source>
			requires callable_returns<std::invoke_result_t<MakeInjector const&, Source>, T, call<T>>
		constexpr auto operator()(Source&& source) const -> T {
			return make_injector(KANGARU5_FWD(source))(call<T>{func});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Function func;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<forwarded_source Source, movable_object Function>
	inline constexpr auto make_source_with_function_call(Source&& source, Function&& function) {
		return with_function_call<std::decay_t<Source>, std::decay_t<Function>>{KANGARU5_FWD(source), KANGARU5_FWD(function)};
	}
	
	template<source Source, source Passthrough, movable_object Constructor>
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
		constexpr KANGARU5_PROVIDE_FUNCTION_DECL(Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).passthrough);
		}
		
		template<unqualified_object T, forwarded<with_construction_original_passthrough> Self>
			requires (callable_template_1t<Constructor const&, T, wrapped_source_t<Self>>)
		constexpr KANGARU5_PROVIDE_FUNCTION_DECL(Self&& source) -> T {
			return std::as_const(source.construction).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_construction_original_passthrough> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> with_construction_original_passthrough<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<Passthrough>, Constructor>
		{
			return with_construction_original_passthrough<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<Passthrough>, Constructor>{
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
	
	template<source Source, movable_object Constructor>
	struct with_construction {
		explicit constexpr with_construction(Source source) noexcept
			requires std::default_initializable<Constructor> :
			source{std::move(source)} {}
		
		constexpr with_construction(Source source, Constructor construction) noexcept :
			source{std::move(source)},
			construction{std::move(construction)} {}
		
		// TODO: Should we really prefer construction as opposed to passthrough?
		template<injectable T, forwarded<with_construction> Self>
			requires (
				    not callable_template_1t<Constructor const&, T, wrapped_source_t<Self>>
				and wrapping_source_of<Self, T>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_DECL(Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<unqualified_object T, forwarded<with_construction> Self>
			requires (callable_template_1t<Constructor const&, T, wrapped_source_t<Self>>)
		constexpr KANGARU5_PROVIDE_FUNCTION_DECL(Self&& source) -> T {
			return std::as_const(source.construction).template operator()<T>(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source));
		}
		
		template<forwarded<with_construction> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept
			-> with_construction_original_passthrough<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<Source>, Constructor>
		{
			return with_construction_original_passthrough<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<Source>, Constructor>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_NO_ADL(ref)(original.source),
				KANGARU5_FWD(original).construction
			};
		}
		
		Source source;
		
	private:
		Constructor construction;
	};
	
	template<source Source>
	using with_non_empty_construction = with_construction<Source, non_empty_constructor>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_non_empty_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, non_empty_constructor>{KANGARU5_FWD(source), non_empty_constructor{}};
	}
	
	template<source Source>
	using with_exhaustive_construction = with_construction<Source, exhaustive_constructor>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_exhaustive_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, exhaustive_constructor>{KANGARU5_FWD(source), exhaustive_constructor{}};
	}
	
	template<source Source>
	using with_unsafe_exhaustive_construction = with_construction<Source, unsafe_exhaustive_constructor>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_unsafe_exhaustive_construction(Source&& source) {
		return with_construction<std::decay_t<Source>, unsafe_exhaustive_constructor>{KANGARU5_FWD(source), unsafe_exhaustive_constructor{}};
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
	
	template<rebindable_source Source>
	struct with_recursion {
		Source source;
		
		template<injectable T, forwarded<with_recursion> Self> requires (not wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_DECL(Self&& source) -> T requires(
			source_of<
				wrapped_source_rebind_result_t<
					Self&,
					detail::recursive_source::leaf_as_alternative<with_recursion<ref_result_t<wrapped_source_t<Self>>>>
				>,
				T
			>
		) {
			return kangaru::provide<T>(
				kangaru::rebind(
					source,
					detail::recursive_source::leaf_as_alternative<with_recursion<ref_result_t<wrapped_source_t<Self>>>>{
						KANGARU5_NO_ADL(ref)(source.source)
					}
				).source
			);
		}
		
		template<injectable T, forwarded<with_recursion> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_DECL(Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
	};
	
	// This deduction guide is required for clang 16 to work
	template<typename Source>
	with_recursion(Source const& source) -> with_recursion<Source>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_recursion(Source&& source) {
		return with_recursion<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<typename Tree, typename Type>
	concept construction_tree_needs = injectable<Tree> and injectable<Type> and not source_of<
		with_recursion<
			with_function_call<
				none_source,
				basic_placeholder_constructor_except<Type, make_strict_spread_injector_function>
			>
		>,
		Tree
	>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
