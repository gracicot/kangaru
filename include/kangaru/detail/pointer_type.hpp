#pragma once

#include "utils.hpp"

#include <memory>

namespace kgr {

template<typename T>
struct Service;

namespace detail {

template<typename Ptr>
struct PointerType {};

template<typename T>
struct PointerType<T*> {
	using Type = T*;
	using ServiceType = T;
	
	template<typename ...Args>
	static Type make_pointer(Args&&... args) {
		return new T(std::forward<Args>(args)...);
	}
};

template<typename T>
struct PointerType<std::shared_ptr<T>> {
	using Type = std::shared_ptr<T>;
	using ServiceType = T;
	
	template<typename ...Args>
	static Type make_pointer(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
};

template<typename T>
struct PointerType<std::unique_ptr<T>> {
	using Type = std::unique_ptr<T>;
	using ServiceType = T;
	
	template<typename ...Args>
	static Type make_pointer(Args&&... args) {
		return make_unique<T>(std::forward<Args>(args)...);
	}
};

template <bool, typename T>
struct pointer_type_helper {
	using type = typename Service<T>::template PointerType<T>;
};

template <typename T>
struct pointer_type_helper<false, T> {
	using type = PointerType<std::shared_ptr<T>>;
};

template<typename T>
struct has_pointer_type {
private:
	using yes = char;
	using no = struct { char array[2]; };

	template<typename C> static yes test(typename Service<C>::template PointerType<C>*);
	template<typename C> static no test(...);
	
public:
	constexpr static bool value = sizeof(test<T>(0)) == sizeof(yes);
};

template<typename T> using ptr_type = typename detail::pointer_type_helper<detail::has_pointer_type<T>::value, T>::type::Type;

} // namespace detail
} // namespace kgr
