#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP

#include "utils.hpp"
#include "single.hpp"
#include "traits.hpp"

namespace kgr {

struct Container;

namespace detail {

template<typename T, typename = void>
struct BaseVirtualInjectedHelper {
	virtual ~BaseVirtualInjectedHelper() = default;
	kgr::detail::Sink forward();
};

template<typename T>
struct BaseVirtualInjectedHelper<T, void_t<ServiceType<T>>> {
	virtual ~BaseVirtualInjectedHelper() = default;
	virtual ServiceType<T> forward() = 0;
};

template<typename T>
struct BaseVirtualInjected : BaseVirtualInjectedHelper<T> {};

template<typename T>
struct VirtualInjected final : BaseVirtualInjected<T> {
	friend struct kgr::Container;
	
	template<typename... Args, enable_if_t<is_brace_constructible<T, Args...>::value, int> = 0>
	explicit VirtualInjected(Args&&... args) : _service{std::forward<Args>(args)...} {}
	
	template<typename... Args, enable_if_t<!is_brace_constructible<T, Args...>::value && std::is_constructible<T, Args...>::value, int> = 0>
	explicit VirtualInjected(Args&&... args) : _service(std::forward<Args>(args)...) {}
	
	ServiceType<T> forward() override {
		return _service.forward();
	}
	
private:
	T& get() {
		return _service;
	}
	
	const T& get() const {
		return _service;
	}
	
	T _service;
};

template<typename Original, typename Service>
struct ServiceOverride final : BaseVirtualInjected<Service> {
	explicit ServiceOverride(Original& service) : _service{service} {}
	
	ServiceType<Service> forward() override {
		return static_cast<ServiceType<Service>>(_service.forward());
	}
	
private:
	Original& _service;
};

template<typename T>
struct Injected final {
	friend struct kgr::Container;
	
	template<typename... Args, enable_if_t<is_brace_constructible<T, Args...>::value, int> = 0>
	explicit Injected(Args&&... args) : _service{std::forward<Args>(args)...} {}
	
	template<typename... Args, enable_if_t<!is_brace_constructible<T, Args...>::value && std::is_constructible<T, Args...>::value, int> = 0>
	explicit Injected(Args&&... args) : _service(std::forward<Args>(args)...) {}
	
	ServiceType<T> forward() {
		return _service.forward();
	}
	
private:
	T& get() {
		return _service;
	}
	
	const T& get() const {
		return _service;
	}
	
	T _service;
};

template<typename>
struct original;

template<typename T>
struct original<BaseVirtualInjected<T>&> {
	using type = T;
};

template<typename T>
struct original<Injected<T>&&> {
	using type = T;
};

template<typename T>
struct original<Injected<T>&> {
	using type = T;
};

template<typename T>
using original_t = typename original<T>::type;

template<std::size_t n, typename F>
using injected_argument_t = original_t<function_argument_t<n, F>>;

template<typename T>
using injected_wrapper_t = typename std::conditional<is_single<T>::value && is_virtual<T>::value,
	BaseVirtualInjected<T>, Injected<T>
>::type;

template<typename T>
using injected_concrete_t = typename std::conditional<is_single<T>::value && is_virtual<T>::value,
	VirtualInjected<T>, Injected<T>
>::type;

} // namespace detail

template<typename T>
using Inject = typename std::conditional<!detail::is_service<T>::value || detail::is_single<T>::value,
	detail::injected_wrapper_t<T>&,
	detail::injected_wrapper_t<T>&&>::type;

template<typename... Args>
std::tuple<detail::remove_rvalue_reference_t<Args>...> inject(Args&&... args) {
	return std::tuple<detail::remove_rvalue_reference_t<Args>...>{std::forward<Args>(args)...};
}

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
