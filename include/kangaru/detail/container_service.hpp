#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP

#include <type_traits>

namespace kgr {

struct Container;

struct ContainerService {
	explicit ContainerService(Container& instance) : _instance{&instance} {}
	
	inline Container& forward() {
		return *_instance;
	}
	
private:
	Container* _instance;
};

auto service_map(const Container&) -> ContainerService;

namespace detail {

template<typename T>
using is_container_service = std::is_same<ContainerService, T>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
