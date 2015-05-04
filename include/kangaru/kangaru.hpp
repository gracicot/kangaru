#pragma once

#include "detail/callback_holder.hpp"
#include "detail/function_traits.hpp"
#include "detail/pointer_type.hpp"
#include "detail/utils.hpp"

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

template<typename T>
struct Service;

struct Single {
	using ParentTypes = std::tuple<>;
};

struct Unique {
	template<typename T> using PointerType = detail::PointerType<std::unique_ptr<T>>;
};

struct Raw {
	template<typename T> using PointerType = detail::PointerType<T*>;
};

template<typename... Types>
struct Overrides : Single {
	using ParentTypes = std::tuple<Types...>;
};

class Container : public std::enable_shared_from_this<Container> {
	template<typename Condition, typename T = detail::enabler> using enable_if = detail::enable_if_t<Condition::value, T>;
	template<typename Condition, typename T = detail::enabler> using disable_if = detail::enable_if_t<!Condition::value, T>;
	template<typename T> using is_service_single = std::is_base_of<Single, Service<T>>;
	template<typename T> using is_abstract = std::is_abstract<T>;
	template<typename T> using is_base_of_container = std::is_base_of<Container, T>;
	template<typename T> using is_container = std::is_same<T, Container>;
	template<typename T> using dependency_types = typename Service<T>::DependenciesTypes;
	template<typename T> using parent_types = typename Service<T>::ParentTypes;
	template<typename Tuple> using tuple_seq = typename detail::seq_gen<std::tuple_size<Tuple>::value>::type;
	template<int S, typename T> using parent_element = typename std::tuple_element<S, parent_types<T>>::type;
	template<int S, typename Tuple> using tuple_element = typename std::tuple_element<S, Tuple>::type;
	using holder_ptr = std::unique_ptr<detail::Holder>;
	using callback_cont = std::unordered_map<detail::type_id_fn, holder_ptr>;
	using instance_cont = std::unordered_map<detail::type_id_fn, std::shared_ptr<void>>;
	template<int S, typename Services> using service_ptrs = service_ptr<typename std::tuple_element<S, Services>::type>;
	constexpr static detail::enabler null = {};
	
public:
	Container(const Container &) = delete;
	Container(Container &&) = delete;
	Container& operator =(const Container &) = delete;
	Container& operator =(Container &&) = delete;
	virtual ~Container() = default;

	template <typename T, typename... Args, enable_if<is_base_of_container<T>> = null>
	static std::shared_ptr<T> make_container(Args&&... args) {
		auto container = std::shared_ptr<T>(new T {std::forward<Args>(args)...});
		static_cast<Container&>(*container).init();
		
		return container;
	}

	template<typename T>
	void instance(std::shared_ptr<T> service) {
		static_assert(!std::is_base_of<Unique, Service<T>>::value, "Single cannot be unique");
		static_assert(std::is_same<service_ptr<T>, std::shared_ptr<T>>::value || std::is_same<service_ptr<T>, T*>::value, "Single pointer type can only be raw of shared_ptr.");
		static_assert(is_service_single<T>::value, "instance only accept Single Service instance.");

		call_save_instance(std::move(service), tuple_seq<parent_types<T>>{});
	}

	template<typename T, enable_if<std::is_same<service_ptr<T>, T*>> = null>
	void instance(T* service) {
		static_assert(is_service_single<T>::value, "instance only accept Single Service instance.");

		call_save_instance(std::shared_ptr<T>(service), tuple_seq<parent_types<T>>{});
	}
	
	template<typename T, typename ...Args>
	void instance(Args&& ...args) {
		static_assert(is_service_single<T>::value, "instance only accept Single Service instance.");

		instance(make_service_instance<T>(std::forward<Args>(args)...));
	}
	
	template<typename T, typename ...Args, disable_if<is_abstract<T>> = null, disable_if<is_base_of_container<T>> = null>
	service_ptr<T> service(Args&& ...args) {
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
	service_ptr<T> service() {
		auto it = _services.find(&detail::type_id<T>);
		
		if (it != _services.end()) {
			return std::static_pointer_cast<T>(it->second);
		}
		
		return {};
	}
	
	template<typename T, typename U>
	void callback(U callback) {
		static_assert(!is_service_single<T>::value, "instance does not accept Single Service.");
		
		call_save_callback<T>(tuple_seq<dependency_types<T>>{}, callback);
	}
	
	template<typename U>
	void callback(U callback) {
		using T = typename detail::PointerType<detail::function_result_t<U>>::ServiceType;

		this->callback<T,U>(callback);
	}
	
protected:
	Container() = default;
	virtual void init(){}	
	
private:
	template<typename T, enable_if<is_service_single<T>> = null>
	service_ptr<T> get_service() {
		auto it = _services.find(&detail::type_id<T>);
		
		if (it == _services.end()) {
			auto service = std::shared_ptr<T>(make_service_instance<T>());
			instance(service);
			
			return detail::PointerConverter<T, service_ptr<T>, std::shared_ptr<T>>::convert(std::move(service));
		}
		
		return detail::PointerConverter<T, service_ptr<T>, std::shared_ptr<T>>::convert(std::static_pointer_cast<T>(it->second));
	}
	
	template<typename T, typename ...Args, disable_if<is_service_single<T>> = null>
	service_ptr<T> get_service(Args ...args) {
		return make_service_instance<T>(std::forward<Args>(args)...);
	}

	template<typename T, int ...S>
	void call_save_instance(std::shared_ptr<T> service, detail::seq<S...>) {
		save_instance<T, parent_element<S, T>...>(std::move(service));
	}
	
	template<typename Tuple, int ...S>
	std::tuple<service_ptrs<S, Tuple>...> dependency(detail::seq<S...>) {
		return std::make_tuple(service<tuple_element<S, Tuple>>()...);
	}
	
	template<typename T, typename Tuple, int ...S, typename ...Args>
	service_ptr<T> callback_make_service(detail::seq<S...>, Tuple dependencies, Args&& ...args) const {
		auto it = _callbacks.find(&detail::type_id<T, tuple_element<S, Tuple>..., Args...>);
		
		if (it != _callbacks.end()) {
			return static_cast<detail::CallbackHolder<T, tuple_element<S, Tuple>..., Args...>&>(*it->second.get())(std::get<S>(dependencies)..., std::forward<Args>(args)...);
		}
		
		return {};
	}
	
	template<typename T, typename Tuple, int ...S, typename ...Args, enable_if<is_service_single<T>> = null>
	service_ptr<T> make_service_dependency(detail::seq<S...>, Tuple dependencies, Args&& ...args) const {
		return make_service<T>(std::move(std::get<S>(dependencies))..., std::forward<Args>(args)...);
	}
	
	template<typename T, typename Tuple, int ...S, typename ...Args, disable_if<is_service_single<T>> = null>
	service_ptr<T> make_service_dependency(detail::seq<S...> seq, Tuple dependencies, Args&& ...args) const {
		auto service = callback_make_service<T, Tuple>(seq, dependencies, std::forward<Args>(args)...);
		return service ? service : make_service<T>(std::get<S>(dependencies)..., std::forward<Args>(args)...);
	}
	
	template <typename T, typename ...Args>
	service_ptr<T> make_service_instance(Args&& ...args) {
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
		_services[&detail::type_id<T>] = std::move(service);
	}
	
	template<typename T, int ...S, typename U>
	void call_save_callback(detail::seq<S...>, U callback) {
		using argument_dependency_types = std::tuple<detail::function_argument_t<S, U>...>;
		using dependency_ptr = std::tuple<service_ptrs<S, dependency_types<T>>...>;
		static_assert(std::is_same<dependency_ptr, argument_dependency_types>::value, "The callback should receive the dependencies in the right order as first parameters");
		static_assert(std::is_same<detail::function_result_t<U>, service_ptr<T>>::value, "The callback should return the right type of pointer.");
		
		save_callback<T, detail::function_arguments_t<U>>(tuple_seq<detail::function_arguments_t<U>>{}, callback);
	}
	
	template<typename T, typename Tuple, int ...S, typename U>
	void save_callback (detail::seq<S...>, U callback) {
		_callbacks[&detail::type_id<T, tuple_element<S, Tuple>...>] = detail::make_unique<detail::CallbackHolder<T, tuple_element<S, Tuple>...>>(callback);
	}
	
	callback_cont _callbacks;
	instance_cont _services;
};

template<typename T = Container, typename ...Args>
std::shared_ptr<T> make_container(Args&& ...args) {
	static_assert(std::is_base_of<Container, T>::value, "make_container only accept container types.");

	return Container::make_container<T>(std::forward<Args>(args)...);
}

}  // namespace kgr
