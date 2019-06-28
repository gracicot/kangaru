#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_STORAGE_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_STORAGE_SERVICE_HPP

#include <vector>
#include <algorithm>

#include "single.hpp"
#include "injected.hpp"
#include "service_storage.hpp"

namespace kgr {
namespace detail {

struct override_storage {
	using overrides_t = std::vector<std::vector<std::pair<type_id_t, service_storage>>>;
	
	template<typename P>
	auto filter(P predicate) const -> overrides_t {
		overrides_t fork;
		fork.reserve(overrides.capacity());
		
		std::transform(
			overrides.begin(),
			overrides.end(),
			std::back_inserter(fork),
			[&predicate](overrides_t::const_reference overrides) -> overrides_t::value_type {
				overrides_t::value_type filtered;
				filtered.reserve(overrides.capacity());
				std::copy_if(
					overrides.begin(),
					overrides.end(),
					std::back_inserter(filtered),
					[&predicate](overrides_t::value_type::const_reference service) -> bool {
						return predicate(service.first);
					}
				);
				
				return filtered;
			}
		);
		
		return fork;
	}
	
	overrides_t overrides;
};

struct override_storage_service : single {
	override_storage service;
	
	inline static auto construct() -> kgr::inject_result<> {
		return kgr::inject();
	}
	
	inline auto forward() -> override_storage& {
		return service;
	}
	
	inline auto forward() const -> override_storage const& {
		return service;
	}
};

template<typename>
struct index_storage;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_STORAGE_SERVICE_HPP
