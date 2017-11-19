#ifndef KGR_KANGARU_INCLUDE_KANGARU_CONTAINER_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_CONTAINER_HPP

#include "detail/traits.hpp"
#include "detail/validity_check.hpp"
#include "detail/utils.hpp"
#include "detail/container_service.hpp"
#include "detail/invoke.hpp"
#include "detail/single.hpp"
#include "detail/exception.hpp"
#include "detail/service_storage.hpp"
#include "detail/injected.hpp"
#include "detail/error.hpp"
#include "predicate.hpp"

#include <unordered_map>
#include <memory>
#include <type_traits>
#include <tuple>
#include <algorithm>
#include <vector>

namespace kgr {

/**
 * The kangaru container class.
 * 
 * This class will construct services and share single instances for a given definition.
 * It is the class that parses and manage dependency graphs and call autocall functions.
 */
struct container final {
private:
	template<typename Condition, typename T = int> using enable_if = detail::enable_if_t<Condition::value, T>;
	template<typename Condition, typename T = int> using disable_if = detail::enable_if_t<!Condition::value, T>;
	template<typename T> using instance_ptr = std::unique_ptr<T, void(*)(void*)>;
	using instance_cont = std::vector<instance_ptr<void>>;
	using service_cont = std::unordered_map<type_id_t, detail::service_storage>;
	using service_cont_value = service_cont::value_type::second_type;
	template<typename T> using contained_service_t = typename std::conditional<
		detail::is_single<T>::value,
		instance_ptr<T>,
		T>::type;
	using unpack = int[];
	
	template<typename T>
	static void deleter(void* i) {
		delete static_cast<T*>(i);
	}
	
	template<typename T, typename... Args, enable_if<std::is_constructible<T, Args...>> = 0>
	static instance_ptr<T> make_instance_ptr(Args&&... args) {
		return instance_ptr<T>{
			new T(std::forward<Args>(args)...),
			&container::deleter<T>
		};
	}
	
	template<typename T, typename... Args, enable_if<detail::is_only_brace_constructible<T, Args...>> = 0>
	static instance_ptr<T> make_instance_ptr(Args&&... args) {
		return instance_ptr<T>{
			new T{std::forward<Args>(args)...},
			&container::deleter<T>
		};
	}
	
	template<typename T, enable_if<detail::is_virtual<T>> = 0>
	static detail::injected_wrapper<T> make_wrapper(service_cont_value instance) {
		return detail::injected_wrapper<T>{instance.cast<T>()};
	}
	
	template<typename T, disable_if<detail::is_virtual<T>> = 0>
	static detail::injected_wrapper<T> make_wrapper(service_cont_value instance) {
		return detail::injected_wrapper<T>{*static_cast<T*>(instance.service)};
	}
	
public:
	explicit container() = default;
	container(const container &) = delete;
	container& operator=(const container &) = delete;
	container(container&&) = default;
	container& operator=(container&&) = default;
	~container() = default;
	
	/*
	 * This function construct and save in place a service definition with the provided arguments.
	 * The service is only constructed if it is not found.
	 * It is usually used to instanciate supplied services.
	 * It returns if the service has been constructed.
	 * This function require the service to be single.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_construction_valid<T, Args...>> = 0>
	bool emplace(Args&&... args) {
		// TODO: We're doing two search in the map: One before constructing the service and one to insert. We should do only one.
		return contains<T>() ? false : (autocall(save_instance<T>(make_service_instance<T>(std::forward<Args>(args)...))), true);
	}
	
	/*
	 * The following two overloads are called in a case where the service is invalid,
	 * or is called when provided arguments don't match the constructor.
	 * In GCC, a diagnostic is provided.
	 */
	template<typename T, enable_if<std::is_default_constructible<detail::service_error<T>>> = 0>
	bool emplace(detail::service_error<T> = {}) = delete;
	
	template<typename T, typename... Args>
	bool emplace(detail::service_error<T, detail::identity_t<Args>...>, Args&&...) = delete;
	
	/*
	 * This function construct and save in place a service definition with the provided arguments.
	 * The inserted instance of the service will be used for now on.
	 * It does not delete the old instance if any.
	 * This function require the service to be single.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_construction_valid<T, Args...>> = 0>
	void replace(Args&&... args) {
		autocall(save_instance<T>(make_service_instance<T>(std::forward<Args>(args)...)));
	}
	
	/*
	 * The following two overloads are called in a case where the service is invalid,
	 * or is called when provided arguments don't match the constructor.
	 * In GCC, a diagnostic is provided.
	 */
	template<typename T, enable_if<std::is_default_constructible<detail::service_error<T>>> = 0>
	void replace(detail::service_error<T> = {}) = delete;
	
	template<typename T, typename... Args>
	void replace(detail::service_error<T, detail::identity_t<Args>...>, Args&&...) = delete;
	
	/*
	 * This function returns the service given by service definition T.
	 * T must be a valid service and must be constructible with arguments passed as parameters
	 * In case of a non-single service, it takes additional arguments to be sent to the T::construct function.
	 * T must not be a polymorphic type.
	 */
	template<typename T, typename... Args, enable_if<detail::is_service_valid<T, Args...>> = 0>
	service_type<T> service(Args&&... args) {
		return definition<T>(std::forward<Args>(args)...).forward();
	}
	
	/*
	 * The following two overloads are called in a case where the service is invalid,
	 * or is called when provided arguments don't match the constructor.
	 * In GCC, a diagnostic is provided.
	 */
	template<typename T, typename... Args>
	void service(detail::service_error<T, detail::identity_t<Args>...>, Args&&...) = delete;
	
	template<typename T, enable_if<std::is_default_constructible<detail::service_error<T>>> = 0>
	void service(detail::service_error<T> = {}) = delete;
	
	/*
	 * This function returns the result of the callable object of type U.
	 * Args are additional arguments to be sent to the function after services arguments.
	 * This function will deduce arguments from the function signature.
	 */
	template<typename Map = map<>, typename U, typename... Args,
		enable_if<detail::is_map<Map>> = 0,
		enable_if<detail::is_invoke_valid<Map, detail::decay_t<U>, Args...>> = 0>
	detail::invoke_function_result_t<Map, detail::decay_t<U>, Args...> invoke(U&& function, Args&&... args) {
		return invoke_helper<Map>(
			detail::tuple_seq_minus<detail::invoke_function_arguments_t<Map, detail::decay_t<U>, Args...>, sizeof...(Args)>{},
			std::forward<U>(function),
			std::forward<Args>(args)...
		);
	}
	
	/*
	 * This function returns the result of the callable object of type U.
	 * It will call the function with the sevices listed in the `Services` parameter pack.
	 */
	template<typename... Services, typename U, typename... Args, detail::int_t<
		enable_if<detail::is_service_valid<Services>>...,
		disable_if<detail::is_map<Services>>...,
		detail::enable_if_t<(sizeof...(Services) > 0)>> = 0>
	auto invoke(U&& function, Args&&... args) -> decltype(std::declval<U>()(std::declval<service_type<Services>>()..., std::declval<Args>()...)) {
		return std::forward<U>(function)(service<Services>()..., std::forward<Args>(args)...);
	}
	
	/*
	 * This oveload is called when the function mannot be invoked.
	 * It will provide diagnostic on GCC.
	 */
	template<typename... Services>
	void invoke(detail::not_invokable_error = {}, ...) = delete;
	
	/*
	 * This function clears this container.
	 * Every single services are invalidated after calling this function.
	 */
	inline void clear() {
		_instances.clear();
		_services.clear();
	}
	
	/*
	 * This function fork the container into a new container.
	 * The new container will have the copied state of the first container.
	 * Construction of new services within the new container will not affect the original one.
	 * The new container must exist within the lifetime of the original container.
	 * 
	 * It takes a predicate type as template argument.
	 * The default predicate is kgr::All.
	 * 
	 * This version of the function takes a predicate that is default constructible.
	 * It will call fork() with a predicate as parameter.
	 */
	template<typename Predicate = all, enable_if<std::is_default_constructible<Predicate>> = 0>
	container fork() const {
		return fork(Predicate{});
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
	container fork(Predicate predicate) const {
		container c;
		
		c._services.reserve(_services.size());
		
		std::copy_if(
			_services.begin(), _services.end(),
			std::inserter(c._services, c._services.begin()),
			[&predicate](service_cont::const_reference i) {
				return predicate(i.first);
			}
		);
		
		return c;
	}
	
	/*
	 * This function merges a container with another.
	 * The receiving container will prefer it's own instances in a case of conflicts.
	 */
	inline void merge(container&& other) {
		_instances.insert(_instances.end(), std::make_move_iterator(other._instances.begin()), std::make_move_iterator(other._instances.end()));
		_services.insert(other._services.begin(), other._services.end());
	}
	
	
	/**
	 * This function will add all services form the container sent as parameter into this one.
	 * Note that the lifetime of the container sent as parameter must be at least as long as this one.
	 * If the container you rebase from won't live long enough, consider using the merge function.
	 * 
	 * It takes a predicate type as template argument.
	 * The default predicate is kgr::All.
	 * 
	 * This version of the function takes a predicate that is default constructible.
	 * It will call rebase() with a predicate as parameter.
	 */
	template<typename Predicate = all, enable_if<std::is_default_constructible<Predicate>> = 0>
	container rebase(const container& other) const {
		return rebase(other, Predicate{});
	}
	
	/**
	 * This function will add all services form the container sent as parameter into this one.
	 * Note that the lifetime of the container sent as parameter must be at least as long as this one.
	 * If the container you rebase from won't live long enough, consider using the merge function.
	 * 
	 * It takes a predicate type as argument to filter.
	 */
	template<typename Predicate>
	void rebase(const container& other, Predicate predicate) {
		std::copy_if(
			other._services.begin(), other._services.end(),
			std::inserter(_services, _services.end()),
			[&predicate](service_cont::const_reference i) {
				return predicate(i.first);
			}
		);
	}
	
	/*
	 * This function return true if the container contains the service T.
	 * T nust be a single service.
	 */
	template<typename T, enable_if<detail::is_service<T>> = 0, enable_if<detail::is_single<T>> = 0>
	bool contains() const {
		return _services.find(type_id<T>()) != _services.end();
	}
	
private:
	///////////////////////
	//   save instance   //
	///////////////////////
	
	/*
	 * This function will create a new instance and save it.
	 * It also returns a reference to the constructed service.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		disable_if<detail::is_virtual<T>> = 0,
		disable_if<detail::is_supplied_service<T>> = 0,
		disable_if<detail::is_abstract_service<T>> = 0>
	T& save_new_instance(Args&&... args) {
		auto& service = save_instance<T>(make_service_instance<T>(std::forward<Args>(args)...));
		
		autocall(service);
		
		return service;
	}
	
	/*
	 * This function will create a new instance and save it.
	 * It also returns a reference to the constructed service.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_virtual<T>> = 0,
		disable_if<detail::is_supplied_service<T>> = 0,
		disable_if<detail::is_abstract_service<T>> = 0>
	detail::typed_service_storage<T> save_new_instance(Args&&... args) {
		auto& service = save_instance<T>(make_service_instance<T>(std::forward<Args>(args)...));
		
		autocall(service);
		
		static_assert(std::is_same<decltype(service), T&>::value, "save_instance returned a different service type than the required one!");
		
		return detail::typed_service_storage<T>{&service, get_forward<T>()};
	}
	
	/*
	 * This function is a specialization of save_new_instance for abstract classes.
	 * Since you cannot construct an abstract class, this function always throw.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_abstract_service<T>> = 0,
		disable_if<detail::has_default<T>> = 0>
	detail::typed_service_storage<T> save_new_instance(Args&&...) {
		throw abstract_not_found{};
	}
	
	/*
	 * This function is a specialization of save_new_instance for abstract classes.
	 * Since you cannot construct an abstract class, this function always throw.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_supplied_service<T>> = 0,
		disable_if<detail::is_abstract_service<T>> = 0>
	detail::conditional_t<detail::is_virtual<T>::value, detail::typed_service_storage<T>, T&> save_new_instance(Args&&...) {
		throw supplied_not_found{};
	}
	
	/*
	 * This function is a specialization of save_new_instance for abstract classes.
	 * Since that abstract service has a default service specified, we can contruct that one.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		disable_if<detail::is_supplied_service<T>> = 0,
		enable_if<detail::is_abstract_service<T>> = 0,
		enable_if<detail::has_default<T>> = 0>
	detail::typed_service_storage<T> save_new_instance(Args&&...) {
		auto&& storage = save_new_instance<detail::default_type<T>>();
		
		// The static assert is still required here, if other checks fails and allow
		// a call to this function where the default service don't overrides T, it would be UB.
		static_assert(detail::is_overriden_by<T, detail::default_type<T>>::value,
			"The default service type of an abstract service must override that abstract service."
		);
		
		return detail::typed_service_storage<T>{storage.service, get_override_forward<T, detail::default_type<T>>()};
	}
	
	/*
	 * This function will save the instance sent as arguments.
	 * It will save the instance and will save overrides if any.
	 */
	template<typename T>
	T& save_instance(contained_service_t<T> service) {
		return save_instance<T>(detail::tuple_seq<detail::parent_types<T>>{}, std::move(service));
	}
	
	/*
	 * This function implements the logic to save a service in the container.
	 * This function saves the instance and it's overrides if any.
	 */
	template<typename T, std::size_t... S>
	T& save_instance(detail::seq<S...>, contained_service_t<T> service) {
		auto& serviceRef = *service;
		
		// This codes loop over S to expand to enclosed code for each values of S.
		(void)unpack{(
			save_override<detail::meta_list_element_t<S, detail::parent_types<T>>>(serviceRef)
		, 0)..., 0};
		
		emplace_or_assign<T>(service.get(), get_forward<T>());
		_instances.emplace_back(std::move(service));
		
		return serviceRef;
	}
	
	/*
	 * This function is the iteration that saves overrides to the container.
	 * It will be called as many times as there's overrides to save.
	 */
	template<typename Override, typename T>
	void save_override(T& service) {
		static_assert(detail::is_virtual<Override>::value,
			"The overriden service must be virtual"
		);
		
		static_assert(!detail::is_final_service<Override>::value,
			"A final service cannot be overriden"
		);
		
		emplace_or_assign<Override>(&service, get_override_forward<Override, T>());
	}
	
	template<typename Override, typename T, enable_if<detail::is_virtual<T>> = 0>
	detail::forward_ptr<Override> get_override_forward() {
		return [](void* s) -> service_type<Override> {
			return static_cast<service_type<Override>>(static_cast<T*>(s)->forward());
		};
	}
	
	template<typename T, enable_if<detail::is_virtual<T>> = 0>
	detail::forward_ptr<T> get_forward() {
		return [](void* service) -> service_type<T> {
			return static_cast<T*>(service)->forward();
		};
	}
	
	template<typename T, disable_if<detail::is_virtual<T>> = 0>
	detail::forward_ptr<T> get_forward() {
		return nullptr;
	}
	
	template<typename T>
	void emplace_or_assign(void* service, detail::forward_ptr<T> forward) {
		auto it = _services.find(type_id<T>());
		
		if (it == _services.end()) {
			_services.emplace(type_id<T>(), detail::typed_service_storage<T>{service, forward});
		} else {
			it->second = detail::typed_service_storage<T>{service, forward};
		}
	}
	
	///////////////////////
	//   make instance   //
	///////////////////////
	
	/*
	 * This function creates an instance of a service.
	 * It forward the work to make_service_instance_helper with an integer sequence.
	 */
	template<typename T, typename... Args>
	contained_service_t<T> make_service_instance(Args&&... args) {
		return make_service_instance_helper<T>(detail::construct_result_seq<T, Args...>{}, std::forward<Args>(args)...);
	}
	
	/*
	 * This function is the helper for make_service_instance.
	 * It construct the service using the values returned by construct.
	 * It forward it's work to make_contained_service.
	 */
	template<typename T, typename... Args, std::size_t... S>
	contained_service_t<T> make_service_instance_helper(detail::seq<S...>, Args&&... args) {
		auto constructArgs = invoke_raw(detail::construct_function<T, Args...>::value, std::forward<Args>(args)...);
		
		// This line is used to shut unused-variable warning, since S can be empty.
		static_cast<void>(constructArgs);
		
		return make_contained_service<T>(std::forward<detail::tuple_element_t<S, decltype(constructArgs)>>(std::get<S>(constructArgs))...);
	}
	
	/*
	 * This function create a service with the received arguments.
	 * It creating it in the right type for the container to contain it in it's container.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_someway_constructible<T, in_place_t, Args...>> = 0>
	contained_service_t<T> make_contained_service(Args&&... args) {
		return make_instance_ptr<T>(detail::in_place, std::forward<Args>(args)...);
	}
	
	/*
	 * This function create a service with the received arguments.
	 * It creating it in the right type for the container return it and inject it without overhead.
	 */
	template<typename T, typename... Args,
		disable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_someway_constructible<T, in_place_t, Args...>> = 0>
	contained_service_t<T> make_contained_service(Args&&... args) {
		return T{detail::in_place, std::forward<Args>(args)...};
	}
	
	/*
	 * This function create a service with the received arguments.
	 * It creating it in the right type for the container to contain it in it's container.
	 * This version of the function is called when the service definition has no valid constructor.
	 * It will try to call an emplace function that construct the service in a lazy way.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		disable_if<detail::is_someway_constructible<T, in_place_t, Args...>> = 0,
		enable_if<detail::is_emplaceable<T, Args...>> = 0>
	contained_service_t<T> make_contained_service(Args&&... args) {
		auto service = make_instance_ptr<T>();
		
		service->emplace(std::forward<Args>(args)...);
		
		return service;
	}
	
	/*
	 * This function create a service with the received arguments.
	 * It creating it in the right type for the container return it and inject it without overhead.
	 * This version of the function is called when the service definition has no valid constructor.
	 * It will try to call an emplace function that construct the service in a lazy way.
	 */
	template<typename T, typename... Args,
		disable_if<detail::is_single<T>> = 0,
		disable_if<detail::is_someway_constructible<T, in_place_t, Args...>> = 0,
		enable_if<detail::is_emplaceable<T, Args...>> = 0>
	contained_service_t<T> make_contained_service(Args&&... args) {
		T service;
		
		service.emplace(std::forward<Args>(args)...);
		
		return service;
	}
	
	///////////////////////
	//      service      //
	///////////////////////
	
	/*
	 * This function call service using the service map.
	 * This function is called when the service map `Map` is valid for a given `T`
	 */
	template<typename Map, typename T, enable_if<detail::is_complete_map<Map, T>> = 0>
	auto mapped_service() -> decltype(service<service_map_t<T, Map>>()) {
		return service<service_map_t<T, Map>>();
	}
	
	///////////////////////
	//    definition     //
	///////////////////////
	
	/*
	 * This function returns a service definition.
	 * This version of this function create the service each time it is called.
	 */
	template<typename T, typename... Args, disable_if<detail::is_single<T>> = 0, disable_if<detail::is_container_service<T>> = 0>
	detail::injected_wrapper<T> definition(Args&&... args) {
		auto service = make_service_instance<T>(std::forward<Args>(args)...);
		
		autocall(service);
		
		return detail::injected_wrapper<T>{std::move(service)};
	}
	
	/*
	 * This function returns a service definition.
	 * This version of this function is specific to a container service.
	 */
	template<typename T, enable_if<detail::is_container_service<T>> = 0>
	detail::injected_wrapper<T> definition() {
		return detail::injected<container_service>{container_service{*this}};
	}
	
	/*
	 * This function returns a service definition.
	 * This version of this function create the service if it was not created before.
	 * It is called when getting a service definition for a single, non virtual service
	 */
	template<typename T,
		enable_if<detail::is_single<T>> = 0,
		disable_if<detail::is_container_service<T>> = 0>
	detail::injected_wrapper<T> definition() {
		auto it = _services.find(type_id<T>());
		
		if (it != _services.end()) {
			return make_wrapper<T>(it->second);
		} else {
			return detail::injected_wrapper<T>{save_new_instance<T>()};
		}
	}
	
	///////////////////////
	//      invoke       //
	///////////////////////
	
	/*
	 * This function is an helper for the public invoke function.
	 * It unpacks arguments of the function with an integer sequence.
	 */
	template<typename Map, typename U, typename... Args, std::size_t... S>
	detail::invoke_function_result_t<Map, detail::decay_t<U>, Args...> invoke_helper(detail::seq<S...>, U&& function, Args&&... args) {
		return std::forward<U>(function)(
			mapped_service<Map, detail::invoke_function_argument_t<S, Map, detail::decay_t<U>, Args...>>()...,
			std::forward<Args>(args)...
		);
	}
	
	/*
	 * This function is the same as invoke but it sends service definitions instead of the service itself.
	 * It is called with some autocall function and the make_service_instance function.
	 */
	template<typename U, typename... Args>
	detail::function_result_t<U> invoke_raw(U function, Args&&... args) {
		return invoke_raw_helper(
			detail::tuple_seq_minus<detail::function_arguments_t<detail::decay_t<U>>, sizeof...(Args)>{},
			function,
			std::forward<Args>(args)...
		);
	}
	
	/*
	 * This function is an helper of the invoke_raw function.
	 * It unpacks arguments of the function U with an integer sequence.
	 */
	template<typename U, typename... Args, std::size_t... S>
	detail::function_result_t<U> invoke_raw_helper(detail::seq<S...>, U function, Args&&... args) {
		return function(definition<detail::injected_argument_t<S, U>>()..., std::forward<Args>(args)...);
	}
	
	///////////////////////
	//     autocall      //
	///////////////////////
	
	/*
	 * This function starts the iteration (autocall_helper).
	 */
	template<typename T, enable_if<detail::has_autocall<T>> = 0>
	void autocall(T& service) {
		autocall(detail::tuple_seq<typename T::autocall_functions>{}, service);
	}
	
	/*
	 * This function is the iteration for autocall.
	 */
	template<typename T, std::size_t... S, enable_if<detail::has_autocall<T>> = 0>
	void autocall(detail::seq<S...>, T& service) {
		(void)unpack{(invoke_autocall(
			detail::function_seq<detail::autocall_nth_function_t<T, S>>{}, service, detail::autocall_nth_function<T, S>::value
		), 0)..., 0};
	}
	
	/*
	 * This function is the invoke_autocall that take the method to invoke as parameter.
	 * It invokes the function sent as parameter.
	 */
	template<typename T, typename F, std::size_t... S>
	void invoke_autocall(detail::seq<S...>, T& service, F function) {
		invoke_raw([&service, &function](detail::function_argument_t<S, F>... args) {
			(service.*function)(std::forward<decltype(args)>(args)...);
		});
	}
	
	/*
	 * This function is called when there is no autocall to do.
	 */
	template<typename T, disable_if<detail::has_autocall<T>> = 0>
	void autocall(T&) {}
	
	///////////////////////
	//     instances     //
	///////////////////////
	
	instance_cont _instances;
	service_cont _services;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_CONTAINER_HPP
