#ifndef KGR_KANGARU_INCLUDE_KANGARU_PREDICATE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_PREDICATE_HPP

#include "type_id.hpp"

namespace kgr {
namespace detail {

template<typename First, typename... Ts>
struct find_id_predicate {
protected:
	constexpr bool find(type_id_t id) const noexcept {
		return find<First, Ts...>(id);
	}
	
	template<typename Compared, typename Second, typename... Rest>
	constexpr bool find(type_id_t id) const noexcept {
		return id == type_id<Compared>() && find<Second, Rest...>(id);
	}
	
	template<typename Compared>
	constexpr bool find(type_id_t id) const noexcept {
		return id == type_id<Compared>();
	}
};

} // namespace detail

/*
 * This predicate always returns true.
 * 
 * This is the default predicate used by the container.
 */
struct all {
	constexpr inline bool operator()(type_id_t) const noexcept {
		return true;
	}
};

/*
 * This predicate returns true for all services except those specified.
 */
template<typename First, typename... Ts>
struct except : private detail::find_id_predicate<First, Ts...> {
	constexpr bool operator()(type_id_t id) const noexcept {
		return !this->find(id);
	}
};

/*
 * Predicate that returns false for all services, except those passed as argument.
 */
template<typename First, typename... Ts>
struct only : private detail::find_id_predicate<First, Ts...> {
	constexpr bool operator()(type_id_t id) const noexcept {
		return this->find(id);
	}
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_PREDICATE_HPP
