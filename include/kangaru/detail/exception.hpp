#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP

#include <exception>

namespace kgr {

/*
 * This exception is the base exception type for all kind of exception caused by a service not found.
 */
struct service_not_found : std::exception {};

/*
 * This exception is thrown when an abstract service is not found, yet asked for it's definition.
 */
struct abstract_not_found : service_not_found {
	inline const char* what() const noexcept override {
		return "No instance found for the requested abstract service";
	}
};

/*
 * This exception is thrown when a supplied service is not found, yet asked for it's definition.
 */
struct supplied_not_found : service_not_found {
	inline const char* what() const noexcept override {
		return "No instance found for the requested supplied service";
	}
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_EXCEPTION_HPP
