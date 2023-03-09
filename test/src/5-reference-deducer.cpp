#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

TEST_CASE("Deducer can deduce reference types", "[deducer]") {
	enum how_t {
		by_value,
		by_lvalue_reference,
		by_rvalue_reference,
		by_lvalue_const_reference,
		by_rvalue_const_reference
	};

	struct Sneezy {
		how_t how;
	};
	
	auto by_plain_value = Sneezy{.how = by_value};
	auto by_reference = Sneezy{.how = by_lvalue_reference};
	auto const by_reference_const = Sneezy{.how = by_lvalue_const_reference};
	auto by_rvalue = Sneezy{.how = by_rvalue_reference};
	auto const by_rvalue_const = Sneezy{.how = by_rvalue_const_reference};
	
	auto source_by_value = kangaru::object_source{by_plain_value};
	auto source_by_lvalue_ref = kangaru::reference_source{by_reference};
	auto source_by_lvalue_ref_const = kangaru::reference_source{by_reference_const};
	auto source_by_rvalue_ref = kangaru::rvalue_source{std::move(by_rvalue)};
	auto source_by_rvalue_ref_const = kangaru::rvalue_source{std::move(by_rvalue_const)};
	
	SECTION("Will deduce the right type of reference") {
		auto source = kangaru::tie(
			source_by_value,
			source_by_lvalue_ref,
			source_by_lvalue_ref_const,
			source_by_rvalue_ref_const,
			source_by_rvalue_ref
		);
		
		auto ded = kangaru::exclude_prvalue_deducer<kangaru::deducer<decltype(source)&>>{kangaru::deducer<decltype(source)&>{source}};
		[](Sneezy const&&){}(ded);
		[](Sneezy&&){}(ded);
		[](Sneezy&){}(ded);
		[](Sneezy const&){}(ded);
		
		auto injector = kangaru::simple_injector{source};
		
		//injector([](Sneezy s) {
		//	CHECK(s.how == by_value);
		//});
		
		injector([](Sneezy& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](Sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](Sneezy const& s) {
			CHECK(s.how == by_lvalue_const_reference);
		});
		
		injector([](Sneezy const&& s) {
			CHECK(s.how == by_rvalue_const_reference);
		});
	}
	
	SECTION("Will fallback to mutable ref if no const is available") {
		auto source = kangaru::tie(
			source_by_lvalue_ref,
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::simple_injector{source};
		
		injector([](Sneezy& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](Sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](Sneezy const& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](Sneezy const&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
	}

	SECTION("Will fallback to rvalue ref if no const lvalue is available") {
		auto source = kangaru::tie(
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::simple_injector{source};
		
		injector([](Sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](Sneezy const& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](Sneezy const&& s) {
			CHECK(s.how == by_rvalue_reference);
		});

		SECTION("But prefers const") {
			auto source = kangaru::tie(
				source_by_rvalue_ref,
				source_by_rvalue_ref_const
			);
			
			auto injector = kangaru::simple_injector{source};
			
			injector([](Sneezy&& s) {
				CHECK(s.how == by_rvalue_reference);
			});
			
			injector([](Sneezy const& s) {
				CHECK(s.how == by_rvalue_const_reference);
			});
			
			injector([](Sneezy const&& s) {
				CHECK(s.how == by_rvalue_const_reference);
			});
		}
	}

	SECTION("Can copy simple values") {
		auto source = kangaru::tie(
			source_by_value
		);
		
		auto injector = kangaru::simple_injector{source};
		
		injector([](Sneezy s) {
			CHECK(s.how == by_value);
		});
		
		injector([](Sneezy&& s) {
			CHECK(s.how == by_value);
		});
		
		injector([](Sneezy const& s) {
			CHECK(s.how == by_value);
		});
		
		injector([](Sneezy const&& s) {
			CHECK(s.how == by_value);
		});
	}
}
