#include <kangaru/kangaru.hpp>

#include <concepts>
#include <optional>
#include <fmt/core.h>

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
