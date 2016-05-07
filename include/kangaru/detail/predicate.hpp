#pragma once

#include "utils.hpp"

namespace kgr {

struct All {
	inline bool operator()(detail::type_id_t) {
		return true;
	}
};

template<typename First, typename... Ts>
struct NoneOf {
	bool operator()(detail::type_id_t id) {
		return !compare<detail::type_id<First>, detail::type_id<Ts>...>(id);
	}
	
private:
	template<detail::type_id_t comp, detail::type_id_t second, detail::type_id_t... others>
	bool compare(detail::type_id_t id) {
		return id == comp && compare<second, others...>(id);
	}
	
	template<detail::type_id_t comp>
	bool compare(detail::type_id_t id) {
		return id == comp;
	}
};

template<typename First, typename... Ts>
struct AnyOf {
	bool operator()(detail::type_id_t id) {
		return compare<detail::type_id<First>, detail::type_id<Ts>...>(id);
	}
	
private:
	template<detail::type_id_t comp, detail::type_id_t second, detail::type_id_t... others>
	bool compare(detail::type_id_t id) {
		return id == comp && compare<second, others...>(id);
	}
	
	template<detail::type_id_t comp>
	bool compare(detail::type_id_t id) {
		return id == comp;
	}
};

}
