#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP

#include "utils.hpp"
#include "single.hpp"
#include "traits.hpp"

namespace kgr {
namespace detail {

/*
 * This defines the pointer to a forward function.
 * This is simply a shortcut for not writing the function pointer type everywhere.
 */
template<typename T>
using forward_ptr = ServiceType<T>(*)(void*);

/*
 * This class is a non virtual service being injected.
 * The service injected in this is either a single or a non-single.
 * 
 * When T is a reference type, the service definition T is a single.
 */
template<typename T>
struct Injected final {
	template<typename... Args, enable_if_t<is_brace_constructible<T, Args...>::value, int> = 0>
	explicit Injected(Args&&... args) : _service{std::forward<Args>(args)...} {}
	
	template<typename... Args, enable_if_t<!is_brace_constructible<T, Args...>::value && std::is_constructible<T, Args...>::value, int> = 0>
	explicit Injected(Args&&... args) : _service(std::forward<Args>(args)...) {}
	
	ServiceType<decay_t<T>> forward() {
		return _service.forward();
	}
	
private:
	T _service;
};

/*
 * This class is a virtual service being injected.
 * It contains a pointer to the service and it's forward function.
 */
template<typename T>
struct VirtualInjected final {
	explicit VirtualInjected(void* service, forward_ptr<T> f) noexcept : _service{service}, _forward{f} {}
	explicit VirtualInjected(std::pair<void*, forward_ptr<T>> data) noexcept : _service{data.first}, _forward{data.second} {}
	
	ServiceType<T> forward() {
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
struct injected_service;

/*
 * This is for non-single, non-virtual services.
 */
template<typename T>
struct injected_service<Injected<T>&&> {
	using type = T;
};

/*
 * This is for non-virtual single services.
 */
template<typename T>
struct injected_service<Injected<T&>&&> {
	using type = T;
};

/*
 * This is for vritual services.
 */
template<typename T>
struct injected_service<VirtualInjected<T>&&> {
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
 */
template<typename T>
using injected_wrapper = typename std::conditional<detail::is_service<T>::value && !detail::is_single<T>::value,
	detail::Injected<T>,
	typename std::conditional<detail::is_virtual<T>::value,
		detail::VirtualInjected<T>,
		detail::Injected<T&>
	>::type
>::type;

template<std::size_t n, typename F>
using injected_argument_t = injected_service_t<function_argument_t<n, F>>;

} // namespace detail

/*
 * This is an injected argument type in construct functions.
 */
template<typename T>
using Inject = detail::injected_wrapper<T>&&;

/*
 * The function makes a tuple of either l-value reference, or objects.
 * The is because non-single service don't live past the statement that `Definition::construct` is called.
 * But we need the non-singles to live past that, so they are moved into the tuple of injected arguments.
 * 
 * Single service live as long as the container lives. They can, and should be l-value references.
 */
template<typename... Args>
std::tuple<detail::remove_rvalue_reference_t<Args>...> inject(Args&&... args) {
	return std::tuple<detail::remove_rvalue_reference_t<Args>...>{std::forward<Args>(args)...};
}

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
