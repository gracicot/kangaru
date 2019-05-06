#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP

#include "utils.hpp"
#include "single.hpp"
#include "traits.hpp"
#include "service_storage.hpp"

namespace kgr {
namespace detail {

/*
 * This class is a non virtual service being injected.
 * The service injected in this is either a single or a non-single.
 * 
 * When T is a reference type, the service definition T is a single.
 */
template<typename T>
struct injected {
private:
	using Type = conditional_t<is_single<T>::value, T&, T>;
	
public:
	// TODO: kangaru 5: use () parens contructor by default
	template<typename... Args, enable_if_t<is_brace_constructible<Type, Args...>::value, int> = 0>
	explicit injected(Args&&... args) noexcept(noexcept(Type{std::forward<Args>(args)...})) : _service{std::forward<Args>(args)...} {}
	
	template<typename... Args, enable_if_t<!is_brace_constructible<Type, Args...>::value && std::is_constructible<Type, Args...>::value, int> = 0>
	explicit injected(Args&&... args) noexcept(std::is_nothrow_constructible<Type, Args...>::value) : _service(std::forward<Args>(args)...) {}
	
	service_type<T> forward() {
		return _service.forward();
	}
	
private:
	Type _service;
};

/*
 * This class is a virtual service being injected.
 * It contains a pointer to the service and it's forward function.
 */
template<typename T>
struct virtual_injected {
	explicit virtual_injected(void* service, forward_ptr<T> f) noexcept : _service{service}, _forward{f} {}
	explicit virtual_injected(const typed_service_storage<T>& storage) noexcept : _service{storage.service}, _forward{storage.forward} {}
	
	service_type<T> forward() {
		return _forward(_service);
	}
	
private:
	void* _service;
	forward_ptr<T> _forward;
};

/*
 * This class is a simple type trait, that receive an injected service type,
 * and extract the definition type.
 */
template<typename>
struct injected_service {};

/*
 * This is for non-virtual services.
 */
template<typename T>
struct injected_service<injected<T>&&> {
	using type = T;
};

/*
 * This is for virtual services.
 */
template<typename T>
struct injected_service<virtual_injected<T>&&> {
	using type = T;
};

/*
 * This is an alias for the typedef in injected_service
 */
template<typename T>
using injected_service_t = typename injected_service<T>::type;

/*
 * This is the contrary of injected_service.
 * We get a service definition type, and we return the wrapper type.
 * We cannot use conditional_t here, since it makes MSVC to crash when instanciating construct functions.
 */
template<typename T>
using injected_wrapper = typename std::conditional<is_polymorphic<T>::value,
	virtual_injected<T>,
	injected<T>
>::type;

template<std::size_t n, typename F>
using injected_argument_t = injected_service_t<function_argument_t<n, F>>;

template<typename... Ts>
struct inject_result_helper {
	using type = std::tuple<typename remove_rvalue_reference<Ts>::type...>;
};

template<typename, typename>
struct single_insertion_result;

template<typename T, typename... Parents>
struct single_insertion_result<T, meta_list<Parents...>> {
	using type = std::tuple<detail::typed_service_storage<T>, detail::typed_service_storage<Parents>...>;
};

template<typename T>
using single_insertion_result_t = typename single_insertion_result<T, parent_types<T>>::type;

} // namespace detail

/*
 * This is an injected argument type in construct functions.
 */
template<typename T>
using inject_t = detail::injected_wrapper<T>&&;

/*
 * The function makes a tuple of either l-value reference, or objects.
 * The is because non-single service don't live past the statement that `Definition::construct` is called.
 * But we need the non-singles to live past that, so they are moved into the tuple of injected arguments.
 * 
 * Single service live as long as the container lives. They can, and should be l-value references.
 */
template<typename... Args>
constexpr auto inject(Args&&... args) -> std::tuple<detail::remove_rvalue_reference_t<Args>...> {
	return std::tuple<detail::remove_rvalue_reference_t<Args>...>{std::forward<Args>(args)...};
}

/*
 * Alias to help write a return type for construct functions.
 * Yield the return type of inject(Ts...)
 */
template<typename... Ts>
using inject_result = typename detail::inject_result_helper<Ts...>::type;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
