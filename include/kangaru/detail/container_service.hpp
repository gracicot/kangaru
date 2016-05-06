#pragma once

#include "utils.hpp"
#include "injected.hpp"

namespace kgr {

struct Container;

namespace detail {

struct ContainerServiceTag {};

template<typename T>
using is_container_service = std::is_base_of<detail::ContainerServiceTag, T>;

template<typename Original, typename Service>
struct ServiceOverride final : BaseInjected<Service> {
    explicit ServiceOverride(Original& service) : _service{service} {}

    ServiceType<Service> forward() override {
        return static_cast<ServiceType<Service>>(_service.forward());
    }

private:
    Original& _service;
};

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
