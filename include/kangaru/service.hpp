#pragma once

#include <type_traits>

namespace kgr {

struct Single {
	using ParentTypes = std::tuple<>;
};

template<typename... Types>
struct Overrides : Single {
	using ParentTypes = std::tuple<Types...>;
};

template<typename Service>
struct Type {
	using ServiceType = Service;
};

template<typename... Args>
struct Dependency {};

template<typename...>
struct Injector;

template<typename CRTP, typename... Deps>
struct Injector<CRTP, Dependency<Deps...>> {
	static CRTP construct(Deps&&... deps) {
		using C = typename CRTP::C;
		return C::makeService(std::forward<Deps>(deps)...);
	}
};

template<typename CRTP>
struct Injector<CRTP> {
	static CRTP construct() {
		using C = typename CRTP::C;
		return C::makeService();
	}
};

template<typename CRTP, typename ContainedType, typename ST>
struct BaseGenericService {
	using ServiceType = ST;
	using Self = CRTP;
	
	BaseGenericService() = default;
	BaseGenericService(BaseGenericService&&) = default;
	BaseGenericService(const BaseGenericService&) = default;
	BaseGenericService& operator=(const BaseGenericService&) = default;
	BaseGenericService(ContainedType instance) {
		setInstance(std::move(instance));
	}
	
	~BaseGenericService() {
		if (_initiated) {
			getInstance().~ContainedType();
		}
	}
	
	template<typename T, typename = typename std::enable_if<std::is_base_of<CRTP, T>::value>::type>
	operator T () const & {
		
	}
	
	template<typename T, typename = typename std::enable_if<std::is_base_of<CRTP, T>::value>::type>
	operator T () const && {
		
	}
	
	template<typename...> friend struct Injector;
	
protected:
	ContainedType& getInstance() {
		return *reinterpret_cast<ContainedType*>(&_instance);
	}
	
private:
	void setInstance(ContainedType instance) {
		new (_instance) ContainedType{std::move(instance)};
		_initiated = true;
	}
	
	bool _initiated = false;
	char _instance[sizeof(ContainedType)];
// 	static_assert(sizeof(ContainedType) != sizeof(BaseGenericService<CRTP, ContainedType, ST>::_instance), "Wrong instance size");
};

template<typename...>
struct GenericService;

template<typename CRTP, typename ContainedType, typename ST, typename... Deps>
struct GenericService<CRTP, ContainedType, ST, Dependency<Deps...>> : 
	Injector<GenericService<CRTP, ContainedType, ST, Dependency<Deps...>>, Dependency<Deps...>>,
	BaseGenericService<CRTP, ContainedType, ST>
{
	using BaseGenericService<CRTP, ContainedType, ST>::BaseGenericService;
	using C = CRTP;
};

template<typename CRTP, typename ContainedType, typename ST>
struct GenericService<CRTP, ContainedType, ST> : 
	Injector<GenericService<CRTP, ContainedType, ST>>,
	BaseGenericService<CRTP, ContainedType, ST>
{
	using BaseGenericService<CRTP, ContainedType, ST>::BaseGenericService;
	using C = CRTP;
};

template<typename...>
struct SingleService;

template<typename Type>
struct SingleService<Type> : GenericService<SingleService<Type>, Type, Type*>, Single {
private:
	using Parent = GenericService<SingleService<Type>, Type, Type*>;
	
public:
	using Parent::Parent;
	SingleService(const SingleService&) = delete;
	SingleService& operator=(const SingleService&) = delete;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	Type* forward() {
		return &this->getInstance();
	}
};

template<typename Type, typename... Deps>
struct SingleService<Type, Dependency<Deps...>> : GenericService<SingleService<Type, Dependency<Deps...>>, Type, Type*, Dependency<Deps...>>, Single {
private:
	using Parent = GenericService<SingleService<Type, Dependency<Deps...>>, Type, Type*, Dependency<Deps...>>;
	
public:
	using Parent::Parent;
	SingleService(const SingleService&) = delete;
	SingleService& operator=(const SingleService&) = delete;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	Type* forward() {
		return &this->getInstance();
	}
};

template<typename Type, typename... O>
struct SingleService<Type, Overrides<O...>> : GenericService<SingleService<Type>, Type, Type*>, Overrides<O...> {
private:
	using Parent = GenericService<SingleService<Type>, Type, Type*>;
	
public:
	using Parent::Parent;
	SingleService(const SingleService&) = delete;
	SingleService& operator=(const SingleService&) = delete;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	Type* forward() {
		return &this->getInstance();
	}
};

template<typename Type, typename... Deps, typename... O>
struct SingleService<Type, Dependency<Deps...>, Overrides<O...>> : GenericService<SingleService<Type, Dependency<Deps...>>, Type, Type*, Dependency<Deps...>>, Overrides<O...> {
private:
	using Parent = GenericService<SingleService<Type, Dependency<Deps...>>, Type, Type*, Dependency<Deps...>>;
	
public:
	using Parent::Parent;
	SingleService(const SingleService&) = delete;
	SingleService& operator=(const SingleService&) = delete;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	Type* forward() {
		return &this->getInstance();
	}
};


template<typename...>
struct Service;

template<typename Type>
struct Service<Type> : GenericService<Service<Type>, Type, Type> {
private:
	using Parent = GenericService<Service<Type>, Type, Type>;
	
public:
	using Parent::Parent;
	Service(const Service&) = delete;
	Service& operator=(const Service&) = delete;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	Type forward() {
		return std::move(this->getInstance());
	}
};

template<typename Type, typename... Deps>
struct Service<Type, Dependency<Deps...>> : GenericService<Service<Type, Dependency<Deps...>>, Type, Type, Dependency<Deps...>>, Single {
private:
	using Parent = GenericService<Service<Type, Dependency<Deps...>>, Type, Type, Dependency<Deps...>>;
	
public:
	using Parent::Parent;
	Service(const Service&) = delete;
	Service& operator=(const Service&) = delete;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	Type forward() {
		return std::move(this->getInstance());
	}
};

}