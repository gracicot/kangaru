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
struct PointerType<std::weak_ptr<T>> {
	using Type = std::weak_ptr<T>;
	using ServiceType = T;
};

template<typename T>
struct PointerType<std::unique_ptr<T>> {
	using Type = std::unique_ptr<T>;
	using ServiceType = T;
	
	template<typename ...Args>
	static Type make_pointer(Args&&... args) {
		return kgr::detail::make_unique<T>(std::forward<Args>(args)...);
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

template<typename Service, typename Target, typename Input>
struct PointerConverter;

template<typename Service>
struct PointerConverter<Service, Service*, std::shared_ptr<Service>> {
	static Service* convert(std::shared_ptr<Service>&& input) {
		return input.get();
	}
	static Service* convert(const std::shared_ptr<Service>& input) {
		return input.get();
	}
};

template<typename Service>
struct PointerConverter<Service, std::shared_ptr<Service>, std::shared_ptr<Service>> {
	static std::shared_ptr<Service> convert(std::shared_ptr<Service>&& input) {
		return std::move(input);
	}
	static std::shared_ptr<Service> convert(const std::shared_ptr<Service>& input) {
		return input;
	}
};

} // namespace detail

template<typename T> using service_ptr = typename detail::pointer_type_helper<detail::has_pointer_type<T>::value, T>::type::Type;

template<typename T, typename ...Args>
service_ptr<T> make_service(Args&&... args) {
	return detail::pointer_type_helper<detail::has_pointer_type<T>::value, T>::type::make_pointer(std::forward<Args>(args)...);
}

} // namespace kgr
