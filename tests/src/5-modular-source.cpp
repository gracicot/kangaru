#include "kangaru/detail/injector.hpp"
#include "kangaru/detail/recursive_source.hpp"
#include "kangaru/detail/source.hpp"
#include "kangaru/detail/source_reference_wrapper.hpp"
#include "kangaru/detail/source_types.hpp"
#include "kangaru/detail/type_traits.hpp"
#include "kangaru/detail/utility.hpp"
#include <concepts>
#include <kangaru/kangaru.hpp>

#include <kangaru/detail/define.hpp>

#include <fmt/core.h>

namespace kangaru {
	namespace detail::modular_source {
		template<kangaru::source Source>
		using injection_source = with_recursion<with_construction<Source, exhaustive_constructor>>;
		
		template<function_object MakeHeadSource, kangaru::source Constructed>
			requires(
				   callable<MakeHeadSource&&, injection_source<Constructed>>
				or callable<detail::type_traits::call_result_t<make_strict_spread_injector_function, injection_source<Constructed>>, MakeHeadSource>
			)
		inline constexpr auto construct_head_source(MakeHeadSource&& make_head, Constructed constructed) {
			auto const source = with_recursion{with_construction{constructed, exhaustive_constructor{}}};
			if constexpr (callable<MakeHeadSource, injection_source<Constructed>>) {
				return std::move(make_head)(std::move(source));
			} else {
				return make_strict_spread_injector(std::move(source))(std::move(make_head));
			}
		}
		
		template<typename Function, typename Context>
		concept module_member_initializer =
			    kangaru::source<Context>
			and function_object<Function>
			and (
				   callable<Function&&, injection_source<Context>>
				or reflectable_function<Function, 8>
			);
		
		template<function_object MakeHeadSource, kangaru::source Constructed>
			requires module_member_initializer<MakeHeadSource, Constructed>
		struct constructed_head_source {};
		
		template<function_object MakeHeadSource, kangaru::source Constructed>
			requires(callable<MakeHeadSource&&, injection_source<Constructed>>)
		struct constructed_head_source<MakeHeadSource, Constructed> {
			using type = type_traits::call_result_t<MakeHeadSource&&, injection_source<Constructed>>;
		};
		
		template<function_object MakeHeadSource, kangaru::source Constructed>
			requires(
				    not callable<MakeHeadSource&&, injection_source<Constructed>>
				and reflectable_function<MakeHeadSource, 8>
			)
		struct constructed_head_source<MakeHeadSource, Constructed> {
			using type = reflected_return_type<MakeHeadSource, 8>;
		};
		
		template<typename MakeHeadSource, kangaru::source Constructed>
			requires module_member_initializer<MakeHeadSource, Constructed>
		using constructed_head_source_t = typename constructed_head_source<MakeHeadSource, Constructed>::type;
		
		template<typename Function, typename Context>
		concept callable_module_member_initializer =
			    module_member_initializer<Function, Context>
			and (
				   callable<Function&&, injection_source<Context>>
				or callable<
					detail::type_traits::call_result_t<
						make_strict_spread_injector_function,
						injection_source<Context>
					>,
					Function
				>
			);
		
		template<kangaru::source Constructed, function_object... Modules>
		inline constexpr auto usable_as_head_v = false;
		
		template<kangaru::source Constructed, function_object Head, function_object... Tail>
			requires module_member_initializer<Head, Constructed>
		inline constexpr auto usable_as_head_v<Constructed, Head, Tail...> =
			    callable_module_member_initializer<Head, Constructed>
			and usable_as_head_v<composed_source_cat_t<Constructed, tied_source<constructed_head_source_t<Head, Constructed>>>, Tail...>;
		
		template<kangaru::source Constructed>
		inline constexpr auto usable_as_head_v<Constructed> = true;
		
		template<kangaru::source Constructed, function_object... Functions>
		struct modular_source_impl;
		
		template<kangaru::source Constructed, function_object Head, function_object... Tail>
		struct modular_source_impl<Constructed, Head, Tail...> {
			using head_t = constructed_head_source_t<Head, Constructed>;
			using tail_constructed_t = composed_source_cat_t<Constructed, tied_source<head_t>>;
			using tail_t = modular_source_impl<tail_constructed_t, Tail...>;
			
			modular_source_impl(modular_source_impl const&) = delete;
			auto operator=(modular_source_impl const&) -> modular_source_impl& = delete;
			modular_source_impl(modular_source_impl&&) = delete;
			auto operator=(modular_source_impl&&) -> modular_source_impl& = delete;
			~modular_source_impl() = default;
			
			constexpr modular_source_impl(Head make_head, Tail... tail) requires std::same_as<composed_source<>, Constructed> :
				head{construct_head_source(std::move(make_head), tie())},
				tail{KANGARU5_NO_ADL(tie)(head), tail...} {}
			
			constexpr modular_source_impl(Constructed accumulated, Head make_head, Tail... tail) :
				head{construct_head_source(std::move(make_head), accumulated)},
				tail{KANGARU5_NO_ADL(composed_source_cat)(accumulated, KANGARU5_NO_ADL(tie)(head)), tail...} {}
			
			
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
		
		template<kangaru::source Constructed, function_object Head>
		struct modular_source_impl<Constructed, Head> {
			constexpr modular_source_impl(Constructed accumulated, Head make_head) :
				head{construct_head_source(std::move(make_head), accumulated)} {}
			
			using head_t = constructed_head_source_t<Head, Constructed>;
			
			template<injectable T, forwarded<modular_source_impl> Self> requires source_of<head_t, T>
			constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
				return kangaru::provide<T>(KANGARU5_FWD(source).head);
			}
			
			head_t head;
		};
		
		template<kangaru::source Constructed>
		struct modular_source_impl<Constructed> {
			modular_source_impl() = default;
			explicit constexpr modular_source_impl(Constructed) {}
		};
		
		template<kangaru::source Source>
		struct use_source {
			Source source;
			
			constexpr auto operator()() && { return std::move(source); }
		};
		
		template<function_object Function>
		struct make_module_function {
			constexpr auto operator()(auto&&... args) -> with_source_reference_wrapping<reference_source<detail::type_traits::call_result_t<Function, decltype(args)...>>> requires callable<Function const&, decltype(args)...> {
				using type = decltype(function(KANGARU5_FWD(args)...));
				auto construct_source = in_place_construct{[&]() -> type { return function(KANGARU5_FWD(args)...); }};
				return with_source_reference_wrapping{reference_source<type>{construct_source}};
			};
			
			Function function;
		};
		
		template<typename Function>
		constexpr auto make_module(Function function) {
			return make_module_function<Function>{function};
		}
		
		template<function_object... MakeModuleSources>
			requires detail::modular_source::usable_as_head_v<composed_source<>, decltype(make_module(std::declval<MakeModuleSources>()))...>
		struct modular_container_impl {
		private:
			using modules_t = detail::modular_source::modular_source_impl<composed_source<>, decltype(make_module(std::declval<MakeModuleSources>()))...>;
			
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
			
		//private:
			template<typename T, typename Self, std::size_t... S>
			constexpr static auto index_of(std::index_sequence<S...>) {
				return ((source_of<source_reference_wrapper<reflected_return_type<MakeModuleSources, 8>>, T> ? S : 0) + ...);
			}
			
			modules_t modules;
		};
		
		// TODO: Do we actually need a constrait for that?
		template<typename SourceOrFunction>
		concept make_modular_source_parameter =
			   kangaru::source<SourceOrFunction>
			or (
				    reflectable_function<SourceOrFunction, 8>
				and std::default_initializable<SourceOrFunction>
				and not pointer<SourceOrFunction>
			);
		
		// TODO: We already wrap calling the function, and the function itself is now wrapped.
		//       Can we remove a level? Can we go back to a simple `constructor_function` or
		//       lazy strictly requires having two paths for everything?
		//
		// TODO: It seems like if a module initializer is a function taking a source but is not callable,
		//       we end up with the function in the modular source, which is not what we intended.
		template<make_modular_source_parameter SourceOrFunction>
		struct module_initializer {
			template<typename... Args>
				requires (
					    reflectable_function<SourceOrFunction, 8>
					and std::default_initializable<SourceOrFunction>
					and not pointer<SourceOrFunction>
					and callable<SourceOrFunction, Args&&...>
				)
			constexpr auto operator()(Args&&... args) const -> decltype(auto) {
				return SourceOrFunction{}(KANGARU5_FWD(args)...);
			}
			
			constexpr auto operator()(auto&&... args) const requires(
				    constructor_callable<SourceOrFunction, decltype(args)...>
				and not (
					    reflectable_function<SourceOrFunction, 8>
					and std::default_initializable<SourceOrFunction>
					and not pointer<SourceOrFunction>
				)
			) {
				return KANGARU5_NO_ADL(constructor<SourceOrFunction>)(KANGARU5_FWD(args)...);
			}
		};
	}
	
	template<source Source = none_source, function_object... Lambdas>
		requires detail::modular_source::usable_as_head_v<composed_source<>, detail::modular_source::use_source<Source>, Lambdas...>
	struct modular_source {
	private:
		using impl_t = detail::modular_source::modular_source_impl<composed_source<>, detail::modular_source::use_source<Source>, Lambdas...>;
		
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
	
	template<source Source, function_object... Lambdas>
		requires std::constructible_from<modular_source<Source, Lambdas...>, Source&&, Lambdas&...>
	inline constexpr auto make_modular_source(Source source, Lambdas... lambdas) {
		return modular_source<Source, Lambdas...>{std::move(source), lambdas...};
	}
	
	template<source... Sources, source Source>
		requires std::constructible_from<
			modular_source<Source, detail::modular_source::module_initializer<Sources>...>,
			Source&&,
			detail::modular_source::module_initializer<Sources>...
		>
	inline constexpr auto make_modular_source(Source source) {
		return modular_source<Source, detail::modular_source::module_initializer<Sources>...>{
			std::move(source),
			detail::modular_source::module_initializer<Sources>{}...
		};
	}
	
	template<source... Sources>
		requires std::constructible_from<
			modular_source<none_source, detail::modular_source::module_initializer<Sources>...>,
			none_source,
			detail::modular_source::module_initializer<Sources>...
		>
	inline constexpr auto make_modular_source() {
		return modular_source<none_source, detail::modular_source::module_initializer<Sources>...>{
			none_source{},
			detail::modular_source::module_initializer<Sources>{}...
		};
	}
	
	template<detail::modular_source::make_modular_source_parameter Source, function_object... Lambdas>
		requires std::constructible_from<modular_source<Source, Lambdas...>, Source&&, Lambdas&...>
	inline constexpr auto make_modular_source_in_place(Source source, Lambdas... lambdas) {
		return in_place_construct{
			[source = std::move(source), ...lambdas = std::move(lambdas)]() mutable {
				return modular_source<Source, Lambdas...>{std::move(source), lambdas...};
			},
		};
	}
	
	template<detail::modular_source::make_modular_source_parameter... Sources, source Source>
		requires std::constructible_from<
			modular_source<Source, detail::modular_source::module_initializer<Sources>...>,
			Source&&,
			detail::modular_source::module_initializer<Sources>...
		>
	inline constexpr auto make_modular_source_in_place(Source source) {
		return in_place_construct{
			[source = std::move(source)]() mutable {
				return modular_source<Source, detail::modular_source::module_initializer<Sources>...>{
					std::move(source),
					detail::modular_source::module_initializer<Sources>{}...
				};
			},
		};
	}
	
	template<detail::modular_source::make_modular_source_parameter... Sources>
		requires std::constructible_from<
			modular_source<none_source, detail::modular_source::module_initializer<Sources>...>,
			none_source,
			detail::modular_source::module_initializer<Sources>...
		>
	inline constexpr auto make_modular_source_in_place() {
		return in_place_construct{
			[] {
				return modular_source<none_source, detail::modular_source::module_initializer<Sources>...>{
					none_source{},
					detail::modular_source::module_initializer<Sources>{}...
				};
			},
		};
	}
	
	template<function_object... Modules>
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
	
	template<typename... Modules> requires((... and (std::is_function_v<Modules> or function_object<Modules>)))
	using module_dependencies = tied_source<reflected_return_type<std::decay_t<Modules>, 8>...>;

	// TODO: Completely rework
	template<source Source, source FromSource>
	struct lazy2 {
		explicit constexpr lazy2(FromSource from) : from{std::move(from)} {}
		
		template<injectable T> requires source_of<Source&, T>
		constexpr auto provide() & -> T {
			ensure_initialized();
			return kangaru::provide<T>(*source);
		}
		
		template<injectable T> requires source_of<Source&&, T>
		constexpr auto provide() && -> T {
			ensure_initialized();
			return kangaru::provide<T>(std::move(*source));
		}
		
	private:
		constexpr auto ensure_initialized() -> void {
			if (not source) {
				source.emplace(KANGARU5_NO_ADL(make_strict_spread_injector)(KANGARU5_NO_ADL(ref)(from))(constructor_function<Source>{}));
			}
		}
		
		FromSource from;
		std::optional<Source> source;
	};
	
	// TODO: Completely rework
	template<source Source>
	struct lazy_init {
		constexpr auto operator()(forwarded_source auto&& from)
			-> lazy2<Source, std::decay_t<decltype(from)>>
		/* requires(
			callable<strict_spread_injector<ref_result_t<decltype(from)&>>, constructor_function<Source>>
		) */ {
			return lazy2<Source, std::decay_t<decltype(from)>>{KANGARU5_FWD(from)};
		}
	};
}

// Here's many classes, all have some relations with others

struct service_1_a { int i; };
struct service_1_b { service_1_a s1a; };

struct agg1 {
	service_1_a a;
	service_1_b& b;
};

struct service_2_a { service_1_a s1a; agg1 agg; };
struct service_2_b { service_1_b& s1b; service_2_a& s2a; };
struct service_2_c {
	explicit constexpr service_2_c(service_2_b& s2b) : s2b{s2b} {
		fmt::println("init service_2_c");
	}
	
	service_2_b& s2b;
};

struct agg2 {
	agg1 agg;
	service_1_a s1a;
	service_1_b& s1b;
	service_2_a& s2a;
};

// TODO: Figure out how to properly inject aggregate bases.
struct service_3_a_base { service_2_b& s2b; service_1_a s1a; };
struct service_3_a : service_3_a_base {
	explicit constexpr service_3_a(service_2_b& s2b, service_1_a s1a) : service_3_a_base{.s2b = s2b, .s1a = s1a} {}
};

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

// Let's see a simple case. We return a modular source with one member.
constexpr auto module0() {
	// Modular sources are immovable, but we can still return them because of RVO.
	return kangaru::modular_source{
		// We start this modular source from nothing.
		// TODO: Figure out if we can omit this parameter.
		kangaru::none_source{},
		[]{
			return kangaru::object_source{int{9}};
		}
	};
}

// A more complex example. Our module have one dependency.
// We also return the modular source in a type erased wrapper.
// If you hover the module0 function and inspect the return type, you'll understand why.
// Types containing lambdas can explode pretty quick.
constexpr auto module1(kangaru::module_dependencies<decltype(module0)> dependencies) -> kangaru::any_source_of<service_1_a, service_1_b&> {
	return kangaru::make_modular_source_in_place(dependencies,
		// Can be lambdas just like we see in module0.
		[](int i) { // int dependency, from module0. We can write them explicitly like this, but generally, you won't.
			return kangaru::object_source<service_1_a>{i};
		},
		// Let's use generic constructor functions, and let kangaru deduce the parameters!
		kangaru::constructor_function<kangaru::reference_source<service_1_b>>{}
	);
}

// Here we decided to return auto, this is more optimizable, but we can't separate cpp/hpp like this.
auto module2(kangaru::module_dependencies<decltype(module1)> dependencies) {
	// We don't need to use in_place_construction since we don't construct a any_source_of.
	// However, the make_modular_source function is still useful.
	// We can simply make a list of all the sources the module should be composed of, and let kangaru autowire everything.
	return kangaru::make_modular_source<
		kangaru::reference_source<service_2_a>,
		kangaru::reference_source<service_2_b>,
		kangaru::lazy_init<kangaru::shared_pointer_source<service_2_c>>
	>(dependencies);
}

// Let's declare the function then define it, just to show we can separate declaration and definition :D
constexpr auto module3(kangaru::module_dependencies<decltype(module2), decltype(module1)> dependencies)
	-> kangaru::any_source_of<service_3_a_base&, std::shared_ptr<service_3_b_base>, service_3_c&>;

constexpr auto module3(kangaru::module_dependencies<decltype(module2), decltype(module1)> dependencies)
	-> kangaru::any_source_of<service_3_a_base&, std::shared_ptr<service_3_b_base>, service_3_c&> {
	// Here we use the generic make_in_place, that transforms move construction to RVO.
	// Normally, for modular source, you should either just use make_modular_source[_in_place],
	// using the in place version if returning a any_source_of
	return kangaru::make_in_place<kangaru::modular_source>(dependencies,
		kangaru::constructor_function<kangaru::derived_reference_source<service_3_a_base, service_3_a>>{},
		kangaru::constructor_function<kangaru::derived_shared_pointer_source<service_3_b_base, service_3_b>>{},
		// We can hijack the injection process and just get the incremental source instead to create something
		// It is allowed to copy the incremental source, it only contains reference of things that will stay alive
		// at least as long as the object we return from this function.
		[](kangaru::source auto module) {
			auto injector = kangaru::make_strict_spread_injector(module);
			auto constructor = kangaru::constructor_function<kangaru::reference_source<service_3_c>>{};
			return injector(constructor); // Returns a kangaru::reference_source<service_3_c>
		}
	);
}

auto main() -> int {
	auto source = kangaru::modular_container{module0, module1, module2, module3};
	auto injector = kangaru::make_spread_injector(kangaru::ref(source));

	fmt::println("before initializing service_2_c");
	kangaru::provide<std::shared_ptr<service_2_c>>(source);
	fmt::println("after initializing service_2_c");

	injector([](service_2_b& s2b, service_1_a s1a, agg2 agg, service_3_c&) {
		fmt::println("potato {} {} {}", s2b.s1b.s1a.i, s1a.i, agg.s2a.agg.a.i);
	});
}
