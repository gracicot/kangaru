#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFAULT_SOURCE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFAULT_SOURCE_HPP

#include "kangaru/detail/config.hpp"

#include "../type_id.hpp"
#include "traits.hpp"
#include "injected.hpp"
#include "service_storage.hpp"
#include "override_storage_service.hpp"
#include "service_range.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iterator>

#include "define.hpp"

namespace kgr {
namespace detail {

/*
 * Primary storage for services in the container.
 *
 * Also manages override meta information.
 */
struct default_source {
private:
	using alias_t = void*;
	
	template<typename T> using instance_ptr = std::unique_ptr<T, void(*)(alias_t) KGR_KANGARU_CXX17_NOEXCEPT>;
	
	using instance_cont = std::vector<instance_ptr<void>>;
	using service_cont = std::unordered_map<type_id_t, detail::service_storage>;
	
	template<typename T>
	static void deleter(alias_t i) KGR_KANGARU_CXX17_NOEXCEPT {
		delete static_cast<T*>(i);
	}
	
	template<typename T, typename... Args>
	static instance_ptr<memory_block<T>> make_instance_ptr(Args&&... args) {
		return instance_ptr<memory_block<T>>{
			new memory_block<T>{std::forward<Args>(args)...},
			&default_source::deleter<memory_block<T>>
		};
	}
	
	template<typename T>
	static inline auto evaluate_predicate(type_id_t id, T&& predicate) noexcept -> bool {
		auto const kind = type_id_kind(id);
		return kind != service_kind_t::override_storage && (
			kind == service_kind_t::index_storage || predicate(id)
		);
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
		
		auto inserted = emplace_or_assign<Parent>(overriden, get_override_forward<Override, Parent>());
		
		auto& overrides = overrides_of<Parent>(get_override_storage());
		overrides.emplace_back(type_id<Override>(), inserted);
		
		return inserted;
	}
	
	template<typename Override, typename Parent>
	auto get_override_forward() noexcept -> detail::forward_ptr<Parent> {
		static_assert(detail::is_polymorphic<Parent>::value,
			"The overriden service must be virtual"
		);
		
		return [](alias_t s) -> service_type<Parent> {
			return static_cast<service_type<Parent>>(static_cast<Override*>(s)->forward());
		};
	}
	
	template<typename T>
	auto get_forward() noexcept -> detail::forward_ptr<T> {
		return detail::is_polymorphic<T>::value ? detail::forward_ptr<T>{
			[](alias_t service) -> service_type<T> {
				return static_cast<T*>(service)->forward();
			}
		} : nullptr;
	}
	
	inline auto get_override_storage() -> override_storage& {
		auto it = _services.find(type_id<override_storage_service>());
		
		if (it != _services.end()) {
			return it->second.service<override_storage_service>().forward();
		} else {
			_instances.emplace_back(make_instance_ptr<override_storage_service>());
			auto storage = emplace_or_assign<override_storage_service>(_instances.back().get(), nullptr);
			return static_cast<override_storage_service*>(storage.service)->forward();
		}
	}
	
	template<typename T>
	auto overrides_of(override_storage& override_storage) -> std::vector<std::pair<type_id_t, service_storage>>& {
		auto it = _services.find(type_id<index_storage<T>>());
		
		if (it != _services.end()) {
			return override_storage.overrides[it->second.index()];
		} else {
			auto next_index = override_storage.overrides.size();
			override_storage.overrides.emplace_back();
			_services.emplace(type_id<index_storage<T>>(), service_storage{override_index, next_index});
			return override_storage.overrides.back();
		}
	}
	
	template<typename T, enable_if_t<detail::is_polymorphic<T>::value, int> = 0>
	auto insert_self(alias_t service) -> detail::typed_service_storage<T> {
		auto inserted = emplace_or_assign<T>(service, get_forward<T>());
		
		auto& overrides = overrides_of<T>(get_override_storage());
		overrides.emplace_back(type_id<T>(), inserted);
		
		return inserted;
	}
	
	template<typename T, enable_if_t<!detail::is_polymorphic<T>::value, int> = 0>
	auto insert_self(alias_t service) -> detail::typed_service_storage<T> {
		return emplace_or_assign<T>(service, get_forward<T>());
	}
	
public:
	explicit default_source() = default;
	default_source(default_source const&) = delete;
	default_source& operator=(default_source const&) = delete;
	default_source(default_source&&) = default;
	default_source& operator=(default_source&&) = default;
	
#ifdef KGR_KANGARU_REVERSE_DESTRUCTION
	inline ~default_source() {
		for (auto it = _instances.rbegin() ; it != _instances.rend() ; ++it) {
			it->reset();
		}
	}
#else
	~default_source() = default;
#endif
	
	/*
	 * Adds a service in the service source
	 */
	template<typename T, typename... Parents, typename... Args>
	auto emplace(Args&&... args) -> single_insertion_result_t<T> {
		auto instance_ptr = make_instance_ptr<T>(std::forward<Args>(args)...);
		auto ptr = &instance_ptr->service;
		
		_instances.emplace_back(std::move(instance_ptr));
		
		return single_insertion_result_t<T>{insert_self<T>(ptr), insert_override<T, Parents>(ptr)...};
	}
	
	/*
	 * This function clears this container.
	 * Every single services are invalidated after calling this function.
	 */
	inline void clear() noexcept {
#ifdef KGR_KANGARU_REVERSE_DESTRUCTION
		for (auto it = _instances.rbegin() ; it != _instances.rend() ; ++it) {
			it->reset();
		}
#endif
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
		default_source fork;
		
		fork._services.reserve(_services.size());
		
		std::copy_if(
			_services.begin(), _services.end(),
			std::inserter(fork._services, fork._services.begin()),
			[&predicate](service_cont::const_reference i) {
				return evaluate_predicate(i.first, predicate);
			}
		);
		
		auto it = _services.find(type_id<override_storage_service>());
		
		if (it != _services.end()) {
			auto& this_overrides = it->second.service<override_storage_service>();
			auto fork_overrides = std::get<0>(fork.emplace<override_storage_service>());
			static_cast<override_storage*>(fork_overrides.service)->overrides = this_overrides.forward().filter(predicate);
		}
		
		return fork;
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
				return evaluate_predicate(i.first, predicate);
			}
		);
	}
	
	/**
	 * This function finds a service in the source.
	 * When the service is found, it returns the return value of the `found` function.
	 * Otherwise it calls `fails` with no parameter.
	 */
	template<typename T, typename F1, typename F2, typename R1 = call_result_t<F1, detail::injected_wrapper<T>>, typename R2 = call_result_t<F2>>
	auto find(F1 found, F2 fails) noexcept(noexcept(fails()) && noexcept(found(std::declval<detail::injected_wrapper<T>>()))) -> enable_if_t<std::is_same<R1, R2>::value, R1> {
		auto it = _services.find(type_id<T>());
		
		if (it != _services.end()) {
			return found(detail::injected_wrapper<T>{it->second});
		} else {
			return fails();
		}
	}
	
	template<typename T>
	auto overrides() -> override_range<override_iterator<T>> {
		auto& overrides = overrides_of<T>(get_override_storage());
		return override_range<override_iterator<T>>{
			override_iterator<T>{overrides.begin()},
			override_iterator<T>{overrides.end()}
		};
	}
	
	/*
	 * This function return true if the container contains the service T. Returns false otherwise.
	 * T nust be a single service.
	 */
	template<typename T>
	bool contains() const noexcept {
		return _services.find(type_id<T>()) != _services.end();
	}
	
private:
	instance_cont _instances;
	service_cont _services;
};

} // namespace detail
} // namespace kgr

#include "undef.hpp"

#endif
