#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_STORAGE_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_STORAGE_SERVICE_HPP

#include <vector>

#include "single.hpp"
#include "injected.hpp"
#include "service_storage.hpp"

namespace kgr {
namespace detail {

struct override_storage {
	std::vector<std::vector<service_storage>> overrides;
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
