#ifndef KANGARU5_DETAIL_EXCEPTIONS_HPP
#define KANGARU5_DETAIL_EXCEPTIONS_HPP

#ifndef KANGARU5_MODULES
#include <exception>
#endif

namespace kangaru {
	struct kangaru_exception : std::exception {
		virtual ~kangaru_exception() = 0;
	};
	
	kangaru_exception::~kangaru_exception() = default;
	
	struct throwing_source_exception : kangaru_exception {
		auto what() const noexcept -> char const* override {
			return "Throwing source called";
		}
	};
}

#endif // undef KANGARU5_DETAIL_EXCEPTIONS_HPP
