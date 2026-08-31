#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct injected {};
struct value_int {
	int token;
};

struct injected_value_source {
	friend constexpr auto provide(injected_value_source const&) -> injected {
		return injected{};
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

struct not_injectable {};

template<>
struct kangaru::allow_injection_using<not_injectable> : std::false_type {};

template<>
struct kangaru::allow_injection_using<not_injectable&> : std::false_type {};

struct not_injectable_source {
	constexpr auto provide() const {
		return not_injectable{};
	}
};

struct not_injectable_ref_source {
	constexpr auto provide() -> not_injectable& {
		return ni;
	}
	
	not_injectable ni;
};

struct not_injectable_const_ref_source {
	constexpr auto provide() const -> not_injectable const& {
		return ni;
	}
	
	not_injectable ni;
};

static_assert(kangaru::deducible_prvalue<injected, injected_value_source>);
static_assert(not kangaru::deducible_lvalue<injected, injected_value_source>);
static_assert(not kangaru::deducible_rvalue<injected, injected_value_source>);
static_assert(not kangaru::deducible_lvalue_const<injected, injected_value_source>);
static_assert(not kangaru::deducible_rvalue_const<injected, injected_value_source>);

static_assert(kangaru::deducible_prvalue<injected, kangaru::reference_source<injected>>);
static_assert(kangaru::deducible_lvalue<injected, kangaru::reference_source<injected>>);
static_assert(not kangaru::deducible_rvalue<injected, kangaru::reference_source<injected>>);
static_assert(kangaru::deducible_lvalue_const<injected, kangaru::reference_source<injected>>);
static_assert(not kangaru::deducible_rvalue_const<injected, kangaru::reference_source<injected>>);

static_assert(kangaru::deducible_prvalue<injected, kangaru::reference_source<injected const>>);
static_assert(not kangaru::deducible_lvalue<injected, kangaru::reference_source<injected const>>);
static_assert(not kangaru::deducible_rvalue<injected, kangaru::reference_source<injected const>>);
static_assert(kangaru::deducible_lvalue_const<injected, kangaru::reference_source<injected const>>);
static_assert(not kangaru::deducible_rvalue_const<injected, kangaru::reference_source<injected const>>);

static_assert(kangaru::deducible_prvalue<injected, kangaru::rvalue_source<injected>>);
static_assert(not kangaru::deducible_lvalue<injected, kangaru::rvalue_source<injected>>);
static_assert(kangaru::deducible_rvalue<injected, kangaru::rvalue_source<injected>>);
static_assert(kangaru::deducible_lvalue_const<injected, kangaru::rvalue_source<injected>>);
static_assert(kangaru::deducible_rvalue_const<injected, kangaru::rvalue_source<injected>>);

static_assert(kangaru::deducible_prvalue<injected, kangaru::rvalue_source<injected const>>);
static_assert(not kangaru::deducible_lvalue<injected, kangaru::rvalue_source<injected const>>);
static_assert(not kangaru::deducible_rvalue<injected, kangaru::rvalue_source<injected const>>);
static_assert(kangaru::deducible_lvalue_const<injected, kangaru::rvalue_source<injected const>>);
static_assert(kangaru::deducible_rvalue_const<injected, kangaru::rvalue_source<injected const>>);

// Strict does not allow deduction of other value categories
static_assert(not kangaru::deducible_strict_prvalue<injected, kangaru::reference_source<injected>>);
static_assert(kangaru::deducible_strict_lvalue<injected, kangaru::reference_source<injected>>);
static_assert(not kangaru::deducible_strict_rvalue<injected, kangaru::reference_source<injected>>);
static_assert(not kangaru::deducible_strict_lvalue_const<injected, kangaru::reference_source<injected>>);
static_assert(not kangaru::deducible_strict_rvalue_const<injected, kangaru::reference_source<injected>>);

static_assert(not kangaru::deducible_strict_prvalue<injected, kangaru::reference_source<injected const>>);
static_assert(not kangaru::deducible_strict_lvalue<injected, kangaru::reference_source<injected const>>);
static_assert(not kangaru::deducible_strict_rvalue<injected, kangaru::reference_source<injected const>>);
static_assert(kangaru::deducible_strict_lvalue_const<injected, kangaru::reference_source<injected const>>);
static_assert(not kangaru::deducible_strict_rvalue_const<injected, kangaru::reference_source<injected const>>);

static_assert(not kangaru::deducible_strict_prvalue<injected, kangaru::rvalue_source<injected>>);
static_assert(not kangaru::deducible_strict_lvalue<injected, kangaru::rvalue_source<injected>>);
static_assert(kangaru::deducible_strict_rvalue<injected, kangaru::rvalue_source<injected>>);
static_assert(not kangaru::deducible_strict_lvalue_const<injected, kangaru::rvalue_source<injected>>);
static_assert(not kangaru::deducible_strict_rvalue_const<injected, kangaru::rvalue_source<injected>>);

static_assert(not kangaru::deducible_strict_prvalue<injected, kangaru::rvalue_source<injected const>>);
static_assert(not kangaru::deducible_strict_lvalue<injected, kangaru::rvalue_source<injected const>>);
static_assert(not kangaru::deducible_strict_rvalue<injected, kangaru::rvalue_source<injected const>>);
static_assert(not kangaru::deducible_strict_lvalue_const<injected, kangaru::rvalue_source<injected const>>);
static_assert(kangaru::deducible_strict_rvalue_const<injected, kangaru::rvalue_source<injected const>>);

// Not injectable tests
static_assert(not kangaru::deducible_prvalue<not_injectable, not_injectable_source>);
static_assert(not kangaru::deducible_lvalue<not_injectable, not_injectable_source>);
static_assert(not kangaru::deducible_rvalue<not_injectable, not_injectable_source>);
static_assert(not kangaru::deducible_lvalue_const<not_injectable, not_injectable_source>);
static_assert(not kangaru::deducible_rvalue_const<not_injectable, not_injectable_source>);

static_assert(not kangaru::deducible_prvalue<not_injectable, not_injectable_ref_source>);
static_assert(not kangaru::deducible_lvalue<not_injectable, not_injectable_ref_source>);
static_assert(not kangaru::deducible_rvalue<not_injectable, not_injectable_ref_source>);
static_assert(not kangaru::deducible_lvalue_const<not_injectable, not_injectable_ref_source>);
static_assert(not kangaru::deducible_rvalue_const<not_injectable, not_injectable_ref_source>);

static_assert(not kangaru::deducible_prvalue<not_injectable, not_injectable_const_ref_source>);
static_assert(not kangaru::deducible_lvalue<not_injectable, not_injectable_const_ref_source>);
static_assert(not kangaru::deducible_rvalue<not_injectable, not_injectable_const_ref_source>);
static_assert(kangaru::deducible_lvalue_const<not_injectable, not_injectable_const_ref_source>);
static_assert(not kangaru::deducible_rvalue_const<not_injectable, not_injectable_const_ref_source>);

static_assert(not kangaru::deducible_strict_prvalue<not_injectable, not_injectable_source>);
static_assert(not kangaru::deducible_strict_lvalue<not_injectable, not_injectable_source>);
static_assert(not kangaru::deducible_strict_rvalue<not_injectable, not_injectable_source>);
static_assert(not kangaru::deducible_strict_lvalue_const<not_injectable, not_injectable_source>);
static_assert(not kangaru::deducible_strict_rvalue_const<not_injectable, not_injectable_source>);

static_assert(not kangaru::deducible_strict_prvalue<not_injectable, not_injectable_ref_source>);
static_assert(not kangaru::deducible_strict_lvalue<not_injectable, not_injectable_ref_source>);
static_assert(not kangaru::deducible_strict_rvalue<not_injectable, not_injectable_ref_source>);
static_assert(not kangaru::deducible_strict_lvalue_const<not_injectable, not_injectable_ref_source>);
static_assert(not kangaru::deducible_strict_rvalue_const<not_injectable, not_injectable_ref_source>);

static_assert(not kangaru::deducible_strict_prvalue<not_injectable, not_injectable_const_ref_source>);
static_assert(not kangaru::deducible_strict_lvalue<not_injectable, not_injectable_const_ref_source>);
static_assert(not kangaru::deducible_strict_rvalue<not_injectable, not_injectable_const_ref_source>);
static_assert(kangaru::deducible_strict_lvalue_const<not_injectable, not_injectable_const_ref_source>);
static_assert(not kangaru::deducible_strict_rvalue_const<not_injectable, not_injectable_const_ref_source>);

TEST_CASE("Sources can provide", "[source]") {
	CHECK((std::same_as<injected, decltype(kangaru::provide<injected>(injected_value_source{}))>));
	
	CHECK((kangaru::source_of<injected_value_source, injected>));
	
	SECTION("Object source") {
		auto grumpy_source = kangaru::object_source{value_int{.token = 9}};
		
		CHECK(kangaru::provide<value_int>(grumpy_source).token == 9);
	}
	
	SECTION("External reference source") {
		auto g = value_int{.token = 0};
		auto grumpy_source = kangaru::external_reference_source{g};
		
		g.token = 8;
		
		CHECK(kangaru::provide<value_int&>(grumpy_source).token == 8);
	}
	
	SECTION("External rvalue source") {
		auto g = value_int{.token = 0};
		auto grumpy_source = kangaru::external_rvalue_source{std::move(g)}; // not moved from yet
		
		g.token = 8;
		
		// provide returns the rvalue that can be moved from
		CHECK(kangaru::provide<value_int&&>(grumpy_source).token == 8);
		CHECK(kangaru::provide<value_int&&>(std::move(grumpy_source)).token == 8);
		
		// provide returns the rvalue that can be moved from
		CHECK(std::same_as<value_int&&, decltype(kangaru::provide<value_int&&>(grumpy_source))>);
		CHECK(std::same_as<value_int&&, decltype(kangaru::provide<value_int&&>(std::move(grumpy_source)))>);
	}
	
	SECTION("Reference source") {
		auto grumpy_source = kangaru::reference_source{value_int{.token = 9}};
		
		CHECK(kangaru::provide<value_int&>(grumpy_source).token == 9);
		kangaru::provide<value_int&>(grumpy_source).token = 2;
		CHECK(kangaru::provide<value_int&>(grumpy_source).token == 2);
		CHECK(kangaru::provide<value_int const&>(std::as_const(grumpy_source)).token == 2);
		
		static_assert(not kangaru::source_of<decltype(grumpy_source), value_int const&>);
		static_assert(kangaru::source_of<decltype(std::as_const(grumpy_source)), value_int const&>);
	}
	
	SECTION("Rvalue source") {
		auto grumpy_source = kangaru::rvalue_source{value_int{.token = 9}};
		
		CHECK(kangaru::provide<value_int&&>(grumpy_source).token == 9);
		value_int&& g = kangaru::provide<value_int&&>(grumpy_source);
		g.token = 2;
		CHECK(kangaru::provide<value_int&&>(grumpy_source).token == 2);
	}
	
	SECTION("Compose source composes together") {
		auto source1 = injected_value_source{};
		auto source2 = kangaru::reference_source{value_int{.token = 1}};
		auto source = kangaru::tie(source1, source2);
		
		CHECK(kangaru::provide<value_int&>(source).token == 1);
		CHECK(std::same_as<injected, decltype(kangaru::provide<injected>(source))>);
		
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
		
		SECTION("Can concat composed source") {
			auto source3 = kangaru::object_source<int>{2};
			auto concatenated = kangaru::composed_source_cat(source, kangaru::tie(source3));
			static_assert(std::same_as<
				kangaru::composed_source<
					kangaru::source_reference_wrapper<injected_value_source>,
					kangaru::source_reference_wrapper<kangaru::reference_source<value_int>>,
					kangaru::source_reference_wrapper<kangaru::object_source<int>>
				>,
				decltype(concatenated)
			>);
			CHECK(kangaru::provide<int>(concatenated) == 2);
			source3 = kangaru::object_source{12};
			CHECK(kangaru::provide<int>(concatenated) == 12);
		}
	}
	
	SECTION("Tuple source") {
		auto source = kangaru::tuple_source(std::tuple{injected{}, value_int{.token = 4}});
		
		CHECK(kangaru::provide<value_int>(source).token == 4);
		static_assert(kangaru::source_of<decltype(source), injected>);
	}
	
	SECTION("Derived reference source") {
		auto source = kangaru::derived_reference_source<abstract, concrete>{1};
		static_assert(not kangaru::source_of<decltype(source), concrete&>);
		static_assert(kangaru::source_of<decltype(source), abstract&>);
		CHECK(kangaru::provide<abstract&>(source).a == 1);
	}
	
	SECTION("Derived shared pointer source") {
		auto source = kangaru::derived_shared_pointer_source<abstract, concrete>{1};
		static_assert(not kangaru::source_of<decltype(source), std::shared_ptr<concrete>>);
		static_assert(kangaru::source_of<decltype(source), std::shared_ptr<abstract>>);
		CHECK(kangaru::provide<std::shared_ptr<abstract>>(source)->a == 1);
	}
	
	// TODO: Should pointer source act more like shared pointer source?
	SECTION("Pointer source") {
		auto grumpy_source = kangaru::pointer_source{value_int{.token = 9}};
		
		CHECK(kangaru::provide<value_int*>(grumpy_source)->token == 9);
		kangaru::provide<value_int*>(grumpy_source)->token = 2;
		CHECK(kangaru::provide<value_int*>(grumpy_source)->token == 2);
		CHECK(kangaru::provide<value_int*>(std::as_const(grumpy_source))->token == 2);
		
		static_assert(not kangaru::source_of<decltype(grumpy_source), value_int const*>);
		static_assert(not kangaru::source_of<decltype(std::as_const(grumpy_source)), value_int const*>);
		static_assert(kangaru::source_of<decltype(std::as_const(grumpy_source)), value_int*>);
	}
	
	SECTION("Shared pointer source") {
		auto grumpy_source = kangaru::shared_pointer_source{value_int{.token = 9}};
		
		CHECK(kangaru::provide<std::shared_ptr<value_int>>(grumpy_source)->token == 9);
		kangaru::provide<std::shared_ptr<value_int>>(grumpy_source)->token = 2;
		CHECK(kangaru::provide<std::shared_ptr<value_int>>(grumpy_source)->token == 2);
		CHECK(kangaru::provide<std::shared_ptr<value_int>>(std::as_const(grumpy_source))->token == 2);
		
		static_assert(not kangaru::source_of<decltype(grumpy_source), std::shared_ptr<value_int const>>);
		static_assert(kangaru::source_of<decltype(std::as_const(grumpy_source)), std::shared_ptr<value_int>>);
	}
	
	SECTION("Function source") {
		auto source = kangaru::function_source{[]{ return 42; }};
		CHECK(kangaru::provide<int>(source) == 42);
	}
}
