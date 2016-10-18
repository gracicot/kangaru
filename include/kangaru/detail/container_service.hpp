#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP

#include "utils.hpp"
#include "injected.hpp"

namespace kgr {

struct Container;

namespace detail {

struct ContainerServiceTag {};

} // namespace detail

struct ContainerService : detail::ContainerServiceTag {
	explicit ContainerService(Container& instance) : _instance{instance} {}
	
	inline Container& forward() {
		return _instance;
	}
	
private:
	Container& _instance;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
