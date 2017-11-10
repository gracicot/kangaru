#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP

#include <exception>

namespace kgr {

/*
 * This exception is thrown when an abstract service is not found, yet asked for it's definition.
 */
struct AbstractNotFound : std::exception {
	inline const char* what() const noexcept override {
		return "No instance found for the requested abstract service";
	}
};

/*
 * This exception is thrown when a supplied service is not found, yet asked for it's definition.
 */
struct SuppliedNotFound : std::exception {
	inline const char* what() const noexcept override {
		return "No instance found for the requested supplied service";
	}
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP
