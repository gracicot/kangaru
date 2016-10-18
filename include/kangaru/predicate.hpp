#ifndef KGR_KANGARU_INCLUDE_KANGARU_PREDICATE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_PREDICATE_HPP

#include "type_id.hpp"

namespace kgr {

struct All {
	constexpr inline bool operator()(type_id_t) const {
		return true;
	}
};

template<typename First, typename... Ts>
struct NoneOf {
	constexpr bool operator()(type_id_t id) const {
		return !compare<type_id<First>, type_id<Ts>...>(id);
	}
	
private:
	template<type_id_t comp, type_id_t second, type_id_t... others>
	constexpr bool compare(type_id_t id) const {
		return id == comp && compare<second, others...>(id);
	}
	
	template<type_id_t comp>
	constexpr bool compare(type_id_t id) const {
		return id == comp;
	}
};

template<typename First, typename... Ts>
struct AnyOf {
	constexpr bool operator()(type_id_t id) const {
		return compare<type_id<First>, type_id<Ts>...>(id);
	}
	
private:
	template<type_id_t comp, type_id_t second, type_id_t... others>
	constexpr bool compare(type_id_t id) const {
		return id == comp && compare<second, others...>(id);
	}
	
	template<type_id_t comp>
	constexpr bool compare(type_id_t id) const {
		return id == comp;
	}
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_PREDICATE_HPP
