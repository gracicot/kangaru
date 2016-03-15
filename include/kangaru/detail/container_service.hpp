#pragma once

#include <tuple>
#include "utils.hpp"

namespace kgr {

struct Container;

namespace detail {

struct ContainerServiceTag {};

template<typename Original, typename Service>
struct ServiceOverride : Service {
    virtual ~ServiceOverride() {}
    ServiceOverride(Original& service) : _service{service} {}

    ServiceType<Service> forward() override {
        return static_cast<ServiceType<Service>>(_service.forward());
    }

private:
    Original& _service;
};

}

struct Single {
	Single() = default;
	virtual ~Single() = default;
	Single(const Single&) = delete;
	Single& operator=(const Single&) = delete;
	Single(Single&&) = default;
	Single& operator=(Single&&) = default;
};

template<typename... Types>
struct Overrides {
	using ParentTypes = std::tuple<Types...>;
};

struct ContainerService : detail::ContainerServiceTag {
	ContainerService(Container& instance) : _instance{instance} {}
	
	inline Container& forward() {
		return _instance;
	}
	
private:
	Container& _instance;
};

}
