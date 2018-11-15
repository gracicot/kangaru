#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>
#include <csignal>

static void abort_handler(int signal)
{
	if (signal == SIGABRT) {
		SUCCEED();
		std::_Exit(EXIT_SUCCESS);
	}
}

TEST_CASE("Should abort when exceptions are disabled", "[noexcept]") {
	std::signal(SIGABRT, abort_handler);
	
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

