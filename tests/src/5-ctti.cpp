#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

struct service_a {};
struct service_b {};

TEST_CASE("CTTI Gives reliable names", "[ctti]") {
	#ifndef _MSC_VER
	CHECK(kangaru::type_name<kangaru::reference_source<service_a>>() == "kangaru::reference_source<service_a>");
	#else
	CHECK(kangaru::type_name<kangaru::reference_source<service_a>>() == "kangaru::reference_source<struct service_a>");
	#endif
}
