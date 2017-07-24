#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP

#include "utils.hpp"
#include "single.hpp"
#include "traits.hpp"

namespace kgr {
namespace detail {

template<typename T>
using forward_ptr = ServiceType<T>(*)(void*);

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

template<typename T>
struct VirtualInjected final {
	explicit VirtualInjected(void* service, forward_ptr<T> f) noexcept : _service{service}, _forward{f} {}
	explicit VirtualInjected(std::pair<void*, forward_ptr<T>> data) noexcept : _service{data.first}, _forward{data.second} {}
	
	ServiceType<T> forward() {
		return _forward(_service);
	}
	
private:
	void* _service;
	forward_ptr<T> _forward;;
};

template<typename>
struct injected_service;

template<typename T>
struct injected_service<Injected<T>&&> {
	using type = T;
};

template<typename T>
struct injected_service<Injected<T&>&&> {
	using type = T;
};

template<typename T>
struct injected_service<VirtualInjected<T>&&> {
	using type = T;
};

template<typename T>
using injected_service_t = typename injected_service<T>::type;

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

template<typename T>
using Inject = detail::injected_wrapper<T>&&;

template<typename... Args>
std::tuple<detail::remove_rvalue_reference_t<Args>...> inject(Args&&... args) {
	return std::tuple<detail::remove_rvalue_reference_t<Args>...>{std::forward<Args>(args)...};
}

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
