#include "kangaru/detail/source_reference_wrapper.hpp"
#include <kangaru/kangaru.hpp>

#include <kangaru/detail/define.hpp>

#include <fmt/core.h>

namespace kangaru {
	namespace detail::modular_source {
		template<kangaru::source Source, typename, typename... Functions>
		struct modular_source_impl;
		
		template<kangaru::source Source, typename... Constructed, typename Head, typename... Tail>
		struct modular_source_impl<Source, std::tuple<Constructed...>, Head, Tail...> {
			modular_source_impl(modular_source_impl const&) = delete;
			auto operator=(modular_source_impl const&) -> modular_source_impl& = delete;
			modular_source_impl(modular_source_impl&&) = delete;
			auto operator=(modular_source_impl&&) -> modular_source_impl& = delete;
			~modular_source_impl() = default;
			
			constexpr modular_source_impl(Source source, Constructed... constructed, Head make_head, Tail... tail) :
				head{make_strict_spread_injector(compose(source, constructed...))(make_head)},
				tail{source, constructed..., ref(head), tail...} {}
			
			using service_type = decltype(make_strict_spread_injector(compose(std::declval<Source>(), std::declval<Constructed>()...))(std::declval<Head>()));
			using tail_service_type = modular_source_impl<Source, std::tuple<Constructed..., ref_result_t<service_type&>>, Tail...>;
			
			template<injectable T, forwarded<modular_source_impl> Self> requires source_of<service_type, T>
			constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
				return kangaru::provide<T>(KANGARU5_FWD(source).head);
			}
			
			template<injectable T, forwarded<modular_source_impl> Self> requires source_of<tail_service_type, T>
			constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
				return kangaru::provide<T>(KANGARU5_FWD(source).tail);
			}
			
			service_type head;
			modular_source_impl<Source, std::tuple<Constructed..., ref_result_t<service_type&>>, Tail...> tail;
		};
		
		// TODO: This takes memory space
		template<kangaru::source Source, typename... Constructed>
		struct modular_source_impl<Source, std::tuple<Constructed...>> {
			constexpr modular_source_impl(Source source, Constructed...) {}
		};
	}
	
	template<source Source = none_source, typename... Lambdas>
	struct modular_source {
	private:
		using impl_t = detail::modular_source::modular_source_impl<Source, std::tuple<>, Lambdas...>;

	public:
		constexpr modular_source() requires(sizeof...(Lambdas) == 0 and std::default_initializable<Source>) :
			impl{Source{}} {}
		
		explicit(sizeof...(Lambdas) == 0) constexpr modular_source(Source source, Lambdas... lambdas) : impl{std::move(source), std::move(lambdas)...} {}
		
		template<injectable T, forwarded<modular_source> Self> requires source_of<detail::utility::forward_like_t<Self, impl_t>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).impl);
		}
	
	private:
		impl_t impl;
	};
	
	template<source Source, typename... Lambdas>
	auto make_modular_source(Source source, Lambdas... lambdas) {
		return modular_source<Source, Lambdas...>{std::move(source), lambdas...};
	}

	template<typename Function>
	constexpr auto make_module(Function function) {
		return [function](auto&&... args) -> with_source_reference_wrapping<reference_source<detail::type_traits::call_result_t<Function, decltype(args)...>>> requires callable<Function const&, decltype(args)...> {
			using type = decltype(function(KANGARU5_FWD(args)...));
			auto construct_source = in_place_construct{[&]() -> type { return function(KANGARU5_FWD(args)...); }};
			return with_source_reference_wrapping{reference_source<type>{construct_source}};
		};
	}

	template<auto function>
	constexpr auto make_module() {
		return [](auto&&... args) -> with_source_reference_wrapping<reference_source<kangaru::detail::type_traits::call_result_t<decltype(function), decltype(args)...>>> requires callable<decltype(function), decltype(args)...> {
			using type = decltype(function(KANGARU5_FWD(args)...));
			auto construct_source = in_place_construct{[&]() -> type { return function(KANGARU5_FWD(args)...); }};
			return with_source_reference_wrapping{reference_source<type>{construct_source}};
		};
	}
	
	template<typename Source, typename... MakeModuleSources>
	struct modular_container {
	private:
		using modules_t = modular_source<Source, decltype(make_module(std::declval<MakeModuleSources>()))...>;

	public:
		explicit constexpr modular_container(Source source, MakeModuleSources... modules) : modules{std::move(source), make_module(modules)...} {}

		template<injectable T, forwarded<modular_container> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T
		requires (
			((source_of<source_reference_wrapper<reflected_return_type<MakeModuleSources, 8>>, T> ? 1 : 0) + ...) == 1
		) {
			using source_t = std::tuple_element_t<index_of<T, decltype(source)>(std::index_sequence_for<MakeModuleSources...>{}), std::tuple<source_reference_wrapper<reflected_return_type<MakeModuleSources, 8>>...>>;
			return kangaru::provide<T>(
				kangaru::provide<source_t>(KANGARU5_FWD(source).modules)
			);
		}

	private:
		template<typename T, typename Self, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			return ((source_of<source_reference_wrapper<reflected_return_type<MakeModuleSources, 8>>, T> ? S : 0) + ...);
		}
		
		modules_t modules;
	};
}

struct service_1_a { int i; };
struct service_1_b { service_1_a s1a; };

struct service_2_a { service_1_a s1a; int i; };
struct service_2_b { service_1_b& s1b; service_2_a& s2a; };

auto module0() {
	return kangaru::modular_source{};
}

auto module1(kangaru::source_reference_wrapper<kangaru::reflected_return_type<decltype(module0), 8>> module0) {
	return kangaru::modular_source{
		module0,
		[] {
			fmt::println("call service_1_a");
			return kangaru::reference_source{service_1_a{9}};
		},
		[](service_1_a& s) { return kangaru::reference_source{service_1_b{s}}; }
	};
}

auto module2(kangaru::source_reference_wrapper<kangaru::reflected_return_type<decltype(module1), 8>> module1) {
	return kangaru::modular_source{
		module1,
		[](service_1_a& s1a) { return kangaru::reference_source{service_2_a{s1a, 8}}; },
		[](service_1_b& s1b, service_2_a& s2a) {
			fmt::println("call service_2_b");
			return kangaru::reference_source{service_2_b{s1b, s2a}};
		}
	};
}

auto main() -> int {
	auto source = kangaru::modular_container{kangaru::none_source{}, module0, module1, module2};
	auto injector = kangaru::make_spread_injector(kangaru::ref(source));
	
	injector([](service_2_b& s2b) {
		fmt::println("patate {}", s2b.s1b.s1a.i);
	});
}
