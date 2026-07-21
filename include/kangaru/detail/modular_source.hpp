#ifndef KANGARU5_DETAIL_MODULAR_SOURCE_HPP
#define KANGARU5_DETAIL_MODULAR_SOURCE_HPP

#include "concepts.hpp"
#include "source.hpp"
#include "utility.hpp"
#include "incremental_source.hpp"
#include "recursive_source.hpp"
#include "injector.hpp"
#include "constructor.hpp"

#ifndef KANGARU5_MODULES
#include <utility>
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru::detail::modular_source_private {
	template<source Source>
	struct use_source {
		Source&& source;
		
		// We ignore the source since we have one already constructed
		constexpr auto operator()(forwarded_source auto&&) && -> deduced_source_type<Source&&> { return std::move(source); }
	};
} // namespace kangaru::detail::modular_source_private

KANGARU5_EXPORT namespace kangaru {
	template<function_object Function, make_injector MakeInjector = make_strict_spread_injector_function, construction Construction = exhaustive_construction>
	struct modular_source_initializer {
		explicit constexpr modular_source_initializer(Function function) : function{std::move(function)} {}
		constexpr modular_source_initializer(Function function, Construction construction) :
			function{std::move(function)}, construction{std::move(construction)} {}
		
		template<forwarded_source Source>
			requires(
				not callable<
					Function,
					with_recursion<with_construction<sealed_source<std::remove_reference_t<Source>>, Construction>>
				>
				and callable<
					detail::call_result_t<
						make_strict_spread_injector_function,
						with_recursion<with_construction<sealed_source<std::remove_reference_t<Source>>, Construction>>
					>,
					Function
				>
			)
		constexpr auto operator()(Source&& source) && {
			auto injection_source = with_recursion{with_construction{seal_source(KANGARU5_FWD(source)), construction}};
			return make_injector(KANGARU5_NO_ADL(ref)(injection_source))(std::move(function));
		}
		
		template<forwarded_source Source>
			requires(
				callable<
					Function,
					with_recursion<with_construction<sealed_source<std::remove_reference_t<Source>>, Construction>>
				>
			)
		constexpr auto operator()(Source&& source) && {
			return std::move(function)(with_recursion{with_construction{seal_source(KANGARU5_FWD(source)), construction}});
		}
		
	private:
		Function function;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector = {};
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction = {};
	};
	
	template<function_object Function>
	inline constexpr auto make_modular_source_initializer(Function function) {
		return modular_source_initializer<Function>{std::move(function)};
	}
	
	template<function_object Function, make_injector MakeInjector>
	inline constexpr auto make_modular_source_initializer(Function function, MakeInjector make_injector) {
		return modular_source_initializer<Function, make_strict_spread_injector_function>{
			std::move(function),
			std::move(make_injector),
		};
	}
	
	template<function_object Function, make_injector MakeInjector, construction Construction>
	inline constexpr auto make_modular_source_initializer(Function function, MakeInjector make_injector, Construction construction) {
		return modular_source_initializer<Function, Construction>{std::move(function), std::move(make_injector), std::move(construction)};
	}
	
	template<source Source, construction Construction = exhaustive_construction>
	struct modular_source_initializer_using_lazy {
	private:
		template<typename>
		using pick_source = Source;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
		
	public:
		modular_source_initializer_using_lazy() = default;
		explicit constexpr modular_source_initializer_using_lazy(Construction construction) :
			construction{std::move(construction)} {}
		
		constexpr auto operator()(forwarded_source auto&& from)
		requires(
			callable<strict_spread_injector<ref_result_t<decltype(from)&>>, constructor_function<Source>>
		) {
			return make_source_with_provide_using_source<pick_source>(
				make_source_with_lazy_evaluation_of<Source>(
					with_construction{
						KANGARU5_FWD(from),
						construction,
					}
				)
			);
		}
	};
	
	template<source Source>
	inline constexpr auto make_modular_source_initializer_using_lazy() {
		return modular_source_initializer_using_lazy<Source, make_strict_spread_injector_function>{};
	}
	
	template<source Source, copiable_object Construction>
	inline constexpr auto make_modular_source_initializer_using_lazy(Construction construction) {
		return modular_source_initializer_using_lazy<Source, Construction>{std::move(construction)};
	}
	
	template<construction Construction = exhaustive_construction, source Source = none_source, function_object... Lambdas>
		requires(
			std::constructible_from<
				incremental_source<
					detail::modular_source_private::use_source<Source>,
					modular_source_initializer<Lambdas, make_strict_spread_injector_function, Construction>...
				>,
				detail::modular_source_private::use_source<Source>,
				modular_source_initializer<Lambdas, make_strict_spread_injector_function, Construction>...
			>
		)
	struct modular_source {
	private:
		using impl_t = incremental_source<
			detail::modular_source_private::use_source<Source>,
			modular_source_initializer<Lambdas, make_strict_spread_injector_function, Construction>...
		>;
		
		impl_t impl;
		
		template<typename Self>
		using next_t = detail::forward_like_t<Self, decltype(std::declval<Self>().impl.next)>;
		
	public:
		explicit(sizeof...(Lambdas) == 1)
		constexpr modular_source(Lambdas... lambdas) requires(std::same_as<exhaustive_construction, Construction> and std::same_as<none_source, Source>) :
			impl{
				detail::modular_source_private::use_source<Source>{Source{}},
				modular_source_initializer<Lambdas, make_strict_spread_injector_function, Construction>{std::move(lambdas)}...
			} {}
		
		template<forwarded<Source> S>
		explicit(sizeof...(Lambdas) == 0)
		constexpr modular_source(S&& source, Lambdas... lambdas)
			requires(std::same_as<exhaustive_construction, Construction> and not callable<Source>) :
			impl{
				detail::modular_source_private::use_source<Source>{KANGARU5_FWD(source)},
				modular_source_initializer<Lambdas, make_strict_spread_injector_function, Construction>{std::move(lambdas)}...
			} {}
		
		template<forwarded<Source> S>
		constexpr modular_source(Construction construction, S&& source, Lambdas... lambdas)
			requires(not callable<Construction> and not callable<Source>) :
			impl{
				detail::modular_source_private::use_source<Source>{KANGARU5_FWD(source)},
				modular_source_initializer<Lambdas, make_strict_spread_injector_function, Construction>{std::move(lambdas), construction}...
			} {}
		
		// Workaround for GCC bug 126312
		// Without the deleted constructors, GCC may produce bad codegen by omitting mandatory elision
		modular_source(modular_source&&) = delete;
		auto operator=(modular_source&&) -> modular_source& = delete;
		modular_source(modular_source const&) = delete;
		auto operator=(modular_source const&) -> modular_source& = delete;
		
		template<injectable T, forwarded<modular_source> Self>
			requires(source_of<next_t<Self>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			// Here we must skip first head of the incremental source. This is because we don't want to provide
			// from source of other modules. We let the modular container select the right source instead.
			return kangaru::provide<T>(KANGARU5_FWD(source).impl.next);
		}
	};
	
	template<callable First, typename... Rest>
	modular_source(First, Rest...) -> modular_source<exhaustive_construction, none_source, First, Rest...>;
	
	template<source Source, typename First, typename... Rest>
		requires(
			    not callable<Source>
			and std::constructible_from<
				incremental_source<
					detail::modular_source_private::use_source<Source>,
					modular_source_initializer<First, make_strict_spread_injector_function, exhaustive_construction>,
					modular_source_initializer<Rest, make_strict_spread_injector_function, exhaustive_construction>...
				>,
				detail::modular_source_private::use_source<Source>,
				modular_source_initializer<First, make_strict_spread_injector_function, exhaustive_construction>,
				modular_source_initializer<Rest, make_strict_spread_injector_function, exhaustive_construction>...
			>
		)
	modular_source(Source const&, First const&, Rest const&...) -> modular_source<exhaustive_construction, Source, First, Rest...>;
	
	template<construction Construction, source Source, typename First, typename... Rest>
		requires(
			    not callable<Construction>
			and not callable<Source>
			and std::constructible_from<
				incremental_source<
					detail::modular_source_private::use_source<Source>,
					modular_source_initializer<First, make_strict_spread_injector_function, Construction>,
					modular_source_initializer<Rest, make_strict_spread_injector_function, Construction>...
				>,
				detail::modular_source_private::use_source<Source>,
				modular_source_initializer<First, make_strict_spread_injector_function, Construction>,
				modular_source_initializer<Rest, make_strict_spread_injector_function, Construction>...
			>
		)
	modular_source(Construction const&, Source const&, First const&, Rest const&...) -> modular_source<Construction, Source, First, Rest...>;
	
	template<source Source>
		requires(not callable<Source>)
	modular_source(Source const&) -> modular_source<exhaustive_construction, Source>;
	
	// For all the following factory functions, we do not unwrap the source type for in_place_construct.
	// Instead, modular_source will internally unwrap it properly. This is why we use std::decay_t directly.
	inline constexpr auto make_modular_source() {
		return modular_source<exhaustive_construction, none_source>{};
	}
	
	template<callable First, function_object... Lambdas>
		requires(
			std::constructible_from<
				modular_source<exhaustive_construction, none_source, First, Lambdas...>,
				First&&,
				Lambdas&&...
			>
		)
	inline constexpr auto make_modular_source(First first, Lambdas... lambdas) {
		return modular_source<exhaustive_construction, none_source, First, Lambdas...>{std::move(first), std::move(lambdas)...};
	}
	
	template<forwarded_source Source, function_object FirstLambda, function_object... Lambdas>
		requires(
			    not callable<Source>
			and std::constructible_from<
				modular_source<exhaustive_construction, std::decay_t<Source>, FirstLambda, Lambdas...>,
				Source&&,
				FirstLambda&&,
				Lambdas&&...
			>
		)
	inline constexpr auto make_modular_source(Source&& source, FirstLambda first, Lambdas... lambdas) {
		return modular_source<exhaustive_construction, std::decay_t<Source>, FirstLambda, Lambdas...>{KANGARU5_FWD(source), std::move(first), std::move(lambdas)...};
	}
	
	template<construction Construction, forwarded_source Source, function_object FirstLambda, function_object... Lambdas>
		requires(
			    not callable<Construction>
			and not callable<Source>
			and std::constructible_from<
				modular_source<Construction, std::decay_t<Source>, FirstLambda, Lambdas...>,
				Construction&&,
				Source&&,
				FirstLambda&&,
				Lambdas&&...
			>
		)
	inline constexpr auto make_modular_source(Construction construction, Source&& source, FirstLambda first, Lambdas... lambdas) {
		return modular_source<Construction, std::decay_t<Source>, FirstLambda, Lambdas...>{std::move(construction), KANGARU5_FWD(source), std::move(first), std::move(lambdas)...};
	}
	
	template<source... Sources>
		requires(
			    sizeof...(Sources) > 0
			and std::constructible_from<
				modular_source<exhaustive_construction, none_source, constructor_function<Sources>...>,
				none_source,
				constructor_function<Sources>...
			>
		)
	inline constexpr auto make_modular_source() {
		return modular_source<exhaustive_construction, none_source, constructor_function<Sources>...>{
			none_source{},
			constructor_function<Sources>{}...
		};
	}
	
	template<source... Sources, forwarded_source Source>
		requires(
			    sizeof...(Sources) > 0
			and std::constructible_from<
				modular_source<exhaustive_construction, std::decay_t<Source>, constructor_function<Sources>...>,
				Source&&,
				constructor_function<Sources>...
			>
		)
	inline constexpr auto make_modular_source(Source&& source) {
		return modular_source<exhaustive_construction, std::decay_t<Source>, constructor_function<Sources>...>{
			KANGARU5_FWD(source),
			constructor_function<Sources>{}...
		};
	}
	
	template<source... Sources, construction Construction, forwarded_source Source>
		requires(
			    sizeof...(Sources) > 0
			and std::constructible_from<
				modular_source<Construction, std::decay_t<Source>, constructor_function<Sources>...>,
				Construction&&,
				Source&&,
				constructor_function<Sources>...
			>
		)
	inline constexpr auto make_modular_source(Construction construction, Source&& source) {
		return modular_source<Construction, std::decay_t<Source>, constructor_function<Sources>...>{
			std::move(construction),
			KANGARU5_FWD(source),
			constructor_function<Sources>{}...
		};
	}
	
	template<forwarded_source Source>
		requires(not callable<Source>)
	inline constexpr auto make_modular_source(Source&& source) {
		return modular_source<exhaustive_construction, std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	inline constexpr auto make_modular_source_in_place() {
		return KANGARU5_NO_ADL(construct_in_place<modular_source<exhaustive_construction, none_source>>)();
	}
	
	template<callable First, function_object... Lambdas>
		requires(
			std::constructible_from<
				modular_source<exhaustive_construction, none_source, First, Lambdas...>,
				First&&,
				Lambdas&&...
			>
		)
	inline constexpr auto make_modular_source_in_place(First first, Lambdas... lambdas) {
		return KANGARU5_NO_ADL(construct_in_place<modular_source<exhaustive_construction, none_source, First, Lambdas...>>)(std::move(first), std::move(lambdas)...);
	}
	
	template<forwarded_source Source, function_object FirstLambda, function_object... Lambdas>
		requires(
			    not callable<Source>
			and std::constructible_from<
				modular_source<exhaustive_construction, std::decay_t<Source>, FirstLambda, Lambdas...>,
				Source&&,
				FirstLambda&&,
				Lambdas&&...
			>
		)
	inline constexpr auto make_modular_source_in_place(Source&& source, FirstLambda first, Lambdas... lambdas) {
		return KANGARU5_NO_ADL(construct_in_place<modular_source<exhaustive_construction, std::decay_t<Source>, FirstLambda, Lambdas...>>)(
			KANGARU5_FWD(source), std::move(first), std::move(lambdas)...
		);
	}
	
	template<construction Construction, forwarded_source Source, function_object FirstLambda, function_object... Lambdas>
		requires(
			    not callable<Construction>
			and not callable<Source>
			and std::constructible_from<
				modular_source<Construction, std::decay_t<Source>, FirstLambda, Lambdas...>,
				Construction&&,
				Source&&,
				FirstLambda&&,
				Lambdas&&...
			>
		)
	inline constexpr auto make_modular_source_in_place(Construction construction, Source&& source, FirstLambda first, Lambdas... lambdas) {
		return KANGARU5_NO_ADL(construct_in_place<modular_source<Construction, std::decay_t<Source>, FirstLambda, Lambdas...>>)(
			std::move(construction), KANGARU5_FWD(source), std::move(first), std::move(lambdas)...
		);
	}
	
	template<source... Sources>
		requires(
			    sizeof...(Sources) > 0
			and std::constructible_from<
				modular_source<exhaustive_construction, none_source, constructor_function<Sources>...>,
				none_source,
				constructor_function<Sources>...
			>
		)
	inline constexpr auto make_modular_source_in_place() {
		return KANGARU5_NO_ADL(construct_in_place<modular_source<exhaustive_construction, none_source, constructor_function<Sources>...>>)(
			none_source{},
			constructor_function<Sources>{}...
		);
	}
	
	template<source... Sources, forwarded_source Source>
		requires(
			    sizeof...(Sources) > 0
			and std::constructible_from<
				modular_source<exhaustive_construction, std::decay_t<Source>, constructor_function<Sources>...>,
				std::decay_t<Source>&&,
				constructor_function<Sources>...
			>
		)
	inline constexpr auto make_modular_source_in_place(Source&& source) {
		return KANGARU5_NO_ADL(construct_in_place<modular_source<exhaustive_construction, std::decay_t<Source>, constructor_function<Sources>...>>)(
			KANGARU5_FWD(source),
			constructor_function<Sources>{}...
		);
	}
	
	template<source... Sources, construction Construction, forwarded_source Source>
		requires(
			    sizeof...(Sources) > 0
			and std::constructible_from<
				modular_source<Construction, std::decay_t<Source>, constructor_function<Sources>...>,
				Construction&&,
				std::decay_t<Source>&&,
				constructor_function<Sources>...
			>
		)
	inline constexpr auto make_modular_source_in_place(Construction construction, Source&& source) {
		return KANGARU5_NO_ADL(construct_in_place<modular_source<Construction, std::decay_t<Source>, constructor_function<Sources>...>>)(
			std::move(construction),
			KANGARU5_FWD(source),
			constructor_function<Sources>{}...
		);
	}
	
	template<forwarded_source Source>
		requires(not callable<Source>)
	inline constexpr auto make_modular_source_in_place(Source&& source) {
		return KANGARU5_NO_ADL(construct_in_place<modular_source<exhaustive_construction, std::decay_t<Source>>>)(KANGARU5_FWD(source));
	}
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_MODULAR_SOURCE_HPP
