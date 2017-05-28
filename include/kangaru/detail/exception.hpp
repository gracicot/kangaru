#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP

#include <exception>

namespace kgr {

struct AbstractNotFound : std::exception {
	inline const char* what() const noexcept override {
		return "No instance found for the requested abstract service";
	}
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP
