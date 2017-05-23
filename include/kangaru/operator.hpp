#ifndef KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP

#include "container.hpp"
#include "detail/lazy_base.hpp"

namespace kgr {
namespace detail {

template<typename CRTP, template<typename> class Map>
struct InvokerBase {
	template<typename F, typename... Args, enable_if_t<is_invoke_valid<Map, decay_t<F>, Args...>::value, int> = 0>
	invoke_function_result_t<Map, decay_t<F>, Args...> operator()(F&& f, Args&&... args) {
		return static_cast<CRTP*>(this)->container().template invoke<Map>(std::forward<F>(f), std::forward<Args>(args)...);
	}
	
	Sink operator()(detail::NotInvokableError = {}, ...) = delete;
};

template<typename CRTP, typename T>
struct GeneratorBase {
	static_assert(!is_single<T>::value, "Generator only work with non-single services.");
	
	template<typename... Args, enable_if_t<is_service_valid<T, Args...>::value, int> = 0>
	ServiceType<T> operator()(Args&&... args) {
		return static_cast<CRTP*>(this)->container().template service<T>(std::forward<Args>(args)...);
	}
	
	template<typename U = T, enable_if_t<std::is_default_constructible<ServiceError<U>>::value, int> = 0>
	Sink operator()(ServiceError<U> = {}) = delete;
	
	template<typename... Args>
	Sink operator()(ServiceError<T, identity_t<Args>...>, Args&&...) = delete;
};

} // namespace detail

template<template<typename> class Map>
struct MappedInvoker : detail::InvokerBase<MappedInvoker<Map>, Map> {
	explicit MappedInvoker(Container& container) : _container{&container} {}
	
private:
	kgr::Container& container() {
		return *_container;
	}
	
	const kgr::Container& container() const {
		return *_container;
	}
	
	friend struct detail::InvokerBase<MappedInvoker<Map>, Map>;
	kgr::Container* _container;
};

using Invoker = MappedInvoker<kgr::AdlMap>;

template<template<typename> class Map>
struct ForkedMappedInvoker : detail::InvokerBase<ForkedMappedInvoker<Map>, Map> {
	explicit ForkedMappedInvoker(Container container) : _container{std::move(container)} {}
	
private:
	kgr::Container& container() {
		return _container;
	}
	
	const kgr::Container& container() const {
		return _container;
	}
	
	friend struct detail::InvokerBase<ForkedMappedInvoker<Map>, Map>;
	kgr::Container _container;
};

using ForkedInvoker = ForkedMappedInvoker<kgr::AdlMap>;

template<typename T>
struct Generator : detail::GeneratorBase<Generator<T>, T> {
	explicit Generator(Container& container) : _container{&container} {}
	
private:
	kgr::Container& container() {
		return *_container;
	}
	
	const kgr::Container& container() const {
		return *_container;
	}
	
	friend struct detail::GeneratorBase<Generator<T>, T>;
	kgr::Container* _container;
};

template<typename T>
struct ForkedGenerator : detail::GeneratorBase<ForkedGenerator<T>, T> {
	explicit ForkedGenerator(Container container) : _container{std::move(container)} {}
	
private:
	kgr::Container& container() {
		return _container;
	}
	
	const kgr::Container& container() const {
		return _container;
	}
	
	friend struct detail::GeneratorBase<ForkedGenerator<T>, T>;
	kgr::Container _container;
};

template<typename T>
struct Lazy : detail::LazyBase<Lazy<T>, T> {
	explicit Lazy(kgr::Container& container) : _container{&container} {}
	
private:
	kgr::Container& container() {
		return *_container;
	}
	
	const kgr::Container& container() const {
		return *_container;
	}
	
	friend struct detail::LazyBase<Lazy<T>, T>;
	kgr::Container* _container;
};

template<typename T>
struct ForkedLazy : detail::LazyBase<ForkedLazy<T>, T> {
	explicit ForkedLazy(kgr::Container container) : _container{std::move(container)} {}
	
private:
	kgr::Container& container() {
		return _container;
	}
	
	const kgr::Container& container() const {
		return _container;
	}
	
	friend struct detail::LazyBase<ForkedLazy<T>, T>;
	kgr::Container _container;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_OPERATOR_HPP
