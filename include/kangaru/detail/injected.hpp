#ifndef KGR_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_INJECTED_HPP

#include "utils.hpp"
#include "single.hpp"
#include "traits.hpp"

namespace kgr {

struct Container;

namespace detail {

template<typename T>
struct BaseInjected {
	virtual ~BaseInjected() {}
	virtual ServiceType<T> forward() = 0;
};

template<typename T>
struct SingleInjected final : BaseInjected<T> {
	friend struct kgr::Container;
	
	template<typename... Args, enable_if_t<is_brace_constructible<T, Args...>::value, int> = 0>
	explicit SingleInjected(Args&&... args) : _service{std::forward<Args>(args)...} {}
	
	template<typename... Args, enable_if_t<!is_brace_constructible<T, Args...>::value && std::is_constructible<T, Args...>::value, int> = 0>
	explicit SingleInjected(Args&&... args) : _service(std::forward<Args>(args)...) {}
	
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
struct ServiceOverride final : BaseInjected<Service> {
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

} // namespace detail

template<typename T>
using Inject = typename std::conditional<!detail::is_service<T>::value || detail::is_single<T>::value,
	detail::BaseInjected<T>&,
	detail::Injected<T>&&>::type;

template<typename... Args>
std::tuple<detail::remove_rvalue_reference_t<Args>...> inject(Args&&... args) {
	return std::tuple<detail::remove_rvalue_reference_t<Args>...>{std::forward<Args>(args)...};
}

} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_INJECTED_HPP
