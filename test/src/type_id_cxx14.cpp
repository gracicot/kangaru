#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>

#include <random>
#include <limits>

namespace kgr_test {

TEST_CASE("The hash based typeid give back reproducible values", "[type_id]") {
	struct test;
	class another_type;

	REQUIRE(kgr::detail::type_name<test>() == "test");
	REQUIRE(kgr::type_id<test>() == ((kgr::detail::hash_64_fnv1a("test") & kgr::detail::hash_mask)));

	REQUIRE(kgr::detail::type_name<another_type>() == "another_type");
	REQUIRE(kgr::type_id<another_type>() == ((kgr::detail::hash_64_fnv1a("another_type") & kgr::detail::hash_mask)));

	REQUIRE(kgr::detail::type_name<int>() == "int");
	REQUIRE(kgr::type_id<int>() == ((kgr::detail::hash_64_fnv1a("int") & kgr::detail::hash_mask)));
}

} // namespace kgr_test
