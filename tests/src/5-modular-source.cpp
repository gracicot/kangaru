#include "kangaru/detail/deducer.hpp"
#include "kangaru/detail/injector.hpp"
#include "kangaru/detail/recursive_source.hpp"
#include "kangaru/detail/source.hpp"
#include "kangaru/detail/source_reference_wrapper.hpp"
#include "kangaru/detail/source_types.hpp"
#include "kangaru/detail/type_traits.hpp"
#include "kangaru/detail/utility.hpp"
#include <concepts>
#include <optional>
#include <kangaru/kangaru.hpp>

#include <kangaru/detail/define.hpp>

#include <fmt/core.h>

namespace kangaru {
	namespace detail::modular_source_private {
		template<typename Function, typename Context>
		concept callable_module_member_initializer =
			    source<Context>
			and function_object<Function>
			and callable<Function&&, Context>
			and source<detail::call_result_t<Function&&, Context>>;
		
		template<source Constructed, function_object... Modules>
		inline constexpr auto incremental_source_complete_v = false;
		
		template<source... Constructed, function_object Head, function_object... Tail>
			requires callable_module_member_initializer<Head, composed_source<Constructed...>>
		inline constexpr auto incremental_source_complete_v<composed_source<Constructed...>, Head, Tail...> =
			    callable_module_member_initializer<Head, composed_source<Constructed...>>
			and incremental_source_complete_v<composed_source<Constructed..., source_reference_wrapper<detail::call_result_t<Head, composed_source<Constructed...>>>>, Tail...>;
		
		template<source... Constructed>
		inline constexpr auto incremental_source_complete_v<composed_source<Constructed...>> = true;
		
		template<source Source>
		struct use_source {
			Source&& source;
			
			// We ignore the source since we have one already constructed
			constexpr auto operator()(forwarded_source auto&&) && { return std::move(source); }
		};
		
		template<source Source>
		using injection_source = with_recursion<with_construction<Source, exhaustive_construction>>;
		
		template<function_object Function>
		struct modular_module_initializer {
			template<forwarded_source Source>
			constexpr auto operator()(Source&& source) && ->
				with_source_reference_wrapping<reference_source<detail::call_result_t<detail::call_result_t<make_strict_spread_injector_function, injection_source<std::remove_reference_t<Source>>>, Function>>>
			requires(
				callable<detail::call_result_t<make_strict_spread_injector_function, injection_source<std::remove_reference_t<Source>>>, Function>
			) {
				auto const injection_source = with_recursion{with_construction{KANGARU5_FWD(source), exhaustive_construction{}}};
				auto injector = KANGARU5_NO_ADL(make_strict_spread_injector)(KANGARU5_NO_ADL(ref)(injection_source));
				using type = decltype(std::move(injector)(std::move(function)));
				auto construct_source = in_place_construct{[&]() -> type { return std::move(injector)(std::move(function)); }};
				return with_source_reference_wrapping{reference_source<type>{std::move(construct_source)}};
			};
			
			Function function;
		};
		
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
		
		// This type is to workaround clang 18 and older. If those entities are defined in the private scope of a class,
		// clang will reject this code.
		template<typename...  Modules>
		struct modular_container_base {
			template<typename T, std::size_t... S>
			consteval static auto index_of(std::index_sequence<S...>) {
				return ((source_of<source_reference_wrapper<reflected_return_type<Modules, 8>>, T> ? S : 0) + ...);
			}
			
			struct module_for_type {
				template<injectable T>
				using type = std::tuple_element_t<
					index_of<T>(std::index_sequence_for<Modules...>{}),
					std::tuple<source_reference_wrapper<reflected_return_type<Modules, 8>>...>
				>;
			};
		};
	} // namespace detail::modular_source_private
	
	template<typename... Functions>
		requires(
			   detail::modular_source_private::incremental_source_complete_v<Functions...>
			or detail::modular_source_private::incremental_source_complete_v<composed_source<>, Functions...>
		)
	struct incremental_source;
	
	template<source... Constructed, function_object MakeSource, function_object... Next>
		requires detail::modular_source_private::incremental_source_complete_v<composed_source<Constructed...>, MakeSource, Next...>
	struct incremental_source<composed_source<Constructed...>, MakeSource, Next...> {
	private:
		using source_t = detail::call_result_t<MakeSource, composed_source<Constructed...>>;
		using constructed_t = composed_source<Constructed..., source_reference_wrapper<source_t>>;
		using next_t = incremental_source<constructed_t, Next...>;
		
	public:
		incremental_source(incremental_source const&) = delete;
		auto operator=(incremental_source const&) -> incremental_source& = delete;
		incremental_source(incremental_source&&) = delete;
		auto operator=(incremental_source&&) -> incremental_source& = delete;
		~incremental_source() = default;
		
		constexpr incremental_source(composed_source<Constructed...> accumulated, MakeSource make_source, Next... next) :
			source{std::move(make_source)(accumulated)},
			next{KANGARU5_NO_ADL(composed_source_cat)(accumulated, KANGARU5_NO_ADL(tie)(source)), next...} {}
		
		template<injectable T, forwarded<incremental_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<incremental_source> Self> requires source_of<detail::forward_like_t<Self, next_t>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).next);
		}
		
		source_t source;
		next_t next;
	};
	
	template<function_object MakeSource, function_object... Next>
		requires detail::modular_source_private::incremental_source_complete_v<composed_source<>, MakeSource, Next...>
	struct incremental_source<MakeSource, Next...> {
	private:
		using source_t = detail::call_result_t<MakeSource, composed_source<>>;
		using constructed_t = tied_source<source_t>;
		using next_t = incremental_source<constructed_t, Next...>;
		
	public:
		incremental_source(incremental_source const&) = delete;
		auto operator=(incremental_source const&) -> incremental_source& = delete;
		incremental_source(incremental_source&&) = delete;
		auto operator=(incremental_source&&) -> incremental_source& = delete;
		~incremental_source() = default;
		
		constexpr incremental_source(MakeSource make_source, Next... next) :
			source{std::move(make_source)(tie())},
			next{KANGARU5_NO_ADL(tie)(source), std::move(next)...} {}
		
		template<injectable T, forwarded<incremental_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<incremental_source> Self> requires source_of<detail::forward_like_t<Self, next_t>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).next);
		}
		
		source_t source;
		next_t next;
	};
	
	template<source... Constructed, function_object MakeSource>
		requires detail::modular_source_private::incremental_source_complete_v<composed_source<Constructed...>, MakeSource>
	struct incremental_source<composed_source<Constructed...>, MakeSource> {
	private:
		using source_t = detail::call_result_t<MakeSource, composed_source<Constructed...>>;
		
	public:
		constexpr incremental_source(composed_source<Constructed...> accumulated, MakeSource make_head) :
			source{std::move(make_head)(accumulated)} {}
		
		template<injectable T, forwarded<incremental_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		source_t source;
	};
	
	template<function_object Head>
		requires detail::modular_source_private::incremental_source_complete_v<composed_source<>, Head>
	struct incremental_source<Head> {
	private:
		using source_t = detail::call_result_t<Head, composed_source<>>;
		
	public:
		explicit constexpr incremental_source(Head make_head) :
			source{std::move(make_head)(tie())} {}
		
		template<injectable T, forwarded<incremental_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		source_t source;
	};
	
	template<source... Constructed>
	struct incremental_source<composed_source<Constructed...>> {
		explicit constexpr incremental_source(composed_source<Constructed...>) {}
	};
	
	template<>
	struct incremental_source<> {};
	
	template<typename... Functions>
	incremental_source(Functions...) -> incremental_source<Functions...>;
	
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
		using impl_t = incremental_source<detail::modular_source_private::use_source<Source>, detail::modular_source_private::modular_source_initializer<Lambdas>...>;
		
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
		
		template<injectable T, forwarded<modular_source> Self> requires source_of<detail::forward_like_t<Self, decltype(std::declval<impl_t>().next)>, T>
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
	
	template<function_object... Modules>
		requires(
			std::constructible_from<
				incremental_source<
					detail::modular_source_private::modular_module_initializer<Modules>...
				>,
				detail::modular_source_private::modular_module_initializer<Modules>...
			>
		)
	struct modular_container {
	private:
		using impl_type = incremental_source<detail::modular_source_private::modular_module_initializer<Modules>...>;
		
	public:
		explicit(sizeof...(Modules) == 1) constexpr modular_container(Modules... modules) :
			impl{detail::modular_source_private::modular_module_initializer<Modules>{std::move(modules)}...} {}
		
		template<injectable T, forwarded<modular_container> Self> requires(
			source_of<
				with_recursion<
					with_construction<
						sealed_source<
							with_provide_using_source<
								fwd_ref_result_t<detail::forward_like_t<Self, impl_type>&&>,
								detail::modular_source_private::modular_container_base<Modules...>::module_for_type::template type
							>
						>,
						exhaustive_construction
					>
				>,
				T
			>
		)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(
				with_recursion{
					with_construction{
						sealed_source{
							make_source_with_provide_using_source<detail::modular_source_private::modular_container_base<Modules...>::module_for_type::template type>(
								KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).impl)
							)
						}, exhaustive_construction{}
					}
				}
			);
		}
		
	private:
		impl_type impl;
	};
	
	template<typename... Modules> requires((... and (std::is_function_v<Modules> or function_object<Modules>)))
	using module_dependencies = tied_source<reflected_return_type<std::decay_t<Modules>, 8>...>;
	
	template<source Source>
	struct make_lazy_initialized_source_function {
	private:
		template<typename>
		using pick_source = Source;
		
	public:
		constexpr auto operator()(forwarded_source auto&& from)
		requires(
			callable<strict_spread_injector<ref_result_t<decltype(from)&>>, constructor_function<Source>>
		) {
			return sealed_source{
				make_source_with_provide_using_source<pick_source>(
					make_source_with_lazy_evaluation_of<Source>(
						with_construction{
							KANGARU5_FWD(from),
							basic_exhaustive_construction{make_strict_spread_injector_function{}},
						}
					)
				),
			};
		}
	};
	
	template<source Source>
	inline constexpr auto make_lazy_initialized_source(forwarded_source auto&& from) {
		return make_lazy_initialized_source_function<Source>{}(KANGARU5_FWD(from));
	}
} // namespace kangaru

// Here's many classes, all have some relations with others
struct service_1_a { int i; };
struct service_1_b { service_1_a& s1a; int i; };
struct service_1_c { service_1_b s1b; };
struct service_1_d {
	explicit service_1_d(service_1_c& s1c) : s1c{s1c} {
		fmt::println("init service_1_d");
	}
	
	service_1_c& s1c;
};

struct agg1 {
	service_1_a& a;
	service_1_b b;
	service_1_c c;
};

struct service_2_a { service_1_a& s1a; agg1 agg; };
struct service_2_b { service_1_c& s1c; service_2_a& s2a; };

struct agg2 {
	agg1 agg;
	service_1_a& s1a;
	service_1_b s1b;
	service_1_c& s1c;
	service_2_a& s2a;
};

struct service_3_a_base { service_2_b& s2b; service_1_a& s1a; };
struct service_3_a : service_3_a_base {};

struct service_3_b_base {};
struct service_3_b : service_3_b_base {
	explicit constexpr service_3_b(service_3_a_base& s3a) : s3a{s3a} {}
	service_3_a_base& s3a;
};

struct service_3_c {
	explicit service_3_c(std::shared_ptr<service_3_b_base> s3b) : s3b(s3b) {}
	std::shared_ptr<service_3_b_base> s3b;
};


// Here comes the modular sources!!
//
// I'm kinda showcasing here all choices of syntax we can use to define modular source.
// I'm also showing all utilities I defined to create modules.
//
// Initially, I was set up to have only one way to create a module:
//
//     return kangaru::modular_source{
//         dependencies,
//         lambda1,
//         lambda2,
//         lambda3,
//         ...
//     };
//
// But having something like this made it more practical and much shorter:
//
//    return kangaru::make_modular_source<
//        type1,
//        type2,
//        type3,
//        ...
//    >(dependencies);
//
// But then, I needed to deal with the fact that modular sources are immovable and we want to
// be compatible with type erasure. Thus the _in_place functions were added.

// Let's see a simple case. We return a modular source with one member, an object source of int.
constexpr auto module0() {
	// Modular sources are immovable, but we can still return them because of RVO.
	return kangaru::modular_source{
		// We start this modular source from nothing.
		[]{
			return kangaru::object_source{int{9}};
		}
	};
}

// A more complex example. Our module have one dependency.
// We also return the modular source in a type erased wrapper.
// If you hover the module0 function and inspect the return type, you'll understand why.
// Types containing lambdas can explode pretty quick.
auto module1(kangaru::module_dependencies<decltype(module0)> dependencies) -> kangaru::any_source_of<service_1_a&, service_1_b, service_1_c&, std::shared_ptr<service_1_d>> {
	return kangaru::make_modular_source_in_place(dependencies,
		// Can be lambdas just like we see in module0.
		[](int i) { // int dependency, from module0. We can write them explicitly like this, but generally, you won't.
			return kangaru::reference_source<service_1_a>{i};
		},
		[](service_1_a& s1a, int i) { // service_1_a dependency from the source above and int dependency from module0.
			return kangaru::object_source<service_1_b>{s1a, i};
		},
		// Instead of listing dependencies, let's use a generic constructor function, and let kangaru deduce the parameters!
		kangaru::constructor_function<kangaru::reference_source<service_1_c>>{},
		kangaru::make_lazy_initialized_source_function<kangaru::shared_pointer_source<service_1_d>>{}
	);
}

// Here we decided to return auto, this is more optimizable, but we can't separate cpp/hpp like this.
auto module2(kangaru::module_dependencies<decltype(module1)> dependencies) {
	// We don't need to use in_place_construction since we don't construct a any_source_of.
	// However, the make_modular_source function is still useful.
	// We can simply make a list of all the sources the module should be composed of, and let kangaru autowire everything.
	return kangaru::make_modular_source<
		kangaru::reference_source<service_2_a>,
		kangaru::reference_source<service_2_b>
	>(dependencies);
}

// Let's declare the function then define it, just to show we can separate declaration and definition :D
auto module3(kangaru::module_dependencies<decltype(module2), decltype(module1)> dependencies)
	-> kangaru::any_source_of<service_3_a_base&, std::shared_ptr<service_3_b_base>, service_3_c&>;

auto module3(kangaru::module_dependencies<decltype(module2), decltype(module1)> dependencies)
	-> kangaru::any_source_of<service_3_a_base&, std::shared_ptr<service_3_b_base>, service_3_c&> {
	// Here we use the generic construct_in_place, that transforms move construction to RVO.
	// Normally, for modular source, you should either just use make_modular_source[_in_place],
	// using the in place version if returning a any_source_of
	return kangaru::construct_in_place<kangaru::modular_source>(dependencies,
		kangaru::constructor_function<kangaru::derived_reference_source<service_3_a_base, service_3_a>>{},
		kangaru::constructor_function<kangaru::derived_shared_pointer_source<service_3_b_base, service_3_b>>{},
		// We can hijack the injection process and just get the incremental source instead to create something.
		// It is allowed to copy the incremental source, it only contains reference of things that will stay alive
		// at least as long as the object we return from this function.
		[](kangaru::source auto module) {
			auto injector = kangaru::make_strict_spread_injector(module);
			auto constructor = kangaru::constructor_function<kangaru::reference_source<service_3_c>>{};
			return std::move(injector)(constructor); // Returns a kangaru::reference_source<service_3_c>
		}
	);
}

auto main() -> int {
	auto source = kangaru::modular_container{module0, module1, module2, module3};
	auto injector = kangaru::make_spread_injector(kangaru::ref(source));
	
	fmt::println("before initializing service_1_d");
	kangaru::provide<std::shared_ptr<service_1_d>>(source);
	fmt::println("after initializing service_1_d");
	
	injector([](service_2_b& s2b, service_1_b s1b, agg2 agg, service_3_c&) {
		fmt::println("potato {} {} {}", s2b.s1c.s1b.i, s1b.i, agg.s2a.agg.a.i);
	});
}
