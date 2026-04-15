#include <kangaru/kangaru.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Allocator is an alternative interface to allocate any kind of objects", "[allocator]") {
	auto a = kangaru::default_allocator{};
	
	SECTION("Object") {
		auto obj = a.allocate_object<int>(1);
		std::construct_at(obj, 4);
		REQUIRE(*obj == 4);
		a.deallocate_object<int>(obj);
	}
	
	SECTION("Bytes") {
		auto b = a.allocate_bytes(sizeof(int), alignof(int));
		auto obj = new (b) int{4};
		REQUIRE(*obj == 4);
		a.deallocate_bytes(b, sizeof(int), alignof(int));
	}
}

static_assert([]{
	auto a = kangaru::default_allocator{};
	auto obj = a.allocate_object<int>(1);
	std::construct_at(obj, 4);
	int result = *obj;
	a.deallocate_object<int>(obj);
	return result;
}() == 4);

