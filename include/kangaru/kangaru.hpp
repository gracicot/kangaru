#pragma once

#include "detail/function_traits.hpp"
#include "detail/utils.hpp"

#include <unordered_map>
#include <memory>
#include <type_traits>
#include <tuple>

namespace kgr {

struct Single {
	using ParentTypes = std::tuple<>;
};

template<typename... Types>
struct Overrides : Single {
	using ParentTypes = std::tuple<Types...>;
};

template<typename Service>
struct Type {
	using ServiceType = Service;
};

struct ContainerService {
	
};

class Container {
	template<typename Condition, typename T = detail::enabler> using enable_if = detail::enable_if_t<Condition::value, T>;
	template<typename Condition, typename T = detail::enabler> using disable_if = detail::enable_if_t<!Condition::value, T>;
	template<typename T> using decay = typename std::decay<T>::type;
	template<typename T> using service_type = typename decay<T>::ServiceType;
	template<typename T> using is_single = std::is_base_of<Single, decay<T>>;
	template<typename T> using is_base_of_container = std::is_base_of<Container, T>;
	template<typename T> using is_container = std::is_same<T, Container>;
	template<typename T> using parent_types = typename decay<T>::ParentTypes;
	template<typename T> using is_abstract = std::is_abstract<service_type<T>>;
	template<int S, typename T> using parent_element = typename std::tuple_element<S, parent_types<T>>::type;
	template<typename Tuple> using tuple_seq = typename detail::seq_gen<std::tuple_size<Tuple>::value>::type;
	template<typename Tuple, int n> using tuple_seq_minus = typename detail::seq_gen<std::tuple_size<Tuple>::value - n>::type;
	using instance_cont = std::unordered_map<detail::type_id_t, std::shared_ptr<void>>;

	constexpr static detail::enabler null = {};
	
public:
	Container(const Container &) = delete;
	Container(Container &&) = delete;
	Container& operator =(const Container &) = delete;
	Container& operator =(Container &&) = delete;
	virtual ~Container() = default;

	template <typename T, typename... Args, enable_if<is_base_of_container<T>> = null>
	static std::unique_ptr<T> make_container(Args&&... args) {
		auto container = std::unique_ptr<T>(new T {std::forward<Args>(args)...});
		static_cast<Container&>(*container).init();
		
		return container;
	}

	template<typename T>
	void instance(T&& service) {
		static_assert(is_single<T>::value, "instance() only accept Single Service instance.");
		
		save_instance(std::forward<T>(service));
	}
	
	template<typename T>
	T release() {
		static_assert(is_single<T>::value, "release() only accept Single Service instance.");
		
		auto s = service<T>();
		_services.erase(detail::type_id<T>);
		
		return std::move(s);
	}
	
	template<typename T, typename... Args>
	void instance(Args&& ...args) {
		instance(make_service_instance(std::forward<Args>(args)...));
	}
	
	template<typename T, typename... Args, disable_if<is_abstract<T>> = null, disable_if<is_base_of_container<T>> = null>
	service_type<T> service(Args&& ...args) {
		return get_service<T>().forward();
	}
	
	template<typename T, enable_if<is_container<T>> = null>
	T* service() {
		return this;
	}
	
	template<typename T, disable_if<is_container<T>> = null, enable_if<is_base_of_container<T>> = null>
	T* service() {
		return dynamic_cast<T*>(this);
	}
	
	template<typename T, enable_if<is_abstract<T>> = null>
	service_type<T> service() {
		auto it = _services.find(detail::type_id<T>);
		
		if (it != _services.end()) {
			return std::static_pointer_cast<T>(it->second);
		}
		
		return {};
	}
	
	template<typename U, typename ...Args>
	detail::function_result_t<U> invoke(U&& function, Args&&... args) {
		return invoke_helper(tuple_seq_minus<detail::function_arguments_t<U>, sizeof...(Args)>{}, std::forward<U>(function), std::forward<Args>(args)...);
	}
	
	template<template<typename> class Map, typename U, typename ...Args>
	detail::function_result_t<U> invoke(U&& function, Args&&... args) {
		return invoke_helper<Map>(tuple_seq_minus<detail::function_arguments_t<U>, sizeof...(Args)>{}, std::forward<U>(function), std::forward<Args>(args)...);
	}
	
protected:
	Container() = default;
	virtual void init(){}
	
private:
	template<typename T>
	void save_instance(T&& service) {
		save_instance(std::forward<T>(service), tuple_seq<parent_types<T>>{});
	}
	
	template<typename T, int... S>
	void save_instance(T&& service, detail::seq<S...>) {
		save_instance_helper<T, parent_element<S, T>...>(std::forward<T>(service));
	}
	
	template<typename T>
	void save_instance_helper(T&& service) {
		_services.emplace(detail::type_id<decay<T>>,  std::make_shared<decay<T>>(service));
	}
	
	template<typename T, typename Save, typename... Others>
	void save_instance_helper(T&& service) {
		_services.emplace(detail::type_id<Save>, std::make_shared<decay<T>>(service));
		save_instance_helper<T, Others...>(std::forward<T>(service));
	}
	
	template<typename T, typename... Args, disable_if<is_single<T>> = null>
	T get_service(Args ...args) {
		return make_service_instance<T>(std::forward<Args>(args)...);
	}
	
	template<typename T, enable_if<is_single<T>> = null>
	T get_service() {
		auto it = _services.find(detail::type_id<T>);
		
		if (it == _services.end()) {
			auto service = make_service_instance<T>();
			instance(service);
			
			return service;
		}
		
		return *static_cast<T*>(it->second.get());
	}
	
	template<typename T, typename... Args>
	T make_service_instance(Args&&... args) {
		return invoke(&T::construct, std::forward<Args>(args)...);
	}
	
	template<typename U, typename ...Args, int... S>
	detail::function_result_t<U> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return function(get_service<detail::function_argument_t<S, U>>()..., std::forward<Args>(args)...);
	}
	
	template<template<typename> class Map, typename U, typename ...Args, int... S>
	detail::function_result_t<U> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return function(service<typename Map<detail::function_argument_t<S, U>>::Service>()..., std::forward<Args>(args)...);
	}
	
	template<typename T>
	static void deleter(void* ptr) {
		delete static_cast<decay<T>*>(ptr);
	}
	
	instance_cont _services;
};

template<typename T = Container, typename ...Args>
std::unique_ptr<T> make_container(Args&& ...args) {
	static_assert(std::is_base_of<Container, T>::value, "make_container only accept container types.");

	return Container::make_container<T>(std::forward<Args>(args)...);
}

}  // namespace kgr
