#include <kangaru/kangaru.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Exception has a specific hierarchy", "[exceptions]") {
	int branch = 0;
	
	SECTION("Basic") {
		try {
			throw kangaru::not_found_exception{};
		} catch (kangaru::not_found_exception const&) {
			branch = 1;
		} catch (kangaru::exception const&) {
			branch = 2;
		} catch (std::exception const&) {
			branch = 3;
		}
	}
	
	SECTION("Kangaru exception") {
		try {
			throw kangaru::not_found_exception{};
		} catch (kangaru::exception const&) {
			branch = 1;
		} catch (std::exception const&) {
			branch = 3;
		}
	}
	
	SECTION("Kangaru exception") {
		try {
			throw kangaru::not_found_exception{};
		} catch (std::exception const&) {
			branch = 1;
		}
	}
	
	REQUIRE(branch == 1);
}
