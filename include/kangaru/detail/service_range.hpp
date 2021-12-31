#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP

#include "service_storage.hpp"
#include "lazy_storage.hpp"
#include "override_storage_service.hpp"

#include "../type_id.hpp"

#include <iterator>
#include <vector>

namespace kgr {

template<typename Iterator>
struct override_range {
	using iterator = Iterator;
	using service = typename Iterator::service;
	using service_type = kgr::service_type<service>;
	
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
	using storage = lazy_storage<service_type<T>>;
	
	static_assert(
		is_trivially_copy_constructible<storage>::value &&
		std::is_trivially_destructible<storage>::value,
		"override_iterator only support services that yield trivial types"
	);
	
public:
	using value_type = typename storage::value_type;
	using reference = typename storage::reference;
	using const_reference = typename storage::const_reference;
	using pointer = typename storage::pointer;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::input_iterator_tag;
	
	explicit override_iterator(std::vector<std::pair<type_id_t, service_storage>>::iterator internal) noexcept :
		_internal{internal} {}
	
	friend auto operator!=(override_iterator const& lhs, override_iterator const& rhs) -> bool {
		return lhs._internal != rhs._internal;
	}
	
	friend auto operator==(override_iterator const& lhs, override_iterator const& rhs) -> bool {
		return lhs._internal == rhs._internal;
	}
	
	auto operator++() -> override_iterator& {
		++_internal;
		_service = {};
		return *this;
	}
	
	auto operator++(int) -> override_storage {
		auto prev = *this;
		++*this;
		_service = {};
		return prev;
	}
	
	auto operator*() -> typename storage::reference {
		return get();
	}
	
	auto operator->() -> typename storage::pointer {
		return &get();
	}
	
private:
	auto get() -> typename storage::reference {
		if (!_service) {
			auto const& typed_storage = _internal->second.cast<T>();
			_service.construct(typed_storage.forward(typed_storage.service));
		}
		
		return _service.value();
	}
	
	using service = T;
	friend struct override_range<override_iterator<T>>;
	std::vector<std::pair<type_id_t, service_storage>>::iterator _internal;
	storage _service;
};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_RANGE_HPP
