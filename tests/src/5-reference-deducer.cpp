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
		
		auto injector = kangaru::make_simple_injector(source);
		
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
		
		auto injector = kangaru::make_simple_injector(source);
		
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
		
		auto injector = kangaru::make_simple_injector(source);
		
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
			
			auto injector = kangaru::make_simple_injector(source);
			
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
		auto source = source_by_value;
		
		auto injector = kangaru::make_simple_injector(source);
		
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
		auto injector_rvalue = kangaru::make_simple_injector(source_by_rvalue_ref);
		auto injector_const_rvalue = kangaru::make_simple_injector(source_by_rvalue_ref_const);
		auto injector_lvalue = kangaru::make_simple_injector(source_by_lvalue_ref);
		
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

struct agg_0000 {
	injected i;
};

struct agg_0001 {
	injected& i;
};

struct agg_0010 {
	injected const& i;
};

struct agg_0100 {
	injected&& i;
};

struct agg_1000 {
	injected const&& i;
};

struct level2_type_0000 { type_0000 member; };
struct level2_type_0001 { type_0001 member; };
struct level2_type_0010 { type_0010 member; };
struct level2_type_0100 { type_0100 member; };
struct level2_type_1000 { type_1000 member; };

struct level2_agg_0000 { agg_0000 member; };
struct level2_agg_0001 { agg_0001 member; };
struct level2_agg_0010 { agg_0010 member; };
struct level2_agg_0100 { agg_0100 member; };
struct level2_agg_1000 { agg_1000 member; };

TEST_CASE("Strict deducer strictly deduce", "[deducer]") {
	SECTION("Strict reference deducer") {
		SECTION("Class type with constructors") {
			using constructor_0000_t = kangaru::constructor_function<type_0000>;
			using constructor_0001_t = kangaru::constructor_function<type_0001>;
			using constructor_0010_t = kangaru::constructor_function<type_0010>;
			using constructor_0011_t = kangaru::constructor_function<type_0011>;
			using constructor_0100_t = kangaru::constructor_function<type_0100>;
			using constructor_0101_t = kangaru::constructor_function<type_0101>;
			using constructor_0110_t = kangaru::constructor_function<type_0110>;
			using constructor_0111_t = kangaru::constructor_function<type_0111>;
			using constructor_1000_t = kangaru::constructor_function<type_1000>;
			using constructor_1001_t = kangaru::constructor_function<type_1001>;
			using constructor_1010_t = kangaru::constructor_function<type_1010>;
			using constructor_1011_t = kangaru::constructor_function<type_1011>;
			using constructor_1100_t = kangaru::constructor_function<type_1100>;
			using constructor_1101_t = kangaru::constructor_function<type_1101>;
			using constructor_1110_t = kangaru::constructor_function<type_1110>;
			using constructor_1111_t = kangaru::constructor_function<type_1111>;
			
			auto constructor_0000 = constructor_0000_t{};
			auto constructor_0001 = constructor_0001_t{};
			auto constructor_0010 = constructor_0010_t{};
			auto constructor_0011 = constructor_0011_t{};
			auto constructor_0100 = constructor_0100_t{};
			auto constructor_0101 = constructor_0101_t{};
			auto constructor_0110 = constructor_0110_t{};
			auto constructor_0111 = constructor_0111_t{};
			auto constructor_1000 = constructor_1000_t{};
			auto constructor_1001 = constructor_1001_t{};
			auto constructor_1010 = constructor_1010_t{};
			auto constructor_1011 = constructor_1011_t{};
			auto constructor_1100 = constructor_1100_t{};
			auto constructor_1101 = constructor_1101_t{};
			auto constructor_1110 = constructor_1110_t{};
			auto constructor_1111 = constructor_1111_t{};
			
			CHECK((kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0000_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0001_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0010_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0011_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0100_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0101_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0110_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0111_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1000_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1001_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1010_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1011_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1100_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1101_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1110_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1111_t, 0, 1>));
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0000_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0001_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0010_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0011_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference_and_lvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0100_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0101_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference_and_rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0110_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_const_reference_and_rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0111_t, 0, 1>()
			) == kangaru::reference_kind::all_except_rvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1000_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1001_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference_and_rvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1010_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_const_reference_and_rvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1011_t, 0, 1>()
			) == kangaru::reference_kind::all_except_rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1100_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_reference_and_rvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1101_t, 0, 1>()
			) == kangaru::reference_kind::all_except_lvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1110_t, 0, 1>()
			) == kangaru::reference_kind::all_except_lvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1111_t, 0, 1>()
			) == kangaru::reference_kind::all_reference_kind);
			
			CHECK(kangaru::construction_tree_needs<type_0000, injected>);
			CHECK(not kangaru::construction_tree_needs<type_0000, injected&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, injected const&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, injected&&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0000, int>);
			CHECK(not kangaru::construction_tree_needs<type_0000, int&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0001, injected>);
			CHECK(kangaru::construction_tree_needs<type_0001, injected&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, injected const&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, injected&&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0001, int>);
			CHECK(not kangaru::construction_tree_needs<type_0001, int&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0010, injected>);
			CHECK(not kangaru::construction_tree_needs<type_0010, injected&>);
			CHECK(kangaru::construction_tree_needs<type_0010, injected const&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, injected&&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0010, int>);
			CHECK(not kangaru::construction_tree_needs<type_0010, int&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, int const&&>);
			
			//static_assert(kangaru::construction_tree_needs<type_0011, injected&>);
			//static_assert(kangaru::construction_tree_needs<type_0011, injected const&>);
			////static_assert(not kangaru::construction_tree_needs<type_0011, injected&&>);
			////static_assert(not kangaru::construction_tree_needs<type_0011, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0100, injected>);
			CHECK(not kangaru::construction_tree_needs<type_0100, injected&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, injected const&>);
			CHECK(kangaru::construction_tree_needs<type_0100, injected&&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0100, int>);
			CHECK(not kangaru::construction_tree_needs<type_0100, int&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, int const&&>);
			
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
			
			CHECK(not kangaru::construction_tree_needs<type_1000, injected>);
			CHECK(not kangaru::construction_tree_needs<type_1000, injected&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, injected const&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, injected&&>);
			CHECK(kangaru::construction_tree_needs<type_1000, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_1000, int>);
			CHECK(not kangaru::construction_tree_needs<type_1000, int&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, int const&&>);
			
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
		
		SECTION("Class type aggregate") {
			using constructor_agg_0000_t = kangaru::constructor_function<agg_0000>;
			using constructor_agg_0001_t = kangaru::constructor_function<agg_0001>;
			using constructor_agg_0010_t = kangaru::constructor_function<agg_0010>;
			using constructor_agg_0100_t = kangaru::constructor_function<agg_0100>;
			using constructor_agg_1000_t = kangaru::constructor_function<agg_1000>;
			
			auto constructor_agg_0000 = constructor_agg_0000_t{};
			auto constructor_agg_0001 = constructor_agg_0001_t{};
			auto constructor_agg_0010 = constructor_agg_0010_t{};
			auto constructor_agg_0100 = constructor_agg_0100_t{};
			auto constructor_agg_1000 = constructor_agg_1000_t{};
			
			CHECK((kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_0000_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_0001_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_0010_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_0100_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_1000_t, 0, 1>));
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_0000_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_0001_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_0100_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_0010_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_1000_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_const_reference);
		}
		
		SECTION("Wrapped class type aggregate") {
			using constructor_source_agg_0000_t = kangaru::constructor_function<kangaru::reference_source<agg_0000>>;
			using constructor_source_agg_0001_t = kangaru::constructor_function<kangaru::reference_source<agg_0001>>;
			using constructor_source_agg_0010_t = kangaru::constructor_function<kangaru::reference_source<agg_0010>>;
			using constructor_source_agg_0100_t = kangaru::constructor_function<kangaru::reference_source<agg_0100>>;
			using constructor_source_agg_1000_t = kangaru::constructor_function<kangaru::reference_source<agg_1000>>;
			
			auto constructor_source_agg_0000 = constructor_source_agg_0000_t{};
			auto constructor_source_agg_0001 = constructor_source_agg_0001_t{};
			auto constructor_source_agg_0010 = constructor_source_agg_0010_t{};
			auto constructor_source_agg_0100 = constructor_source_agg_0100_t{};
			auto constructor_source_agg_1000 = constructor_source_agg_1000_t{};
			
			CHECK((kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_0000_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_0001_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_0010_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_0100_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_1000_t, 0, 1>));
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_0000_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_0001_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_0100_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_0010_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_1000_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_const_reference);
		}
		
		SECTION("Level 2 aggregate with aggregate member") {
			using constructor_level2_agg_0000_t = kangaru::constructor_function<level2_agg_0000>;
			using constructor_level2_agg_0001_t = kangaru::constructor_function<level2_agg_0001>;
			using constructor_level2_agg_0010_t = kangaru::constructor_function<level2_agg_0010>;
			using constructor_level2_agg_0100_t = kangaru::constructor_function<level2_agg_0100>;
			using constructor_level2_agg_1000_t = kangaru::constructor_function<level2_agg_1000>;
			
			auto constructor_level2_agg_0000 = constructor_level2_agg_0000_t{};
			auto constructor_level2_agg_0001 = constructor_level2_agg_0001_t{};
			auto constructor_level2_agg_0010 = constructor_level2_agg_0010_t{};
			auto constructor_level2_agg_0100 = constructor_level2_agg_0100_t{};
			auto constructor_level2_agg_1000 = constructor_level2_agg_1000_t{};
			
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_0000_t, 0, 1>);
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_0001_t, 0, 1>);
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_0010_t, 0, 1>);
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_0100_t, 0, 1>);
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_1000_t, 0, 1>);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_0000_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_0001_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_0100_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_0010_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_1000_t, 0, 1>()
			) == kangaru::reference_kind::none);
		}
	}
}
