#include <catch2/catch.hpp>
#include <kangaru-prev/kangaru.hpp>

namespace {
	bool called = false;
	
	void function() noexcept {
		called = true;
	}

	TEST_CASE("Container can invoke noexcept functions", "[noexcept_invoke]") {
		kgr::container{}.invoke(function);
		REQUIRE(called);
	}
}
