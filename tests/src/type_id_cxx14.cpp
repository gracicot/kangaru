#include <catch2/catch_test_macros.hpp>
#include <kangaru-prev/kangaru.hpp>

#include <random>
#include <limits>

namespace kgr_test {

struct test {};
class another_type;

TEST_CASE("The hash based typeid give back reproducible values", "[type_id]") {
	REQUIRE(kgr::detail::type_name<test>() == "kgr_test::test");
	REQUIRE(kgr::type_id<test>() == ((kgr::detail::hash_64_fnv1a("kgr_test::test") & kgr::detail::hash_mask)));

	REQUIRE(kgr::detail::type_name<another_type>() == "kgr_test::another_type");
	REQUIRE(kgr::type_id<another_type>() == ((kgr::detail::hash_64_fnv1a("kgr_test::another_type") & kgr::detail::hash_mask)));

	REQUIRE(kgr::detail::type_name<int>() == "int");
	REQUIRE(kgr::type_id<int>() == ((kgr::detail::hash_64_fnv1a("int") & kgr::detail::hash_mask)));
}

} // namespace kgr_test
