#ifndef KANGARU5_DETAIL_MODULAR_SOURCE_HPP
#define KANGARU5_DETAIL_MODULAR_SOURCE_HPP

#include "concepts.hpp"
#include "source.hpp"
#include "source_types.hpp"
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

// TODO: Make type of deducer parameterizable
namespace kangaru::detail::modular_source_private {
	template<source Source>
	struct use_source {
		Source&& source;
		
		// We ignore the source since we have one already constructed
		constexpr auto operator()(forwarded_source auto&&) && { return std::move(source); }
	};
	
	// TODO: Deduplicate
	template<source Source>
	using injection_source = with_recursion<with_construction<Source, exhaustive_construction>>;
	
	template<function_object Function>
	struct modular_source_initializer {
		explicit constexpr modular_source_initializer(Function function) : function{std::move(function)} {}
		
		template<forwarded_source Source>
			requires(
				not callable<
					Function,
					injection_source<std::remove_reference_t<Source>>
				>
				and callable<
					detail::call_result_t<
						make_strict_spread_injector_function,
						injection_source<std::remove_reference_t<Source>>
					>,
					Function
				>
			)
		constexpr auto operator()(Source&& source) && {
			auto injection_source = with_recursion{with_construction{KANGARU5_FWD(source), exhaustive_construction{}}};
			return KANGARU5_NO_ADL(make_strict_spread_injector)(KANGARU5_NO_ADL(ref)(injection_source))(std::move(function));
		}
		
		template<forwarded_source Source>
			requires callable<
				Function,
				injection_source<std::remove_reference_t<Source>>
			>
		constexpr auto operator()(Source&& source) && {
			return std::move(function)(with_recursion{with_construction{KANGARU5_FWD(source), exhaustive_construction{}}});
		}
		
	private:
		Function function;
	};
} // namespace kangaru::detail::modular_source_private

KANGARU5_EXPORT namespace kangaru {
	template<source Source = none_source, function_object... Lambdas>
		requires(
			std::constructible_from<
				incremental_source<
					detail::modular_source_private::use_source<Source>,
					detail::modular_source_private::modular_source_initializer<Lambdas>...
				>,
				detail::modular_source_private::use_source<Source>,
				detail::modular_source_private::modular_source_initializer<Lambdas>...
			>
		)
	struct modular_source {
	private:
		using impl_t = incremental_source<
			detail::modular_source_private::use_source<Source>,
			detail::modular_source_private::modular_source_initializer<Lambdas>...
		>;
		
	public:
		explicit(sizeof...(Lambdas) == 1)
		constexpr modular_source(Lambdas... lambdas) requires(std::same_as<none_source, Source>) :
			impl{
				detail::modular_source_private::use_source<Source>{Source{}},
				detail::modular_source_private::modular_source_initializer<Lambdas>{std::move(lambdas)}...
			} {}
		
		constexpr modular_source() requires(sizeof...(Lambdas) == 0 and std::default_initializable<Source>) :
			impl{detail::modular_source_private::use_source<Source>{Source{}}} {}
		
		explicit(sizeof...(Lambdas) == 0)
		constexpr modular_source(Source source, Lambdas... lambdas) :
			impl{
				detail::modular_source_private::use_source<Source>{std::move(source)},
				detail::modular_source_private::modular_source_initializer<Lambdas>{std::move(lambdas)}...
			} {}
		
		template<injectable T, forwarded<modular_source> Self>
			requires(source_of<detail::forward_like_t<Self, decltype(std::declval<impl_t>().next)>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			// Here we must skip first head of the incremental source. This is because we don't want to provide
			// from source of other modules.
			return kangaru::provide<T>(KANGARU5_FWD(source).impl.next);
		}
		
	private:
		impl_t impl;
	};
	
	template<callable First, typename... Rest>
	modular_source(First, Rest...) -> modular_source<none_source, First, Rest...>;
	
	template<source Source, typename... Rest> requires(not callable<Source>)
	modular_source(Source, Rest...) -> modular_source<Source, Rest...>;
	
	template<source Source, function_object... Lambdas>
		requires std::constructible_from<modular_source<Source, Lambdas...>, Source&&, Lambdas&...>
	inline constexpr auto make_modular_source(Source source, Lambdas... lambdas) {
		return modular_source<Source, Lambdas...>{std::move(source), lambdas...};
	}
	
	template<source... Sources, source Source>
		requires std::constructible_from<
			modular_source<Source, constructor_function<Sources>...>,
			Source&&,
			constructor_function<Sources>...
		>
	inline constexpr auto make_modular_source(Source source) {
		return modular_source<Source, constructor_function<Sources>...>{
			std::move(source),
			constructor_function<Sources>{}...
		};
	}
	
	template<source... Sources>
		requires std::constructible_from<
			modular_source<none_source, constructor_function<Sources>...>,
			none_source,
			constructor_function<Sources>...
		>
	inline constexpr auto make_modular_source() {
		return modular_source<none_source, constructor_function<Sources>...>{
			none_source{},
			constructor_function<Sources>{}...
		};
	}
	
	template<source Source, function_object... Lambdas>
		requires std::constructible_from<modular_source<Source, Lambdas...>, Source&&, Lambdas&...>
	inline constexpr auto make_modular_source_in_place(Source source, Lambdas... lambdas) {
		return in_place_construct{
			[source = std::move(source), ...lambdas = std::move(lambdas)]() mutable {
				return modular_source<Source, Lambdas...>{std::move(source), lambdas...};
			},
		};
	}
	
	template<source... Sources, source Source>
		requires std::constructible_from<
			modular_source<Source, constructor_function<Sources>...>,
			Source&&,
			constructor_function<Sources>...
		>
	inline constexpr auto make_modular_source_in_place(Source source) {
		return in_place_construct{
			[source = std::move(source)]() mutable {
				return modular_source<Source, constructor_function<Sources>...>{
					std::move(source),
					constructor_function<Sources>{}...
				};
			},
		};
	}
	
	template<source... Sources>
		requires std::constructible_from<
			modular_source<none_source, constructor_function<Sources>...>,
			none_source,
			constructor_function<Sources>...
		>
	inline constexpr auto make_modular_source_in_place() {
		return in_place_construct{
			[] {
				return modular_source<none_source, constructor_function<Sources>...>{
					none_source{},
					constructor_function<Sources>{}...
				};
			},
		};
	}
	
	// lazy modular source initializer
	template<source Source, copiable_object MakeInjector = make_strict_spread_injector_function>
	struct make_lazy_initialized_source_function {
	private:
		template<typename>
		using pick_source = Source;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
		
	public:
		make_lazy_initialized_source_function() = default;
		explicit constexpr make_lazy_initialized_source_function(MakeInjector make_injector) :
			make_injector{std::move(make_injector)} {}
		
		constexpr auto operator()(forwarded_source auto&& from)
		requires(
			callable<strict_spread_injector<ref_result_t<decltype(from)&>>, constructor_function<Source>>
		) {
			return sealed_source{
				make_source_with_provide_using_source<pick_source>(
					make_source_with_lazy_evaluation_of<Source>(
						with_construction{
							KANGARU5_FWD(from),
							basic_exhaustive_construction{make_injector},
						}
					)
				),
			};
		}
	};
	
	template<source Source, copiable_object MakeInjector = make_strict_spread_injector_function>
	inline constexpr auto make_lazy_initialized_source(forwarded_source auto&& from, MakeInjector make_injector) {
		return make_lazy_initialized_source_function<Source, MakeInjector>{}(KANGARU5_FWD(from), std::move(make_injector));
	}
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_MODULAR_SOURCE_HPP
