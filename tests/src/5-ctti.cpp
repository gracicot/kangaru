#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

struct service_a {};
struct service_b {};

TEST_CASE("CTTI Gives reliable names", "[ctti]") {
	using namespace kangaru::detail::ctti;

	#ifndef _MSC_VER
	CHECK(type_name<kangaru::injectable_reference_source<service_a>>() == "kangaru::injectable_reference_source<service_a>");
	#else
	// TODO: Create a parser that produce reliable cross platform results
	CHECK(type_name<kangaru::injectable_reference_source<service_a>>() == "kangaru::injectable_reference_source<struct service_a>");
	#endif
}
