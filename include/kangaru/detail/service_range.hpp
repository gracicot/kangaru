#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP

#include "service_storage.hpp"
#include <vector>

namespace kgr {

template<typename Iterator>
struct override_range {
	using iterator = Iterator;
	using service = typename Iterator::service;
	using service_type = service_type<service>;
	
	explicit override_range(iterator begin, iterator end) noexcept :
		_begin{begin}, _end{end} {}
	
	auto begin() const noexcept -> iterator {
		return _begin;
	}
	
	auto end() const noexcept -> iterator {
		return _end;
	}
	
private:
	iterator _begin;
	iterator _end;
};

namespace detail {

template<typename T>
struct override_iterator {
	explicit override_iterator(std::vector<service_storage>::iterator internal) noexcept :
		_internal{internal} {}
	
	friend
	auto operator!=(override_iterator const& lhs, override_iterator const& rhs) -> bool {
		return lhs._internal != rhs._internal;
	}
	
	auto operator++() -> override_iterator& {
		++_internal;
		return *this;
	}
	
	auto operator++(int) -> override_storage {
		auto prev = *this;
		++*this;
		return prev;
	}
	
	auto operator*() const -> service_type<T> {
		auto const& typed_storage = _internal->cast<T>();
		return typed_storage.forward(typed_storage.service);
	}
	
private:
	using service = T;
	friend struct override_range<override_iterator<T>>;
	std::vector<service_storage>::iterator _internal;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP
