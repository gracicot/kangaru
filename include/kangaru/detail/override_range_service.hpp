#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_RANGE_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_RANGE_SERVICE_HPP

#include "service_range.hpp"

namespace kgr {
namespace detail {

struct override_range_service_tag {};

} // namespace detail

template<typename T>
struct override_range_service : detail::override_range_service_tag {
	explicit override_range_service(override_range<detail::override_iterator<T>> range) noexcept : _range{range} {}
	
	auto forward() -> override_range<detail::override_iterator<T>> {
		return _range;
	}
	
private:
	override_range<detail::override_iterator<T>> _range;
};

namespace detail {

template<typename T>
using is_override_range_service = std::is_base_of<override_range_service_tag, T>;

template<typename>
struct override_range_service_type {};

template<typename T>
struct override_range_service_type<override_range_service<T>> {
	using type = T;
};

template<typename T>
using override_range_service_type_t = typename is_override_range_service<T>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_RANGE_SERVICE_HPP
