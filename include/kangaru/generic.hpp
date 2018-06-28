#ifndef KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP

#include "container.hpp"
#include "detail/injected.hpp"
#include "detail/traits.hpp"

namespace kgr {
namespace detail {

template<typename Generic, typename Type, typename = void>
struct generic_service_destruction {
	~generic_service_destruction() {
		static_cast<Generic&>(*this).instance().~Type();
	}
};

template<typename Generic, typename Type>
struct generic_service_destruction<Generic, Type, enable_if_t<std::is_trivially_destructible<Type>::value>> {};

} // namespace detail

/*
 * A generic storage for services. It also provides the emplace function and constructors.
 */
template<typename Type>
struct generic_service : detail::generic_service_destruction<generic_service<Type>, Type> {
	generic_service() = default;
	
	generic_service(generic_service&& other) noexcept {
		emplace(std::move(other.instance()));
	}
	
	generic_service& operator=(generic_service&& other) noexcept {
		emplace(std::move(other.instance()));
		return *this;
	}
	
	generic_service(const generic_service& other) = delete;
	generic_service& operator=(const generic_service& other) = delete;
	
	template<typename... Args, detail::enable_if_t<detail::is_someway_constructible<Type, Args...>::value, int> = 0>
	generic_service(in_place_t, Args&&... args) {
		emplace(std::forward<Args>(args)...);
	}
	
protected:
	Type& instance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
	const Type& instance() const {
		return *reinterpret_cast<const Type*>(&_instance);
	}
	
private:
	friend container;
	friend detail::generic_service_destruction<generic_service<Type>, Type>;
	template<typename, typename...> friend struct detail::has_emplace_helper;
	
	template<typename... Args, detail::enable_if_t<std::is_constructible<Type, Args...>::value, int> = 0>
	void emplace(Args&&... args) {
		new (&_instance) Type(std::forward<Args>(args)...);
	}
	
	template<typename... Args, detail::enable_if_t<detail::is_only_brace_constructible<Type, Args...>::value, int> = 0>
	void emplace(Args&&... args) {
		new (&_instance) Type{std::forward<Args>(args)...};
	}
	
	detail::aligned_storage_t<sizeof(Type), alignof(Type)> _instance;
};

/*
 * A generic storage for services. It also provides the emplace function and constructors.
 * 
 * This specialisation is for lvalue reference types. Mainly used for external services.
 */
template<typename Type>
struct generic_service<Type&> {
	generic_service() = default;
	~generic_service() = default;
	
	generic_service(generic_service&& other) = default;
	generic_service& operator=(generic_service&& other) = default;
	
	generic_service(const generic_service& other) = delete;
	generic_service& operator=(const generic_service& other) = delete;
	
	generic_service(in_place_t, Type& ref) {
		emplace(ref);
	}
	
	generic_service(in_place_t, Type&& ref) = delete;
	
protected:
	Type& instance() {
		return *_instance;
	}
	
	const Type& instance() const {
		return *_instance;
	}
	
private:
	friend container;
	friend detail::generic_service_destruction<generic_service<Type>, Type>;
	template<typename, typename...> friend struct detail::has_emplace_helper;
	
	void emplace(Type& ref) {
		_instance = &ref;
	}
	
	Type* _instance;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP
