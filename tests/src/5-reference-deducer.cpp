#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <iostream>

TEST_CASE("Deducer can deduce reference types", "[deducer]") {
	enum how_t {
		by_value,
		by_lvalue_reference,
		by_rvalue_reference,
		by_lvalue_const_reference,
		by_rvalue_const_reference
	};

	struct sneezy {
		how_t how;
	};
	
	auto by_plain_value = sneezy{.how = by_value};
	auto by_reference = sneezy{.how = by_lvalue_reference};
	auto const by_reference_const = sneezy{.how = by_lvalue_const_reference};
	auto by_rvalue = sneezy{.how = by_rvalue_reference};
	auto const by_rvalue_const = sneezy{.how = by_rvalue_const_reference};
	
	auto source_by_value = kangaru::object_source{by_plain_value};
	auto source_by_lvalue_ref = kangaru::external_reference_source{by_reference};
	auto source_by_lvalue_ref_const = kangaru::external_reference_source{by_reference_const};
	auto source_by_rvalue_ref = kangaru::external_rvalue_source{std::move(by_rvalue)};
	auto source_by_rvalue_ref_const = kangaru::external_rvalue_source{std::move(by_rvalue_const)};
	
	SECTION("Will deduce the right type of reference") {
		auto source = kangaru::tie(
			source_by_value,
			source_by_lvalue_ref,
			source_by_lvalue_ref_const,
			source_by_rvalue_ref_const,
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::simple_injector{source};
		
		injector([](sneezy s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy const& s) {
			CHECK(s.how == by_lvalue_const_reference);
		});
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_rvalue_const_reference);
		});
	}
	
	SECTION("Will fallback to mutable ref if no const is available") {
		auto source = kangaru::tie(
			source_by_lvalue_ref,
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::simple_injector{source};
		
		injector([](sneezy& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy const& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
	}

	SECTION("Will fallback to rvalue ref if no const lvalue is available") {
		auto source = kangaru::tie(
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::simple_injector{source};
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy const& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_rvalue_reference);
		});

		SECTION("But prefers const") {
			auto source = kangaru::tie(
				source_by_rvalue_ref,
				source_by_rvalue_ref_const
			);
			
			auto injector = kangaru::simple_injector{source};
			
			injector([](sneezy&& s) {
				CHECK(s.how == by_rvalue_reference);
			});
			
			injector([](sneezy const& s) {
				CHECK(s.how == by_rvalue_const_reference);
			});
			
			injector([](sneezy const&& s) {
				CHECK(s.how == by_rvalue_const_reference);
			});
		}
	}

	SECTION("Can copy simple values") {
		auto source = kangaru::tie(
			source_by_value
		);
		
		auto injector = kangaru::simple_injector{source};
		
		injector([](sneezy s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy const& s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_value);
		});
	}

	SECTION("Can convert convertible references") {
		auto injector_rvalue = kangaru::simple_injector{source_by_rvalue_ref};
		auto injector_const_rvalue = kangaru::simple_injector{source_by_rvalue_ref_const};
		auto injector_lvalue = kangaru::simple_injector{source_by_lvalue_ref};
		
		injector_rvalue([](sneezy const& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector_const_rvalue([](sneezy const& s) {
			CHECK(s.how == by_rvalue_const_reference);
		});
		
		injector_lvalue([](sneezy const& s) {
			CHECK(s.how == by_lvalue_reference);
		});
	}
}

struct injected {};

struct type_0000 {
	explicit type_0000(injected);
};

struct type_0001 {
	explicit type_0001(injected&);
};

struct type_0010 {
	explicit type_0010(injected const&);
};

struct type_0011 {
	explicit type_0011(injected&);
	explicit type_0011(injected const&);
};

struct type_0100 {
	explicit type_0100(injected&&);
};

struct type_0101 {
	explicit type_0101(injected&);
	explicit type_0101(injected&&);
};

struct type_0110 {
	explicit type_0110(injected const&);
	explicit type_0110(injected&&);
};

struct type_0111 {
	explicit type_0111(injected&);
	explicit type_0111(injected const&);
	explicit type_0111(injected&&);
};

struct type_1000 {
	explicit type_1000(injected const&&);
};

struct type_1001 {
	explicit type_1001(injected&);
	explicit type_1001(injected const&&);
};

struct type_1010 {
	explicit type_1010(injected const&);
	explicit type_1010(injected const&&);
};

struct type_1011 {
	explicit type_1011(injected&);
	explicit type_1011(injected const&);
	explicit type_1011(injected const&&);
};

struct type_1100 {
	explicit type_1100(injected&&);
	explicit type_1100(injected const&&);
};

struct type_1101 {
	explicit type_1101(injected&);
	explicit type_1101(injected&&);
	explicit type_1101(injected const&&);
};

struct type_1110 {
	explicit type_1110(injected const&);
	explicit type_1110(injected&&);
	explicit type_1110(injected const&&);
};

struct type_1111 {
	explicit type_1111(injected&);
	explicit type_1111(injected const&);
	explicit type_1111(injected&&);
	explicit type_1111(injected const&&);
};

template<typename T>
auto restricted_constructor() {
	auto const construct = kangaru::non_empty_construction{};
	return [construct](kangaru::deducer auto... deduce) -> decltype(construct.template operator()<T>(deduce...)) {
		return construct.template operator()<T>(deduce...);
	};
}

TEST_CASE("Strict deducer strictly deduce", "[deducer]") {
	SECTION("Strict reference deducer") {
		auto constructor_0000 = restricted_constructor<type_0000>();
		using constructor_0000_t = decltype(constructor_0000);
		auto constructor_0001 = restricted_constructor<type_0001>();
		using constructor_0001_t = decltype(constructor_0001);
		auto constructor_0010 = restricted_constructor<type_0010>();
		using constructor_0010_t = decltype(constructor_0010);
		auto constructor_0011 = restricted_constructor<type_0011>();
		using constructor_0011_t = decltype(constructor_0011);
		auto constructor_0100 = restricted_constructor<type_0100>();
		using constructor_0100_t = decltype(constructor_0100);
		auto constructor_0101 = restricted_constructor<type_0101>();
		using constructor_0101_t = decltype(constructor_0101);
		auto constructor_0110 = restricted_constructor<type_0110>();
		using constructor_0110_t = decltype(constructor_0110);
		auto constructor_0111 = restricted_constructor<type_0111>();
		using constructor_0111_t = decltype(constructor_0111);
		auto constructor_1000 = restricted_constructor<type_1000>();
		using constructor_1000_t = decltype(constructor_1000);
		auto constructor_1001 = restricted_constructor<type_1001>();
		using constructor_1001_t = decltype(constructor_1001);
		auto constructor_1010 = restricted_constructor<type_1010>();
		using constructor_1010_t = decltype(constructor_1010);
		auto constructor_1011 = restricted_constructor<type_1011>();
		using constructor_1011_t = decltype(constructor_1011);
		auto constructor_1100 = restricted_constructor<type_1100>();
		using constructor_1100_t = decltype(constructor_1100);
		auto constructor_1101 = restricted_constructor<type_1101>();
		using constructor_1101_t = decltype(constructor_1101);
		auto constructor_1110 = restricted_constructor<type_1110>();
		using constructor_1110_t = decltype(constructor_1110);
		auto constructor_1111 = restricted_constructor<type_1111>();
		using constructor_1111_t = decltype(constructor_1111);

		CHECK((kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_0000_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_0001_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_0010_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_0011_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_0100_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_0101_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_0110_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_0111_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_1000_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_1001_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_1010_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_1011_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_1100_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_1101_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_1110_t, 0, 1>()));
		CHECK(not (kangaru::detail::deducer::is_nth_parameter_prvalue<constructor_1111_t, 0, 1>()));

		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0000_t, 0, 1>()
		) == kangaru::reference_kind::none);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0001_t, 0, 1>()
		) == kangaru::reference_kind::lvalue_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0010_t, 0, 1>()
		) == kangaru::reference_kind::lvalue_const_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0011_t, 0, 1>()
		) == kangaru::reference_kind::lvalue_reference_and_lvalue_const_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0100_t, 0, 1>()
		) == kangaru::reference_kind::rvalue_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0101_t, 0, 1>()
		) == kangaru::reference_kind::lvalue_reference_and_rvalue_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0110_t, 0, 1>()
		) == kangaru::reference_kind::lvalue_const_reference_and_rvalue_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0111_t, 0, 1>()
		) == kangaru::reference_kind::all_except_rvalue_const_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1000_t, 0, 1>()
		) == kangaru::reference_kind::rvalue_const_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1001_t, 0, 1>()
		) == kangaru::reference_kind::lvalue_reference_and_rvalue_const_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1010_t, 0, 1>()
		) == kangaru::reference_kind::lvalue_const_reference_and_rvalue_const_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1011_t, 0, 1>()
		) == kangaru::reference_kind::all_except_rvalue_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1100_t, 0, 1>()
		) == kangaru::reference_kind::rvalue_reference_and_rvalue_const_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1101_t, 0, 1>()
		) == kangaru::reference_kind::all_except_lvalue_const_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1110_t, 0, 1>()
		) == kangaru::reference_kind::all_except_lvalue_reference);
		
		CHECK((
			kangaru::detail::deducer::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1111_t, 0, 1>()
		) == kangaru::reference_kind::all_reference_kind);
		
		CHECK(kangaru::construction_tree_needs<type_0000, injected>);
		CHECK(not kangaru::construction_tree_needs<type_0000, injected&>);
		CHECK(not kangaru::construction_tree_needs<type_0000, injected const&>);
		CHECK(not kangaru::construction_tree_needs<type_0000, injected&&>);
		CHECK(not kangaru::construction_tree_needs<type_0000, injected const&&>);
		
		CHECK(kangaru::construction_tree_needs<type_0001, injected&>);
		CHECK(not kangaru::construction_tree_needs<type_0001, injected const&>);
		CHECK(not kangaru::construction_tree_needs<type_0001, injected&&>);
		CHECK(not kangaru::construction_tree_needs<type_0001, injected const&&>);
		
		CHECK(not kangaru::construction_tree_needs<type_0010, injected&>);
		CHECK(kangaru::construction_tree_needs<type_0010, injected const&>);
		CHECK(not kangaru::construction_tree_needs<type_0010, injected&&>);
		CHECK(not kangaru::construction_tree_needs<type_0010, injected const&&>);
		
		//static_assert(kangaru::construction_tree_needs<type_0011, injected&>);
		//static_assert(kangaru::construction_tree_needs<type_0011, injected const&>);
		////static_assert(not kangaru::construction_tree_needs<type_0011, injected&&>);
		////static_assert(not kangaru::construction_tree_needs<type_0011, injected const&&>);
		
		CHECK(not kangaru::construction_tree_needs<type_0100, injected&>);
		CHECK(not kangaru::construction_tree_needs<type_0100, injected const&>);
		CHECK(kangaru::construction_tree_needs<type_0100, injected&&>);
		CHECK(not kangaru::construction_tree_needs<type_0100, injected const&&>);
		
		//static_assert(kangaru::construction_tree_needs<type_0101, injected&>);
		////static_assert(not kangaru::construction_tree_needs<type_0101, injected const&>);
		//static_assert(kangaru::construction_tree_needs<type_0101, injected&&>);
		////static_assert(not kangaru::construction_tree_needs<type_0101, injected const&&>);
		
		////static_assert(not kangaru::construction_tree_needs<type_0110, injected&>);
		//static_assert(kangaru::construction_tree_needs<type_0110, injected const&>);
		//static_assert(kangaru::construction_tree_needs<type_0110, injected&&>);
		////static_assert(not kangaru::construction_tree_needs<type_0110, injected const&&>);
		
		//static_assert(kangaru::construction_tree_needs<type_0111, injected&>);
		//static_assert(kangaru::construction_tree_needs<type_0111, injected const&>);
		//static_assert(kangaru::construction_tree_needs<type_0111, injected&&>);
		////static_assert(not kangaru::construction_tree_needs<type_0111, injected const&&>);
		
		CHECK(not kangaru::construction_tree_needs<type_1000, injected&>);
		CHECK(not kangaru::construction_tree_needs<type_1000, injected const&>);
		CHECK(not kangaru::construction_tree_needs<type_1000, injected&&>);
		CHECK(kangaru::construction_tree_needs<type_1000, injected const&&>);
		
		//static_assert(kangaru::construction_tree_needs<type_1001, injected&>);
		////static_assert(not kangaru::construction_tree_needs<type_1001, injected const&>);
		////static_assert(not kangaru::construction_tree_needs<type_1001, injected&&>);
		//static_assert(kangaru::construction_tree_needs<type_1001, injected const&&>);
		
		////static_assert(not kangaru::construction_tree_needs<type_1010, injected&>);
		//static_assert(kangaru::construction_tree_needs<type_1010, injected const&>);
		////static_assert(not kangaru::construction_tree_needs<type_1010, injected&&>);
		//static_assert(kangaru::construction_tree_needs<type_1010, injected const&&>);
		
		//static_assert(kangaru::construction_tree_needs<type_1011, injected&>);
		//static_assert(kangaru::construction_tree_needs<type_1011, injected const&>);
		////static_assert(not kangaru::construction_tree_needs<type_1011, injected&&>);
		//static_assert(kangaru::construction_tree_needs<type_1011, injected const&&>);
		
		////static_assert(not kangaru::construction_tree_needs<type_1100, injected&>);
		////static_assert(not kangaru::construction_tree_needs<type_1100, injected const&>);
		//static_assert(kangaru::construction_tree_needs<type_1100, injected&&>);
		////static_assert(kangaru::construction_tree_needs<type_1100, injected const&&>);
		
		//static_assert(kangaru::construction_tree_needs<type_1101, injected&>);
		////static_assert(not kangaru::construction_tree_needs<type_1101, injected const&>);
		//static_assert(kangaru::construction_tree_needs<type_1101, injected&&>);
		//static_assert(kangaru::construction_tree_needs<type_1101, injected const&&>);
		
		////static_assert(not kangaru::construction_tree_needs<type_1110, injected&>);
		//static_assert(kangaru::construction_tree_needs<type_1110, injected const&>);
		//static_assert(kangaru::construction_tree_needs<type_1110, injected&&>);
		//static_assert(kangaru::construction_tree_needs<type_1110, injected const&&>);
		
		//static_assert(kangaru::construction_tree_needs<type_1111, injected&>);
		//static_assert(kangaru::construction_tree_needs<type_1111, injected const&>);
		//static_assert(kangaru::construction_tree_needs<type_1111, injected&&>);
		//static_assert(kangaru::construction_tree_needs<type_1111, injected const&&>);
	}
}
