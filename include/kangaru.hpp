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

template <typename U>
struct function_traits
    : function_traits<decltype(&U::operator())>
{};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const> {
    using return_type = R;
	using argument_types = std::tuple<Args...>;
};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...)> {
    using return_type = R;
	using argument_types = std::tuple<Args...>;
};

template <typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using return_type = R;
	using argument_types = std::tuple<Args...>;
};

struct Holder {};

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

template <typename T, typename ...Args> void type_id() {}
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
	Container() = default;
	Container(const Container &) = default;
	Container(Container &&) = default;
	Container& operator =(const Container &) = default;
	Container& operator =(Container &&) = default;
	~Container() = default;

	template<typename T>
	void instance(std::shared_ptr<T> service) {
		static_assert(is_service_single<T>::value, "instance only accept Single Service instance.");

		call_save_instance(std::move(service), tuple_seq<parent_types<T>>{});
	}
	
	template<typename T, typename ...Args>
	void instance(Args&& ...args) {
		static_assert(is_service_single<T>::value, "instance only accept Single Service instance.");

		instance(make_service<T>(std::forward<Args>(args)...));
	}
	
	template<typename T, typename ...Args, disable_if<is_abstract<T>> = null, disable_if<is_base_of_container<T>> = null>
	std::shared_ptr<T> service(Args&& ...args) {
		return get_service<T>(std::forward<Args>(args)...);
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
			return static_cast<detail::InstanceHolder<T>*>(it->second.get())->getInstance();
		}
		
		return {};
	}
	
	template<typename T, typename U>
	void callback(U callback) {
		using arguments = typename detail::function_traits<U>::argument_types;
		static_assert(!is_service_single<T>::value, "instance does not accept Single Service.");
		
		call_save_callback<T, arguments>(tuple_seq<dependency_types<T>>{}, callback);
	}
	
	template<typename U>
	void callback(U callback) {
		using T = typename detail::function_traits<U>::return_type::element_type;
		using arguments = typename detail::function_traits<U>::argument_types;
		static_assert(!is_service_single<T>::value, "instance does not accept Single Service.");
		
		call_save_callback<T, arguments>(tuple_seq<dependency_types<T>>{}, callback);
	}
	
	virtual void init(){}
	
private:
	template<typename T, enable_if<is_service_single<T>> = null>
	std::shared_ptr<T> get_service() {
		auto it = _services.find(&detail::type_id<T>);
		
		if (it == _services.end()) {
			auto service = make_service<T>();
			instance(service);
			
			return service;
		} else {
			return static_cast<detail::InstanceHolder<T>*>(it->second.get())->getInstance();
		}
		
		return {};
	}
	
	template<typename T, typename ...Args, disable_if<is_service_single<T>> = null>
	std::shared_ptr<T> get_service(Args ...args) {
		return make_service<T>(std::forward<Args>(args)...);
	}
	
	template<typename T, int ...S>
	void call_save_instance(std::shared_ptr<T> service, detail::seq<S...>) {
		save_instance<T, parent_element<S, T>...>(std::move(service));
	}
	
	template<typename Tuple, int ...S>
	std::tuple<std::shared_ptr<tuple_element<S, Tuple>>...> dependency(detail::seq<S...>) {
		return std::make_tuple(service<tuple_element<S, Tuple>>()...);
	}
	
	template<typename T, typename Tuple, int ...S, typename ...Args>
	std::shared_ptr<T> callback_make_service(detail::seq<S...>, Tuple dependencies, Args&& ...args) const {
		auto it = _callbacks.find(&detail::type_id<T, tuple_element<S, Tuple>..., Args...>);
		
		if (it != _callbacks.end()) {
			return (*static_cast<detail::CallbackHolder<T, tuple_element<S, Tuple>..., Args...>*>(it->second.get()))(std::get<S>(dependencies)..., std::forward<Args>(args)...);
		}
		
		return {};
	}
	
	template<typename T, typename Tuple, int ...S, typename ...Args, enable_if<is_service_single<T>> = null>
	std::shared_ptr<T> make_service_dependency(detail::seq<S...> seq, Tuple dependencies, Args&& ...args) const {
		return std::make_shared<T>(std::get<S>(dependencies)..., std::forward<Args>(args)...);
	}
	
	template<typename T, typename Tuple, int ...S, typename ...Args, disable_if<is_service_single<T>> = null>
	std::shared_ptr<T> make_service_dependency(detail::seq<S...> seq, Tuple dependencies, Args&& ...args) const {
		auto service = callback_make_service<T, Tuple>(seq, dependencies, std::forward<Args>(args)...);
		
		if (service) {
			return service;
		}
		
		return std::make_shared<T>(std::get<S>(dependencies)..., std::forward<Args>(args)...);
	}

	template <typename T, typename ...Args>
	std::shared_ptr<T> make_service(Args&& ...args) {
		auto seq = tuple_seq<dependency_types<T>>{};
		return make_service_dependency<T>(seq, dependency<dependency_types<T>>(seq), std::forward<Args>(args)...);
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
	void call_save_callback(detail::seq<S...>, U callback) {
		using argument_dependency_types = std::tuple<tuple_element<S, Tuple>...>;
		using dependency_ptr = std::tuple<std::shared_ptr<tuple_element<S, dependency_types<T>>>...>;
		static_assert(std::is_same<dependency_ptr, argument_dependency_types>::value, "The callback should receive the dependencies in the right order as first parameters");
		
		save_callback<T, Tuple>(tuple_seq<Tuple>{}, callback);
	}
	
	template<typename T, typename Tuple, int ...S, typename U>
	void save_callback (detail::seq<S...>, U callback) {
		_callbacks[&detail::type_id<T, tuple_element<S, Tuple>...>] = detail::make_unique<detail::CallbackHolder<T, tuple_element<S, Tuple>...>>(callback);
	}
	
	holder_cont _callbacks;
	holder_cont _services;
};

template<typename T = Container, typename ...Args>
std::shared_ptr<T> make_container(Args&& ...args) {
	static_assert(std::is_base_of<Container, T>::value, "make_container only accept container types.");
	
	auto container = std::make_shared<T>(std::forward<Args>(args)...);
	container->init();
	
	return container;
}

}  // namespace kgr
