#pragma once

#include <type_traits>

namespace kgr {

struct Single {
	Single() = default;
	~Single() = default;
	Single(const Single&) = delete;
	Single& operator=(const Single&) = delete;
	Single(Single&&) = default;
	Single& operator=(Single&&) = default;
	
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
	static CRTP construct(typename std::conditional<std::is_base_of<Single, Deps>::value, Deps&, Deps&&>::type... deps) {
		using C = typename CRTP::C;
		return C::makeService(deps...);
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
		T service;
		
		if (_initiated) {
			service.setInstance(getInstance());
		}
		
		return service;
	}
	
	template<typename T, typename = typename std::enable_if<std::is_base_of<CRTP, T>::value>::type>
	operator T () && {
		T service;
		
		if (_initiated) {
			service.setInstance(std::move(getInstance()));
		}
		
		return service;
	}
	
protected:
	ContainedType& getInstance() {
		return *reinterpret_cast<ContainedType*>(&_instance);
	}
	
private:
	void setInstance(ContainedType instance) {
		new (&_instance) ContainedType{std::move(instance)};
		_initiated = true;
	}
	
	bool _initiated = false;
	typename std::aligned_storage<sizeof(ContainedType), alignof(ContainedType)>::type _instance;
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
	using Self = GenericService<CRTP, ContainedType, ST, Dependency<Deps...>>;
};

template<typename CRTP, typename ContainedType, typename ST>
struct GenericService<CRTP, ContainedType, ST> : 
	Injector<GenericService<CRTP, ContainedType, ST>>,
	BaseGenericService<CRTP, ContainedType, ST>
{
	using BaseGenericService<CRTP, ContainedType, ST>::BaseGenericService;
	using C = CRTP;
	using Self = GenericService<CRTP, ContainedType, ST>;
};

template<typename...>
struct SingleService;

template<typename Type>
struct SingleService<Type> : GenericService<SingleService<Type>, Type, Type&>, Single {
	using Parent = GenericService<SingleService<Type>, Type, Type&>;
	virtual ~SingleService() = default;
	SingleService() = default;
	SingleService(SingleService&&) = default;
	SingleService& operator=(SingleService&&) = default;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	virtual Type& forward() {
		return this->getInstance();
	}
};

template<typename Type, typename... Deps>
struct SingleService<Type, Dependency<Deps...>> : GenericService<SingleService<Type, Dependency<Deps...>>, Type, Type&, Dependency<Deps...>>, Single {
	using Parent = GenericService<SingleService<Type, Dependency<Deps...>>, Type, Type&, Dependency<Deps...>>;
	virtual ~SingleService() = default;
	SingleService() = default;
	SingleService(SingleService&&) = default;
	SingleService& operator=(SingleService&&) = default;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	virtual Type& forward() {
		return this->getInstance();
	}
};

template<typename Type, typename... O>
struct SingleService<Type, Overrides<O...>> : GenericService<SingleService<Type, Overrides<O...>>, Type, Type&>, Overrides<O...> {
	using Parent = GenericService<SingleService<Type, Overrides<O...>>, Type, Type&>;
	virtual ~SingleService() = default;
	SingleService() = default;
	SingleService(SingleService&&) = default;
	SingleService& operator=(SingleService&&) = default;

	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	virtual Type& forward() {
		return this->getInstance();
	}
};

template<typename Type, typename... Deps, typename... O>
struct SingleService<Type, Dependency<Deps...>, Overrides<O...>> : GenericService<SingleService<Type, Dependency<Deps...>>, Type, Type&, Dependency<Deps...>>, Overrides<O...> {
	using Parent = GenericService<SingleService<Type, Dependency<Deps...>>, Type, Type&, Dependency<Deps...>>;
	virtual ~SingleService() = default;
	SingleService() = default;
	SingleService(SingleService&&) = default;
	SingleService& operator=(SingleService&&) = default;
	
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	virtual Type& forward() {
		return this->getInstance();
	}
};

template<typename...>
struct Service;

template<typename Type>
struct Service<Type> : GenericService<Service<Type>, Type, Type> {
	using Parent = GenericService<Service<Type>, Type, Type>;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{std::forward<Args>(args)...}};
	}
	
	Type forward() {
		return std::move(this->getInstance());
	}
};

template<typename Type, typename... Deps>
struct Service<Type, Dependency<Deps...>> : GenericService<Service<Type, Dependency<Deps...>>, Type, Type, Dependency<Deps...>> {
	using Parent = GenericService<Service<Type, Dependency<Deps...>>, Type, Type, Dependency<Deps...>>;
	
	template<typename... Args>
	static Parent makeService(Args&&... args) {
		return Parent{Type{args.forward()...}};
	}
	
	Type forward() {
		return std::move(this->getInstance());
	}
};

}