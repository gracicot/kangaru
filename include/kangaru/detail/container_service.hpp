#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP

#include "utils.hpp"
#include "injected.hpp"

namespace kgr {

struct Container;

namespace detail {

struct ContainerServiceTag {};

template<typename T>
using is_container_service = std::is_base_of<ContainerServiceTag, T>;

} // namespace detail

struct ContainerService : detail::ContainerServiceTag {
	explicit ContainerService(Container& instance) : _instance{instance} {}
	
	inline Container& forward() {
		return _instance;
	}
	
private:
	Container& _instance;
};

auto service_map(const Container&) -> ContainerService;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
