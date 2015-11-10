#pragma once

#include <type_traits>

#include "detail/utils.hpp"

namespace kgr {

struct Single {
	Single() = default;
	virtual ~Single() = default;
	Single(const Single&) = delete;
	Single& operator=(const Single&) = delete;
	Single(Single&&) = default;
	Single& operator=(Single&&) = default;
};

template<typename... Types>
struct Overrides {
	using ParentTypes = std::tuple<Types...>;
};

template<typename... Args>
struct Dependency {};

namespace detail {

template<typename T> using InjectType_t = typename std::conditional<std::is_base_of<Single, T>::value, T&, T&&>::type;

template<typename...>
struct Injector;

template<typename CRTP, typename... Deps>
struct Injector<CRTP, Dependency<Deps...>> {
	static CRTP construct(InjectType_t<Deps>... deps) {
		using C = typename CRTP::C;
		return C::makeService(deps.forward()...);
	}
};

template<typename CRTP>
struct Injector<CRTP> {
	static CRTP construct() {
		using C = typename CRTP::C;
		return C::makeService();
	}
};

template<typename CRTP, typename ContainedType>
struct BaseGenericService {
	BaseGenericService() = default;
	
    BaseGenericService(BaseGenericService&& other) {
		setInstance(std::move(other.getInstance()));
    }
    
    BaseGenericService& operator=(BaseGenericService&& other) {
		setInstance(std::move(other.getInstance()));
		return *this;
    }
    
	BaseGenericService(const BaseGenericService& other) {
		setInstance(other.getInstance());
	}
	
	BaseGenericService& operator=(const BaseGenericService& other) {
		setInstance(other.getInstance());
		return *this;
	}
	
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
	
	template<typename F, F f, typename... T>
	void autocall(InjectType_t<T>... others) {
		(getInstance().*f)(others.forward()...);
	}
	
protected:
	ContainedType& getInstance() {
		return *reinterpret_cast<ContainedType*>(&_instance);
	}
	
private:
	void setInstance(ContainedType instance) {
		new (&_instance) ContainedType(std::move(instance));
		_initiated = true;
	}
	
	bool _initiated = false;
	typename std::aligned_storage<sizeof(ContainedType), alignof(ContainedType)>::type _instance;
};

template<typename Original, typename Service>
struct ServiceOverride : Service {
    virtual ~ServiceOverride() {}
    ServiceOverride(Original& service) : _service {service} {}

    ServiceType<Service> forward() override {
        return static_cast<ServiceType<Service>>(_service.forward());
    }

private:
    Original& _service;
};

} // detail

template<typename...>
struct GenericService;

template<typename CRTP, typename ContainedType, typename... Deps>
struct GenericService<CRTP, ContainedType, Dependency<Deps...>> :
	detail::Injector<GenericService<CRTP, ContainedType, Dependency<Deps...>>, Dependency<Deps...>>,
	detail::BaseGenericService<CRTP, ContainedType>
{
    template<typename...> friend struct detail::Injector;
    using Self = GenericService<CRTP, ContainedType, Dependency<Deps...>>;
    using detail::BaseGenericService<CRTP, ContainedType>::BaseGenericService;

private:
    using C = CRTP;
};

template<typename CRTP, typename ContainedType>
struct GenericService<CRTP, ContainedType> :
	detail::Injector<GenericService<CRTP, ContainedType>>,
	detail::BaseGenericService<CRTP, ContainedType>
{
    template<typename...> friend struct detail::Injector;
    using Self = GenericService<CRTP, ContainedType>;
    using detail::BaseGenericService<CRTP, ContainedType>::BaseGenericService;

private:
    using C = CRTP;
};

template<typename...>
struct SingleService;

template<typename Type>
struct SingleService<Type> : GenericService<SingleService<Type>, Type>, Single {
    using typename GenericService<SingleService<Type>, Type>::Self;
	using Self::Self;
	
    virtual ~SingleService() = default;
    SingleService() = default;
    SingleService(SingleService&&) = default;
    SingleService& operator=(SingleService&&) = default;
    SingleService(const SingleService&) = delete;
    SingleService& operator=(const SingleService&) = delete;

    template<typename... Args>
    static Self makeService(Args&&... args) {
        return Self{Type{std::forward<Args>(args)...}};
    }

    virtual Type& forward() {
        return this->getInstance();
    }
};

template<typename Type, typename... Deps>
struct SingleService<Type, Dependency<Deps...>> : GenericService<SingleService<Type, Dependency<Deps...>>, Type, Dependency<Deps...>>, Single {
    using typename GenericService<SingleService<Type, Dependency<Deps...>>, Type, Dependency<Deps...>>::Self;
	using Self::Self;
	
    virtual ~SingleService() = default;
    SingleService() = default;
    SingleService(SingleService&&) = default;
    SingleService& operator=(SingleService&&) = default;
    SingleService(const SingleService&) = delete;
    SingleService& operator=(const SingleService&) = delete;

    template<typename... Args>
    static Self makeService(Args&&... args) {
        return Self(Type(std::forward<Args>(args)...));
    }

    virtual Type& forward() {
        return this->getInstance();
    }
};

template<typename...>
struct Service;

template<typename Type>
struct Service<Type> : GenericService<Service<Type>, Type> {
    using typename GenericService<Service<Type>, Type>::Self;
	using Self::Self;

    template<typename... Args>
    static Self makeService(Args&&... args) {
        return Self{Type{std::forward<Args>(args)...}};
    }

    Type forward() {
        return std::move(this->getInstance());
    }
};

template<typename Type, typename... Deps>
struct Service<Type, Dependency<Deps...>> : GenericService<Service<Type, Dependency<Deps...>>, Type, Dependency<Deps...>> {
    using typename GenericService<Service<Type, Dependency<Deps...>>, Type, Dependency<Deps...>>::Self;
	using Self::Self;

    template<typename... Args>
    static Self makeService(Args&&... args) {
        return Self{Type{std::forward<Args>(args)...}};
    }

    Type forward() {
        return std::move(this->getInstance());
    }
};

template<typename T>
struct AbstractService : Single {
    virtual T& forward() = 0;
};

}
