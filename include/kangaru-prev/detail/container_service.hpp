#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP

#include <type_traits>

namespace kgr {
	
struct container;

/*
 * Special definition.
 * 
 * This is the definition for the service kgr::container.
 * When the container is asked for this definition, il will proceed to inject himself in the constructor.
 */
struct container_service {
	explicit container_service(container& instance) : _instance{&instance} {}
	
	inline container& forward() {
		return *_instance;
	}
	
private:
	container* _instance;
};

namespace detail {

/*
 * Trait that tells if a particular service is the container service.
 */
template<typename T>
using is_container_service = std::is_same<container_service, T>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
