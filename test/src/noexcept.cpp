#include <catch2/catch.hpp>
#include <kangaru-prev/kangaru.hpp>
#include <csignal>

static void abort_handler(int signal) {
	if (signal == SIGABRT) {
		SUCCEED();
		std::_Exit(EXIT_SUCCESS);
	}
}

static void setup_handlers() {
	std::signal(SIGABRT, abort_handler);
	
	// On windows we must prevent the "Abort, Retry, Ignore" window from opening.
#if defined(_WIN32) || defined(_WIN64)
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
}

TEST_CASE("Should abort when exceptions are disabled", "[noexcept]") {
	setup_handlers();

#if defined(KGR_KANGARU_TEST_ABSTRACT_ABORT)
	SECTION("Supplied service error") {
		struct S {};
		struct Supplied : kgr::single_service<S>, kgr::supplied {};
		
		kgr::container{}.service<Supplied>();
	}
#elif defined(KGR_KANGARU_TEST_SUPPLIED_ABORT)
	SECTION("Abstract service error") {
		struct A {};
		struct Abstract : kgr::abstract_service<A> {};
		
		kgr::container{}.service<Abstract>();
	}
#else
	#error "Specify a test to run"
#endif
}

