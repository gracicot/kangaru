#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>
#include <csignal>

static void abort_handler(int signal)
{
	if (signal == SIGABRT) {
		SUCCEED();
		std::exit(0);
	}
}

#if defined(KGR_KANGARU_TEST_ABSTRACT_ABORT)

TEST_CASE("Should abort when exceptions are disabled", "[noexcept]") {
	std::signal(SIGABRT, abort_handler);
	struct A {};
	struct Abstract : kgr::abstract_service<A> {};
	
	kgr::container{}.service<Abstract>();
}

#elif KGR_KANGARU_TEST_SUPPLIED_ABORT

TEST_CASE("Should abort when exceptions are disabled", "[noexcept]") {
	std::signal(SIGABRT, abort_handler);
	struct S {};
	struct Supplied : kgr::single_service<S>, kgr::supplied {};
	
	kgr::container{}.service<Supplied>();
}

#endif
