#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

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

constexpr auto module0() {
	return kangaru::modular_source{
		[]{
			return kangaru::object_source{int{9}};
		}
	};
}

auto module1(kangaru::module_dependencies<decltype(module0)> dependencies) -> kangaru::any_source_of<service_1_a&, service_1_b, service_1_c&, std::shared_ptr<service_1_d>> {
	return kangaru::make_modular_source_in_place(dependencies,
		kangaru::constructor_function<kangaru::reference_source<service_1_a>>{},
		kangaru::constructor_function<kangaru::object_source<service_1_b>>{},
		kangaru::constructor_function<kangaru::reference_source<service_1_c>>{},
		kangaru::modular_source_initializer_using_lazy<kangaru::shared_pointer_source<service_1_d>>{}
	);
}

auto module2(kangaru::module_dependencies<decltype(module1)> dependencies) {
	return kangaru::make_modular_source<
		kangaru::reference_source<service_2_a>,
		kangaru::reference_source<service_2_b>
	>(dependencies);
}

auto module3(kangaru::module_dependencies<decltype(module2), decltype(module1)> dependencies)
	-> kangaru::any_source_of<service_3_a_base&, std::shared_ptr<service_3_b_base>, service_3_c&>;

auto module3(kangaru::module_dependencies<decltype(module2), decltype(module1)> dependencies)
	-> kangaru::any_source_of<service_3_a_base&, std::shared_ptr<service_3_b_base>, service_3_c&> {
	return kangaru::construct_in_place<kangaru::modular_source>(dependencies,
		kangaru::constructor_function<kangaru::derived_reference_source<service_3_a_base, service_3_a>>{},
		kangaru::constructor_function<kangaru::derived_shared_pointer_source<service_3_b_base, service_3_b>>{},
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
