#pragma once

#include <type_traits>

#include "detail/utils.hpp"
#include "detail/generic.hpp"
#include "detail/container_service.hpp"

namespace kgr {

template<typename Type, typename Deps = Dependency<>>
struct SingleService : GenericService<SingleService<Type, Deps>, Type, Deps>, Single {
	private: using Parent = GenericService<SingleService<Type, Deps>, Type, Deps>;
	
public:
	using typename Parent::Self;
	using Parent::Parent;

	template<typename... Args>
	static Self makeService(Args&&... args) {
		return Self{Type{std::forward<Args>(args)...}};
	}

	virtual Type& forward() {
		return this->getInstance();
	}
	
	template<typename R, typename... Args, typename... T>
	static auto call(Type& instance, R (Type::*method)(Args...), T&&... args) -> decltype((std::declval<Type>().*method)(std::forward<Args>(args)...)) {
		return (instance.*method)(std::forward<Args>(args)...);
	}
};

template<typename Type, typename Deps = Dependency<>>
struct Service : GenericService<Service<Type, Deps>, Type, Deps> {
	private: using Parent = GenericService<Service<Type, Deps>, Type, Deps>;
	
public:
	using typename Parent::Self;
	using Parent::Parent;

	template<typename... Args>
	static Self makeService(Args&&... args) {
		return Self{Type{std::forward<Args>(args)...}};
	}

	Type forward() {
		return std::move(this->getInstance());
	}
	
	template<typename R, typename... Args, typename... T>
	static auto call(Type& instance, R (Type::*method)(Args...), T&&... args) -> decltype((std::declval<Type>().*method)(std::forward<Args>(args)...)) {
		return (instance.*method)(std::forward<Args>(args)...);
	}
};

template<typename Type, typename Deps = Dependency<>>
struct NoMoveService : GenericService<NoMoveService<Type, Deps>, std::unique_ptr<Type>, Deps>, Single {
	private: using Parent = GenericService<NoMoveService<Type, Deps>, std::unique_ptr<Type>, Deps>;
	
public:
	using typename Parent::Self;
	using Parent::Parent;

	template<typename... Args>
	static Self makeService(Args&&... args) {
		return Self{std::unique_ptr<Type>{new Type{std::forward<Args>(args)...}}};
	}

	virtual Type& forward() {
		return *this->getInstance();
	}
	
	template<typename R, typename... Args, typename... T>
	static auto call(std::unique_ptr<Type>& instance, R (Type::*method)(Args...), T&&... args) -> decltype((std::declval<Type>().*method)(std::forward<Args>(args)...)) {
		return ((*instance).*method)(std::forward<Args>(args)...);
	}
};

template<typename Type, typename Deps = Dependency<>>
struct UniqueService : GenericService<UniqueService<Type, Deps>, std::unique_ptr<Type>, Deps> {
	private: using Parent = GenericService<UniqueService<Type, Deps>, std::unique_ptr<Type>, Deps>;
	
public:
	using typename Parent::Self;
	using Parent::Parent;
	
	template<typename... Args>
	static Self makeService(Args&&... args) {
		return Self{std::unique_ptr<Type>{new Type{std::forward<Args>(args)...}}};
	}
	
	std::unique_ptr<Type> forward() {
		return std::move(this->getInstance());
	}
	
	template<typename R, typename... Args, typename... T>
	static auto call(std::unique_ptr<Type>& instance, R (Type::*method)(Args...), T&&... args) -> decltype((std::declval<Type>().*method)(std::forward<Args>(args)...)) {
		return ((*instance).*method)(std::forward<Args>(args)...);
	}
};

template<typename Type, typename Deps = Dependency<>>
struct SharedService : GenericService<SharedService<Type, Deps>, std::shared_ptr<Type>, Deps>, Single {
	private: using Parent = GenericService<SharedService<Type, Deps>, std::shared_ptr<Type>, Deps>;
	
public:
	using typename Parent::Self;
	using Parent::Parent;
	
	template<typename... Args>
	static Self makeService(Args&&... args) {
		return Self{std::make_shared<Type>(std::forward<Args>(args)...)};
	}
	
	virtual std::shared_ptr<Type> forward() {
		return this->getInstance();
	}
	
	template<typename R, typename... Args, typename... T>
	static auto call(std::shared_ptr<Type>& instance, R (Type::*method)(Args...), T&&... args) -> decltype((std::declval<Type>().*method)(std::forward<Args>(args)...)) {
		return ((*instance).*method)(std::forward<Args>(args)...);
	}
};

template<typename T>
struct AbstractService : Single {
	virtual T& forward() = 0;
};

template<typename T>
struct AbstractSharedService : Single {
	virtual std::shared_ptr<T> forward() = 0;
};

template<typename T>
struct AbstractUniqueService : Single {
	virtual std::unique_ptr<T> forward() = 0;
};

}
