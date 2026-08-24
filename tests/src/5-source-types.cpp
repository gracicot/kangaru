#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct sleepy {};
struct grumpy {
	int token;
};

struct sleepy_source {
	friend constexpr auto provide(sleepy_source const&) -> sleepy {
		return sleepy{};
	}
};

struct abstract {
	int a;
	explicit constexpr abstract(int a) : a{a} {}
	virtual ~abstract() = 0;
};

abstract::~abstract() = default;

struct concrete : abstract {
	explicit constexpr concrete(int a) : abstract{a} {}
};

TEST_CASE("Sources can provide", "[source]") {
	CHECK((std::same_as<sleepy, decltype(kangaru::provide<sleepy>(sleepy_source{}))>));
	
	CHECK((kangaru::source_of<sleepy_source, sleepy>));
	
	SECTION("Object source") {
		auto grumpy_source = kangaru::object_source{grumpy{.token = 9}};
		
		CHECK(kangaru::provide<grumpy>(grumpy_source).token == 9);
	}
	
	SECTION("External reference source") {
		auto g = grumpy{.token = 0};
		auto grumpy_source = kangaru::external_reference_source{g};
		
		g.token = 8;
		
		CHECK(kangaru::provide<grumpy&>(grumpy_source).token == 8);
	}
	
	SECTION("External rvalue source") {
		auto g = grumpy{.token = 0};
		auto grumpy_source = kangaru::external_rvalue_source{std::move(g)}; // not moved from yet
		
		g.token = 8;
		
		// provide returns the rvalue that can be moved from
		CHECK(kangaru::provide<grumpy&&>(grumpy_source).token == 8);
		CHECK(kangaru::provide<grumpy&&>(std::move(grumpy_source)).token == 8);
		
		// provide returns the rvalue that can be moved from
		CHECK(std::same_as<grumpy&&, decltype(kangaru::provide<grumpy&&>(grumpy_source))>);
		CHECK(std::same_as<grumpy&&, decltype(kangaru::provide<grumpy&&>(std::move(grumpy_source)))>);
	}
	
	SECTION("Reference source") {
		auto grumpy_source = kangaru::reference_source{grumpy{.token = 9}};
		
		CHECK(kangaru::provide<grumpy&>(grumpy_source).token == 9);
		kangaru::provide<grumpy&>(grumpy_source).token = 2;
		CHECK(kangaru::provide<grumpy&>(grumpy_source).token == 2);
		CHECK(kangaru::provide<grumpy const&>(std::as_const(grumpy_source)).token == 2);
		
		static_assert(not kangaru::source_of<decltype(grumpy_source), grumpy const&>);
		static_assert(kangaru::source_of<decltype(std::as_const(grumpy_source)), grumpy const&>);
	}
	
	SECTION("Rvalue source") {
		auto grumpy_source = kangaru::rvalue_source{grumpy{.token = 9}};
		
		CHECK(kangaru::provide<grumpy&&>(grumpy_source).token == 9);
		grumpy&& g = kangaru::provide<grumpy&&>(grumpy_source);
		g.token = 2;
		CHECK(kangaru::provide<grumpy&&>(grumpy_source).token == 2);
	}
	
	SECTION("Compose source composes together") {
		auto source1 = sleepy_source{};
		auto source2 = kangaru::reference_source{grumpy{.token = 1}};
		auto source = kangaru::tie(source1, source2);
		
		CHECK(kangaru::provide<grumpy&>(source).token == 1);
		CHECK(std::same_as<sleepy, decltype(kangaru::provide<sleepy>(source))>);
		
		SECTION("Compose don't provide ambiguous") {
			auto source = kangaru::composed_source{kangaru::object_source{42}, kangaru::object_source{32}};
			static_assert(not kangaru::source_of<decltype(source), int>);
			
			// reference source can become ambiguous dependending on constness
			auto source2 = kangaru::composed_source{
				kangaru::reference_source<int>{42},
				kangaru::reference_source<int const>{32}
			};
			
			static_assert(not kangaru::source_of<decltype(source2), int>);
			static_assert(kangaru::source_of<decltype(source2), int&>);
			static_assert(kangaru::source_of<decltype(source2), int const&>);
			static_assert(not kangaru::source_of<decltype(std::as_const(source2)), int&>);
			static_assert(not kangaru::source_of<decltype(std::as_const(source2)), int const&>);
		}
	}
	
	SECTION("Tuple source") {
		auto source = kangaru::tuple_source(std::tuple{sleepy{}, grumpy{.token = 4}});
		
		CHECK(kangaru::provide<grumpy>(source).token == 4);
		CHECK(std::same_as<sleepy, decltype(kangaru::provide<sleepy>(source))>);
	}
	
	SECTION("Source reference wrapper") {
		auto source = sleepy_source{};
		auto source_ref = kangaru::ref(source);
		CHECK(std::addressof(source) == std::addressof(source_ref.unwrap()));
		CHECK(std::same_as<sleepy, decltype(kangaru::provide<sleepy>(source_ref))>);
	}
	
	SECTION("Derived reference to source") {
		auto source = kangaru::derived_reference_source<abstract, concrete>{1};
		static_assert(not kangaru::source_of<decltype(source), concrete&>);
		static_assert(kangaru::source_of<decltype(source), abstract&>);
		CHECK(kangaru::provide<abstract&>(source).a == 1);
	}
}
