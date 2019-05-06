#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFAULT_SOURCE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFAULT_SOURCE_HPP

#include "kangaru/detail/config.hpp"

#include "../type_id.hpp"
#include "traits.hpp"
#include "injected.hpp"

#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iterator>

namespace kgr {
namespace detail {

struct default_source {
private:
	using alias_t = void*;
	
	template<typename T> using instance_ptr = std::unique_ptr<T, void(*)(alias_t) noexcept>;
	using instance_cont = std::vector<instance_ptr<void>>;
	using service_cont = std::unordered_map<type_id_t, detail::service_storage>;
	
	template<typename T>
	static void deleter(alias_t i) noexcept {
		delete static_cast<T*>(i);
	}
	
	template<typename T, typename... Args, enable_if_t<std::is_constructible<T, Args...>::value, int> = 0>
	static instance_ptr<memory_block<T>> make_instance_ptr(Args&&... args) {
		static_assert(
			std::is_standard_layout<memory_block<T>>::value,
			"The service memory block must be standard layout"
		);
		return instance_ptr<memory_block<T>>{
			new memory_block<T>(std::forward<Args>(args)...),
			&default_source::deleter<memory_block<T>>
		};
	}
	
	template<typename T>
	auto emplace_or_assign(alias_t service, detail::forward_ptr<T> forward) -> detail::typed_service_storage<T> {
		auto const it = _services.find(type_id<T>());
		
		if (it == _services.end()) {
			auto inserted = _services.emplace(type_id<T>(), detail::typed_service_storage<T>{service, forward});
			return inserted.first->second.template cast<T>();
		} else {
			it->second = detail::typed_service_storage<T>{service, forward};
			return it->second.template cast<T>();
		}
	}
	
	template<typename Override, typename Parent>
	auto insert_override(alias_t overriden) -> detail::typed_service_storage<Parent> {
		static_assert(detail::is_polymorphic<Parent>::value,
			"The overriden service must be virtual"
		);
		
		static_assert(!detail::is_final_service<Parent>::value,
			"A final service cannot be overriden"
		);
		
		return emplace_or_assign<Parent>(overriden, get_override_forward<Override, Parent>());
	}
	
	template<typename Override, typename Parent, enable_if_t<detail::is_polymorphic<Parent>::value, int> = 0>
	auto get_override_forward() -> detail::forward_ptr<Parent> {
		return [](alias_t s) -> service_type<Parent> {
			return static_cast<service_type<Parent>>(static_cast<Override*>(s)->forward());
		};
	}
	
	template<typename T, enable_if_t<detail::is_polymorphic<T>::value, int> = 0>
	auto get_forward() -> detail::forward_ptr<T> {
		return [](alias_t service) -> service_type<T> {
			return static_cast<T*>(service)->forward();
		};
	}
	
	template<typename T, enable_if_t<!detail::is_polymorphic<T>::value, int> = 0>
	auto get_forward() -> detail::forward_ptr<T> {
		return nullptr;
	}
	
	template<typename T, enable_if_t<is_polymorphic<T>::value, int> = 0>
	static auto make_wrapper(service_storage& instance) -> detail::injected_wrapper<T> {
		return detail::injected_wrapper<T>{instance.cast<T>()};
	}
	
	template<typename T, enable_if_t<!is_polymorphic<T>::value, int> = 0>
	static auto make_wrapper(service_storage& instance) -> detail::injected_wrapper<T> {
		return detail::injected_wrapper<T>{instance.service<T>()};
	}
	
public:
	explicit default_source() = default;
	default_source(default_source const&) = delete;
	default_source& operator=(default_source const&) = delete;
	default_source(default_source&&) = default;
	default_source& operator=(default_source&&) = default;
	
#ifdef KGR_KANGARU_REVERSE_DESTRUCTION
	~default_source() {
		for (auto it = _instances.rbegin() ; it != _instances.rend() ; ++it) {
			it->reset();
		}
	}
#else
	~default_source() = default;
#endif
	
	template<typename T, typename... Parents, typename... Args>
	auto emplace(Args&&... args) -> single_insertion_result_t<T> {
		auto instance_ptr = make_instance_ptr<T>(std::forward<Args>(args)...);
		auto ptr = &instance_ptr->cast();
		
		_instances.emplace_back(std::move(instance_ptr));
		
		return single_insertion_result_t<T>{emplace_or_assign<T>(ptr, get_forward<T>()), insert_override<T, Parents>(ptr)...};
	}
	
	/*
	 * This function clears this container.
	 * Every single services are invalidated after calling this function.
	 */
	inline void clear() {
		_instances.clear();
		_services.clear();
	}
	
	/*
	 * This function fork the container into a new container.
	 * The new container will have the copied state of the first container.
	 * Construction of new services within the new container will not affect the original one.
	 * The new container must exist within the lifetime of the original container.
	 * 
	 * It takes a predicate as argument.
	 */
	template<typename Predicate>
	auto fork(Predicate predicate) const -> default_source {
		default_source s;
		
		s._services.reserve(_services.size());
		
		std::copy_if(
			_services.begin(), _services.end(),
			std::inserter(s._services, s._services.begin()),
			[&predicate](service_cont::const_reference i) {
				return predicate(i.first);
			}
		);
		
		return s;
	}
	
	/*
	 * This function merges a container with another.
	 * The receiving container will prefer it's own instances in a case of conflicts.
	 */
	inline void merge(default_source&& other) {
		_services.insert(other._services.begin(), other._services.end());
		_instances.reserve(_instances.size() + other._instances.size());
		_instances.insert(
			_instances.end(),
			std::make_move_iterator(other._instances.begin()),
			std::make_move_iterator(other._instances.end())
		);
	}
	
	/**
	 * This function will add all services form the container sent as parameter into this one.
	 * Note that the lifetime of the container sent as parameter must be at least as long as this one.
	 * If the container you rebase from won't live long enough, consider using the merge function.
	 * 
	 * It takes a predicate type as argument to filter.
	 */
	template<typename Predicate>
	void rebase(const default_source& other, Predicate predicate) {
		std::copy_if(
			other._services.begin(), other._services.end(),
			std::inserter(_services, _services.end()),
			[&predicate](service_cont::const_reference i) {
				return predicate(i.first);
			}
		);
	}
	
	/**
	 * This function finds a service in the source.
	 * When the service is found, it returns the return value of the `found` function.
	 * Otherwise it calls `fails` with no parameter.
	 */
	template<typename T, typename F1, typename F2, typename R1 = call_result_t<F1, detail::injected_wrapper<T>>, typename R2 = call_result_t<F2>>
	auto find(F1 found, F2 fails) -> enable_if_t<std::is_same<R1, R2>::value, R1> {
		auto it = _services.find(type_id<T>());
		
		if (it != _services.end()) {
			return found(make_wrapper<T>(it->second));
		} else {
			return fails();
		}
	}
	
	/*
	 * This function return true if the container contains the service T. Returns false otherwise.
	 * T nust be a single service.
	 */
	template<typename T>
	bool contains() const {
		return _services.find(type_id<T>()) != _services.end();
	}
	
private:
	instance_cont _instances;
	service_cont _services;
};

} // namespace detail
} // namespace kgr

#endif
