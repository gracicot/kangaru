#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct sleepy {};
struct grumpy {
	int token;
};

struct sleepy_source {
	friend auto provide(kangaru::provide_tag<sleepy>, sleepy_source const&) -> sleepy {
		return sleepy{};
	}
};

struct grumpy_source {
	friend auto provide(kangaru::provide_tag<grumpy>, grumpy_source& source) -> grumpy {
		return grumpy{.token = source.token++};
	}
	
	int token = 0;
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
}

TEST_CASE("Deducer uses sources to deduce", "[deducer]") {
	auto source = grumpy_source{};
	
	SECTION("Can deduce values prvalues") {
		auto deducer = kangaru::basic_deducer<grumpy_source&>{source};
		
		CHECK([](grumpy g) { return g; }(deducer).token == 0);
		CHECK([](grumpy g) { return g; }(deducer).token == 1);
	}
	
	SECTION("Chooses between const& and &&") {
		auto source = kangaru::concat(
			kangaru::reference_source<grumpy const>{grumpy{.token = 1}},
			kangaru::rvalue_source{grumpy{.token = 2}}
		);
		
		auto deducer = kangaru::basic_deducer<decltype(source)&>{source};
		
		CHECK([](grumpy const& g) { return g; }(deducer).token == 1);
		CHECK([](grumpy&& g) { return g; }(deducer).token == 2);
		
		SECTION("Prioritize rvalue when there's a choice") {
			CHECK([](grumpy g) { return g; }(deducer).token == 2);
		}
		
		SECTION("Prioritize prvalue when possible") {
			auto source2 = kangaru::concat(kangaru::object_source{grumpy{.token = 3}}, source);
			auto deducer = kangaru::basic_deducer<decltype(source2)&>{source2};
			
			CHECK([](grumpy g) { return g; }(deducer).token == 3);
		}
	}
	
	SECTION("Correctly deduce const& when has a source of any other kind of references") {
		auto source = kangaru::concat(
			kangaru::reference_source<grumpy>{grumpy{.token = 1}},
			kangaru::rvalue_source{grumpy{.token = 2}}
		);
		
		auto deducer = kangaru::basic_deducer<decltype(source)&>{source};
		
		CHECK([](grumpy const& g) { return g; }(deducer).token == 1);
	}
}
