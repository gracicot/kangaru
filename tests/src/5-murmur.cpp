#include "kangaru/detail/murmur.hpp"
#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <array>
#include <cstddef>
#include <vector>

static_assert(
	   kangaru::detail::murmur::murmur64a(std::array{'a', 'b', 'c'})
	== 0x9cc9c33498a95efbull
);

static_assert(
	kangaru::detail::murmur::murmur64a(
		std::array{'a', 'b', 'c'},
		kangaru::detail::murmur::hash_t{752304984}
	) == 0xe4ba8ad868fa0780ull
);

static_assert(
	   kangaru::detail::murmur::murmur64a("abc")
	== 0x9cc9c33498a95efbull
);

static_assert(
	kangaru::detail::murmur::murmur64a(
		std::vector{std::byte{'a'}, std::byte{'b'}, std::byte{'c'}}
	) == 0x9cc9c33498a95efbull
);

TEST_CASE("Murmur is a hash algorithm", "[murmur]") {
	// All tests are compile time
	REQUIRE(true);
}
