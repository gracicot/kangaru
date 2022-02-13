#include <kangaru/kangaru.hpp>

#define TEST_ERROR(...) int toplevel = (void(__VA_ARGS__), 0)

#if defined(kgr_error_test_not_service_no_forward)
	
	struct test {};
	TEST_ERROR(kgr::debug::service<test>());
	
#elif defined(kgr_error_test_not_service_has_forward)
	
	struct test {
		virtual ~test() = default;
		auto forward() -> int;
	};
	TEST_ERROR(kgr::debug::service<test>());
	
#endif
