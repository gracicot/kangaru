#pragma once

#include <unordered_map>
#include <memory>
#include <type_traits>
#include <tuple>

namespace sdicl {

template<typename... Types>
struct Dependency {
	using DependenciesTypes = std::tuple<Types...>;
};

template<typename T>
struct Service;

namespace detail {

template<int ...>
struct seq { };

template<int N, int ...S>
struct seq_gen : seq_gen<N-1, N-1, S...> { };

template<int ...S>
struct seq_gen<0, S...> {
  typedef seq<S...> type;
};

struct Holder {
	virtual ~Holder() {
		
	}
};

template<typename T>
struct InstanceHolder : Holder {
	InstanceHolder(std::shared_ptr<T> instance) : _instance{instance} {}
	
	std::shared_ptr<T> getInstance() const {
		return _instance;
	}
	
private:
	std::shared_ptr<T> _instance;
};

}

struct Container : std::enable_shared_from_this<Container> {
	template<typename T>
	void single() {
		auto dependencies = dependency<typename Service<T>::Dependencies::DependenciesTypes>(typename detail::seq_gen<std::tuple_size<typename Service<T>::Dependencies::DependenciesTypes>::value>::type());
		auto service = make_service<T, decltype(dependencies)>(typename detail::seq_gen<std::tuple_size<typename Service<T>::Dependencies::DependenciesTypes>::value>::type(), dependencies);
			
		_services[typeid(T).name()] = std::unique_ptr<detail::InstanceHolder<T>>(new detail::InstanceHolder<T>(service));
	}
	
	template<typename T>
	typename std::enable_if<(!std::is_abstract<T>::value && !std::is_base_of<Container, T>::value), std::shared_ptr<T>>::type service() {
		auto it = _services.find(typeid(T).name());
		if (it == _services.end()) {
			auto dependencies = dependency<typename Service<T>::Dependencies::DependenciesTypes>(typename detail::         seq_gen<std::tuple_size<typename Service<T>::Dependencies::DependenciesTypes>::value>::type());
			auto service = make_service<T, decltype(dependencies)>(typename detail::seq_gen<std::tuple_size<typename Service<T>::Dependencies::DependenciesTypes>::value>::type(), dependencies);
			
			return service;
		} else {
			auto holder = dynamic_cast<detail::InstanceHolder<T>*>(it->second.get());
			if (holder) {
				return holder->getInstance();
			} else {
				return nullptr;
			}
		}
	}
	
	template<typename T>
	typename std::enable_if<(std::is_same<T, Container>::value), std::shared_ptr<T>>::type service() {
		return shared_from_this();
	}
	
	template<typename T>
	typename std::enable_if<(std::is_base_of<Container, T>::value && !std::is_same<T, Container>::value), std::shared_ptr<T>>::type service() {
		auto service = std::dynamic_pointer_cast<T>(shared_from_this());
		if (service) {
			return service;
		} else {
			auto it = _services.find(typeid(T).name());
			if (it == _services.end()) {
				auto service = std::make_shared<T>();
				
				_services[typeid(T).name()] = std::unique_ptr<detail::InstanceHolder<T>>(new detail::InstanceHolder<T>(service));
				return service;
			} else {
				auto holder = dynamic_cast<detail::InstanceHolder<T>*>(it->second.get());
				if (holder) {
					return holder->getInstance();
				} else {
					return nullptr;
				}
			}
		}
	}
	
// TODO: Support abstract types
// 	template<typename T>
// 	std::enable_if_t<std::is_abstract<T>::value, std::shared_ptr<T>> service() {
// 		auto it = _services.find(typeid(T).name());
// 		if (it != _services.end()) {
// 			auto holder = std::dynamic_pointer_cast<InstanceHolder<T>>(it->second);
// 			if (holder) {
// 				return holder->getInstance();
// 			} else {
// 				return nullptr;
// 			}
// 		}
// 	}
	
	virtual void init(){}
	
private:
	template<typename Tuple, int ...S>
	std::tuple<std::shared_ptr<typename std::tuple_element<S, Tuple>::type>...> dependency(detail::seq<S...>) {
		return std::make_tuple(service<typename std::tuple_element<S, Tuple>::type>()...);
	}
	
	template<typename T, typename Tuple, int ...S>
	std::shared_ptr<T> make_service(detail::seq<S...>, Tuple dependencies) {
		return std::make_shared<T>(std::get<S>(dependencies)...);
	}
	
	std::unordered_map<std::string, std::unique_ptr<detail::Holder>> _services;
};

template<typename T = Container, typename ...Args>
std::shared_ptr<T> make_container(Args&& ...args) {
	static_assert(std::is_base_of<Container, T>::value, "make_container only accept container types.");
	auto container = std::make_shared<T>(std::forward<Args>(args)...);
	container->init();
	return container;
}

}
