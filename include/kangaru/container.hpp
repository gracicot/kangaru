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
enum test { brsb};
struct Container {
private:
	template<typename Condition, typename T = int> using enable_if = detail::enable_if_t<Condition::value, T>;
	template<typename Condition, typename T = int> using disable_if = detail::enable_if_t<!Condition::value, T>;
	template<typename T> using decay = typename std::decay<T>::type;
	template<typename T> using is_single = std::is_base_of<Single, decay<T>>;
	template<typename T> using is_container_service = std::is_same<T, ContainerService>;
	template<typename T> using is_base_of_container_service = std::is_base_of<detail::ContainerServiceBase, T>;
	template<typename T> using parent_types = typename decay<T>::ParentTypes;
	template<int S, typename T> using parent_element = typename std::tuple_element<S, parent_types<T>>::type;
	template<typename Tuple, int n> using tuple_seq_minus = typename detail::seq_gen<std::tuple_size<Tuple>::value - n>::type;
	template<typename T> using instance_ptr = std::unique_ptr<T, void(*)(void*)>;
	template<template<typename> class Map, typename T> using service_map_t = typename Map<T>::Service;
	
	template<typename T>
	static void deleter(void* i) {
		delete static_cast<T*>(i);
	}
	
	template<typename T, typename... Args, enable_if<std::is_constructible<T, Args...>> = 0>
	static instance_ptr<T> makeInstancePtr(Args&&... args) {
		return instance_ptr<T>{new T(std::forward<Args>(args)...), &Container::deleter<T>};
	}

	template<typename T, typename... Args, disable_if<std::is_constructible<T, Args...>> = 0>
	static instance_ptr<T> makeInstancePtr(Args&&... args) {
		return instance_ptr<T>{new T{std::forward<Args>(args)...}, &Container::deleter<T>};
	}
	
public:
	Container() = default;
	Container(const Container &) = delete;
	Container& operator =(const Container &) = delete;
	Container(Container&& other) = default;
	Container& operator=(Container&& other) = default;
	virtual ~Container() = default;
	
	template<typename T, enable_if<is_single<decay<T>>> = 0>
	void instance(T&& service) {
		save_instance(std::forward<T>(service));
	}
	
	template<typename T, typename... Args, enable_if<is_single<T>> = 0>
	void instance(Args&& ...args) {
		save_new_instance<T>(std::forward<Args>(args)...);
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
		_services.clear();
	}
	
private:
	template<typename U, typename ...Args>
	detail::function_result_t<decay<U>> invoke(U&& function, Args&&... args) {
		return invoke_helper(tuple_seq_minus<detail::function_arguments_t<decay<U>>, sizeof...(Args)>{}, std::forward<U>(function), std::forward<Args>(args)...);
	}
	
	template<typename T, typename... Args, enable_if<is_single<T>> = 0, disable_if<std::is_abstract<T>> = 0>
	void save_new_instance(Args&&... args) {
		save_instance(make_service_instance<T>(std::forward<Args>(args)...));
	}
	
	template<typename T, typename... Args, enable_if<is_single<T>> = 0, enable_if<std::is_abstract<T>> = 0>
	void save_new_instance(Args&&... args) {
		throw std::out_of_range{"No instance found for the requested abstract service"};
	}
	
	// save instance functions
	template<typename T, enable_if<detail::has_overrides<decay<T>>> = 0>
	void save_instance(T&& service) {
		save_instance(std::forward<T>(service), detail::tuple_seq<parent_types<T>>{});
	}
	
	template<typename T, disable_if<detail::has_overrides<decay<T>>> = 0>
	void save_instance(T&& service) {
		using U = decay<T>;
		save_instance_helper<U>(makeInstancePtr<U>(std::move(service)));
	}
	
	template<typename T, int... S>
	void save_instance(T&& service, detail::seq<S...>) {
		using U = decay<T>;
		save_instance_helper<U, parent_element<S, U>...>(makeInstancePtr<U>(std::move(service)));
	}
	
	template<typename T>
	void save_instance_helper(instance_ptr<T> service) {
		_services[detail::type_id<T>].emplace_back(std::move(service));
	}
	
	template<typename T, typename Override, typename... Others>
	void save_instance_helper(instance_ptr<T> service) {
		using ServiceOverride = detail::ServiceOverride<T, Override>;

		_services[detail::type_id<Override>].emplace_back(static_cast<instance_ptr<Override>>(makeInstancePtr<ServiceOverride>(*service)));
		save_instance_helper<T, Others...>(std::move(service));
	}
	
	// get service functions
	template<typename T, typename... Args, disable_if<is_single<T>> = 0, disable_if<is_base_of_container_service<T>> = 0>
	T get_service(Args&&... args) {
		return make_service_instance<T>(std::forward<Args>(args)...);
	}
	
	template<typename T, enable_if<is_container_service<T>> = 0>
	T get_service() {
		return T{*this};
	}
	
	template<typename T, enable_if<is_base_of_container_service<T>> = 0, disable_if<is_container_service<T>> = 0>
	T get_service() {
		return T{*dynamic_cast<typename T::Type*>(this)};
	}
	
	template<typename T, enable_if<is_single<T>> = 0, disable_if<is_base_of_container_service<T>> = 0>
	T& get_service() {
		auto&& list = _services[detail::type_id<T>];
		
		if (!list.size()) {
			save_new_instance<T>();
		}
		
		return *static_cast<T*>(list.back().get());
	}
	
	// make instance
	template<typename T, typename... Args>
	T make_service_instance(Args&&... args) {
		T service = invoke(&T::construct, std::forward<Args>(args)...);
		invoke_service(service);
		return service;
	}
	
	// invoke
	template<typename U, typename ...Args, int... S>
	detail::function_result_t<decay<U>> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return std::forward<U>(function)(get_service<decay<detail::function_argument_t<S, decay<U>>>>()..., std::forward<Args>(args)...);
	}
	
	template<template<typename> class Map, typename U, typename ...Args, int... S>
	detail::function_result_t<decay<U>> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return std::forward<U>(function)(service<service_map_t<Map, detail::function_argument_t<S, decay<U>>>>()..., std::forward<Args>(args)...);
	}
	
	template<typename T, int... S, enable_if<detail::has_invoke<decay<T>>> = 0>
	void invoke_service(T&& service) {
		using U = decay<T>;
		invoke_service_helper<typename U::invoke::Next>(std::forward<T>(service), U::invoke::method);
	}
	
	template<typename Method, typename T, typename F, enable_if<detail::has_next<Method>> = 0>
	void invoke_service_helper(T&& service, F&& function) {
		do_invoke_service(detail::tuple_seq<detail::function_arguments_t<decay<F>>>{}, std::forward<T>(service), std::forward<F>(function));
		invoke_service_helper<typename Method::Next>(std::forward<T>(service), Method::method);
	}
	
	template<typename Method, typename T, typename F, disable_if<detail::has_next<Method>> = 0>
	void invoke_service_helper(T&& service, F&& function) {
		do_invoke_service(detail::tuple_seq<detail::function_arguments_t<decay<F>>>{}, std::forward<T>(service), std::forward<F>(function));
	}
	
	template<typename T, typename F, int... S>
	void do_invoke_service(detail::seq<S...>, T&& service, F&& function) {
		invoke([&service, &function](detail::function_argument_t<S, decay<F>>... args){
			(std::forward<T>(service).*std::forward<F>(function))(std::forward<detail::function_argument_t<S, decay<F>>>(args)...);
		});
	}
	
	template<typename T, disable_if<detail::has_invoke<decay<T>>> = 0>
	void invoke_service(T&&) {}
	
	std::unordered_map<detail::type_id_t, std::vector<instance_ptr<void>>> _services;
};

}  // namespace kgr
