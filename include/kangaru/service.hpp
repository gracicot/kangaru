#ifndef KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP

#include <type_traits>

#include "detail/utils.hpp"
#include "detail/single.hpp"
#include "generic.hpp"

namespace kgr {

template<typename... Args>
struct Dependency {};

template<typename, typename = Dependency<>>
struct SingleService;

template<typename Type, typename... Deps>
struct SingleService<Type, Dependency<Deps...>> : GenericService<Type>, EnableAutoCall<SingleService<Type, Dependency<Deps...>>>, Single {
private:
	using Parent = GenericService<Type>;
	
protected:
	using Parent::instance;
	
public:
	using Parent::Parent;
	
	static auto construct(Inject<Deps>... deps) -> decltype(inject(deps.forward()...)) {
		return inject(deps.forward()...);
	}

	Type& forward() {
		return this->instance();
	}
	
	template<typename T, typename... Args>
	static detail::function_result_t<T> call(T method, Args&&... args) {
		return (instance().*method)(std::forward<Args>(args)...);
	}
};

template<typename, typename = Dependency<>>
struct Service;

template<typename Type, typename... Deps>
struct Service<Type, Dependency<Deps...>> : GenericService<Type>, EnableAutoCall<Service<Type, Dependency<Deps...>>> {
private:
	using Parent = GenericService<Type>;
	
protected:
	using Parent::instance;
	
public:
	using Parent::Parent;
	
	template<typename... Args>
	static auto construct(Inject<Deps>... deps, Args&&... args) -> decltype(inject(deps.forward()..., std::declval<Args>()...)) {
		return inject(deps.forward()..., std::forward<Args>(args)...);
	}

	Type forward() {
		return std::move(this->instance());
	}
	
	template<typename T, typename... Args>
	detail::function_result_t<T> call(T method, Args&&... args) {
		return (instance().*method)(std::forward<Args>(args)...);
	}
};

template<typename, typename = Dependency<>>
struct UniqueService;

template<typename Type, typename... Deps>
struct UniqueService<Type, Dependency<Deps...>> : GenericService<std::unique_ptr<Type>>, EnableAutoCall<UniqueService<Type, Dependency<Deps...>>> {
private:
	using Parent = GenericService<std::unique_ptr<Type>>;
	
protected:
	using Parent::instance;
	
public:
	using Parent::Parent;
	
	template<typename... Args>
	static auto construct(Inject<Deps>... deps, Args&&... args)
	-> decltype(inject(std::unique_ptr<Type>{new Type{deps.forward()..., std::declval<Args>()...}})) {
		return inject(std::unique_ptr<Type>{new Type{deps.forward()..., std::forward<Args>(args)...}});
	}
	
	std::unique_ptr<Type> forward() {
		return std::move(this->instance());
	}
	
	template<typename T, typename... Args>
	detail::function_result_t<T> call(T method, Args&&... args) {
		return (instance()->*method)(std::forward<Args>(args)...);
	}
};

template<typename, typename = Dependency<>>
struct SharedService;

template<typename Type, typename... Deps>
struct SharedService<Type, Dependency<Deps...>> : GenericService<std::shared_ptr<Type>>, EnableAutoCall<SharedService<Type, Dependency<Deps...>>>, Single {
private:
	using Parent = GenericService<std::shared_ptr<Type>>;
	
protected:
	using Parent::instance;
	
public:
	using Parent::Parent;

	static auto construct(Inject<Deps>... deps) -> decltype(inject(std::make_shared<Type>(deps.forward()...))) {
		return inject(std::make_shared<Type>(deps.forward()...));
	}
	
	std::shared_ptr<Type> forward() {
		return instance();
	}
	
	template<typename T, typename... Args>
	detail::function_result_t<T> call(T method, Args&&... args) {
		return (instance()->*method)(std::forward<Args>(args)...);
	}
};

template<typename T>
struct AbstractService : Abstract {
	T& forward();
};

template<typename T>
struct AbstractSharedService : Abstract {
	std::shared_ptr<T> forward();
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP
