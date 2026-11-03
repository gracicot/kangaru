#ifndef KANGARU5_DETAIL_EXCEPTIONS_HPP
#define KANGARU5_DETAIL_EXCEPTIONS_HPP
// IWYU pragma: private, include <kangaru/kangaru.hpp>

#ifndef KANGARU5_MODULES
#include <exception>
#endif

namespace kangaru {
	struct exception : std::exception {
		inline virtual ~exception() = 0;
	};
	
	inline exception::~exception() = default;
	
	struct not_found_exception : exception {
		auto what() const noexcept -> char const* override {
			return "Assume cached object not found";
		}
	};
}

#endif // undef KANGARU5_DETAIL_EXCEPTIONS_HPP
