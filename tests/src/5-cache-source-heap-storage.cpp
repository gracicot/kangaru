#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <array>
#include <fmt/core.h>

struct Base1 {
	virtual auto get() -> int {
		return 0;
	}
	virtual ~Base1() = default;
};

struct Base2 {
	virtual auto get() -> int {
		return 1;
	}
	virtual ~Base2() = default;
};

struct Derived1 final : Base1 {
	explicit Derived1(int id) noexcept : id{id} {}
	int id;
	auto get() -> int override {
		return id;
	}
	
	friend auto attribute(kangaru::overrides_types_in_cache<Derived1*>) -> std::tuple<Base1*>;
};

struct Derived2 final : Base1, Base2 {
	explicit Derived2(int id) noexcept : id{id} {}
	int id;
	auto get() -> int override {
		return id;
	}
	
	friend auto attribute(kangaru::overrides_types_in_cache<Derived2*>) -> std::tuple<Base1*, Base2*>;
};

struct increment_source {
	int n = 0;
	
	constexpr auto provide() -> int {
		return n++;
	}
};

static_assert(kangaru::cache_map<kangaru::polymorphic_map<std::unordered_map<kangaru::type_id, void*>>>);
static_assert(kangaru::cache_map<kangaru::with_cache<kangaru::none_source>>);
static_assert(kangaru::dereferenceable_cache_map<kangaru::source_reference_wrapper<kangaru::with_cache<kangaru::with_cache<kangaru::none_source>>>>);
static_assert(kangaru::cache_map<kangaru::with_cache<kangaru::none_source, kangaru::source_reference_wrapper<kangaru::with_cache<kangaru::none_source>>>>);
static_assert(kangaru::cache_map<kangaru::cache_with_two_step_init<kangaru::with_cache_asymmetric<kangaru::none_source>, kangaru::noop_second_step>>);
static_assert(kangaru::rebindable_source<kangaru::cache_with_two_step_init<kangaru::with_cache_asymmetric<kangaru::none_source>, kangaru::noop_second_step>>);
static_assert(kangaru::cache_map_stores<std::unordered_map<kangaru::type_id, std::any>, std::string_view>);
static_assert(kangaru::cache_map_stores<std::unordered_map<kangaru::type_id, std::any>, int>);
static_assert(kangaru::cache_map_stores<std::unordered_map<kangaru::type_id, std::any>, float>);
static_assert(kangaru::cache_map_stores<std::unordered_map<kangaru::type_id, void*>, float*>);

// Technically a cache_map, but no data insertable
static_assert(kangaru::cache_map<std::unordered_map<std::string_view, std::any>>);

// Not cache_map
static_assert(not kangaru::cache_map<int>);
static_assert(not kangaru::cache_map<std::vector<int>>);
static_assert(not kangaru::cache_map_stores<std::unordered_map<std::string_view, std::any>, std::string_view>);
static_assert(not kangaru::cache_map_stores<std::unordered_map<kangaru::type_id, void*>, float>);

template<kangaru::injectable>
using always_int = int;

static auto provide_count = 0ull;

struct count_second_step {
	auto operator()(auto& inserted, kangaru::forwarded_source auto&& source) const {
		if constexpr (std::same_as<int&, decltype(inserted)>) {
			CHECK(inserted == provide_count++);
			CHECK(inserted == source.n - 1);
		}
	}
};

TEMPLATE_TEST_CASE("Cache act has a subset of the map interface", "[cache]",
	(kangaru::with_cache<increment_source>),
	(kangaru::with_cache<increment_source, std::unordered_map<kangaru::type_id, std::any>>),
	(kangaru::with_cache<increment_source, kangaru::polymorphic_map<std::unordered_map<kangaru::type_id, std::any>>>),
	(kangaru::with_cache_asymmetric<increment_source, std::unordered_map<kangaru::type_id, std::any>, always_int>),
	(kangaru::cache_with_two_step_init<kangaru::with_cache<increment_source>, count_second_step>)
) {
	using namespace std::literals;
	
	provide_count = 0;
	
	auto cache = TestType{increment_source{}};
	auto const& cache_const = cache;
	
	cache.insert(std::pair{kangaru::type_id_for<long>(), long{1}});
	cache.insert(std::pair{kangaru::type_id_for<std::string>(), "potato"s});
	
	SECTION("Can iterate using iterator") {
		auto iterations = 0ull;
		for (auto const& [id, value] : cache) {
			if (id == kangaru::type_id_for<long>()) {
				CHECK(std::any_cast<long>(value) == 1);
			}
			
			if (id == kangaru::type_id_for<std::string>()) {
				CHECK(std::any_cast<std::string>(value) == "potato");
			}
			
			++iterations;
		}
		
		REQUIRE(iterations == 2);
	}
	
	SECTION("Can iterate using iterator const") {
		auto iterations = 0ull;
		for (auto const& [id, value] : cache_const) {
			if (id == kangaru::type_id_for<long>()) {
				CHECK(std::any_cast<long>(value) == 1);
			}
			
			if (id == kangaru::type_id_for<std::string>()) {
				CHECK(std::any_cast<std::string>(value) == "potato");
			}
			
			++iterations;
		}
		
		REQUIRE(iterations == 2);
	}
	
	SECTION("Can find element") {
		auto it = cache.find(kangaru::type_id_for<long>());
		REQUIRE(it != cache.end());
		CHECK(std::any_cast<long>(it->second) == 1);
	}
	
	SECTION("Can find element const") {
		auto it = cache_const.find(kangaru::type_id_for<long>());
		REQUIRE(it != cache_const.end());
		CHECK(std::any_cast<long>(it->second) == 1);
	}
	
	SECTION("Can use contains") {
		CHECK(cache.contains(kangaru::type_id_for<long>()));
		CHECK(cache.contains(kangaru::type_id_for<std::string>()));
		CHECK(not cache.contains(kangaru::type_id_for<float>()));
	}
	
	SECTION("Can use contains const") {
		CHECK(cache_const.contains(kangaru::type_id_for<long>()));
		CHECK(cache_const.contains(kangaru::type_id_for<std::string>()));
		CHECK(not cache_const.contains(kangaru::type_id_for<float>()));
	}
	
	SECTION("Can erase elements") {
		cache.erase(kangaru::type_id_for<std::string>());
		CHECK(not cache.contains(kangaru::type_id_for<std::string>()));
	}
	
	SECTION("Can insert or assign") {
		auto [it, inserted] = cache.insert_or_assign(kangaru::type_id_for<float>(), 4.5f);
		REQUIRE(inserted);
		CHECK(cache.contains(kangaru::type_id_for<float>()));
		CHECK(std::any_cast<float>(it->second) == 4.5f);
		
		{
			auto [it, inserted] = cache.insert_or_assign(kangaru::type_id_for<float>(), 6.5f);
			REQUIRE(not inserted);
		}
	}
	
	SECTION("Can insert a range") {
		auto elems = std::array{
			std::pair{kangaru::type_id{kangaru::type_id_for<float>()}, std::any{1.5f}},
			std::pair{kangaru::type_id{kangaru::type_id_for<double>()}, std::any{0.25}},
			std::pair{kangaru::type_id{kangaru::type_id_for<std::uint64_t>()}, std::any{std::uint64_t{32}}},
		};
		
		cache.insert(elems.begin(), elems.end());
		
		REQUIRE(cache.contains(kangaru::type_id_for<float>()));
		REQUIRE(cache.contains(kangaru::type_id_for<double>()));
		REQUIRE(cache.contains(kangaru::type_id_for<std::uint64_t>()));
		CHECK(std::any_cast<float>(cache.at(kangaru::type_id_for<float>())) == 1.5f);
		CHECK(std::any_cast<double>(cache.at(kangaru::type_id_for<double>())) == 0.25f);
		CHECK(std::any_cast<std::uint64_t>(cache.at(kangaru::type_id_for<std::uint64_t>())) == 32);
	}
	
	SECTION("Can use at") {
		REQUIRE_NOTHROW(cache.at(kangaru::type_id_for<long>()));
		CHECK(std::any_cast<long>(cache.at(kangaru::type_id_for<long>())) == 1);
		REQUIRE_THROWS(cache.at(kangaru::type_id_for<float>()));
	}
	
	SECTION("Can use at const") {
		REQUIRE_NOTHROW(cache_const.at(kangaru::type_id_for<long>()));
		CHECK(std::any_cast<long>(cache_const.at(kangaru::type_id_for<long>())) == 1);
		REQUIRE_THROWS(cache_const.at(kangaru::type_id_for<float>()));
	}
	
	SECTION("Has a size") {
		CHECK(cache.size() == 2);
	}
	
	SECTION("Can use empty and clear") {
		REQUIRE(not cache.empty());
		cache.clear();
		REQUIRE(cache.empty());
	}
	
	SECTION("Can provide the same type as the underlying source") {
		auto provided1 = kangaru::provide<int>(cache);
		REQUIRE(provided1 == 0);
		
		// No increment since the value is cached in the map
		auto provided2 = kangaru::provide<int>(cache);
		REQUIRE(provided2 == 0);
		
		if constexpr (std::same_as<kangaru::cache_with_two_step_init<kangaru::with_cache<increment_source>, count_second_step>, TestType>) {
			REQUIRE(provide_count == 1);
		}
	}
}

TEST_CASE("Runtime source will cache sources results", "[cache]") {
	auto source = increment_source{};
	SECTION("Will cache the result of sources") {
		auto runtime_source = kangaru::with_cache<kangaru::with_heap_storage<decltype(kangaru::ref(source))>>{kangaru::with_heap_storage{kangaru::ref(source)}};
		
		CHECK(*kangaru::provide<int*>(runtime_source) == 0);
		CHECK(*kangaru::provide<int*>(runtime_source) == 0);
		CHECK(kangaru::provide<int>(source) == 1);
		CHECK(*kangaru::provide<int*>(runtime_source) == 0);
	}
	
	SECTION("Can polymorphically store cached types") {
		auto source = kangaru::compose(
			kangaru::object_source{Derived1{3}},
			kangaru::object_source{Base1{}}
		);
		auto runtime_source = kangaru::make_source_with_cache(
			kangaru::make_source_with_heap_storage(source),
			kangaru::polymorphic_map<std::unordered_map<kangaru::type_id, void*>>{}
		);
		
		CHECK(kangaru::provide<Derived1*>(runtime_source)->get() == 3);
		CHECK(kangaru::provide<Base1*>(runtime_source)->get() == 3);
	}
	
	SECTION("Already cached type has priority") {
		auto base_source = kangaru::compose(
			kangaru::object_source{Derived1{3}},
			kangaru::object_source{Derived2{4}},
			kangaru::object_source{Base1{}},
			kangaru::object_source{Base2{}}
		);
		auto source = kangaru::make_source_with_cache(
			kangaru::make_source_with_heap_storage(base_source),
			kangaru::polymorphic_map<std::unordered_map<kangaru::type_id, void*>>{}
		);
		
		SECTION("Base type wins if taken first") {
			CHECK(kangaru::provide<Base1*>(source)->get() == 0);
			CHECK(kangaru::provide<Derived1*>(source)->get() == 3);
			CHECK(kangaru::provide<Base1*>(source)->get() == 0);
		}
		
		SECTION("But another type can win if it also extends another type") {
			CHECK(kangaru::provide<Base1*>(source)->get() == 0);
			CHECK(kangaru::provide<Derived2*>(source)->get() == 4);
			CHECK(kangaru::provide<Base1*>(source)->get() == 0);
			CHECK(kangaru::provide<Base2*>(source)->get() == 4);
		}
	}
	
	SECTION("Can replace types using insert_or_assign") {
		auto base_source = kangaru::compose(
			kangaru::object_source{Derived1{3}},
			kangaru::object_source{Derived2{4}},
			kangaru::object_source{Base1{}},
			kangaru::object_source{Base2{}}
		);
		
		auto source = kangaru::make_source_with_cache(
			kangaru::make_source_with_heap_storage(base_source),
			kangaru::polymorphic_map<std::unordered_map<kangaru::type_id, void*>>{}
		);
		
		SECTION("Replace base with derived") {
			CHECK(kangaru::provide<Base1*>(source)->get() == 0);
			source.insert_or_assign(
				kangaru::type_id_for<Derived1*>(),
				source.source.emplace_from([&] {
					return kangaru::provide<Derived1>(base_source);
				})
			);
			CHECK(kangaru::provide<Base1*>(source)->get() == 3);
		}
		
		SECTION("Replace base with derived") {
			CHECK(kangaru::provide<Base1*>(source)->get() == 0);
			source.insert_or_assign(
				kangaru::type_id_for<Derived1*>(),
				source.source.emplace_from([&] {
					return kangaru::provide<Derived1>(base_source);
				})
			);
			CHECK(kangaru::provide<Base1*>(source)->get() == 3);
		}
	}
}
