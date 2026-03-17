#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct injected1 {};
struct injected2 {};
struct injected3 {};

struct type1 {
	int a;
};

struct type2 {
	int b;
	
	static void call(type2& instance, kangaru::forwarded_source auto&& source) {
		instance.b = 1;
	}
	
	friend auto attribute(kangaru::second_step_init<type2>) -> kangaru::call_function<
		[](type2& instance, kangaru::forwarded_source auto&& source) {
			type2::call(instance, source);
		}
	>;
};

struct type3 {
	int b;
	
	static void call(type3& instance, kangaru::forwarded_source auto&& source) {
		instance.b = 1;
	}
	
	friend auto attribute(kangaru::second_step_init<type3&>) -> kangaru::call_function<
		[](type3& instance, kangaru::forwarded_source auto&& source) {
			type3::call(instance, source);
		}
	>;
};

struct type4 {
	int b;
	void call(injected1) { b = 1; }
	
	friend auto attribute(kangaru::second_step_init<type4&>) -> kangaru::noop_second_step;
	friend auto attribute(kangaru::second_step_init<type4>) -> kangaru::call_injected_member_functions<
		static_cast<void(type4::*)(injected1)>(&type4::call)
	>;
};

struct type5 {
	int a;
	int b;
	void call1(injected1) { a = 1; }
	void call2(injected2) { b = 1; }
	
	friend auto attribute(kangaru::second_step_init<type5&&>) -> kangaru::call_injected_member_functions<
		&type5::call1,
		&type5::call2
	>;
};

struct type6 {
	int b;
	void call(injected1) { b = 1; }
	
	friend auto attribute(kangaru::second_step_init<type6&>) -> kangaru::call_injected_member_functions<
		&type6::call
	>;
};

TEST_CASE("Second step init", "[two-step-init]") {
	SECTION("Fixed two step init") {
		auto source = kangaru::with_two_step_init{
			kangaru::object_source{type1{}},
			// TODO: Can we have call_with_injector?
			[]<kangaru::injectable T>(T& instance, kangaru::object_source<type1>&) {
				instance.a = 1;
			}
		};
		
		CHECK(kangaru::provide<type1>(source).a == 1);
	}
	SECTION("From attribute") {
		auto source = kangaru::with_two_step_init{
			kangaru::composed_source{
				kangaru::object_source{type2{}},
				kangaru::reference_source{type3{}},
				kangaru::object_source{type4{}},
				kangaru::reference_source{type4{}},
				kangaru::rvalue_source{type5{}},
				kangaru::object_source{injected1{}},
				kangaru::object_source{injected2{}},
			},
			kangaru::second_step_from_attribute{}
		};
		
		SECTION("type2") {
			CHECK(kangaru::provide<type2>(source).b == 1);
		}
		
		SECTION("type3") {
			CHECK(kangaru::provide<type3&>(source).b == 1);
		}
		
		SECTION("type4") {
			CHECK(kangaru::provide<type4&>(source).b == 0);
			CHECK(kangaru::provide<type4>(source).b == 1);
		}
		
		SECTION("type5") {
			auto const t5 = kangaru::provide<type5&&>(source);
			CHECK(t5.a == 1);
			CHECK(t5.b == 1);
		}
	}
		
	SECTION("From wrapped source") {
		auto source = kangaru::with_two_step_init{
			kangaru::compose(
				kangaru::make_source_with_cast_from<kangaru::reference_source<type6>>(kangaru::object_source{kangaru::reference_source{type6{}}}),
				kangaru::object_source{injected1{}}
			),
			kangaru::second_step_from_attribute{}
		};
		CHECK(kangaru::provide<type6&>(kangaru::provide<kangaru::reference_source<type6>>(source)).b == 1);
	}
}
