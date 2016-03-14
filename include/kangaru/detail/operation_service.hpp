#pragma once

#include "../container.hpp"

namespace kgr {

struct ForkService {
	ForkService(Container& container) : _container{container.fork()} {}
	
	inline Container forward() {
		return std::move(_container);
	}
	
private:
	Container _container;
};

}
