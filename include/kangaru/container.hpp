#ifndef KGR_KANGARU_INCLUDE_KANGARU_CONTAINER_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_CONTAINER_HPP

#include "detail/default_source.hpp"
#include "detail/traits.hpp"
#include "detail/validity_check.hpp"
#include "detail/utils.hpp"
#include "detail/container_service.hpp"
#include "detail/autocall_traits.hpp"
#include "detail/single.hpp"
#include "detail/exception.hpp"
#include "detail/service_storage.hpp"
#include "detail/injected.hpp"
#include "detail/error.hpp"
#include "predicate.hpp"

#include <unordered_map>
#include <memory>
#include <type_traits>

namespace kgr {

/**
 * The kangaru container class.
 * 
 * This class will construct services and share single instances for a given definition.
 * It is the class that parses and manage dependency graphs and calls autocall functions.
 */
struct container : private detail::default_source {
private:
	template<typename Condition, typename T = int> using enable_if = detail::enable_if_t<Condition::value, T>;
	template<typename Condition, typename T = int> using disable_if = detail::enable_if_t<!Condition::value, T>;
	template<typename T> using contained_service_t = typename std::conditional<
		detail::is_single<T>::value,
		T&,
		T>::type;
	using unpack = int[];
	
	auto source() const noexcept -> default_source const& {
		return static_cast<default_source const&>(*this);
	}
	
	auto source() noexcept -> default_source& {
		return static_cast<default_source&>(*this);
	}
	
	template<typename T, enable_if<detail::is_polymorphic<T>> = 0>
	static detail::injected_wrapper<T> make_wrapper(storage_t& instance) {
		return detail::injected_wrapper<T>{instance.cast<T>()};
	}
	
	template<typename T, disable_if<detail::is_polymorphic<T>> = 0>
	static detail::injected_wrapper<T> make_wrapper(storage_t& instance) {
		return detail::injected_wrapper<T>{instance.service<T>()};
	}
	
	explicit container(default_source&& source) : default_source{std::move(source)} {}
	
public:
	explicit container() = default;
	container(container const&) = delete;
	container& operator=(container const&) = delete;
	container(container&&) = default;
	container& operator=(container&&) = default;
	
	/*
	 * This function construct and save in place a service definition with the provided arguments.
	 * The service is only constructed if it is not found.
	 * It is usually used to instanciate supplied services.
	 * It returns if the service has been constructed.
	 * This function require the service to be single.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_emplace_valid<T, Args...>> = 0>
	bool emplace(Args&&... args) {
		// TODO: We're doing two search in the map: One before constructing the service and one to insert. We should do only one.
		return contains<T>() ? false : (autocall(make_service_instance<T>(std::forward<Args>(args)...)), true);
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
		enable_if<detail::is_emplace_valid<T, Args...>> = 0>
	void replace(Args&&... args) {
		autocall(make_service_instance<T>(std::forward<Args>(args)...));
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
	auto service(Args&&... args) -> service_type<T> {
		return definition<T>(std::forward<Args>(args)...).forward();
	}
	
	/*
	 * The following two overloads are called in a case where the service is invalid,
	 * or is called when provided arguments don't match the constructor.
	 * In GCC, a diagnostic is provided.
	 */
	template<typename T, typename... Args>
	auto service(detail::service_error<T, detail::identity_t<Args>...>, Args&&...) -> detail::sink = delete;
	
	template<typename T, enable_if<std::is_default_constructible<detail::service_error<T>>> = 0>
	auto service(detail::service_error<T> = {}) -> detail::sink = delete;

	/*
	 * This function returns the result of the callable object of type U.
	 * Args are additional arguments to be sent to the function after services arguments.
	 * This function will deduce arguments from the function signature.
	 */
	template<typename Map = map<>, typename U, typename... Args,
		enable_if<detail::is_map<Map>> = 0,
		enable_if<detail::is_invoke_valid<Map, detail::decay_t<U>, Args...>> = 0>
	auto invoke(Map, U&& function, Args&&... args) -> detail::invoke_function_result_t<Map, detail::decay_t<U>, Args...> {
		return invoke_helper<Map>(
			detail::tuple_seq_minus<detail::invoke_function_arguments_t<Map, detail::decay_t<U>, Args...>, sizeof...(Args)>{},
			std::forward<U>(function),
			std::forward<Args>(args)...
		);
	}
	
	/*
	 * This function returns the result of the callable object of type U.
	 * Args are additional arguments to be sent to the function after services arguments.
	 * This function will deduce arguments from the function signature.
	 */
	template<typename Map = map<>, typename U, typename... Args,
		enable_if<detail::is_map<Map>> = 0,
		enable_if<detail::is_invoke_valid<Map, detail::decay_t<U>, Args...>> = 0>
	auto invoke(U&& function, Args&&... args) -> detail::invoke_function_result_t<Map, detail::decay_t<U>, Args...> {
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
	template<typename First, typename... Services, typename U, typename... Args, enable_if<detail::conjunction<
		detail::is_service_valid<First>,
		detail::is_service_valid<Services>...>> = 0>
	auto invoke(U&& function, Args&&... args)
		-> detail::call_result_t<U, service_type<First>, service_type<Services>..., Args...>
	{
		return std::forward<U>(function)(service<First>(), service<Services>()..., std::forward<Args>(args)...);
	}
	
	/*
	 * This oveload is called when the function mannot be invoked.
	 * It will provide diagnostic on GCC.
	 */
	template<typename... Services>
	auto invoke(detail::not_invokable_error = {}, ...) -> detail::sink = delete;
	
		
	/*
	 * This function clears this container.
	 * Every single services are invalidated after calling this function.
	 */
	inline void clear() {
		source().clear();
	}
	
	/*
	 * This function fork the container into a new container.
	 * The new container will have the copied state of the first container.
	 * Construction of new services within the new container will not affect the original one.
	 * The new container must exist within the lifetime of the original container.
	 * 
	 * It takes a predicate type as template argument.
	 * The default predicate is kgr::all.
	 * 
	 * This version of the function takes a predicate that is default constructible.
	 * It will call fork() with a predicate as parameter.
	 */
	template<typename Predicate = all, detail::enable_if_t<std::is_default_constructible<Predicate>::value, int> = 0>
	auto fork() const -> container {
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
	auto fork(Predicate predicate) const -> container {
		return container{source().fork(predicate)};
	}
	
	/*
	 * This function merges a container with another.
	 * The receiving container will prefer it's own instances in a case of conflicts.
	 */
	inline void merge(container&& other) {
		source().merge(std::move(other.source()));
	}
	
	/*
	 * This function merges a container with another.
	 * The receiving container will prefer it's own instances in a case of conflicts.
	 * 
	 * This function consumes the container `other`
	 */
	inline void merge(container& other) {
		merge(std::move(other));
	}
	
	
	/**
	 * This function will add all services form the container sent as parameter into this one.
	 * Note that the lifetime of the container sent as parameter must be at least as long as this one.
	 * If the container you rebase from won't live long enough, consider using the merge function.
	 * 
	 * It takes a predicate type as template argument.
	 * The default predicate is kgr::all.
	 * 
	 * This version of the function takes a predicate that is default constructible.
	 * It will call rebase() with a predicate as parameter.
	 */
	template<typename Predicate = all, detail::enable_if_t<std::is_default_constructible<Predicate>::value, int> = 0>
	void rebase(const container& other) {
		rebase(other, Predicate{});
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
		source().rebase(other.source(), predicate);
	}
	
	/*
	 * This function return true if the container contains the service T. Returns false otherwise.
	 * T nust be a single service.
	 */
	template<typename T, detail::enable_if_t<detail::is_service<T>::value && detail::is_single<T>::value, int> = 0>
	bool contains() const {
		return source().contains<T>();
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
		disable_if<detail::is_polymorphic<T>> = 0,
		disable_if<detail::is_supplied_service<T>> = 0,
		disable_if<detail::is_abstract_service<T>> = 0>
	auto save_new_instance(Args&&... args) -> T& {
		auto& service = make_service_instance<T>(std::forward<Args>(args)...);
		
		autocall(service);
		
		return service;
	}
	
	/*
	 * This function will create a new instance and save it.
	 * It also returns a reference to the constructed service.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_polymorphic<T>> = 0,
		disable_if<detail::is_supplied_service<T>> = 0,
		disable_if<detail::is_abstract_service<T>> = 0>
	auto save_new_instance(Args&&... args) -> detail::typed_service_storage<T> {
		auto& service = make_service_instance<T>(std::forward<Args>(args)...);
		
		autocall(service);
		
		static_assert(
			std::is_same<decltype(service), T&>::value,
			"save_instance returned a different service type than the required one!"
		);
		
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
	auto save_new_instance(Args&&...) -> detail::typed_service_storage<T> {
		KGR_KANGARU_THROW(abstract_not_found{});
	}
	
	/*
	 * This function is a specialization of save_new_instance for abstract classes.
	 * Since you cannot construct an abstract class, this function always throw.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_supplied_service<T>> = 0,
		disable_if<detail::is_abstract_service<T>> = 0>
	auto save_new_instance(Args&&...)
		-> detail::conditional_t<detail::is_polymorphic<T>::value, detail::typed_service_storage<T>, T&>
	{
		KGR_KANGARU_THROW(supplied_not_found{});
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
	auto save_new_instance(Args&&...) -> detail::typed_service_storage<T> {
		auto&& storage = save_new_instance<detail::default_type<T>>();
		
		// The static assert is still required here, if other checks fails and allow
		// a call to this function where the default service don't overrides T, it would be UB.
		static_assert(detail::is_overriden_by<T, detail::default_type<T>>::value,
			"The default service type of an abstract service must override that abstract service."
		);
		
		return detail::typed_service_storage<T>{storage.service, get_override_forward<T, detail::default_type<T>>()};
	}
	
	// TODO: find a way to move these to the source.
	template<typename Override, typename T, enable_if<detail::is_polymorphic<T>> = 0>
	auto get_override_forward() -> detail::forward_ptr<Override> {
		return [](default_source::alias_t s) -> service_type<Override> {
			return static_cast<service_type<Override>>(static_cast<T*>(s)->forward());
		};
	}
	
	template<typename T, enable_if<detail::is_polymorphic<T>> = 0>
	auto get_forward() -> detail::forward_ptr<T> {
		return [](default_source::alias_t service) -> service_type<T> {
			return static_cast<T*>(service)->forward();
		};
	}
	
	template<typename T, disable_if<detail::is_polymorphic<T>> = 0>
	auto get_forward() -> detail::forward_ptr<T> {
		return nullptr;
	}
	
	///////////////////////
	//   make instance   //
	///////////////////////
	
	/*
	 * This function creates an instance of a service.
	 * It forward the work to make_service_instance_helper with an integer sequence.
	 */
	template<typename T, typename... Args>
	auto make_service_instance(Args&&... args) -> contained_service_t<T> {
		return make_service_instance_helper<T>(
			detail::construct_result_seq<T, Args...>{},
			std::forward<Args>(args)...
		);
	}
	
	/*
	 * This function is the helper for make_service_instance.
	 * It construct the service using the values returned by construct.
	 * It forward it's work to make_contained_service.
	 */
	template<typename T, typename... Args, std::size_t... S>
	auto make_service_instance_helper(detail::seq<S...>, Args&&... args) -> contained_service_t<T> {
		auto construct_args = invoke_definition<detail::construct_function<T, Args...>>(std::forward<Args>(args)...);
		
		// This line is used to shut unused-variable warning, since S can be empty.
		static_cast<void>(construct_args);
		
		return make_contained_service<T>(
			std::forward<detail::tuple_element_t<S, decltype(construct_args)>>(std::get<S>(construct_args))...
		);
	}
	
	/*
	 * This function create a service with the received arguments.
	 * It creating it in the right type for the container to contain it in it's container.
	 */
	template<typename T, typename... Args,
		enable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_someway_constructible<T, in_place_t, Args...>> = 0>
	auto make_contained_service(Args&&... args) -> contained_service_t<T> {
		auto storage = source().emplace<T>(get_forward<T>(), detail::in_place, std::forward<Args>(args)...);
		save_overrides<T>(detail::tuple_seq<detail::parent_types<T>>{}, storage.first);
		return storage.second.template service<T>();
	}
	
	/*
	 * This function create a service with the received arguments.
	 * It creating it in the right type for the container return it and inject it without overhead.
	 */
	template<typename T, typename... Args,
		disable_if<detail::is_single<T>> = 0,
		enable_if<detail::is_someway_constructible<T, in_place_t, Args...>> = 0>
	auto make_contained_service(Args&&... args) -> contained_service_t<T> {
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
	auto make_contained_service(Args&&... args) -> contained_service_t<T> {
		auto storage = source().emplace<T>(get_forward<T>());
		auto& service = storage.second.template service<T>();
		
		service.emplace(std::forward<Args>(args)...);
		save_overrides<T>(detail::tuple_seq<detail::parent_types<T>>{}, storage.first);
		
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
	auto make_contained_service(Args&&... args) -> contained_service_t<T> {
		T service;
		
		service.emplace(std::forward<Args>(args)...);
		
		return service;
	}
	
	/*
	 * This function implements the logic to save a service in the container.
	 * This function saves the instance and it's overrides if any.
	 */
	template<typename T, std::size_t... S>
	void save_overrides(detail::seq<S...>, default_source::alias_t service) {
		// if the S sequence is empty, service is an unused variable. This line silence it.
		static_cast<void>(service);
		
		// This codes loop over S to expand to enclosed code for each values of S.
		(void)unpack{(
			save_override<detail::meta_list_element_t<S, detail::parent_types<T>>, T>(service)
		, 0)..., 0};
	}
	
	/*
	 * This function is the iteration that saves overrides to the container.
	 * It will be called as many times as there's overrides to save.
	 */
	template<typename Override, typename T>
	void save_override(default_source::alias_t service) {
		static_assert(detail::is_polymorphic<Override>::value,
			"The overriden service must be virtual"
		);
		
		static_assert(!detail::is_final_service<Override>::value,
			"A final service cannot be overriden"
		);
		
		source().override<Override>(get_override_forward<Override, T>(), service);
	}
	
	///////////////////////
	//      service      //
	///////////////////////
	
	/*
	 * This function call service using the service map.
	 * This function is called when the service map `Map` is valid for a given `T`
	 */
	template<typename Map, typename T,
		enable_if<detail::is_complete_map<Map, T>> = 0,
		enable_if<detail::is_service_valid<detail::detected_t<mapped_service_t, T, Map>>> = 0>
	auto mapped_service() -> service_type<mapped_service_t<T, Map>> {
		return service<mapped_service_t<T, Map>>();
	}
	
	///////////////////////
	//    definition     //
	///////////////////////
	
	/*
	 * This function returns a service definition.
	 * This version of this function create the service each time it is called.
	 */
	template<typename T, typename... Args,
		disable_if<detail::is_single<T>> = 0,
		disable_if<detail::is_container_service<T>> = 0>
	auto definition(Args&&... args) -> detail::injected_wrapper<T> {
		auto service = make_service_instance<T>(std::forward<Args>(args)...);
		
		autocall(service);
		
		return detail::injected_wrapper<T>{std::move(service)};
	}
	
	/*
	 * This function returns a service definition.
	 * This version of this function is specific to a container service.
	 */
	template<typename T, enable_if<detail::is_container_service<T>> = 0>
	auto definition() -> detail::injected_wrapper<T> {
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
	auto definition() -> detail::injected_wrapper<T> {
		return source().find<T>(
			[](default_source::storage_t storage) { return make_wrapper<T>(storage); },
			[this]{ return detail::injected_wrapper<T>{save_new_instance<T>()}; }
		);
	}
	
	///////////////////////
	//      invoke       //
	///////////////////////
	
	/*
	 * This function is an helper for the public invoke function.
	 * It unpacks arguments of the function with an integer sequence.
	 */
	template<typename Map, typename U, typename... Args, std::size_t... S>
	auto invoke_helper(detail::seq<S...>, U&& function, Args&&... args)
		-> detail::invoke_function_result_t<Map, detail::decay_t<U>, Args...>
	{
		return std::forward<U>(function)(
			mapped_service<Map, detail::invoke_function_argument_t<S, Map, detail::decay_t<U>, Args...>>()...,
			std::forward<Args>(args)...
		);
	}
	
	/*
	 * This function is the same as invoke but it sends service definitions instead of the service itself.
	 * It is called with some autocall function and the make_service_instance function.
	 */
	template<typename U, typename... Args, typename F = typename U::value_type>
	auto invoke_definition(Args&&... args) -> detail::function_result_t<F> {
		return invoke_definition_helper<U>(
			detail::tuple_seq_minus<detail::function_arguments_t<F>, sizeof...(Args)>{},
			std::forward<Args>(args)...
		);
	}
	
	/*
	 * This function is an helper of the invoke_definition function.
	 * It unpacks arguments of the function U with an integer sequence.
	 */
	template<typename U, typename... Args, std::size_t... S, typename F = typename U::value_type>
	auto invoke_definition_helper(detail::seq<S...>, Args&&... args) -> detail::function_result_t<F> {
		return U::value(definition<detail::injected_argument_t<S, F>>()..., std::forward<Args>(args)...);
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
		(void)unpack{(void(
			invoke_definition<detail::autocall_nth_function<T, S>>(service)
		), 0)..., 0};
	}
	
	/*
	 * This function is called when there is no autocall to do.
	 */
	template<typename T, disable_if<detail::has_autocall<T>> = 0>
	void autocall(T&) {}
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_CONTAINER_HPP
