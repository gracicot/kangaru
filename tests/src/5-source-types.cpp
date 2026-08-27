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

static_assert(kangaru::deducible_prvalue<sleepy, sleepy_source>);
static_assert(not kangaru::deducible_lvalue<sleepy, sleepy_source>);
static_assert(not kangaru::deducible_rvalue<sleepy, sleepy_source>);
static_assert(not kangaru::deducible_lvalue_const<sleepy, sleepy_source>);
static_assert(not kangaru::deducible_rvalue_const<sleepy, sleepy_source>);

static_assert(kangaru::deducible_prvalue<sleepy, kangaru::reference_source<sleepy>>);
static_assert(kangaru::deducible_lvalue<sleepy, kangaru::reference_source<sleepy>>);
static_assert(not kangaru::deducible_rvalue<sleepy, kangaru::reference_source<sleepy>>);
static_assert(kangaru::deducible_lvalue_const<sleepy, kangaru::reference_source<sleepy>>);
static_assert(not kangaru::deducible_rvalue_const<sleepy, kangaru::reference_source<sleepy>>);

static_assert(kangaru::deducible_prvalue<sleepy, kangaru::reference_source<sleepy const>>);
static_assert(not kangaru::deducible_lvalue<sleepy, kangaru::reference_source<sleepy const>>);
static_assert(not kangaru::deducible_rvalue<sleepy, kangaru::reference_source<sleepy const>>);
static_assert(kangaru::deducible_lvalue_const<sleepy, kangaru::reference_source<sleepy const>>);
static_assert(not kangaru::deducible_rvalue_const<sleepy, kangaru::reference_source<sleepy const>>);

static_assert(kangaru::deducible_prvalue<sleepy, kangaru::rvalue_source<sleepy>>);
static_assert(not kangaru::deducible_lvalue<sleepy, kangaru::rvalue_source<sleepy>>);
static_assert(kangaru::deducible_rvalue<sleepy, kangaru::rvalue_source<sleepy>>);
static_assert(kangaru::deducible_lvalue_const<sleepy, kangaru::rvalue_source<sleepy>>);
static_assert(kangaru::deducible_rvalue_const<sleepy, kangaru::rvalue_source<sleepy>>);

static_assert(kangaru::deducible_prvalue<sleepy, kangaru::rvalue_source<sleepy const>>);
static_assert(not kangaru::deducible_lvalue<sleepy, kangaru::rvalue_source<sleepy const>>);
static_assert(not kangaru::deducible_rvalue<sleepy, kangaru::rvalue_source<sleepy const>>);
static_assert(kangaru::deducible_lvalue_const<sleepy, kangaru::rvalue_source<sleepy const>>);
static_assert(kangaru::deducible_rvalue_const<sleepy, kangaru::rvalue_source<sleepy const>>);

// Strict does not allow deduction of other value categories
static_assert(not kangaru::deducible_strict_prvalue<sleepy, kangaru::reference_source<sleepy>>);
static_assert(kangaru::deducible_strict_lvalue<sleepy, kangaru::reference_source<sleepy>>);
static_assert(not kangaru::deducible_strict_rvalue<sleepy, kangaru::reference_source<sleepy>>);
static_assert(not kangaru::deducible_strict_lvalue_const<sleepy, kangaru::reference_source<sleepy>>);
static_assert(not kangaru::deducible_strict_rvalue_const<sleepy, kangaru::reference_source<sleepy>>);

static_assert(not kangaru::deducible_strict_prvalue<sleepy, kangaru::reference_source<sleepy const>>);
static_assert(not kangaru::deducible_strict_lvalue<sleepy, kangaru::reference_source<sleepy const>>);
static_assert(not kangaru::deducible_strict_rvalue<sleepy, kangaru::reference_source<sleepy const>>);
static_assert(kangaru::deducible_strict_lvalue_const<sleepy, kangaru::reference_source<sleepy const>>);
static_assert(not kangaru::deducible_strict_rvalue_const<sleepy, kangaru::reference_source<sleepy const>>);

static_assert(not kangaru::deducible_strict_prvalue<sleepy, kangaru::rvalue_source<sleepy>>);
static_assert(not kangaru::deducible_strict_lvalue<sleepy, kangaru::rvalue_source<sleepy>>);
static_assert(kangaru::deducible_strict_rvalue<sleepy, kangaru::rvalue_source<sleepy>>);
static_assert(not kangaru::deducible_strict_lvalue_const<sleepy, kangaru::rvalue_source<sleepy>>);
static_assert(not kangaru::deducible_strict_rvalue_const<sleepy, kangaru::rvalue_source<sleepy>>);

static_assert(not kangaru::deducible_strict_prvalue<sleepy, kangaru::rvalue_source<sleepy const>>);
static_assert(not kangaru::deducible_strict_lvalue<sleepy, kangaru::rvalue_source<sleepy const>>);
static_assert(not kangaru::deducible_strict_rvalue<sleepy, kangaru::rvalue_source<sleepy const>>);
static_assert(not kangaru::deducible_strict_lvalue_const<sleepy, kangaru::rvalue_source<sleepy const>>);
static_assert(kangaru::deducible_strict_rvalue_const<sleepy, kangaru::rvalue_source<sleepy const>>);

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
		
		SECTION("Can concat composed source") {
			auto source3 = kangaru::object_source<int>{2};
			auto concatenated = kangaru::composed_source_cat(source, kangaru::tie(source3));
			static_assert(std::same_as<
				kangaru::composed_source<
					kangaru::source_reference_wrapper<sleepy_source>,
					kangaru::source_reference_wrapper<kangaru::reference_source<grumpy>>,
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
