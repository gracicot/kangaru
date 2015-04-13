#pragma once

#include <unordered_map>
#include <memory>
#include <type_traits>
#include <tuple>

namespace kgr {

template<typename... Types>
struct Dependency {
	using DependenciesTypes = std::tuple<Types...>;
};

using NoDependencies = Dependency<>;

struct Single {
	using ParentTypes = std::tuple<>;
};

template<typename T>
struct Service;

template<typename... Types>
struct Overrides : Single {
	using ParentTypes = std::tuple<Types...>;
};

namespace detail {

template<int ...>
struct seq { };

template<int N, int ...S>
struct seq_gen : seq_gen<N-1, N-1, S...> { };

template<int ...S>
struct seq_gen<0, S...> {
	using type = seq<S...>;
};

template <typename Tuple>
using tuple_seq = typename seq_gen<std::tuple_size<Tuple>::value>::type;

template <bool B, typename T>
using enable_if_t = typename std::enable_if<B, T>::type;

struct Holder {
	virtual ~Holder() {
		
	}
};

template<typename T>
struct InstanceHolder : Holder {
	explicit InstanceHolder(std::shared_ptr<T> instance) : _instance{instance} {}
	
	std::shared_ptr<T> getInstance() const {
		return _instance;
	}
	
private:
	std::shared_ptr<T> _instance;
};

template<typename T, typename... Args>
struct CallbackHolder : Holder {
	using callback_t = std::function<std::shared_ptr<T>(Args...)>;

	explicit CallbackHolder(callback_t callback) : _callback{callback} {}
	
	callback_t getCallback() const {
		return _callback;
	}
private:
	callback_t _callback;
};

}  // namespace detail

struct Container : std::enable_shared_from_this<Container> {
	template<typename T>
	void instance(std::shared_ptr<T> service) {
		static_assert(std::is_base_of<Single, Service<T>>::value, "instance only accept Single Service instance.");
		call_save_instance(service, detail::tuple_seq<typename Service<T>::ParentTypes>{});
	}
	
	template<typename T>
	void instance() {
		static_assert(std::is_base_of<Single, Service<T>>::value, "instance only accept Single Service instance.");
		instance(make_service<T>());
	}
	
	template<typename T>
	detail::enable_if_t<(!std::is_abstract<T>::value && !std::is_base_of<Container, T>::value), std::shared_ptr<T>> service() {
		return get_service<T>();
	}
	
	template<typename T>
	detail::enable_if_t<(std::is_same<T, Container>::value), std::shared_ptr<T>> service() {
		return shared_from_this();
	}
	
	template<typename T>
	detail::enable_if_t<(std::is_base_of<Container, T>::value && !std::is_same<T, Container>::value), std::shared_ptr<T>> service() {
		return std::dynamic_pointer_cast<T>(shared_from_this());
	}
	
	template<typename T>
	detail::enable_if_t<std::is_abstract<T>::value, std::shared_ptr<T>> service() {
		auto it = _services.find(typeid(T).name());
		if (it != _services.end()) {
			auto holder = dynamic_cast<detail::InstanceHolder<T>*>(it->second.get());
			if (holder) {
				return holder->getInstance();
			}
		}
		return {};
	}
	
	template<typename T, typename U>
	void callback(U callback) {
		static_assert(!std::is_base_of<Single, Service<T>>::value, "instance does not accept Single Service.");
		using DependenciesTypes = typename Service<T>::DependenciesTypes;
		save_callback<T, DependenciesTypes>(detail::tuple_seq<DependenciesTypes>{}, callback);
	}
	
	virtual void init(){}
	
private:
	using HolderPtr = std::unique_ptr<detail::Holder>;
	using Holders   = std::unordered_map<std::string, HolderPtr>;

	template<typename T>
	detail::enable_if_t<std::is_base_of<Single, Service<T>>::value, std::shared_ptr<T>> get_service() {
		auto it = _services.find(typeid(T).name());
		if (it == _services.end()) {
			auto service = make_service<T>();
			instance(service);
			return service;
		} else {
			auto holder = dynamic_cast<detail::InstanceHolder<T>*>(it->second.get());
			if (holder) {
				return holder->getInstance();
			}
		}
		return {};
	}
	
	template<typename T>
	detail::enable_if_t<!std::is_base_of<Single, Service<T>>::value, std::shared_ptr<T>> get_service() {
		return make_service<T>();
	}
	
	template<typename T, int ...S>
	void call_save_instance(std::shared_ptr<T> service, detail::seq<S...>) {
		save_instance<T, typename std::tuple_element<S, typename Service<T>::ParentTypes>::type...>(service);
	}
	
	template<typename Tuple, int ...S>
	std::tuple<std::shared_ptr<typename std::tuple_element<S, Tuple>::type>...> dependency(detail::seq<S...>) {
		return std::make_tuple(service<typename std::tuple_element<S, Tuple>::type>()...);
	}
	
	template<typename T, typename Tuple, int ...S>
	std::shared_ptr<T> callback_make_service(detail::seq<S...> seq, Tuple dependencies) const {
		auto it = _callbacks.find(typeid(T).name());
		if (it != _callbacks.end()) {
			auto holder = dynamic_cast<detail::CallbackHolder<T, typename std::tuple_element<S, Tuple>::type...>*>(it->second.get());
			if (holder) {
				return holder->getCallback()(std::get<S>(dependencies)...);
			}
		}
		return {};
	}
	
	template<typename T, typename Tuple, int ...S>
	std::shared_ptr<T> make_service(detail::seq<S...> seq, Tuple dependencies) const {
		auto service = callback_make_service<T, Tuple>(seq, dependencies);
		if (service) {
			return service;
		}
		return std::make_shared<T>(std::get<S>(dependencies)...);
	}

	template <typename T>
	std::shared_ptr<T> make_service() {
		using DependenciesTypes = typename Service<T>::DependenciesTypes;
		auto seq = detail::tuple_seq<DependenciesTypes>{};
		return make_service<T>(seq, dependency<DependenciesTypes>(seq));
	}
	
	template<typename T, typename ...Others>
	detail::enable_if_t<(sizeof...(Others) > 0), void> save_instance(std::shared_ptr<T> service) {
		save_instance<T>(service);
		save_instance<Others...>(service);
	}
	
	template<typename T>
	void save_instance (std::shared_ptr<T> service) {
		_services[typeid(T).name()] = HolderPtr{new detail::InstanceHolder<T>(service)};
	}
	
	template<typename T, typename Tuple, int ...S, typename U>
	void save_callback (detail::seq<S...>, U callback) {
		_callbacks[typeid(T).name()] = HolderPtr{new detail::CallbackHolder<T, std::shared_ptr<typename std::tuple_element<S, Tuple>::type>...>(callback)};
	}
	
	Holders _callbacks;
	Holders _services;
};

template<typename T = Container, typename ...Args>
std::shared_ptr<T> make_container(Args&& ...args) {
	static_assert(std::is_base_of<Container, T>::value, "make_container only accept container types.");
	auto container = std::make_shared<T>(std::forward<Args>(args)...);
	container->init();
	return container;
}

}  // namespace kgr

