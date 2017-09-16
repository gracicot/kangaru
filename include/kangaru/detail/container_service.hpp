#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP

#include <type_traits>

namespace kgr {

struct Container;

/*
 * Special definition.
 * 
 * This is the definition for the service kgr::Container.
 * When the container is asked for this definition, il will proceed to inject himself in the constructor.
 */
struct ContainerService {
	explicit ContainerService(Container& instance) : _instance{&instance} {}
	
	inline Container& forward() {
		return *_instance;
	}
	
private:
	Container* _instance;
};

/*
 * We map the container in the service map.
 */
auto service_map(const Container&) -> ContainerService;

namespace detail {

/*
 * Trait that tells if a particular service is the container service.
 */
template<typename T>
using is_container_service = std::is_same<ContainerService, T>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
