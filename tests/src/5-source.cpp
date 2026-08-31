#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct class_type {};
struct not_injectable {};

template<>
struct kangaru::allow_injection_using<not_injectable> : std::false_type {};

template<>
struct kangaru::allow_injection_using<not_injectable&> : std::false_type {};

static_assert(kangaru::injectable<int>);
static_assert(kangaru::injectable<int&>);
static_assert(kangaru::injectable<int&&>);
static_assert(kangaru::injectable<int const&>);
static_assert(kangaru::injectable<int const&&>);
static_assert(not kangaru::injectable<int const>);

static_assert(kangaru::injectable<class_type*>);
static_assert(kangaru::injectable<class_type const*>);
static_assert(not kangaru::injectable<class_type const* const>);

static_assert(kangaru::injectable<class_type>);
static_assert(kangaru::injectable<class_type&>);
static_assert(kangaru::injectable<class_type&&>);
static_assert(kangaru::injectable<class_type const&>);
static_assert(kangaru::injectable<class_type const&&>);
static_assert(not kangaru::injectable<class_type const>);

struct abstract {
	inline virtual ~abstract() = 0;
};

inline abstract::~abstract() = default;

static_assert(not kangaru::injectable<void>);
static_assert(not kangaru::injectable<int[]>);
static_assert(not kangaru::injectable<int[6]>);
static_assert(not kangaru::injectable<abstract>);

static_assert(not kangaru::injectable<void()>);
static_assert(not kangaru::injectable<int()>);
static_assert(not kangaru::injectable<int() const>);

static_assert(kangaru::injectable<int(class_type::*)()>);
static_assert(kangaru::injectable<int(*)()>);
static_assert(kangaru::injectable<int(&)()>);
static_assert(kangaru::injectable<int(&)[6]>);

static_assert(kangaru::injectable<abstract&>);
static_assert(kangaru::injectable<abstract&&>);
static_assert(kangaru::injectable<abstract const&>);
static_assert(kangaru::injectable<abstract const&&>);

static_assert(not kangaru::injectable<not_injectable>);
static_assert(not kangaru::injectable<not_injectable&>);
static_assert(kangaru::injectable<not_injectable&&>);
static_assert(kangaru::injectable<not_injectable const&>);
static_assert(kangaru::injectable<not_injectable const&&>);

static_assert(not kangaru::source<int>);
static_assert(not kangaru::source<int&>);
static_assert(kangaru::source<class_type>);
static_assert(kangaru::source<class_type const>);
static_assert(not kangaru::source<class_type const&>);
static_assert(not kangaru::source<class_type&&>);
static_assert(not kangaru::source<class_type*>);
static_assert(not kangaru::source<int()>);
static_assert(not kangaru::source<int(&)()>);

struct int_source {
	constexpr auto provide() const {
		return value;
	}
	
	int value;
};

struct value_cat_source {
	constexpr auto provide() & {
		return 1;
	}
	
	constexpr auto provide() && {
		return 2;
	}
	
	constexpr auto provide() const& {
		return 3;
	}
	
	constexpr auto provide() const&& {
		return 4;
	}
};

TEST_CASE("Provide using is a callable", "[source]") {
	SECTION("Uses a source to provide") {
		auto provide = kangaru::make_provide_using<int>(int_source{42});
		CHECK(provide() == 42);
	}
	
	SECTION("Forwards value categories") {
		auto provide = kangaru::make_provide_using<int>(value_cat_source{});
		
		CHECK(provide() == 1);
		CHECK(std::move(provide)() == 2);
		CHECK(std::as_const(provide)() == 3);
		CHECK(std::move(std::as_const(provide))() == 4);
	}
}

TEST_CASE("Provide calls provide on source", "[source]") {
	auto source = value_cat_source{};
	CHECK(kangaru::provide<int>(source) == 1);
	CHECK(kangaru::provide<int>(std::move(source)) == 2);
	CHECK(kangaru::provide<int>(std::as_const(source)) == 3);
	CHECK(kangaru::provide<int>(std::move(std::as_const(source))) == 4);
}
