#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

#include <kangaru/detail/define.hpp>

struct service_1_a { int i; };
struct service_1_b { service_1_a s1a; };

struct service_2_a { service_1_a s1a; int i; };
struct service_2_b { service_1_b s1b; service_2_a s2a; };

namespace kangaru {
	template<typename F>
	concept inject_callable =
		    function_object<F>
		and kangaru::reflectable_function<F, 8>;

	template<function_object F>
	using inject_result_t = kangaru::reflected_return_type<F, 8>;
	
	template<source Source, injectable T> requires source_of<Source, T>
	struct with_cache_one {
		Source source;
		
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<with_cache_one> auto&& source) -> T& {
			auto& result = source.cache;
			if (not result) {
				result.emplace(kangaru::provide<T>(KANGARU5_FWD(source).source));
			}
			
			return *result;
		}
		
		optional<T> cache;
	};
	
	template<movable_object Function, movable_object MakeInjector>
	struct cached_function {
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
		explicit constexpr cached_function(Function func) noexcept
			requires std::default_initializable<MakeInjector> : func{std::move(func)} {}
		
		constexpr cached_function(Function func, MakeInjector make_injector) noexcept :
			func{std::move(func)},
			make_injector{std::move(make_injector)} {}
		
		template<injectable T, forwarded_source Source>
			requires callable_returns<detail::type_traits::call_result_t<MakeInjector const&, Source>, T, call<T>>
		constexpr auto operator()(Source&& source) const -> T {
			return make_injector(KANGARU5_FWD(source))(call<T>{func});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Function func;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<injectable... Types>
	struct type_based_cache {
		constexpr type_based_cache() : cache{optional<Types>{}...} {}
		template<injectable T> requires(... or (std::same_as<T, Types>))
		constexpr auto get() -> optional<T>& {
			return std::get<optional<T>>(cache);
		}
		
	private:
		std::tuple<optional<Types>...> cache;
	};
	
	template<source Source, typename Cache>
	struct with_cache_2 {
		explicit constexpr with_cache_2(Source source) noexcept requires(std::default_initializable<Cache>) : source{std::move(source)} {}
		constexpr with_cache_2(Source source, Cache cache) noexcept : source{std::move(source)}, cache{std::move(cache)} {}
		
		Source source;
		
		template<lvalue_reference T, forwarded<with_cache_2> Self>
			requires(
				    requires(Cache c) { { maybe_unwrap(c).template get<std::remove_reference_t<T>>() } -> std::same_as<optional<std::remove_reference_t<T>>&>; }
				and wrapping_source_of<Self, std::remove_reference_t<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			using Type = std::remove_reference_t<T>;
			auto& result = maybe_unwrap(source.cache).template get<std::remove_reference_t<T>>();
			
			if (not result) {
				result.emplace(kangaru::provide<Type>(KANGARU5_FWD(source).source));
			}
			
			return *result;
		}
		
		// TODO: properly rebind cache
		template<forwarded<with_cache_2> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept -> with_cache_2<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<decltype((original.cache))>> {
			return with_cache_2<wrapped_source_rebind_result_t<Original, NewLeaf>, ref_result_t<decltype((original.cache))>>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_NO_ADL(ref)(original.cache)
			};
		}
		
	private:
		Cache cache;
	};
	
	template<injectable... Products>
	inline constexpr auto make_source_with_cache_many(forwarded_source auto&& source) -> with_cache_2<std::decay_t<decltype(source)>, type_based_cache<Products...>> {
		return with_cache_2<std::decay_t<decltype(source)>, type_based_cache<Products...>>{KANGARU5_FWD(source)};
	}
	
	template<typename Something = none_source, movable_object... Functions>
	struct modular_source {
	private:
		using source_t = with_cache_2<
			with_function_call<
				Something,
				overload<function<Functions, make_spread_injector_function>...>
			>,
			type_based_cache<inject_result_t<Functions>...>
		>;
		
	public:
		constexpr modular_source() requires(sizeof...(Functions) == 0 and std::default_initializable<Something>) :
			source{{with_function_call{Something{}, overload{}}}} {}
		
		explicit(sizeof...(Functions) == 0) constexpr modular_source(Something something, Functions... functions) noexcept :
			source{
				KANGARU5_NO_ADL(make_source_with_cache_many<inject_result_t<Functions>...>)(
					with_function_call{
						std::move(something),
						overload{
							function{
								std::move(functions),
								make_spread_injector_function{}
							}...
						}
					}
				)
			} {}
		
		template<injectable T, forwarded<modular_source> Self>
			requires source_of<with_recursion<fwd_ref_result_t<detail::utility::forward_like_t<Self, source_t>>>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(with_recursion{KANGARU5_NO_ADL(fwd_ref)(source.source)});
		}
		
	private:
		source_t source;
	};
	
	template<object Module>
	struct source_from_module {
		explicit constexpr source_from_module(inject_result_t<Module>& source) noexcept : source{KANGARU5_NO_ADL(ref)(source)} {}
		
		template<injectable T, forwarded<source_from_module> Self> requires source_of<source_reference_wrapper<inject_result_t<Module>>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
	private:
		source_reference_wrapper<inject_result_t<Module>> source;
	};
	
	template<source Source, typename... Modules>
	struct modular_container {
		constexpr modular_container() requires(sizeof...(Modules) == 0 and std::default_initializable<Source>) :
			source{{with_function_call{Source{}, overload{}}}} {}
		
		explicit(sizeof...(Modules) == 0) constexpr modular_container(Source source, Modules... modules) noexcept :
			source{
				// KANGARU5_NO_ADL(make_source_with_non_empty_construction)(
					with_source_reference_wrapping{
						KANGARU5_NO_ADL(make_source_with_cache_many<inject_result_t<Modules>...>)(
							with_function_call{
								std::move(source),
								overload{
									function{
										std::move(modules),
										make_spread_injector_function{}
									}...
								},
							}
						),
					}
				// )
			} {}
		
		// TODO: Only allow to construct source_from_module (or only allow modular_container?)
		// with_non_empty_construction<
			with_source_reference_wrapping<
				with_cache_2<
					with_function_call<
						Source,
						overload<function<Modules, make_spread_injector_function>...>
					>,
					type_based_cache<inject_result_t<Modules>...>
			// 	>
			>
		> source;
		
		template<injectable T, forwarded<modular_container> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T
		requires (
			((source_of<inject_result_t<Modules>&, T> ? 1 : 0) + ...) == 1
		) {
			using source_t = std::tuple_element_t<index_of<T, decltype(source)>(std::index_sequence_for<Modules...>{}), std::tuple<Modules...>>;
			return kangaru::provide<T>(
				kangaru::provide<source_reference_wrapper<inject_result_t<source_t>>>(
					with_recursion{KANGARU5_NO_ADL(fwd_ref)(source.source)}
				)
			);
		}
		
	private:
		template<typename T, typename Self, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			return ((source_of<inject_result_t<Modules>&, T> ? S : 0) + ...);
		}
	};
} // namespace kangaru

constexpr auto module0 = []() {
	return kangaru::modular_source{};
};

// NOTE: If using reference_source / object_source, should we do eager injection or lazy injection?
//       Do we really want to reproduce the basic sources as callbacks?
//       We should receive a modular container as parameter? Maybe?
constexpr auto module1 = [](kangaru::source_reference_wrapper<kangaru::reflected_return_type<decltype(module0), 8>> m0) {
	return kangaru::modular_source{
		m0,
		[] {
			fmt::println("call service_1_a");
			return service_1_a{.i = 9};
		},
		[](service_1_a s1a){ return service_1_b{.s1a = s1a}; },
	};
};

constexpr auto module2 = [](kangaru::source_reference_wrapper<kangaru::reflected_return_type<decltype(module1), 8>> m1) {
	return kangaru::modular_source{
		m1,
		[](service_1_a s1a){ return service_2_a{.s1a = s1a, .i = 8}; },
		[](service_1_b s1b, service_2_a s2a) {
			fmt::println("call service_2_b");
			return service_2_b{.s1b = s1b, .s2a = s2a};
		},
	};
};

auto main() -> int {
	auto source = kangaru::modular_container{kangaru::none_source{}, module0, module1, module2};
	auto injector = kangaru::make_spread_injector(kangaru::ref(source));
	
	injector([](service_2_b& s2b) {
		fmt::println("patate {}", s2b.s1b.s1a.i);
	});
}
