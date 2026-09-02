#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

template<typename>
struct tt {};

template<typename, typename>
struct tt2 {};

template<typename T>
struct transparent_rebindable_1 {
	T source;
};

template<typename T, typename P>
struct transparent_rebindable_2 {
	T source;
};

template<typename T, template<typename> typename TT>
struct transparent_rebindable_3 {
	T source;
};

template<typename T>
struct transparent_rebindable_4 {
	T source;
	explicit transparent_rebindable_4(T source) {}
};

template<typename T, typename P>
struct transparent_rebindable_5 {
	T source;
	explicit transparent_rebindable_5(T source) {}
};

template<typename T, template<typename> typename TT>
struct transparent_rebindable_6 {
	T source;
	explicit transparent_rebindable_6(T source) {}
};

template<typename T>
struct transparent_rebindable_7 {
	T source;
	int unrelated;
};

template<typename T, typename P, template<typename> typename TT>
struct not_rebindable_1 {
	T source;
};

template<typename T, typename P>
struct not_rebindable_2 {
	T source;
	P param;
};

template<typename T, typename P>
struct not_rebindable_3 {
	T source;
	explicit not_rebindable_3(T source, P param) {}
};

template<template<typename> typename TT, typename T>
struct not_rebindable_4 {
	T source;
	explicit not_rebindable_4(T source) {}
};

template<int, typename T>
struct not_rebindable_5 {
	T source;
	explicit not_rebindable_5(T source) {}
};

template<typename T, template<typename, typename> typename TT>
struct not_rebindable_6 {
	T source;
};

template<typename T>
struct not_rebindable_7 {
	explicit not_rebindable_7(T source) {}
};

template<kangaru::source Source>
struct stateful_rebindable_1 {
	Source source;
	
	template<kangaru::forwarded<stateful_rebindable_1> Original, kangaru::forwarded_source NewSource>
	static constexpr auto rebind(Original&& self, NewSource&& new_source) {
		return stateful_rebindable_1<kangaru::deduced_source_type<NewSource>>{
			std::forward<NewSource>(new_source)
		};
	}
};

template<kangaru::source Source>
struct stateful_rebindable_2 {
	Source source;
	
	template<kangaru::forwarded<stateful_rebindable_2> Original>
	static constexpr auto rebind(Original&& self, kangaru::none_source&& new_source) {
		return stateful_rebindable_1<kangaru::none_source>{
			kangaru::none_source{}
		};
	}
};


template<kangaru::source Source>
struct adversarial_1 {
	Source source;
	
	template<kangaru::forwarded<adversarial_1> Original, kangaru::different_from<kangaru::none_source> NewSource>
	static constexpr auto rebind(Original&& self, NewSource new_source) {
		return stateful_rebindable_1<NewSource>{
			std::move(new_source)
		};
	}
};

template<typename T>
struct adversarial_2 {
	kangaru::none_source source;
};

static_assert(kangaru::transparent_rebindable_wrapping_source<transparent_rebindable_1<kangaru::none_source>>);
static_assert(kangaru::transparent_rebindable_wrapping_source<transparent_rebindable_2<kangaru::none_source, int>>);
static_assert(kangaru::transparent_rebindable_wrapping_source<transparent_rebindable_3<kangaru::none_source, tt>>);
static_assert(kangaru::transparent_rebindable_wrapping_source<transparent_rebindable_4<kangaru::none_source>>);
static_assert(kangaru::transparent_rebindable_wrapping_source<transparent_rebindable_5<kangaru::none_source, int>>);
static_assert(kangaru::transparent_rebindable_wrapping_source<transparent_rebindable_6<kangaru::none_source, tt>>);
static_assert(kangaru::transparent_rebindable_wrapping_source<transparent_rebindable_7<kangaru::none_source>>);

static_assert(not kangaru::transparent_rebindable_wrapping_source<not_rebindable_1<kangaru::none_source, int, tt>>);
static_assert(not kangaru::transparent_rebindable_wrapping_source<not_rebindable_2<kangaru::none_source, int>>);
static_assert(not kangaru::transparent_rebindable_wrapping_source<not_rebindable_3<kangaru::none_source, int>>);
static_assert(not kangaru::transparent_rebindable_wrapping_source<not_rebindable_4<tt, kangaru::none_source>>);
static_assert(not kangaru::transparent_rebindable_wrapping_source<not_rebindable_5<0, kangaru::none_source>>);
static_assert(not kangaru::transparent_rebindable_wrapping_source<not_rebindable_6<kangaru::none_source, tt2>>);
static_assert(not kangaru::transparent_rebindable_wrapping_source<not_rebindable_7<kangaru::none_source>>);


static_assert(kangaru::stateful_rebindable_wrapping_source<stateful_rebindable_1<kangaru::none_source>>);
static_assert(kangaru::stateful_rebindable_wrapping_source<stateful_rebindable_2<kangaru::none_source>>);

// Careful! Kangaru need the rebind function to accept kangaru::none_source
// Otherwise it might be considered transparent rebindable otherwise.
static_assert(not kangaru::stateful_rebindable_wrapping_source<adversarial_1<kangaru::none_source>>);
static_assert(kangaru::transparent_rebindable_wrapping_source<adversarial_1<kangaru::none_source>>);

// There is a template parameter, is a wrapping source, but the template parameter
// Does not correspond to the wrapped source. Technically transparently rebindable, but
// We cannot verify until we evaluate the rebind function.
// It might just be rebindable if you rebind with none_source specifically!
static_assert(kangaru::transparent_rebindable_wrapping_source<adversarial_2<kangaru::none_source>>);

static_assert(kangaru::rebindable_source<transparent_rebindable_1<kangaru::none_source>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_2<kangaru::none_source, int>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_3<kangaru::none_source, tt>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_4<kangaru::none_source>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_5<kangaru::none_source, int>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_6<kangaru::none_source, tt>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_7<kangaru::none_source>>);

static_assert(not kangaru::rebindable_source<not_rebindable_1<kangaru::none_source, int, tt>>);
static_assert(not kangaru::rebindable_source<not_rebindable_2<kangaru::none_source, int>>);
static_assert(not kangaru::rebindable_source<not_rebindable_3<kangaru::none_source, int>>);
static_assert(not kangaru::rebindable_source<not_rebindable_4<tt, kangaru::none_source>>);
static_assert(not kangaru::rebindable_source<not_rebindable_5<0, kangaru::none_source>>);
static_assert(not kangaru::rebindable_source<not_rebindable_6<kangaru::none_source, tt2>>);

static_assert(kangaru::rebindable_source<stateful_rebindable_1<kangaru::none_source>>);
static_assert(kangaru::rebindable_source<stateful_rebindable_2<kangaru::none_source>>);

static_assert(not kangaru::rebindable_source<transparent_rebindable_1<kangaru::none_source>&>);
static_assert(kangaru::forwarded_rebindable_source<transparent_rebindable_1<kangaru::none_source>&>);

// Unfortunately rebindable
static_assert(kangaru::rebindable_source<adversarial_1<kangaru::none_source>>);

// Not wrapper, so a leaf. A single leaf is rebindable.
static_assert(kangaru::rebindable_source<not_rebindable_7<kangaru::none_source>>);

// Recursive cases
static_assert(kangaru::rebindable_source<transparent_rebindable_2<transparent_rebindable_1<kangaru::none_source>, int>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_3<transparent_rebindable_2<transparent_rebindable_1<kangaru::none_source>, int>, tt>>);
static_assert(not kangaru::rebindable_source<not_rebindable_2<transparent_rebindable_1<kangaru::none_source>, int>>);
static_assert(not kangaru::rebindable_source<transparent_rebindable_3<not_rebindable_2<transparent_rebindable_1<kangaru::none_source>, int>, tt>>);
static_assert(not kangaru::rebindable_source<transparent_rebindable_2<transparent_rebindable_1<not_rebindable_5<0, kangaru::none_source>>, int>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_2<kangaru::source_reference_wrapper<transparent_rebindable_1<kangaru::none_source>>, int>>);
static_assert(kangaru::rebindable_source<transparent_rebindable_3<transparent_rebindable_2<transparent_rebindable_1<kangaru::source_reference_wrapper<kangaru::none_source>>, int>, tt>>);
static_assert(kangaru::rebindable_source<kangaru::source_reference_wrapper<transparent_rebindable_3<transparent_rebindable_2<transparent_rebindable_1<kangaru::none_source>, int>, tt>>>);

TEST_CASE("Source rebind", "[source]") {
	SECTION("Simple rebind") {
		auto source = transparent_rebindable_1{kangaru::object_source{1}};
		auto rebound = kangaru::rebind(source, [](kangaru::object_source<int>& source) {
			return kangaru::ref(source);
		});
		
		static_assert(std::same_as<
			transparent_rebindable_1<kangaru::source_reference_wrapper<kangaru::object_source<int>>>,
			decltype(rebound)
		>);
		
		REQUIRE(std::addressof(source.source) == std::addressof(rebound.source.unwrap()));
	}
	
	SECTION("Rebind through a reference wrapper") {
		auto s1 = kangaru::object_source{1};
		auto source = transparent_rebindable_1{kangaru::ref(s1)};
		auto rebound = kangaru::rebind(source, [](kangaru::object_source<int>& source) {
			return kangaru::ref(source);
		});
		
		static_assert(std::same_as<
			transparent_rebindable_1<kangaru::source_reference_wrapper<kangaru::object_source<int>>>,
			decltype(rebound)
		>);
		
		REQUIRE(std::addressof(s1) == std::addressof(rebound.source.unwrap()));
	}
	
	SECTION("Rebind through a forwarding reference wrapper") {
		auto s1 = kangaru::object_source{1};
		auto source = transparent_rebindable_1{kangaru::fwd_ref(std::move(s1))};
		
		auto rebound1 = kangaru::rebind(source, [](kangaru::object_source<int>& source) {
			return kangaru::ref(source);
		});
		
		auto rebound2 = kangaru::rebind(std::move(source), [](kangaru::object_source<int>&& source) {
			return kangaru::ref(source);
		});
		
		static_assert(std::same_as<
			transparent_rebindable_1<kangaru::source_reference_wrapper<kangaru::object_source<int>>>,
			decltype(rebound1)
		>);
		
		static_assert(std::same_as<
			transparent_rebindable_1<kangaru::source_reference_wrapper<kangaru::object_source<int>>>,
			decltype(rebound2)
		>);
		
		REQUIRE(std::addressof(s1) == std::addressof(rebound1.source.unwrap()));
		REQUIRE(std::addressof(s1) == std::addressof(rebound2.source.unwrap()));
	}
	
	SECTION("Rebind through a stateful wrapping source") {
		auto source = transparent_rebindable_1{stateful_rebindable_1{kangaru::object_source{1}}};
		
		auto rebound = kangaru::rebind(source, [](kangaru::object_source<int>& source) {
			return kangaru::ref(source);
		});
		
		static_assert(std::same_as<
			transparent_rebindable_1<stateful_rebindable_1<kangaru::source_reference_wrapper<kangaru::object_source<int>>>>,
			decltype(rebound)
		>);
		
		REQUIRE(std::addressof(source.source.source) == std::addressof(rebound.source.source.unwrap()));
	}
}
