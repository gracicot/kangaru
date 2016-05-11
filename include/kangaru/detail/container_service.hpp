#ifndef KGR_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE
#define KGR_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE

#include "utils.hpp"
#include "injected.hpp"

namespace kgr {

struct Container;

namespace detail {

struct ContainerServiceTag {};

template<typename T>
using is_container_service = std::is_base_of<detail::ContainerServiceTag, T>;

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

#endif // KGR_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE
