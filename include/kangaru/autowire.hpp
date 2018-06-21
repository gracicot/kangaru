#ifndef KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP

#include "container.hpp"
#include "generic.hpp"
#include "service.hpp"

#include "detail/service_map.hpp"
#include "detail/utils.hpp"
#include "detail/traits.hpp"
#include "detail/container_service.hpp"
#include "detail/injected.hpp"
#include "detail/autowire_traits.hpp"

namespace kgr {

template<typename Type, typename Map, std::size_t max_dependencies>
struct service<Type, detail::autowire_tag<Map, max_dependencies>> : generic_service<Type> {
private:
	using parent = generic_service<Type>;
	
	template<typename... Args>
	using amount_deduced = detail::amount_of_deductible_service<service, Type, Map, max_dependencies, Args...>;
	
protected:
	using parent::instance;
	
public:
	using parent::parent;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> detail::enable_if_t<amount_deduced<Args...>::deductible, typename amount_deduced<Args...>::default_result_t>
	{
		return detail::deduce_construct_default<service, Map>(
			amount_deduced<Args...>::amount, std::move(cont), std::forward<Args>(args)...
		);
	}
	
	auto forward() -> Type {
		return std::move(instance());
	}
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, instance(), std::forward<Args>(args)...);
	}
};

template<typename Type, typename Map, std::size_t max_dependencies>
struct single_service<Type, detail::autowire_tag<Map, max_dependencies>> : generic_service<Type>, single {
private:
	using parent = generic_service<Type>;
	
	template<typename... Args>
	using amount_deduced = detail::amount_of_deductible_service<single_service, Type, Map, max_dependencies, Args...>;
	
protected:
	using parent::instance;
	
public:
	using parent::parent;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> detail::enable_if_t<amount_deduced<Args...>::deductible, typename amount_deduced<Args...>::default_result_t>
	{
		return detail::deduce_construct_default<single_service, Map>(
			amount_deduced<Args...>::amount, std::move(cont), std::forward<Args>(args)...
		);
	}
	
	auto forward() -> Type& {
		return instance();
	}
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, instance(), std::forward<Args>(args)...);
	}
};

template<typename Type, typename Map, std::size_t max_dependencies>
struct unique_service<Type, detail::autowire_tag<Map, max_dependencies>> : generic_service<std::unique_ptr<Type>> {
private:
	using parent = generic_service<std::unique_ptr<Type>>;
	
	template<typename... Args>
	using amount_deduced = detail::amount_of_deductible_service<unique_service, Type, Map, max_dependencies, Args...>;
	
	// This would be replaced by a generic lambda in C++14
	struct inject_function {
		template<typename... Args>
		auto operator()(Args&&... args) {
			return inject(std::unique_ptr<Type>{new Type(std::forward<Args>(args)...)});
		}
	};
	
protected:
	using parent::instance;
	
public:
	using parent::parent;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> detail::enable_if_t<amount_deduced<Args...>::deductible, inject_result<std::unique_ptr<Type>>>
	{
		return detail::deduce_construct<unique_service, Map>(
			amount_deduced<Args...>::amount, inject_function{}, std::move(cont), std::forward<Args>(args)...
		);
	}
	
	auto forward() -> std::unique_ptr<Type> {
		return std::move(instance());
	}
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, *instance(), std::forward<Args>(args)...);
	}
};

template<typename Type, typename Map, std::size_t max_dependencies>
struct shared_service<Type, detail::autowire_tag<Map, max_dependencies>> : generic_service<std::shared_ptr<Type>>, single {
private:
	using parent = generic_service<std::shared_ptr<Type>>;
	
	template<typename... Args>
	using amount_deduced = detail::amount_of_deductible_service<shared_service, Type, Map, max_dependencies, Args...>;
	
	// This would be replaced by a generic lambda in C++14
	struct inject_function {
		template<typename... Args>
		auto operator()(Args&&... args) {
			return inject(std::unique_ptr<Type>{new Type(std::forward<Args>(args)...)});
		}
	};
	
protected:
	using parent::instance;
	
public:
	using parent::parent;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> detail::enable_if_t<amount_deduced<Args...>::deductible, inject_result<std::shared_ptr<Type>>>
	{
		return detail::deduce_construct<shared_service, Map>(
			amount_deduced<Args...>::amount, inject_function{}, std::move(cont), std::forward<Args>(args)...
		);
	}
	
	auto forward() -> std::shared_ptr<Type> {
		return instance();
	}
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, *instance(), std::forward<Args>(args)...);
	}
};

using autowire = detail::autowire_map<service, detail::decay_t, map<>, detail::default_max_dependency>;
using autowire_single = detail::autowire_map<single_service, detail::decay_t, map<>, detail::default_max_dependency>;
using autowire_unique = detail::autowire_map<unique_service, detail::unwrap_pointer_t, map<>, detail::default_max_dependency>;
using autowire_shared = detail::autowire_map<shared_service, detail::unwrap_pointer_t, map<>, detail::default_max_dependency>;

template<typename Map, std::size_t max_dependencies = detail::default_max_dependency>
using mapped_autowire = detail::autowire_map<service, detail::decay_t, Map, max_dependencies>;

template<typename Map, std::size_t max_dependencies = detail::default_max_dependency>
using mapped_autowire_single = detail::autowire_map<single_service, detail::decay_t, Map, max_dependencies>;

template<typename Map, std::size_t max_dependencies = detail::default_max_dependency>
using mapped_autowire_unique = detail::autowire_map<unique_service, detail::unwrap_pointer_t, Map, max_dependencies>;

template<typename Map, std::size_t max_dependencies = detail::default_max_dependency>
using mapped_autowire_shared = detail::autowire_map<shared_service, detail::unwrap_pointer_t, Map, max_dependencies>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_service = single_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_single_service = single_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_unique_service = unique_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_shared_service = shared_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP
