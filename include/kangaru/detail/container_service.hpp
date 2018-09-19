#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP

#include <type_traits>

namespace kgr {
	
namespace detail {
	struct basic_container;
} // namespace detail

/*
 * Special definition.
 * 
 * This is the definition for the service kgr::container.
 * When the container is asked for this definition, il will proceed to inject himself in the constructor.
 */
struct container_service {
	explicit container_service(detail::basic_container& instance) : _instance{&instance} {}
	
	inline detail::basic_container& forward() {
		return *_instance;
	}
	
private:
	detail::basic_container* _instance;
};

namespace detail {

/*
 * We map the container in the service map.
 */
auto service_map(const basic_container&) -> container_service;

/*
 * Trait that tells if a particular service is the container service.
 */
template<typename T>
using is_container_service = std::is_same<container_service, T>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONTAINER_SERVICE_HPP
