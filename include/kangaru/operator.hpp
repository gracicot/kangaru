#ifndef KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP

#include "container.hpp"
#include "detail/lazy_base.hpp"

namespace kgr {
namespace detail {

template<typename CRTP, typename Map>
struct invoker_base {
	template<typename F, typename... Args, enable_if_t<is_invoke_valid<Map, decay_t<F>, Args...>::value, int> = 0>
	invoke_function_result_t<Map, decay_t<F>, Args...> operator()(F&& f, Args&&... args) {
		container& c = static_cast<CRTP*>(this)->container();
		return c.invoke<Map>(std::forward<F>(f), std::forward<Args>(args)...);
	}
	
	Sink operator()(detail::not_invokable_error = {}, ...) = delete;
};

template<typename CRTP, typename T>
struct generator_base {
	static_assert(!is_single<T>::value, "Generator only work with non-single services.");
	
	template<typename... Args, enable_if_t<is_service_valid<T, Args...>::value, int> = 0>
	service_type<T> operator()(Args&&... args) {
		container& c = static_cast<CRTP*>(this)->container();
		return c.service<T>(std::forward<Args>(args)...);
	}
	
	template<typename U = T, enable_if_t<std::is_default_constructible<service_error<U>>::value, int> = 0>
	Sink operator()(service_error<U> = {}) = delete;
	
	template<typename... Args>
	Sink operator()(service_error<T, identity_t<Args>...>, Args&&...) = delete;
};

} // namespace detail

template<typename Map>
struct mapped_invoker : detail::invoker_base<mapped_invoker<Map>, Map> {
	explicit mapped_invoker(kgr::container& container) : _container{&container} {}
	
private:
	kgr::container& container() {
		return *_container;
	}
	
	const kgr::container& container() const {
		return *_container;
	}
	
	friend struct detail::invoker_base<mapped_invoker<Map>, Map>;
	kgr::container* _container;
};

using invoker = mapped_invoker<map<>>;

template<typename Map>
struct forked_mapped_invoker : detail::invoker_base<forked_mapped_invoker<Map>, Map> {
	explicit forked_mapped_invoker(kgr::container container) : _container{std::move(container)} {}
	
private:
	kgr::container& container() {
		return _container;
	}
	
	const kgr::container& container() const {
		return _container;
	}
	
	friend struct detail::invoker_base<forked_mapped_invoker<Map>, Map>;
	kgr::container _container;
};

using forked_invoker = forked_mapped_invoker<map<>>;

template<typename T>
struct generator : detail::generator_base<generator<T>, T> {
	explicit generator(kgr::container& container) : _container{&container} {}
	
private:
	kgr::container& container() {
		return *_container;
	}
	
	const kgr::container& container() const {
		return *_container;
	}
	
	friend struct detail::generator_base<generator<T>, T>;
	kgr::container* _container;
};

template<typename T>
struct forked_generator : detail::generator_base<forked_generator<T>, T> {
	explicit forked_generator(kgr::container container) : _container{std::move(container)} {}
	
private:
	kgr::container& container() {
		return _container;
	}
	
	const kgr::container& container() const {
		return _container;
	}
	
	friend struct detail::generator_base<forked_generator<T>, T>;
	kgr::container _container;
};

template<typename T>
struct lazy : detail::lazy_base<lazy<T>, T> {
	explicit lazy(kgr::container& container) : _container{&container} {}
	
private:
	kgr::container& container() {
		return *_container;
	}
	
	const kgr::container& container() const {
		return *_container;
	}
	
	friend struct detail::lazy_base<lazy<T>, T>;
	kgr::container* _container;
};

template<typename T>
struct forked_lazy : detail::lazy_base<forked_lazy<T>, T> {
	explicit forked_lazy(kgr::container container) : _container{std::move(container)} {}
	
private:
	kgr::container& container() {
		return _container;
	}
	
	const kgr::container& container() const {
		return _container;
	}
	
	friend struct detail::lazy_base<forked_lazy<T>, T>;
	kgr::container _container;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP
