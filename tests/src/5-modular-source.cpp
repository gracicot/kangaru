#include "kangaru/detail/recursive_source.hpp"
#include "kangaru/detail/source.hpp"
#include "kangaru/detail/source_reference_wrapper.hpp"
#include "kangaru/detail/source_types.hpp"
#include "kangaru/detail/utility.hpp"
#include <concepts>
#include <kangaru/kangaru.hpp>

#include <kangaru/detail/define.hpp>

#include <fmt/core.h>

namespace kangaru {
	namespace detail::modular_source {
		template<typename... Sources>
		using injection_source = with_recursion<with_construction<composed_source<Sources...>, exhaustive_constructor>>;
		
		template<typename MakeHeadSource, typename... Constructed>
			requires callable<detail::type_traits::call_result_t<make_strict_spread_injector_function, injection_source<Constructed...>>, MakeHeadSource>
		inline constexpr auto construct_head_source(MakeHeadSource&& make_head, Constructed... constructed) {
			return make_strict_spread_injector(with_recursion{with_construction{compose(constructed...), exhaustive_constructor{}}})(std::move(make_head));
		}
		
		template<typename MakeHeadSource>
		using constructed_head_source_t = reflected_return_type<MakeHeadSource, 8>;
		
		template<typename, typename...>
		inline constexpr auto usable_as_head_v = false;
		
		template<typename... Back, typename Head, typename... Tail>
		inline constexpr auto usable_as_head_v<std::tuple<Back...>, Head, Tail...> = callable<
			detail::type_traits::call_result_t<
				make_strict_spread_injector_function,
				injection_source<Back...>
			>,
			Head
		> && usable_as_head_v<std::tuple<Back..., ref_result_t<constructed_head_source_t<Head>&>>, Tail...>;
		
		template<typename... Back>
		inline constexpr auto usable_as_head_v<std::tuple<Back...>> = true;
		
		template<typename... Functions>
		struct modular_source_impl;
		
		template<typename Head, typename... Tail>
		struct modular_source_impl<Head, Tail...> {
			modular_source_impl(modular_source_impl const&) = delete;
			auto operator=(modular_source_impl const&) -> modular_source_impl& = delete;
			modular_source_impl(modular_source_impl&&) = delete;
			auto operator=(modular_source_impl&&) -> modular_source_impl& = delete;
			~modular_source_impl() = default;
			
			constexpr modular_source_impl(Head make_head, Tail... tail, reference_wrapper auto... constructed) :
				head{construct_head_source(std::move(make_head), constructed...)},
				tail{tail..., constructed..., ref(head)} {}
			
			using head_t = constructed_head_source_t<Head>;
			using tail_t = modular_source_impl<Tail...>;
			
			template<injectable T, forwarded<modular_source_impl> Self> requires source_of<head_t, T>
			constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
				return kangaru::provide<T>(KANGARU5_FWD(source).head);
			}
			
			template<injectable T, forwarded<modular_source_impl> Self> requires source_of<tail_t, T>
			constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
				return kangaru::provide<T>(KANGARU5_FWD(source).tail);
			}
			
			head_t head;
			tail_t tail;
		};
		
		template<typename Head>
		struct modular_source_impl<Head> {
			constexpr modular_source_impl(Head make_head, reference_wrapper auto... constructed) :
				head{construct_head_source(std::move(make_head), constructed...)} {}
			
			using head_t = constructed_head_source_t<Head>;
			
			template<injectable T, forwarded<modular_source_impl> Self> requires source_of<head_t, T>
			constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
				return kangaru::provide<T>(KANGARU5_FWD(source).head);
			}
			
			head_t head;
		};
		
		template<>
		struct modular_source_impl<> {};
		
		template<kangaru::source Source>
		struct use_source {
			Source source;
			
			constexpr auto operator()() && { return std::move(source); }
		};
		
		template<typename Function>
		constexpr auto make_module(Function function) {
			return [function](auto&&... args) -> with_source_reference_wrapping<reference_source<detail::type_traits::call_result_t<Function, decltype(args)...>>> requires callable<Function const&, decltype(args)...> {
				using type = decltype(function(KANGARU5_FWD(args)...));
				auto construct_source = in_place_construct{[&]() -> type { return function(KANGARU5_FWD(args)...); }};
				return with_source_reference_wrapping{reference_source<type>{construct_source}};
			};
		}
		
		template<typename... MakeModuleSources>
			requires detail::modular_source::usable_as_head_v<std::tuple<>, decltype(make_module(std::declval<MakeModuleSources>()))...>
		struct modular_container_impl {
		private:
			using modules_t = detail::modular_source::modular_source_impl<decltype(make_module(std::declval<MakeModuleSources>()))...>;
			
		public:
			explicit constexpr modular_container_impl(MakeModuleSources... modules) : modules{make_module(modules)...} {}
			
			template<injectable T, forwarded<modular_container_impl> Self>
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
	
	template<source Source = none_source, typename... Lambdas>
		requires detail::modular_source::usable_as_head_v<std::tuple<>, detail::modular_source::use_source<Source>, Lambdas...>
	struct modular_source {
	private:
		using impl_t = detail::modular_source::modular_source_impl<detail::modular_source::use_source<Source>, Lambdas...>;
		
	public:
		constexpr modular_source() requires(sizeof...(Lambdas) == 0 and std::default_initializable<Source>) :
			impl{detail::modular_source::use_source<Source>{Source{}}} {}
		
		explicit(sizeof...(Lambdas) == 0) constexpr modular_source(Source source, Lambdas... lambdas) : impl{detail::modular_source::use_source<Source>{std::move(source)}, std::move(lambdas)...} {}
		
		template<injectable T, forwarded<modular_source> Self> requires source_of<detail::utility::forward_like_t<Self, typename impl_t::tail_t>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			// Here we must skip first head of the incremental source. This is because we don't want to provide
			// from source of other modules.
			return kangaru::provide<T>(KANGARU5_FWD(source).impl.tail);
		}
		
	private:
		impl_t impl;
	};
	
	template<source Source, typename... Lambdas>
	inline constexpr auto make_modular_source(Source source, Lambdas... lambdas) {
		return modular_source<Source, Lambdas...>{std::move(source), lambdas...};
	}
	
	template<source... Sources, source Source>
	inline constexpr auto make_modular_source(Source source) {
		return modular_source<Source, kangaru::constructor_function<Sources>...>{std::move(source), kangaru::constructor_function<Sources>{}...};
	}
	
	template<source... Sources>
	inline constexpr auto make_modular_source() {
		return modular_source<none_source, kangaru::constructor_function<Sources>...>{none_source{}, kangaru::constructor_function<Sources>{}...};
	}
	
	template<typename... Modules>
		requires std::constructible_from<detail::modular_source::modular_container_impl<Modules...>, Modules...>
	struct modular_container {
	private:
		using impl_type = detail::modular_source::modular_container_impl<Modules...>;
		
	public:
		explicit(sizeof...(Modules) == 1) constexpr modular_container(Modules... modules) : impl{std::move(modules)...} {}
		
		template<injectable T, forwarded<modular_container> Self> requires source_of<with_recursion<with_construction<fwd_ref_result_t<detail::utility::forward_like_t<Self, impl_type>&&>, exhaustive_constructor>>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			// Here we must skip first head of the incremental source. This is because we don't want to provide
			// from source of other modules.
			return kangaru::provide<T>(with_recursion{with_construction{fwd_ref(KANGARU5_FWD(source).impl), exhaustive_constructor{}}});
		}
		
	private:
		impl_type impl;
	};
	
	template<typename... Modules>
	using module_dependencies = tied_source<reflected_return_type<Modules, 8>...>;
}

struct service_1_a { int i; };
struct service_1_b { service_1_a s1a; };

struct agg1 {
	service_1_a a;
	service_1_b& b;
};

struct service_2_a { service_1_a s1a; agg1 agg; };
struct service_2_b { service_1_b& s1b; service_2_a& s2a; };

struct agg2 {
	agg1 agg;
	service_1_a s1a;
	service_1_b& s1b;
	service_2_a& s2a;
};

constexpr auto module0() {
	return kangaru::modular_source{
		kangaru::none_source{},
		[]{
			return kangaru::object_source{int{9}};
		},
	};
}

constexpr auto module1(kangaru::module_dependencies<decltype(module0)> dependencies) {
	return kangaru::make_modular_source<
		kangaru::object_source<service_1_a>,
		kangaru::reference_source<service_1_b>
	>(dependencies);
}

constexpr auto module2(kangaru::module_dependencies<decltype(module1), decltype(module0)> dependencies) {
	return kangaru::make_modular_source<
		kangaru::reference_source<service_2_a>,
		kangaru::reference_source<service_2_b>
	>(dependencies);
}

auto main() -> int {
	auto source = kangaru::modular_container{module0, module1, module2};
	auto injector = kangaru::make_spread_injector(kangaru::ref(source));
	
	injector([](service_2_b& s2b, service_1_a s1a, agg2 agg) {
		fmt::println("patate {} {} {}", s2b.s1b.s1a.i, s1a.i, agg.s2a.agg.a.i);
	});
}
