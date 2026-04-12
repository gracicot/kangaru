#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

#include <kangaru/detail/define.hpp>

struct service_1_a { int i; };
struct service_1_b { service_1_a s1a; };

struct service_2_a { service_1_a s1a; int i; };
struct service_2_b { service_1_b s1b; service_2_a s2a; };

template<typename F>
concept inject_callable =
		kangaru::function_object<F>
	and kangaru::reflectable_function<F, 8>;

template<kangaru::function_object F>
using inject_result_t = kangaru::reflected_return_type<F, 8>;

template<kangaru::source Source, kangaru::injectable T> requires kangaru::source_of<Source, T>
struct with_cache_one {
	Source source;
	
	constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS kangaru::forwarded<with_cache_one> auto&& source) -> T& {
		auto& result = source.cache;
		if (not result) {
			result.emplace(kangaru::provide<T>(KANGARU5_FWD(source).source));
		}
		
		return *result;
	}
	
	kangaru::optional<T> cache;
};

template<kangaru::movable_object Function, kangaru::movable_object MakeInjector>
struct cached_function {
private:
	template<kangaru::injectable T>
	struct call {
		Function const& func;
		constexpr auto operator()(kangaru::deducer auto... deduce) const -> T
		requires kangaru::callable_returns<
			T,
			Function const&,
			kangaru::exclude_deducer<T, decltype(deduce)>...
		> {
			return func(kangaru::exclude_deduction<T>(deduce)...);
		}
	};
	
public:
	explicit constexpr cached_function(Function func) noexcept
		requires std::default_initializable<MakeInjector> : func{std::move(func)} {}
	
	constexpr cached_function(Function func, MakeInjector make_injector) noexcept :
		func{std::move(func)},
		make_injector{std::move(make_injector)} {}
	
	template<kangaru::injectable T, kangaru::forwarded_source Source>
		requires kangaru::callable_returns<T, kangaru::detail::call_result_t<MakeInjector const&, Source>, call<T>>
	constexpr auto operator()(Source&& source) const -> T {
		return make_injector(KANGARU5_FWD(source))(call<T>{func});
	}
	
private:
	KANGARU5_NO_UNIQUE_ADDRESS
	Function func;
	
	KANGARU5_NO_UNIQUE_ADDRESS
	MakeInjector make_injector;
};

template<kangaru::injectable... Types>
struct type_based_cache {
	constexpr type_based_cache() : cache{kangaru::optional<Types>{}...} {}
	template<kangaru::injectable T> requires(... or (std::same_as<T, Types>))
	constexpr auto get() -> kangaru::optional<T>& {
		return std::get<kangaru::optional<T>>(cache);
	}
	
private:
	std::tuple<kangaru::optional<Types>...> cache;
};

template<kangaru::source Source, typename Cache>
struct with_cache_2 {
	explicit constexpr with_cache_2(Source source) noexcept requires(std::default_initializable<Cache>) : source{std::move(source)} {}
	constexpr with_cache_2(Source source, Cache cache) noexcept : source{std::move(source)}, cache{std::move(cache)} {}
	
	Source source;
	
	template<kangaru::lvalue_reference T, kangaru::forwarded<with_cache_2> Self>
		requires(
				requires(Cache c) { { maybe_unwrap(c).template get<std::remove_reference_t<T>>() } -> std::same_as<kangaru::optional<std::remove_reference_t<T>>&>; }
			and kangaru::wrapping_source_of<Self, std::remove_reference_t<T>>
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
	template<kangaru::forwarded<with_cache_2> Original, kangaru::forwarded_function_object ReplaceLeaf>
	static constexpr auto rebind(Original&& original, ReplaceLeaf&& replace_leaf) noexcept -> with_cache_2<kangaru::wrapped_source_rebind_result_t<Original, ReplaceLeaf>, kangaru::ref_result_t<decltype((original.cache))>> {
		return with_cache_2<kangaru::wrapped_source_rebind_result_t<Original, ReplaceLeaf>, kangaru::ref_result_t<decltype((original.cache))>>{
			kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(replace_leaf)),
			kangaru::ref(original.cache)
		};
	}
	
private:
	Cache cache;
};

template<kangaru::injectable... Products>
inline constexpr auto make_source_with_cache_many(kangaru::forwarded_source auto&& source) -> with_cache_2<std::decay_t<decltype(source)>, type_based_cache<Products...>> {
	return with_cache_2<std::decay_t<decltype(source)>, type_based_cache<Products...>>{KANGARU5_FWD(source)};
}

template<typename Something = kangaru::none_source, kangaru::movable_object... Functions>
struct modular_source {
private:
	using source_t = with_cache_2<
		kangaru::with_function_call<
			Something,
			kangaru::call_with_injector<Functions, kangaru::make_spread_injector_function>...
		>,
		type_based_cache<inject_result_t<Functions>...>
	>;
	
public:
	constexpr modular_source() requires(sizeof...(Functions) == 0 and std::default_initializable<Something>) :
		source{{kangaru::with_function_call{Something{}}}} {}
	
	explicit(sizeof...(Functions) == 0) constexpr modular_source(Something something, Functions... functions) noexcept :
		source{
			make_source_with_cache_many<inject_result_t<Functions>...>(
				kangaru::with_function_call{
					std::move(something),
						kangaru::call_with_injector{
							std::move(functions),
							kangaru::make_spread_injector_function{}
						}...
				}
			)
		} {}
	
	template<kangaru::injectable T, kangaru::forwarded<modular_source> Self>
		requires kangaru::source_of<kangaru::with_recursion<kangaru::fwd_ref_result_t<kangaru::detail::forward_like_t<Self, source_t>>>, T>
	constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
		return kangaru::provide<T>(kangaru::with_recursion{kangaru::fwd_ref(source.source)});
	}
	
private:
	source_t source;
};

template<kangaru::object Module>
struct source_from_module {
	explicit constexpr source_from_module(inject_result_t<Module>& source) noexcept : source{kangaru::ref(source)} {}
	
	template<kangaru::injectable T, kangaru::forwarded<source_from_module> Self> requires kangaru::source_of<kangaru::source_reference_wrapper<inject_result_t<Module>>, T>
	constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
		return kangaru::provide<T>(KANGARU5_FWD(source).source);
	}
	
private:
	kangaru::source_reference_wrapper<inject_result_t<Module>> source;
};

template<kangaru::source Source, typename... Modules>
struct modular_container {
	constexpr modular_container() requires(sizeof...(Modules) == 0 and std::default_initializable<Source>) :
		source{{kangaru::with_function_call{Source{}}}} {}
	
	explicit(sizeof...(Modules) == 0) constexpr modular_container(Source source, Modules... modules) noexcept :
		source{
			// KANGARU5_NO_ADL(make_source_with_non_empty_construction)(
				kangaru::with_source_reference_wrapping{
					make_source_with_cache_many<inject_result_t<Modules>...>(
						kangaru::with_function_call{
							std::move(source),
								kangaru::call_with_injector{
									std::move(modules),
									kangaru::make_spread_injector_function{}
								}...
						}
					),
				}
			// )
		} {}
	
	// TODO: Only allow to construct source_from_module (or only allow modular_container?)
	// with_non_empty_construction<
		kangaru::with_source_reference_wrapping<
			with_cache_2<
				kangaru::with_function_call<
					Source,
					kangaru::call_with_injector<Modules, kangaru::make_spread_injector_function>...
				>,
				type_based_cache<inject_result_t<Modules>...>
		// 	>
		>
	> source;
	
	template<kangaru::injectable T, kangaru::forwarded<modular_container> Self>
	constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T
	requires (
		((kangaru::source_of<inject_result_t<Modules>&, T> ? 1 : 0) + ...) == 1
	) {
		using source_t = std::tuple_element_t<index_of<T, decltype(source)>(std::index_sequence_for<Modules...>{}), std::tuple<Modules...>>;
		return kangaru::provide<T>(
			kangaru::provide<kangaru::source_reference_wrapper<inject_result_t<source_t>>>(
				kangaru::with_recursion{kangaru::fwd_ref(source.source)}
			)
		);
	}
	
private:
	template<typename T, typename Self, std::size_t... S>
	constexpr static auto index_of(std::index_sequence<S...>) {
		return ((kangaru::source_of<inject_result_t<Modules>&, T> ? S : 0) + ...);
	}
};

constexpr auto module0 = []() {
	return modular_source{};
};

// NOTE: If using reference_source / object_source, should we do eager injection or lazy injection?
//       Do we really want to reproduce the basic sources as callbacks?
//       We should receive a modular container as parameter? Maybe?
constexpr auto module1 = [](kangaru::source_reference_wrapper<kangaru::reflected_return_type<decltype(module0), 8>> m0) {
	return modular_source{
		m0,
		[] {
			fmt::println("call service_1_a");
			return service_1_a{.i = 9};
		},
		[](service_1_a s1a){ return service_1_b{.s1a = s1a}; },
	};
};

constexpr auto module2 = [](kangaru::source_reference_wrapper<kangaru::reflected_return_type<decltype(module1), 8>> m1) {
	return modular_source{
		m1,
		[](service_1_a s1a){ return service_2_a{.s1a = s1a, .i = 8}; },
		[](service_1_b s1b, service_2_a s2a) {
			fmt::println("call service_2_b");
			return service_2_b{.s1b = s1b, .s2a = s2a};
		},
	};
};

auto main() -> int {
	auto source = modular_container{kangaru::none_source{}, module0, module1, module2};
	auto injector = kangaru::make_spread_injector(kangaru::ref(source));
	
	injector([](service_2_b& s2b) {
		fmt::println("patate {}", s2b.s1b.s1a.i);
	});
}
