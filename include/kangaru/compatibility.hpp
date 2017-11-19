#ifndef KGR_KANGARU_INCLUDE_KANGARU_COMPATIBILITY_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_COMPATIBILITY_HPP

#include "kangaru.hpp"

namespace kgr {

using Abstract = abstract;
using AbstractNotFound = abstract_not_found;

template<typename T>
using AbstractService = abstract_service<T>;

template<typename T>
using AbstractSharedService = abstract_shared_service<T>;

using All = all;

template<typename... Ts>
using AnyOf = any_of<Ts...>;

template<typename... Ts>
using AutoCall = autocall<Ts...>;

template<typename... Ts>
using AutoCallNoMap = autocall<Ts...>;

using Container = container;
using ContainerService = container_service;

template<typename T>
using Default = defaults_to<T>;

using DefaultForkedInvoker = forked_invoker;
using DefaultForkedInvokerService = forked_invoker_service;
using DefaultInvoker = invoker;
using DefaultInvokerService = invoker_service;

template<typename... Deps>
using Dependency = dependency<Deps...>;

template<typename Predicate>
using FilteredForkService = filtered_fork_service<Predicate>;

using ForkService = fork_service;

template<typename Service>
using ForkedGenerator = forked_generator<Service>;

template<typename Service>
using ForkedGeneratorService = forked_generator_service<Service>;

template<typename Map>
using ForkedInvoker = forked_mapped_invoker<Map>;

template<typename Map>
using ForkedInvokerService = forked_mapped_invoker_service<Map>;

template<typename Service>
using ForkedLazy = forked_lazy<Service>;

template<typename Service>
using ForkedLazyService = forked_lazy_service<Service>;

template<typename Service>
using Generator = generator<Service>;

template<typename Service>
using GeneratorService = generator_service<Service>;

template<typename /* CRTP */, typename Type>
using GenericService = generic_service<Type>;

template<typename T>
using Inject = inject_t<T>;

template<typename T, typename... Ps>
using Invoke = invoke<T, Ps...>;

template<typename Map>
using Invoker = mapped_invoker<Map>;

template<typename Map>
using InvokerService = mapped_invoker_service<Map>;

template<typename Service>
using Lazy = lazy<Service>;

template<typename Service>
using LazyService = lazy_service<Service>;

template<typename T>
using Map = map_t<T>;

template<typename... Ts>
using NoneOf = none_of<Ts...>;

template<typename... Ts>
using Overrides = overrides<Ts...>;

template<typename T>
using ServiceType = service_type<T>;

template<typename Type, typename Deps>
using Service = service<Type, Deps>;

template<typename Type, typename Deps>
using SharedService = shared_service<Type, Deps>;

using Single = single;

template<typename Type, typename Deps>
using SingleService = single_service<Type, Deps>;

template<typename Type, typename Deps>
using UniqueService = unique_service<Type, Deps>;

}

#endif // KGR_KANGARU_INCLUDE_KANGARU_COMPATIBILITY_HPP
