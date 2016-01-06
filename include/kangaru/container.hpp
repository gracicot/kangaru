#pragma once

#include "detail/function_traits.hpp"
#include "detail/utils.hpp"
#include "detail/container_service.hpp"

#include <unordered_map>
#include <memory>
#include <type_traits>
#include <tuple>
#include <algorithm>
#include <vector>

namespace kgr {

struct Container {
private:
	template<typename Condition, typename T = detail::enabler> using enable_if = detail::enable_if_t<Condition::value, T>;
	template<typename Condition, typename T = detail::enabler> using disable_if = detail::enable_if_t<!Condition::value, T>;
	template<typename T> using decay = typename std::decay<T>::type;
	template<typename T> using is_single = std::is_base_of<Single, decay<T>>;
	template<typename T> using is_container_service = std::is_same<T, ContainerService>;
	template<typename T> using is_base_of_container_service = std::is_base_of<detail::ContainerServiceBase, T>;
	template<typename T> using parent_types = typename decay<T>::ParentTypes;
	template<int S, typename T> using parent_element = typename std::tuple_element<S, parent_types<T>>::type;
	template<typename Tuple> using tuple_seq = typename detail::seq_gen<std::tuple_size<Tuple>::value>::type;
	template<typename Tuple, int n> using tuple_seq_minus = typename detail::seq_gen<std::tuple_size<Tuple>::value - n>::type;
	template<typename T> using instance_ptr = std::unique_ptr<T, void(*)(void*)>;
	
	constexpr static detail::enabler null = detail::null;
	
	template<typename T>
	static void deleter(void* i) {
		delete static_cast<T*>(i);
	}
	
public:
	Container() = default;
	Container(const Container &) = delete;
	Container& operator =(const Container &) = delete;
	virtual ~Container() = default;
	
	Container(Container&& other) : _instances{std::move(other._instances)}, _services{std::move(other._services)} {}
	
	Container& operator =(Container&& other) {
		std::swap(other._instances, _instances);
		std::swap(other._services, _services);
		
		return *this;
	}
	
	template<typename T>
	void instance(T&& service) {
		static_assert(is_single<decay<T>>::value, "instance() only accept Single Service instance.");
		
		save_instance(std::forward<T>(service));
	}
	
	template<typename T, typename... Args>
	void instance(Args&& ...args) {
		static_assert(is_single<T>::value, "instance() only accept Single Service instance.");
		save_instance(make_service_instance<T>(std::forward<Args>(args)...));
	}
	
	template<typename T, typename... Args>
	ServiceType<T> service(Args&& ...args) {
		return get_service<T>(std::forward<Args>(args)...).forward();
	}
	
	template<template<typename> class Map, typename U, typename ...Args>
	detail::function_result_t<decay<U>> invoke(U&& function, Args&&... args) {
		return invoke_helper<Map>(tuple_seq_minus<detail::function_arguments_t<decay<U>>, sizeof...(Args)>{}, std::forward<U>(function), std::forward<Args>(args)...);
	}
	
	inline void clear() {
		_instances.clear();
		_services.clear();
	}
	
	template<typename T, enable_if<detail::has_overrides<decay<T>>> = null>
	void reset() {
		static_assert(is_single<T>::value, "reset() only accept Single Service instance.");
		reset_overrides<T>(tuple_seq<parent_types<T>>{});
	}
	
	template<typename T, disable_if<detail::has_overrides<decay<T>>> = null>
	void reset() {
		static_assert(is_single<T>::value, "reset() only accept Single Service instance.");
		reset_helper<T>();
	}
	
private:
	template<typename T, int... S>
	void reset_overrides(detail::seq<S...>) {
		reset_overrides_helper<T, parent_element<S, T>...>();
	}
	
	template<typename T, typename Override, typename... Others>
	void reset_overrides_helper() {
		reset_helper<Override, detail::ServiceOverride<T, Override>>();
		reset_overrides_helper<T, Others...>();
	}
	
	template<typename T>
	void reset_overrides_helper() {}
	
	template<typename Type, typename TrueType = Type>
	void reset_helper() {
		auto&& list = _services[detail::type_id<Type>];
		list.erase(
			std::remove_if(list.begin(), list.end(), [this](void*& subject) {
				if(typeid(*static_cast<Type*>(subject)) == typeid(TrueType)) {
					_instances.erase(
						std::remove_if(_instances.begin(), _instances.end(), [subject](instance_ptr<void>& subject2){
							return subject == subject2.get();
						}), _instances.end()
					);
					return true;
				}
				return false;
			}), list.end()
		);
	}
	
	template<typename U, typename ...Args>
	detail::function_result_t<decay<U>> invoke(U&& function, Args&&... args) {
		return invoke_helper(tuple_seq_minus<detail::function_arguments_t<decay<U>>, sizeof...(Args)>{}, std::forward<U>(function), std::forward<Args>(args)...);
	}
	
	// save instance functions
	template<typename T, enable_if<detail::has_overrides<decay<T>>> = null>
	void save_instance(T&& service) {
		save_instance(std::forward<T>(service), tuple_seq<parent_types<T>>{});
	}
	
	template<typename T, disable_if<detail::has_overrides<decay<T>>> = null>
	void save_instance(T&& service) {
		using U = decay<T>;
		save_instance_helper<U>(instance_ptr<U>{new U{std::move(service)}, &Container::deleter<U>});
	}
	
	template<typename T, int... S>
	void save_instance(T&& service, detail::seq<S...>) {
		using U = decay<T>;
		save_instance_helper<U, parent_element<S, U>...>(instance_ptr<U>{new U{std::move(service)}, &Container::deleter<U>});
	}
	
	template<typename T>
	void save_instance_helper(instance_ptr<T> service) {
		_services[detail::type_id<T>].emplace_back(service.get());
		_instances.emplace_back(std::move(service));
	}
	
	template<typename T, typename Override, typename... Others>
	void save_instance_helper(instance_ptr<T> service) {
		using ServiceOverride = detail::ServiceOverride<T, Override>;

		instance_ptr<Override> baseService{new ServiceOverride{*service}, &Container::deleter<ServiceOverride>};
		_services[detail::type_id<Override>].emplace_back(baseService.get());
		_instances.emplace_back(std::move(baseService));
		save_instance_helper<T, Others...>(std::move(service));
	}
	
	// get service functions
	template<typename T, typename... Args, disable_if<is_single<T>> = null, disable_if<is_base_of_container_service<T>> = null>
	T get_service(Args&&... args) {
		auto service = make_service_instance<T>(std::forward<Args>(args)...);
		invoke_service(service);
		return service;
	}
	
	template<typename T, enable_if<is_container_service<T>> = null>
	T get_service() {
		return T{*this};
	}
	
	template<typename T, enable_if<is_base_of_container_service<T>> = null, disable_if<is_container_service<T>> = null>
	T get_service() {
		return T{*dynamic_cast<typename T::Type*>(this)};
	}
	
	template<typename T, enable_if<std::is_abstract<T>> = null, enable_if<is_single<T>> = null, disable_if<is_base_of_container_service<T>> = null>
	T& get_service() {
		auto&& list = _services[detail::type_id<T>];
		if (!list.size()) {
			throw std::out_of_range{"No instance found for the requested abstract service"};
		}
		return *static_cast<T*>(list.back());
	}
	
	template<typename T, enable_if<is_single<T>> = null, disable_if<is_base_of_container_service<T>> = null, disable_if<std::is_abstract<T>> = null>
	T& get_service() {
		auto&& list = _services[detail::type_id<T>];
		
		if (!list.size()) {
			save_instance(make_service_instance<T>());
			
			auto& service = *static_cast<T*>(list.back());
			invoke_service(service);
			return service;
		} else {
			return *static_cast<T*>(list.back());
		}
	}
	
	// make instance
	template<typename T, typename... Args>
	T make_service_instance(Args&&... args) {
		return invoke(&T::construct, std::forward<Args>(args)...);
	}
	
	// invoke
	template<typename U, typename ...Args, int... S>
	detail::function_result_t<decay<U>> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return function(get_service<decay<detail::function_argument_t<S, decay<U>>>>()..., std::forward<Args>(args)...);
	}
	
	template<template<typename> class Map, typename U, typename ...Args, int... S>
	detail::function_result_t<decay<U>> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return function(service<typename Map<detail::function_argument_t<S, decay<U>>>::Service>()..., std::forward<Args>(args)...);
	}
	
	template<typename T, int... S, enable_if<detail::has_invoke<decay<T>>> = null>
	void invoke_service(T&& service) {
		using U = decay<T>;
		invoke_service_helper<typename U::invoke::Next>(std::forward<T>(service), U::invoke::method);
	}
	
	template<typename Method, typename T, typename F, enable_if<detail::has_next<Method>> = null>
	void invoke_service_helper(T&& service, F&& function) {
		do_invoke_service(tuple_seq<detail::function_arguments_t<decay<F>>>{}, std::forward<T>(service), std::forward<F>(function));
		invoke_service_helper<typename Method::Next>(std::forward<T>(service), Method::method);
	}
	
	template<typename Method, typename T, typename F, disable_if<detail::has_next<Method>> = null>
	void invoke_service_helper(T&& service, F&& function) {
		do_invoke_service(tuple_seq<detail::function_arguments_t<decay<F>>>{}, std::forward<T>(service), std::forward<F>(function));
	}
	
	template<typename T, typename F, int... S>
	void do_invoke_service(detail::seq<S...>, T&& service, F&& function) {
		invoke([&service, &function](detail::function_argument_t<S, decay<F>>... args){
			(service.*function)(std::forward<detail::function_argument_t<S, decay<F>>>(args)...);
		});
	}
	
	template<typename T, disable_if<detail::has_invoke<decay<T>>> = null>
	void invoke_service(T&&) {}
	
	std::vector<instance_ptr<void>> _instances;
	std::unordered_map<detail::type_id_t, std::vector<void*>> _services;
};

}  // namespace kgr
