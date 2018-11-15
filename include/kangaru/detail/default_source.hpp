#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFAULT_SOURCE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFAULT_SOURCE_HPP

#include "kangaru/detail/config.hpp"

#include "../type_id.hpp"
#include "traits.hpp"

#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iterator>

namespace kgr {
namespace detail {

struct default_source {
	// TODO: Make them private
	using storage_t = detail::service_storage;
	using alias_t = void*;
	
private:
	template<typename T> using instance_ptr = std::unique_ptr<T, void(*)(alias_t)>;
	using instance_cont = std::vector<instance_ptr<void>>;
	using service_cont = std::unordered_map<type_id_t, detail::service_storage>;
	
	template<typename T>
	static void deleter(alias_t i) {
		delete static_cast<T*>(i);
	}
	
	template<typename T, typename... Args, enable_if_t<std::is_constructible<T, Args...>::value, int> = 0>
	static instance_ptr<T> make_instance_ptr(Args&&... args) {
		return instance_ptr<T>{
			new T(std::forward<Args>(args)...),
			&default_source::deleter<T>
		};
	}
	
	template<typename T, typename... Args, enable_if_t<detail::is_only_brace_constructible<T, Args...>::value, int> = 0>
	static instance_ptr<T> make_instance_ptr(Args&&... args) {
		return instance_ptr<T>{
			new T{std::forward<Args>(args)...},
			&default_source::deleter<T>
		};
	}
	
	template<typename T>
	auto emplace_or_assign(alias_t service, detail::forward_ptr<T> forward) -> detail::service_storage& {
		auto it = _services.find(type_id<T>());
		
		if (it == _services.end()) {
			auto inserted = _services.emplace(type_id<T>(), detail::typed_service_storage<T>{service, forward});
			return inserted.first->second;
		} else {
			it->second = detail::typed_service_storage<T>{service, forward};
			return it->second;
		}
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
	
	template<typename T>
	auto storage() -> storage_t& {
		return _services[type_id<T>()];
	}
	
	template<typename T, typename... Args>
	auto emplace(detail::forward_ptr<T> forward, Args&&... args) -> std::pair<alias_t, storage_t&> {
		auto instance_ptr = make_instance_ptr<T>(std::forward<Args>(args)...);
		auto ptr = instance_ptr.get();
		
		_instances.emplace_back(std::move(instance_ptr));
		
		return std::pair<alias_t, storage_t&>{ptr, emplace_or_assign<T>(ptr, forward)};
	}
	
	template<typename T>
	auto override(detail::forward_ptr<detail::identity_t<T>> forward, alias_t overriden) -> storage_t& {
		return emplace_or_assign<T>(overriden, forward);
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
	template<typename T, typename F1, typename F2, typename R1 = call_result_t<F1, storage_t>, typename R2 = call_result_t<F2>>
	auto find(F1 found, F2 fails) -> enable_if_t<std::is_same<R1, R2>::value, R1> {
		auto it = _services.find(type_id<T>());
		
		if (it != _services.end()) {
			return found(it->second);
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
