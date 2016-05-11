#pragma once

#include "utils.hpp"

namespace kgr {

struct All {
	inline bool operator()(type_id_t) {
		return true;
	}
};

template<typename First, typename... Ts>
struct NoneOf {
	bool operator()(type_id_t id) {
		return !compare<type_id<First>, type_id<Ts>...>(id);
	}
	
private:
	template<type_id_t comp, type_id_t second, type_id_t... others>
	bool compare(type_id_t id) {
		return id == comp && compare<second, others...>(id);
	}
	
	template<type_id_t comp>
	bool compare(type_id_t id) {
		return id == comp;
	}
};

template<typename First, typename... Ts>
struct AnyOf {
	bool operator()(type_id_t id) {
		return compare<type_id<First>, type_id<Ts>...>(id);
	}
	
private:
	template<type_id_t comp, type_id_t second, type_id_t... others>
	bool compare(type_id_t id) {
		return id == comp && compare<second, others...>(id);
	}
	
	template<type_id_t comp>
	bool compare(type_id_t id) {
		return id == comp;
	}
};

}
