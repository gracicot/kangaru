#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP

#include "service_storage.hpp"
#include <vector>

namespace kgr {
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
	std::vector<service_storage>::iterator _internal;
};

template<typename T>
struct override_range {
	using iterator = override_iterator<T>;
	
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




} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP
