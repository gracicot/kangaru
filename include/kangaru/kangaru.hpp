#pragma once

#include "detail/function_traits.hpp"
#include "detail/utils.hpp"
#include "service.hpp"

#include <unordered_map>
#include <memory>
#include <type_traits>
#include <tuple>
#include <vector>
#include <exception>

namespace kgr {

struct Container;

struct ContainerService {
	using ServiceType = Container&;
	ContainerService(Container& instance) : _instance{instance} {}
	
	static ContainerService construct(Container& container) {
		return {container};
	}
	
	ServiceType forward() {
		return _instance;
	}
	
private:
	Container& _instance;
};

struct Container {
private:
	template<typename Condition, typename T = detail::enabler> using enable_if = detail::enable_if_t<Condition::value, T>;
	template<typename Condition, typename T = detail::enabler> using disable_if = detail::enable_if_t<!Condition::value, T>;
	template<typename T> using decay = typename std::decay<T>::type;
	template<typename T> using service_type = typename decay<T>::ServiceType;
	template<typename T> using is_single = std::is_base_of<Single, decay<T>>;
	template<typename T> using is_base_of_container = std::is_base_of<Container, T>;
	template<typename T> using is_container = std::is_same<T, Container>;
	template<typename T> using parent_types = typename decay<T>::ParentTypes;
	template<typename T> using is_abstract = std::is_abstract<T>;
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
	Container(const Container &) = delete;
	Container& operator =(const Container &) = delete;
	
	Container(Container &&) = default;
	Container& operator =(Container &&) = default;
	~Container() = default;

	template <typename T, typename... Args, enable_if<is_base_of_container<T>> = null>
	static T make_container(Args&&... args) {
		T container{std::forward<Args>(args)...};
		container.init();
		
		return std::move(container);
	}

	template<typename T>
	void instance(T&& service) {
		static_assert(is_single<T>::value, "instance() only accept Single Service instance.");
		
		save_instance<T>(std::forward<T>(service));
	}
	
	template<typename T>
	T release() {
		static_assert(is_single<T>::value, "release() only accept Single Service instance.");
		
		auto s = std::move(*static_cast<decay<T>*>(_services[detail::type_id<T>]));
		_services.erase(detail::type_id<T>);
		
		return std::move(s);
	}
	
	template<typename T, typename... Args>
	void instance(Args&& ...args) {
		save_instance<T>(make_service_instance<T>(std::forward<Args>(args)...));
	}
	
	template<typename T, typename... Args, disable_if<is_base_of_container<decay<T>>> = null>
	service_type<decay<T>> service(Args&& ...args) {
		return get_service<decay<T>>(std::forward<Args>(args)...).forward();
	}
	
	template<typename U, typename ...Args>
	detail::function_result_t<U> invoke(U&& function, Args&&... args) {
		return invoke_helper(tuple_seq_minus<detail::function_arguments_t<U>, sizeof...(Args)>{}, std::forward<U>(function), std::forward<Args>(args)...);
	}
	
	template<template<typename, int> class Map, typename U, typename ...Args>
	detail::function_result_t<U> invoke(U&& function, Args&&... args) {
		return invoke_helper<Map>(tuple_seq_minus<detail::function_arguments_t<U>, sizeof...(Args)>{}, std::forward<U>(function), std::forward<Args>(args)...);
	}
	
protected:
	Container() = default;
	void init(){}
	
private:
	template<typename U, typename T>
	void instance(T&& service) {
		static_assert(is_single<T>::value, "instance() only accept Single Service instance.");
		
		save_instance<U>(std::forward<T>(service));
	}
	
	template<typename U, typename T>
	void save_instance(T&& service) {
		save_instance<U>(std::forward<T>(service), tuple_seq<parent_types<T>>{});
	}
	
	template<typename U, typename T, int... S>
	void save_instance(T&& service, detail::seq<S...>) {
		save_instance_helper<decay<U>, parent_element<S, decay<U>>...>(instance_ptr<decay<U>>(new decay<U>{std::move(service)}, &Container::deleter<U>));
	}
	
	template<typename T>
	void save_instance_helper(instance_ptr<T> service) {
		_services.emplace(detail::type_id<T>, service.get());
		_instances.emplace_back(std::move(service));
	}
	
	template<typename T, typename Save, typename... Others>
	void save_instance_helper(instance_ptr<T> service) {
		struct SaveType : Save {
			virtual ~SaveType() {}
			SaveType(T& service) : _service{service} {}
			
			typename Save::ServiceType forward() override {
				return static_cast<typename Save::ServiceType>(_service.forward());
			}
			
		private:
			T& _service;
		};
		
		auto baseService = instance_ptr<SaveType>{new SaveType{*service.get()}, &Container::deleter<SaveType>};
		_services[detail::type_id<Save>] = baseService.get();
		_instances.emplace_back(std::move(baseService));
		save_instance_helper<T, Others...>(std::move(service));
	}
	
	template<typename T, typename... Args, disable_if<is_single<decay<T>>> = null, disable_if<is_base_of_container<decay<T>>> = null>
	decay<T> get_service(Args ...args) {
		return make_service_instance<decay<T>>(std::forward<Args>(args)...);
	}
	
	template<typename T, enable_if<is_container<decay<T>>> = null>
	decay<T>& get_service() {
		return *this;
	}
	
	template<typename T, disable_if<is_container<decay<T>>> = null, enable_if<is_base_of_container<decay<T>>> = null>
	decay<T>& get_service() {
		return dynamic_cast<T>(this);
	}
	
	template<typename T, enable_if<is_abstract<decay<T>>> = null, enable_if<is_single<decay<T>>> = null, disable_if<is_base_of_container<decay<T>>> = null>
	decay<T>& get_service() {
		auto it = _services.find(detail::type_id<decay<T>>);
		
		if (it != _services.end()) {
			return *static_cast<decay<T>*>(it->second);
		}
		
		throw std::runtime_error("No service instance can be used as abstract service");
	}
	
	template<typename T, enable_if<is_single<decay<T>>> = null, disable_if<is_base_of_container<decay<T>>> = null, disable_if<is_abstract<decay<T>>> = null>
	decay<T>& get_service() {
		auto it = _services.find(detail::type_id<decay<T>>);
		
		if (it == _services.end()) {
			instance(make_service_instance<decay<T>>());
			
			return *static_cast<decay<T>*>(_services[detail::type_id<decay<T>>]);
		}
		
		return *static_cast<decay<T>*>(it->second);
	}
	
	template<typename T, typename... Args>
	decay<T> make_service_instance(Args&&... args) {
		using U = decay<T>;
		return invoke(&U::construct, std::forward<Args>(args)...);
	}
	
	template<typename U, typename ...Args, int... S>
	detail::function_result_t<U> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return function(get_service<detail::function_argument_t<S, U>>()..., std::forward<Args>(args)...);
	}
	
	template<template<typename, int> class Map, typename U, typename ...Args, int... S>
	detail::function_result_t<U> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return function(service<typename Map<detail::function_argument_t<S, U>, S>::Service>()..., std::forward<Args>(args)...);
	}
	
	std::vector<instance_ptr<void>> _instances;
	std::unordered_map<detail::type_id_t, void*> _services;
};

template<typename T = Container, typename ...Args>
T make_container(Args&& ...args) {
	static_assert(std::is_base_of<Container, T>::value, "make_container only accept container types.");

	return Container::make_container<T>(std::forward<Args>(args)...);
}

}  // namespace kgr
