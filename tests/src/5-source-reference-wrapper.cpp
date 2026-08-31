#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct stateful_source {
	constexpr auto provide() {
		return ++state;
	}
	
	int state;
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

template<typename T>
auto test_wrapper_wrapper(kangaru::source_reference_wrapper<T>) -> void;

template<typename T>
concept do_test_wrapper_wrapper = requires(T t) {
	{ test_wrapper_wrapper<T>(kangaru::source_reference_wrapper<T>{t}) } -> std::same_as<void>;
};

TEST_CASE("Source reference wrapper", "[source]") {
	auto source = stateful_source{.state = 10};
	
	SECTION("Source reference wrapper") {
		auto source_ref = kangaru::ref(source);
		CHECK(std::addressof(source) == std::addressof(source_ref.unwrap()));
		CHECK(std::same_as<int, decltype(kangaru::provide<int>(source_ref))>);
		CHECK(kangaru::provide<int>(source) == 11);
		CHECK(source.state == 11);
		CHECK(kangaru::provide<int>(source_ref) == 12);
		CHECK(source.state == 12);
	}
	
	SECTION("Ref helper is equivalent to the constructor") {
		auto ref1 = kangaru::source_reference_wrapper{source};
		auto ref2 = kangaru::ref(source);
		CHECK(std::addressof(ref1.unwrap()) == std::addressof(ref2.unwrap()));
		
		SECTION("Deducing guide") {
			auto ref3 = kangaru::source_reference_wrapper{ref1};
			auto ref4 = kangaru::ref(ref2);
			
			static_assert(std::same_as<decltype(ref1), decltype(ref3)>);
			CHECK(std::addressof(ref1.unwrap()) == std::addressof(ref2.unwrap()));
		}
	}
	
	static_assert(do_test_wrapper_wrapper<stateful_source>);
	static_assert(not do_test_wrapper_wrapper<kangaru::source_reference_wrapper<stateful_source>>);
	static_assert(not do_test_wrapper_wrapper<int>);
}

TEST_CASE("Source forwarding wrapper", "[source]") {
	auto source = stateful_source{.state = 10};
	
	SECTION("Source reference wrapper") {
		auto source_ref = kangaru::fwd_ref(source);
		CHECK(std::addressof(source) == std::addressof(source_ref.unwrap()));
		CHECK(std::same_as<int, decltype(kangaru::provide<int>(source_ref))>);
		CHECK(kangaru::provide<int>(source) == 11);
		CHECK(source.state == 11);
		CHECK(kangaru::provide<int>(source_ref) == 12);
		CHECK(source.state == 12);
	}
	
	SECTION("Ref helper is equivalent to the constructor") {
		auto ref1 = kangaru::source_forwarding_reference_wrapper{source};
		auto ref2 = kangaru::fwd_ref(source);
		CHECK(std::addressof(ref1.unwrap()) == std::addressof(ref2.unwrap()));
		
		SECTION("Deducing guide") {
			auto ref3 = kangaru::source_forwarding_reference_wrapper{ref1};
			auto ref4 = kangaru::fwd_ref(ref2);
			
			static_assert(std::same_as<decltype(ref1), decltype(ref3)>);
			CHECK(std::addressof(ref1.unwrap()) == std::addressof(ref2.unwrap()));
		}
		
		SECTION("Can transform into a normal ref and back") {
			auto ref3 = kangaru::ref(ref1);
			auto ref4 = kangaru::fwd_ref(ref3);
			auto ref5 = kangaru::fwd_ref(std::move(ref3));
			static_assert(std::same_as<decltype(ref4), decltype(ref5)>);
		}
	}
	
	SECTION("Supports rvalue when is itself rvalue") {
		auto source = value_cat_source{};
		auto ref = kangaru::fwd_ref(std::move(source));
		CHECK(kangaru::provide<int>(ref) == 1);
		CHECK(kangaru::provide<int>(std::move(ref)) == 2);
		
		auto const_ref = kangaru::fwd_ref(std::move(std::as_const(source)));
		CHECK(kangaru::provide<int>(const_ref) == 3);
		CHECK(kangaru::provide<int>(std::move(const_ref)) == 4);
		
		SECTION("lvalue never uses rvalue") {
			auto ref = kangaru::fwd_ref(source);
			CHECK(kangaru::provide<int>(ref) == 1);
			CHECK(kangaru::provide<int>(std::move(ref)) == 1);
			
			auto const_ref = kangaru::fwd_ref(std::as_const(source));
			CHECK(kangaru::provide<int>(const_ref) == 3);
			CHECK(kangaru::provide<int>(std::move(const_ref)) == 3);
		}
	}
	
	static_assert(do_test_wrapper_wrapper<stateful_source>);
	static_assert(not do_test_wrapper_wrapper<kangaru::source_forwarding_reference_wrapper<stateful_source&>>);
	static_assert(not do_test_wrapper_wrapper<int>);
}
