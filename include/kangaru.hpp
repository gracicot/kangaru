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

enum class enabler {};

template <bool b, typename T>
using enable_if_t = typename std::enable_if<b, T>::type;

template<int ...>
struct seq {};

template<int n, int ...S>
struct seq_gen : seq_gen<n-1, n-1, S...> {};

template<int ...S>
struct seq_gen<0, S...> {
	using type = seq<S...>;
};

struct Holder {
	virtual ~Holder() = default;
};

template<typename T>
struct InstanceHolder final : Holder {
	explicit InstanceHolder(std::shared_ptr<T> instance) : _instance{std::move(instance)} {}
	
	std::shared_ptr<T> getInstance() const {
		return _instance;
	}
	
private:
	std::shared_ptr<T> _instance;
};

template<typename T, typename... Args>
struct CallbackHolder final : Holder {
	using callback_t = std::function<std::shared_ptr<T>(Args...)>;

	explicit CallbackHolder(callback_t callback) : _callback{std::move(callback)} {}
	
	std::shared_ptr<T> operator ()(Args... args) {
		return _callback(args...);
	}
private:
	callback_t _callback;
};

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}

template <typename T> void type_id() {}
using type_id_fn = void(*)();

} // namespace detail

class Container : public std::enable_shared_from_this<Container> {
	template<typename Condition, typename T = detail::enabler> using enable_if = detail::enable_if_t<Condition::value, T>;
	template<typename Condition, typename T = detail::enabler> using disable_if = detail::enable_if_t<!Condition::value, T>;
	template<typename T> using is_service_single = std::is_base_of<Single, Service<T>>;
	template<typename T> using is_abstract = std::is_abstract<T>;
	template<typename T> using is_base_of_container = std::is_base_of<Container, T>;
	template<typename T> using is_container = std::is_same<T, Container>;
	template<typename T> using dependency_types = typename Service<T>::DependenciesTypes;
	template<typename T> using parent_types = typename Service<T>::ParentTypes;
	template <typename Tuple> using tuple_seq = typename detail::seq_gen<std::tuple_size<Tuple>::value>::type;
	template<int S, typename T> using parent_element = typename std::tuple_element<S, parent_types<T>>::type;
	template<int S, typename Tuple> using tuple_element = typename std::tuple_element<S, Tuple>::type;
	using holder_ptr = std::unique_ptr<detail::Holder>;
	using holder_cont = std::unordered_map<detail::type_id_fn, holder_ptr>;
	constexpr static detail::enabler null = {};
	
public:
	Container(const Container &) = delete;
	Container(Container &&) = delete;
	Container& operator =(const Container &) = delete;
	Container& operator =(Container &&) = delete;
	virtual ~Container() = default;

	template <typename T, typename... Args, 
		 enable_if<is_base_of_container<T>> = null>
	static std::shared_ptr<T> make_container(Args&&... args) {
		auto container = std::make_shared<T>(std::forward<Args>(args)...);
		static_cast<Container&>(*container).init();
		return container;
	}

	template<typename T>
	void instance(std::shared_ptr<T> service) {
		static_assert(is_service_single<T>::value, "instance only accept Single Service instance.");

		call_save_instance(std::move(service), tuple_seq<parent_types<T>>{});
	}
	
	template<typename T>
	void instance() {
		static_assert(is_service_single<T>::value, "instance only accept Single Service instance.");

		instance(make_service<T>());
	}
	
	template<typename T, disable_if<is_abstract<T>> = null, disable_if<is_base_of_container<T>> = null>
	std::shared_ptr<T> service() {
		return get_service<T>();
	}
	
	template<typename T, enable_if<is_container<T>> = null>
	std::shared_ptr<T> service() {
		return shared_from_this();
	}
	
	template<typename T, disable_if<is_container<T>> = null, enable_if<is_base_of_container<T>> = null>
	std::shared_ptr<T> service() {
		return std::dynamic_pointer_cast<T>(shared_from_this());
	}
	
	template<typename T, enable_if<is_abstract<T>> = null>
	std::shared_ptr<T> service() {
		auto it = _services.find(&detail::type_id<T>);
		
		if (it != _services.end()) {
			return static_cast<detail::InstanceHolder<T>&>(*it->second).getInstance();
		}
		
		return {};
	}
	
	template<typename T, typename U>
	void callback(U callback) {
		static_assert(!is_service_single<T>::value, "instance does not accept Single Service.");
		
		save_callback<T, dependency_types<T>>(tuple_seq<dependency_types<T>>{}, callback);
	}
protected:
	Container() = default;
	virtual void init(){}	
private:
	template<typename T, enable_if<is_service_single<T>> = null>
	std::shared_ptr<T> get_service() {
		auto it = _services.find(&detail::type_id<T>);
		
		if (it == _services.end()) {
			auto service = make_service<T>();
			instance(service);
			
			return service;
		} 
		return static_cast<detail::InstanceHolder<T>*>(it->second.get())->getInstance();
	}

	template<typename T, disable_if<is_service_single<T>> = null>
	std::shared_ptr<T> get_service() {
		return make_service<T>();
	}

	template<typename T, int ...S>
	void call_save_instance(std::shared_ptr<T> service, detail::seq<S...>) {
		save_instance<T, parent_element<S, T>...>(std::move(service));
	}
	
	template<typename Tuple, int ...S>
	std::tuple<std::shared_ptr<tuple_element<S, Tuple>>...> dependency(detail::seq<S...>) {
		return std::make_tuple(service<tuple_element<S, Tuple>>()...);
	}
	
	template<typename T, typename Tuple, int ...S>
	std::shared_ptr<T> callback_make_service(detail::seq<S...>, Tuple dependencies) const {
		auto it = _callbacks.find(&detail::type_id<T>);

		if (it != _callbacks.end()) {
			return static_cast<detail::CallbackHolder<T, tuple_element<S, Tuple>...>&>(*it->second)(std::get<S>(dependencies)...);
		}
		
		return {};
	}
	
	template<typename T, typename Tuple, int ...S>
	std::shared_ptr<T> make_service(detail::seq<S...> seq, Tuple dependencies) const {
		auto service = callback_make_service<T, Tuple>(seq, dependencies);
		
		return service ? service : std::make_shared<T>(std::get<S>(dependencies)...);
	}

	template <typename T>
	std::shared_ptr<T> make_service() {
		auto seq = tuple_seq<dependency_types<T>>{};
		return make_service<T>(seq, dependency<dependency_types<T>>(seq));
	}
	
	template<typename T, typename ...Others>
	detail::enable_if_t<(sizeof...(Others) > 0), void> save_instance(std::shared_ptr<T> service) {
		save_instance<T>(service);
		save_instance<Others...>(std::move(service));
	}
	
	template<typename T>
	void save_instance (std::shared_ptr<T> service) {
		_services[&detail::type_id<T>] = detail::make_unique<detail::InstanceHolder<T>>(std::move(service));
	}
	
	template<typename T, typename Tuple, int ...S, typename U>
	void save_callback (detail::seq<S...>, U callback) {
		_callbacks[&detail::type_id<T>] = detail::make_unique<detail::CallbackHolder<T, std::shared_ptr<tuple_element<S, Tuple>>...>>(callback);
	}
	
	holder_cont _callbacks;
	holder_cont _services;
};

template<typename T = Container, typename ...Args>
std::shared_ptr<T> make_container(Args&& ...args) {
	static_assert(std::is_base_of<Container, T>::value, "make_container only accept container types.");

	return Container::make_container<T>(std::forward<Args>(args)...);
}

}  // namespace kgr
