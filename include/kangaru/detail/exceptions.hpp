#ifndef KANGARU5_DETAIL_EXCEPTIONS_HPP
#define KANGARU5_DETAIL_EXCEPTIONS_HPP

#ifndef KANGARU5_MODULES
#include <exception>
#endif

namespace kangaru {
	struct kangaru_exception : std::exception {
		inline virtual ~kangaru_exception() = 0;
	};
	
	inline kangaru_exception::~kangaru_exception() = default;
	
	struct not_found_exception : kangaru_exception {
		auto what() const noexcept -> char const* override {
			return "Assume cached object not found";
		}
	};
}

#endif // undef KANGARU5_DETAIL_EXCEPTIONS_HPP
