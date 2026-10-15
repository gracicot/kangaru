#ifndef KANGARU5_COMPATIBILITY_V4_HPP
#define KANGARU5_COMPATIBILITY_V4_HPP

#include "kangaru.hpp"

#include "detail/define.hpp"
#include "kangaru/detail/concepts.hpp"

namespace kangaru::compatibility_v4 {
	struct in_place_t {} constexpr in_place{};
	
	struct single {};
	struct polymorphic {};
	struct final {};
	struct supplied {};
	struct abstract : polymorphic, single {};
	
	template<typename T>
	struct defaults_to {
		using default_service = T;
	};
	
	template<typename... Types>
	struct overrides {
		using parent_types = std::tuple<Types...>;
	};
}

namespace kangaru::compatibility_v4 {
	struct service_not_found : exception {};
	
	struct supplied_not_found : service_not_found {
		inline char const* what() const noexcept override {
			return "No instance found for the requested supplied service";
		}
	};
	
	struct abstract_not_found : service_not_found {
		inline char const* what() const noexcept override {
			return "No instance found for the requested abstract service";
		}
	};
}

namespace kangaru::compatibility_v4::detail {
	template<typename T>
	concept v4_source = requires(T source) {
		{ source.forward() } -> different_from<void>;
	};
}

namespace kangaru::compatibility_v4::detail {
	template<v4_source>
	struct parent_types_or_empty {
		using type = std::tuple<>;
	};
	
	template<v4_source Service>
		requires(requires{ typename Service::parent_types; })
	struct parent_types_or_empty<Service> {
		using type = typename Service::parent_types;
	};
	
	template<v4_source T>
	using parent_types_or_empty_t = typename parent_types_or_empty<T>::type;
	
	
	template<v4_source Service>
	struct default_or_service {
		using type = Service;
	};
	
	template<v4_source Service>
		requires(requires{ typename Service::default_service; })
	struct default_or_service<Service> {
		using type = typename Service::default_service;
	};
	
	template<v4_source Service>
	using default_or_service_t = typename default_or_service<Service>::type;
}

namespace kangaru::compatibility_v4::detail {
	template<typename T, typename... Args>
	concept v4_source_construct_callable = requires(Args&&... args) {
		{ T::construct(KANGARU5_FWD(args)...) } -> different_from<void>;
	};
	
	template<typename T, typename... Args>
	concept default_constructible_emplaceable =
		    std::constructible_from<T>
		and requires(T& v4source, Args&&... args) {
			{ v4source.emplace(KANGARU5_FWD(args)...) } -> std::same_as<void>;
		};
	
	template<typename T, typename... Args>
	using construct_result_t = decltype(T::construct(std::declval<Args>()...));
	
	template<typename T, typename ConstructResult>
	inline constexpr auto constructible_with_construct_result = false;
	
	template<typename T, typename... Args>
	inline constexpr auto constructible_with_construct_result<T, std::tuple<Args...>> =
		   std::constructible_from<T, in_place_t, Args...>
		or default_constructible_emplaceable<T, Args...>;
	
	template<unqualified_object T, typename... Args>
	constexpr auto v4_constructor_impl(Args&&... args) -> T {
		if constexpr (std::constructible_from<T, in_place_t, decltype(args)...>) {
			return T(in_place, KANGARU5_FWD(args)...);
		} else if constexpr (default_constructible_emplaceable<T, decltype(args)...>) {
			auto result = T();
			result.emplace(KANGARU5_FWD(args)...);
			return result;
		} else {
			static_assert(not std::same_as<T, T>);
		}
	}
	
	template<unqualified_object T, typename... Args>
		requires(constructible_with_construct_result<T, construct_result_t<T, Args&&...>>)
	constexpr auto v4_constructor(Args&&... args) -> T {
		auto injected = T::construct(KANGARU5_FWD(args)...);
		
		return std::apply([]<typename... A>(A&&... a) -> T {
			return v4_constructor_impl<T>(KANGARU5_FWD(a)...);
		}, injected);
	}
	
	template<typename T, typename... Args>
	concept v4_constructor_callable = requires(Args&&... args) {
		{ v4_constructor<T>(KANGARU5_FWD(args)...) } -> std::same_as<T>;
	};
	
	template<typename V4Source>
	struct v4_source_wrapper {
		template<typename... Args> requires(not std::derived_from<V4Source, abstract> and (... and not_self<Args, v4_source_wrapper>) and v4_constructor_callable<V4Source, Args&&...>)
		explicit(sizeof...(Args) == 1) constexpr v4_source_wrapper(Args&&... args) : v4_source(KANGARU5_NO_ADL(v4_constructor<V4Source>)(KANGARU5_FWD(args)...)) {}
		
		template<injectable T, forwarded<v4_source_wrapper> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> decltype(KANGARU5_FWD(source).v4_source.forward()) {
			if constexpr (std::derived_from<V4Source, abstract>) {
				throw abstract_not_found{};
			} else {
				return KANGARU5_FWD(source).v4_source.forward();
			}
		}
		
		constexpr auto v4_service() & {
			return v4_source;
		}
		
		constexpr auto v4_service() && {
			return std::move(v4_source);
		}
		
		constexpr auto v4_service() const& {
			return v4_source;
		}
		
		constexpr auto v4_service() const&& {
			return std::move(v4_source);
		}
		
	private:
		V4Source v4_source;
		
		template<detail::v4_source S>
		friend auto attribute(allow_empty_injection<v4_source_wrapper<S>>) -> std::true_type;
		
		template<detail::v4_source S>
		friend auto attribute(allow_runtime_caching<v4_source_wrapper<S>>) -> std::bool_constant<std::derived_from<S, single>>;
		
		template<detail::v4_source S>
		friend auto attribute(assume_runtime_cached<v4_source_wrapper<S>>) -> std::bool_constant<std::derived_from<S, abstract> or std::derived_from<S, supplied>>;
		
		// TODO: Autocall
		// template<detail::v4_source S>
		// friend auto attribute(second_step_init<v4_source_wrapper<S>>) -> call_second_step_from_attribute_on_member<&rvalue_source<U>::object, U&&>;
	};
	
	template<source Source>
	struct with_v4_source_wrapper {
		template<injectable T, forwarded<with_v4_source_wrapper> Self>
			requires(v4_source<std::remove_cvref_t<T>> and wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			if constexpr (reference<T>) {
				return kangaru::provide<kangaru::detail::forward_like_t<T, v4_source_wrapper<std::remove_cvref_t<T>>>>(KANGARU5_FWD(source).source).v4_service();
			} else {
				return kangaru::provide<v4_source_wrapper<T>>(KANGARU5_FWD(source).source).v4_service();
			}
		}
		
		Source source;
	};
	
	template<typename T>
	struct v4_source_wrapper_type {};
	
	template<typename T>
	struct v4_source_wrapper_type<v4_source_wrapper<T>> {
		using type = T;
	};
	
	template<typename T>
	using v4_source_wrapper_type_t = typename v4_source_wrapper_type<T>::type;
	
	template<unqualified_object Type, reference... Args>
	struct inject_constructor_with_additional_args_impl_function {
		std::tuple<Args...> args;
		
		template<deducer... Deducers> requires(constructor_callable<Type, Deducers..., Args...>)
		constexpr auto operator()(Deducers... deduce) const {
			return std::apply([&](Args... args) {
				return KANGARU5_NO_ADL(constructor<Type>)(deduce..., KANGARU5_FWD(args)...);
			}, std::move(args));
		}
	};
	
	template<unqualified_object Type>
	struct inject_constructor_with_additional_args_function {
		template<forwarded_injector Injector, typename... Args>
			requires(callable<Injector&&, inject_constructor_with_additional_args_impl_function<Type, Args&&...>>)
		constexpr auto operator()(Injector&& inject, Args&&... args) const {
			return inject(inject_constructor_with_additional_args_impl_function<Type, Args&&...>{
				std::forward_as_tuple(KANGARU5_FWD(args)...)
			});
		}
	};
	
	template<unqualified_object Type, forwarded_injector Injector, typename... Args>
		requires(callable<Injector&&, inject_constructor_with_additional_args_impl_function<Type, Args&&...>>)
	auto inject_constructor_with_additional_args(Injector&& inject, Args&&... args) {
		return inject_constructor_with_additional_args_function<Type>{}(KANGARU5_FWD(inject), KANGARU5_FWD(args)...);
	}
	
	template<typename Type, typename Injector, typename... Args>
	concept inject_constructor_with_additional_args_callable =
		    unqualified_object<Type>
		and forwarded_injector<Injector>
		and callable<Injector&&, inject_constructor_with_additional_args_impl_function<Type, Args&&...>>;
	
	template<typename Function, std::size_t max, typename...>
	struct v4_parameter_dependency_sequence {};
	
	template<typename Function, std::size_t max, typename... Args>
		requires(sizeof...(Args) < max and callable<Function, Args...>)
	struct v4_parameter_dependency_sequence<Function, max, Args...> {
		using type = std::index_sequence_for<Args...>;
		using return_type = decltype(
			std::declval<Function>()(
				std::declval<Args>()...
			)
		);
	};
	
	template<typename Function, std::size_t max, typename... Args>
		requires(sizeof...(Args) < max and not callable<Function, Args...>)
	struct v4_parameter_dependency_sequence<Function, max, Args...> :
		v4_parameter_dependency_sequence<Function, max, placeholder_deducer, Args...> {};
	
	template<typename F, std::size_t max>
	using v4_parameter_dependency_sequence_t = typename v4_parameter_dependency_sequence<F, max>::type;
	
	template<typename F, std::size_t max>
	using v4_dependency_return_type_t = typename v4_parameter_dependency_sequence<F, max>::return_type;
	
	template<typename F, kangaru::deducer Deducer, typename>
	inline constexpr auto callable_with_deducer_sequence_v = false;
	
	template<typename F, std::size_t... S, kangaru::deducer Deducer>
	inline constexpr auto callable_with_deducer_sequence_v<F, Deducer, std::index_sequence<S...>> =
		callable_with_deducers<F, kangaru::detail::expand<Deducer, S>...>;
	
	template<source Source, template<source_ref> typename Deducer, std::size_t max>
	struct basic_v4_source_injector {
		explicit constexpr basic_v4_source_injector(Source source) : source(std::move(source)) {}
		
		template<allows_construction_of<Source> S>
			requires(not std::same_as<std::remove_cvref_t<S>, Source>)
		explicit constexpr basic_v4_source_injector(S&& source) : source(KANGARU5_FWD(source)) {}
		
		
		template<typename F, typename..., typename Seq = v4_parameter_dependency_sequence_t<F, max>>
		constexpr auto operator()(F&& function) & -> v4_dependency_return_type_t<F, max> requires callable_with_deducer_sequence_v<F, Deducer<Source&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename..., typename Seq = v4_parameter_dependency_sequence_t<F, max>>
		constexpr auto operator()(F&& function) && -> v4_dependency_return_type_t<F, max> requires callable_with_deducer_sequence_v<F, Deducer<Source&&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
		template<typename F, typename..., typename Seq = v4_parameter_dependency_sequence_t<F, max>>
		constexpr auto operator()(F&& function) const& -> v4_dependency_return_type_t<F, max> requires callable_with_deducer_sequence_v<F, Deducer<Source const&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename..., typename Seq = v4_parameter_dependency_sequence_t<F, max>>
		constexpr auto operator()(F&& function) const&& -> v4_dependency_return_type_t<F, max> requires callable_with_deducer_sequence_v<F, Deducer<Source const&&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
	private:
		template<std::size_t... s>
		static constexpr auto expand_deducers(std::index_sequence<s...>, auto&& function, auto&& source) -> decltype(auto) {
			using deducer = Deducer<decltype(source)>;
			return KANGARU5_NO_ADL(call_with_deducers)(KANGARU5_FWD(function), (void(s), deducer{KANGARU5_FWD(source)})...);
		}
		
		Source source;
	};
	
	template<template<typename> typename Deducer, std::size_t max>
	struct make_basic_v4_source_injector_function {
		template<forwarded_source Source>
		inline constexpr auto operator()(Source&& source) const -> basic_v4_source_injector<deduced_source_type<Source>, Deducer, max> {
			return basic_v4_source_injector<deduced_source_type<Source>, Deducer, max>{
				KANGARU5_FWD(source)
			};
		}
	};
	
	template<template<typename> typename Deducer, std::size_t max>
	inline constexpr auto make_basic_v4_source_injector = make_basic_v4_source_injector_function<Deducer, max>{};
	
	using make_fast_v4_source_injector_function = make_basic_v4_source_injector_function<basic_deducer, 4>;
	using make_v4_source_injector_function = make_basic_v4_source_injector_function<basic_deducer, 8>;
	using make_slow_v4_source_injector_function = make_basic_v4_source_injector_function<basic_deducer, 16>;
	using make_optional_v4_source_injector_function = make_basic_v4_source_injector_function<basic_deducer, 1>;

	inline constexpr auto make_fast_v4_source_injector = make_fast_v4_source_injector_function{};
	inline constexpr auto make_v4_source_injector = make_v4_source_injector_function{};
	inline constexpr auto make_slow_v4_source_injector = make_slow_v4_source_injector_function{};
	inline constexpr auto make_optional_v4_source_injector = make_optional_v4_source_injector_function{};
}

namespace kangaru::compatibility_v4 {
	template<typename T>
	using service_type = decltype(std::declval<T>().forward());
}

namespace kangaru::compatibility_v4::detail {
	template<typename Service>
	struct polymorphic_v4_source : any_source_of_ref<service_type<Service>> {
		using any_source_of_ref<service_type<Service>>::any_source_of_ref;
		
		explicit polymorphic_v4_source(kangaru::any_source_of_one_ref const& source) noexcept :
			any_source_of_ref<service_type<Service>>{source} {}
		
		constexpr auto provide() -> service_type<Service> {
			return kangaru::provide<service_type<Service>>(static_cast<any_source_of_ref<service_type<Service>>&>(*this));
		}
	};
}

// service map
namespace kangaru::compatibility_v4 {
	template<typename...>
	struct map {};
	
	template<typename = void>
	struct map_t {};
}

namespace kangaru::compatibility_v4::detail::service_map_impl {
	auto service_map(auto&&) requires false;
	
	template<injectable T>
	struct exact_placeholder_source {
		auto provide() const -> T;
	};
	
	template<typename... Maps>
	struct map_type_source {
		template<injectable T> requires(... or std::same_as<map_t<Maps>, T>)
		auto provide() const -> T;
	};
	
	template<template<typename...> typename TT>
	struct tt_exists { using type = void; };
	
	template<typename T>
	concept indirect_mapping = requires {
		typename tt_exists<T::template mapped_service>;
	};
	
	template<typename T>
	concept mapping = v4_source<T> or indirect_mapping<T>;
	
	template<typename T>
	concept mapped_unnamed = requires(basic_deducer<exact_placeholder_source<T>&> deducer) {
		{ service_map(deducer) } -> mapping;
	};
	
	template<typename T, typename... Maps>
	concept mapped_named = requires(basic_deducer<exact_placeholder_source<T>&> deducer, basic_deducer<map_type_source<Maps...>&> map) {
		{ service_map(deducer, map) } -> mapping;
	};
	
	template<typename T, typename... Maps>
	concept mapped = mapped_unnamed<T> or mapped_named<T, Maps...>;
	
	template<typename T>
	using service_map_result_t = decltype(service_map(std::declval<basic_deducer<exact_placeholder_source<T>&>>()));
	
	template<typename T, typename... Maps>
	using named_service_map_result_t = decltype(service_map(
		std::declval<basic_deducer<exact_placeholder_source<T>&>>(),
		std::declval<basic_deducer<map_type_source<Maps...>&>>()
	));
	
	template<typename Mapping, typename>
		requires(mapping<Mapping>)
	struct evaluate_indirect_map {
		using type = Mapping;
	};
	
	template<typename IndirectMapping, typename Service>
		requires(indirect_mapping<IndirectMapping>)
	struct evaluate_indirect_map<IndirectMapping, Service> {
		using type = typename IndirectMapping::template mapped_service<Service>;
	};
	
	template<typename Mapping, typename Service>
	using evaluate_indirect_map_t = typename evaluate_indirect_map<Mapping, Service>::type;
	
	template<typename T, typename>
	struct lookup_map {};
	
	template<typename T, typename... Maps>
		requires(mapped_named<T, Maps...>)
	struct lookup_map<T, map<Maps...>> {
		using type = evaluate_indirect_map_t<named_service_map_result_t<T, Maps...>, T>;
	};
	
	template<typename T, typename... Maps>
		requires(mapped_unnamed<T>)
	struct lookup_map<T, map<Maps...>> {
		using type = evaluate_indirect_map_t<service_map_result_t<T>, T>;
	};
	
	template<typename T, typename... Maps>
		requires(mapped<T, Maps...>)
	using lookup_map_t = typename lookup_map<T, Maps...>::type;
}

namespace kangaru::compatibility_v4::detail {
	template<typename T>
	concept map_list = kangaru::detail::is_specialisation_of_v<map, T>;
	
	template<typename T, typename Map>
	concept map_exists_for =
		    injectable<T>
		and map_list<Map>
		and requires{ typename service_map_impl::lookup_map_t<T, Map>; };
}

// service map
namespace kangaru::compatibility_v4 {
	template<injectable T, detail::map_list Map = map<>>
	using mapped_service_t = detail::service_map_impl::lookup_map_t<T, Map>;
}

// autowire impl
namespace kangaru::compatibility_v4::detail {
	template<map_list Map>
	struct autowire_placeholder_deducer {
		using is_deducer = kangaru_strict_deducer_tag;
		
		template<weak_deducible T>
			requires(injectable<T> and detail::map_exists_for<T, Map>)
		operator T ();
		
		template<weak_deducible T>
			requires(injectable<T&> and detail::map_exists_for<T, Map>)
		operator T& () const;
		
		template<weak_deducible T>
			requires(injectable<T const&> and detail::map_exists_for<T, Map>)
		operator T const& () const;
		
		template<weak_deducible T>
			requires(injectable<T&&> and detail::map_exists_for<T, Map>)
		operator T&& () const;
		
		template<weak_deducible T>
			requires(injectable<T const&&> and detail::map_exists_for<T, Map>)
		operator T const&& () const;
	};
	
	template<typename Function, map_list Map, std::size_t... S>
	inline constexpr auto callable_clang_22_workaround =
		callable<Function, kangaru::detail::expand<autowire_placeholder_deducer<Map>, S>...>;
	
	template<typename Function, map_list Map, typename>
	struct autowire_sequence_impl {};
	
	template<typename Function, map_list Map>
		requires(callable<Function>)
	struct autowire_sequence_impl<Function, Map, std::index_sequence<>> {
		using type = std::index_sequence<>;
		using return_type = decltype(std::declval<Function>()());
	};
	
	template<typename Function, map_list Map, std::size_t head, std::size_t... tail>
		requires(callable_clang_22_workaround<Function, Map, head, tail...>)
	struct autowire_sequence_impl<Function, Map, std::index_sequence<head, tail...>> {
		using type = std::index_sequence<head, tail...>;
		using return_type = decltype(
			std::declval<Function>()(
				std::declval<autowire_placeholder_deducer<Map>>(),
				(tail, std::declval<autowire_placeholder_deducer<Map>>())...
			)
		);
	};
	
	template<typename Function, map_list Map, std::size_t head, std::size_t... tail>
	struct autowire_sequence_impl<Function, Map, std::index_sequence<head, tail...>> :
		autowire_sequence_impl<Function, Map, std::index_sequence<(tail - 1)...>> {};
	
	template<typename F, map_list Map, std::size_t max>
	using autowire_sequence = autowire_sequence_impl<F, Map, std::make_index_sequence<max>>;
	
	template<typename F, map_list Map, std::size_t max>
	using autowire_sequence_t = typename autowire_sequence<F, Map, max>::type;
	
	template<typename F, map_list Map, std::size_t max>
	using autowire_return_type_t = typename autowire_sequence<F, Map, max>::return_type;

	template<source Source, template<source_ref> typename Deducer, map_list Map, std::size_t max>
	struct basic_autowire_injector {
		explicit constexpr basic_autowire_injector(Source source) : source(std::move(source)) {}
		
		template<allows_construction_of<Source> S>
			requires(not std::same_as<std::remove_cvref_t<S>, Source>)
		explicit constexpr basic_autowire_injector(S&& source) : source(KANGARU5_FWD(source)) {}
		
		
		template<typename F, typename..., typename Seq = autowire_sequence_t<F, Map, max>>
		constexpr auto operator()(F&& function) & -> autowire_return_type_t<F, Map, max> requires callable_with_deducer_sequence_v<F, Deducer<Source&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename..., typename Seq = autowire_sequence_t<F, Map, max>>
		constexpr auto operator()(F&& function) && -> autowire_return_type_t<F, Map, max> requires callable_with_deducer_sequence_v<F, Deducer<Source&&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
		template<typename F, typename..., typename Seq = autowire_sequence_t<F, Map, max>>
		constexpr auto operator()(F&& function) const& -> autowire_return_type_t<F, Map, max> requires callable_with_deducer_sequence_v<F, Deducer<Source const&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename..., typename Seq = autowire_sequence_t<F, Map, max>>
		constexpr auto operator()(F&& function) const&& -> autowire_return_type_t<F, Map, max> requires callable_with_deducer_sequence_v<F, Deducer<Source const&&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
	private:
		template<std::size_t... s>
		static constexpr auto expand_deducers(std::index_sequence<s...>, auto&& function, auto&& source) -> decltype(auto) {
			using deducer = Deducer<decltype(source)>;
			return KANGARU5_NO_ADL(call_with_deducers)(KANGARU5_FWD(function), (void(s), deducer{KANGARU5_FWD(source)})...);
		}
		
		Source source;
	};
	
	template<template<typename> typename Deducer, map_list Map, std::size_t max>
	struct make_basic_autowire_injector_function {
		template<forwarded_source Source>
		inline constexpr auto operator()(Source&& source) const -> basic_autowire_injector<deduced_source_type<Source>, Deducer, Map, max> {
			return basic_autowire_injector<deduced_source_type<Source>, Deducer, Map, max>{
				KANGARU5_FWD(source)
			};
		}
	};
	
	template<template<typename> typename Deducer, map_list Map, std::size_t max>
	inline constexpr auto make_basic_autowire_injector = make_basic_autowire_injector_function<Deducer, Map, max>{};
	
	using make_fast_autowire_injector_function = make_basic_autowire_injector_function<basic_deducer, map<>, 4>;
	using make_autowire_injector_function = make_basic_autowire_injector_function<basic_deducer, map<>, 8>;
	using make_slow_autowire_injector_function = make_basic_autowire_injector_function<basic_deducer, map<>, 16>;
	using make_optional_autowire_injector_function = make_basic_autowire_injector_function<basic_deducer, map<>, 1>;
	
	inline constexpr auto make_fast_autowire_injector = make_fast_autowire_injector_function{};
	inline constexpr auto make_autowire_injector = make_autowire_injector_function{};
	inline constexpr auto make_slow_autowire_injector = make_slow_autowire_injector_function{};
	inline constexpr auto make_optional_autowire_injector = make_optional_autowire_injector_function{};
}

namespace kangaru::compatibility_v4::detail {
	template<typename T>
	using source_wrapper_for = kangaru::detail::conditional_t<
		std::derived_from<T, single>,
		detail::polymorphic_v4_source<T>,
		detail::v4_source_wrapper<T>
	>;
	
	template<v4_source Service>
	struct construct_injected {
		source_wrapper_for<Service> service;
		
		constexpr auto forward() -> service_type<Service> {
			return kangaru::provide<service_type<Service>>(service);
		}
	};
}

namespace kangaru::compatibility_v4 {
	template<detail::v4_source T>
	using inject_t = detail::construct_injected<T>;
	
	template<typename T>
	using remove_rvalue_reference_t = kangaru::detail::conditional_t<
		std::is_rvalue_reference_v<T>,
		std::remove_reference_t<T>,
		T
	>;
	
	template<typename... Args>
	constexpr auto inject(Args&&... args) -> std::tuple<remove_rvalue_reference_t<Args>...> {
		return std::tuple<remove_rvalue_reference_t<Args>...>{
			KANGARU5_FWD(args)...
		};
	}
	
	template<typename... Ts>
	using inject_result = std::tuple<remove_rvalue_reference_t<Ts>...>;
}

namespace kangaru::compatibility_v4::detail {
	template<typename T>
	struct override_wrap_service {
		using type = std::tuple<>;
	};
	
	template<typename... Parents>
	struct override_wrap_service<std::tuple<Parents...>> {
		using type = std::tuple<polymorphic_v4_source<Parents>...>;
	};
	
	template<typename OverrideTuple>
	using override_wrap_service_t = typename override_wrap_service<OverrideTuple>::type;
}

namespace kangaru::compatibility_v4::detail {
	template<source Source, injectable Primary>
	struct with_polymorphic_v4_cast {
		Source source;
		
		template<injectable T, forwarded<with_polymorphic_v4_cast> Self> requires(wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		constexpr operator any_source_of_one_ref() noexcept {
			return any_source_of_one_ref{any_source_of_ref<Primary>{source}};
		}
		
		template<injectable T> requires source_of<Source&, T>
		constexpr operator any_source_of_ref<T>() & {
			return any_source_of_ref<T>{source};
		}
		
		template<injectable T> requires source_of<Source const&, T>
		constexpr operator any_source_of_ref<T>() const& {
			return any_source_of_ref<T>{source};
		}
		
		// TODO: Should we remove?
		// template<kangaru::source S, injectable P>
		// friend auto attribute(overrides_types_in_cache<with_polymorphic_v4_cast<S, P>>)
		// 	-> detail::override_wrap_service_t<overrides_types_in_cache_t<P>>;
		
		// TODO: Autocall
		// template<kangaru::source S, injectable P>
		// friend auto attribute(second_step_init<with_polymorphic_v4_cast<S, P>>)
		// 	-> call_second_step_from_attribute_on_wrapped_source;
	};
}

namespace kangaru {
	template<compatibility_v4::detail::v4_source Service>
		requires(
			    requires{ typename Service::parent_types; }
			and requires{ typename compatibility_v4::detail::override_wrap_service_t<typename Service::parent_types>; }
		)
	struct overrides_types_in_cache<compatibility_v4::detail::with_polymorphic_v4_cast<with_cast_from<compatibility_v4::detail::v4_source_wrapper<Service>, compatibility_v4::service_type<Service>>, compatibility_v4::service_type<Service>>> {
		using type = typename compatibility_v4::detail::override_wrap_service_t<typename Service::parent_types>;
	};
}

namespace kangaru::compatibility_v4 {
	template<typename Type>
	struct generic_service {
		generic_service() = default;
		~generic_service() requires(std::is_trivially_destructible_v<Type>) = default;
		constexpr ~generic_service() requires(not std::is_trivially_destructible_v<Type>) {
			std::destroy_at(object);
		}
		
		constexpr generic_service(generic_service&& other) noexcept {
			emplace(std::move(other.instance()));
		}
		
		constexpr generic_service& operator=(generic_service&& other) noexcept {
			emplace(std::move(other.instance()));
			return *this;
		}
		
		generic_service(const generic_service& other) = delete;
		generic_service& operator=(const generic_service& other) = delete;
		
		template<typename... Args>
			requires(constructor_callable<Type, Args&&...>)
		explicit(sizeof...(Args) == 0) constexpr generic_service(in_place_t, Args&&... args) :
			object{KANGARU5_NO_ADL(constructor<Type>)(KANGARU5_FWD(args)...)} {}
		
		template<typename... Args> requires(constructor_callable<Type, Args&&...>)
		constexpr void emplace(Args&&... args) {
			std::construct_at(std::addressof(object), KANGARU5_NO_ADL(constructor<Type>)(KANGARU5_FWD(args)...));
		}
		
	protected:
		constexpr auto instance() -> Type& {
			return object;
		}
		
		constexpr auto instance() const -> Type const& {
			return object;
		}
		
	private:
		union {
			char empty = {};
			Type object;
		};
	};
	
	template<typename Type>
	struct generic_service<Type&> {
		generic_service() = default;
		
		generic_service(generic_service&& other) = default;
		generic_service& operator=(generic_service&& other) = default;
		
		generic_service(const generic_service& other) = delete;
		generic_service& operator=(const generic_service& other) = delete;
		
		constexpr generic_service(in_place_t, Type& ref) {
			emplace(ref);
		}
		
		constexpr void emplace(Type& ref) {
			object = &ref;
		}
		
	protected:
		constexpr auto instance() -> Type& {
			return *object;
		}
		
		constexpr auto instance() const -> Type const& {
			return *object;
		}
		
	private:
		Type* object;
	};
	
	/**
	 * This class is simply a list of definition the current service depends on to be constructed.
	 */
	template<typename... Args>
	using dependency = std::tuple<Args...>;
}

namespace kangaru::compatibility_v4::detail {
	/**
	 * This is the base for default non-single and single service.
	 * 
	 * Contains all the shared logic between single and non single services.
	 */
	template<typename Type>
	struct basic_service_base : generic_service<Type> {
		using generic_service<Type>::generic_service;
		
		template<typename T, typename... Args>
		constexpr auto call(T method, Args&&... args) -> std::invoke_result_t<T, Type&, Args...> {
			return std::invoke(method, this->instance(), KANGARU5_FWD(args)...);
		}
	};
	
	template<typename, typename>
	struct basic_service;
	
	template<typename Type, typename... Deps>
	struct basic_service<Type, dependency<Deps...>> : basic_service_base<Type> {
		using basic_service_base<Type>::basic_service_base;
		
		template<typename... Args>
		static constexpr auto construct(inject_t<Deps>... deps, Args&&... args) -> inject_result<service_type<Deps>..., Args...> {
			return inject(deps.forward()..., KANGARU5_FWD(args)...);
		}
	};
} // namespace detail

namespace kangaru::compatibility_v4 {
	/**
	 * This class is the default single service.
	 * 
	 * It hold the service as value, and returns it by reference.
	 */
	template<typename Type, typename Deps = dependency<>>
	struct single_service : detail::basic_service<Type, Deps>, single {
		using detail::basic_service<Type, Deps>::basic_service;
		
		constexpr auto forward() -> Type& {
			return this->instance();
		}
	};
	
	/**
	 * This class is a service definition for a single service managed by an external system.
	 * 
	 * It hold the service as a reference to the instance, and returns it by reference.
	 */
	template<typename Type>
	struct extern_service : single_service<Type&>, supplied {};
	
	/**
	 * This is the default non-single service.
	 * 
	 * It hold and return the service by value.
	 */
	template<typename Type, typename Deps = dependency<>>
	struct service : detail::basic_service<Type, Deps> {
		using detail::basic_service<Type, Deps>::basic_service;
		
		constexpr auto forward() -> Type {
			return std::move(this->instance());
		}
	};
	
	/**
	 * This class is the service definition for a non-single heap allocated service.
	 * 
	 * It works for both case where you need a shared pointer non-single service,
	 * because they are implicitly constructible from a unique pointer.
	 * 
	 * It will hold the service as a std::unique_ptr, and inject it as a std::unique_ptr
	 */
	template<typename, typename = dependency<>>
	struct unique_service;
	
	template<typename Type, typename... Deps>
	struct unique_service<Type, dependency<Deps...>> : generic_service<std::unique_ptr<Type>> {
		using generic_service<std::unique_ptr<Type>>::generic_service;
		
		template<typename... Args> requires(std::constructible_from<Type, service_type<Deps>..., Args...>)
		static constexpr auto construct(inject_t<Deps>... deps, Args&&... args) -> inject_result<std::unique_ptr<Type>>
		{
			return inject(std::make_unique<Type>(deps.forward()..., KANGARU5_FWD(args)...));
		}
		
		constexpr auto forward() -> std::unique_ptr<Type> {
			return std::move(this->instance());
		}
		
		template<typename T, typename... Args>
		constexpr auto call(T method, Args&&... args) -> std::invoke_result_t<T, Type&, Args...> {
			return std::invoke(method, *this->instance(), KANGARU5_FWD(args)...);
		}
	};
	
	/**
	 * This class is a service definition when a single should be injected as a shared pointer.
	 * 
	 * It will hold the service as a std::shared_ptr and inject it a s a std::shared_ptr
	 */
	template<typename, typename = dependency<>>
	struct shared_service;
	
	template<typename Type, typename... Deps>
	struct shared_service<Type, dependency<Deps...>> : generic_service<std::shared_ptr<Type>>, single {
		using generic_service<std::shared_ptr<Type>>::generic_service;
		
		template<typename... Args> requires(std::constructible_from<Type, service_type<Deps>..., Args...>)
		static constexpr auto construct(inject_t<Deps>... deps, Args&&... args) -> inject_result<std::shared_ptr<Type>>
		{
			return inject(std::make_shared<Type>(deps.forward()..., KANGARU5_FWD(args)...));
		}
		
		constexpr auto forward() -> std::shared_ptr<Type> {
			return this->instance();
		}
		
		template<typename T, typename... Args>
		constexpr auto call(T method, Args&&... args) -> std::invoke_result_t<T, Type&, Args...> {
			return std::invoke(method, *this->instance(), KANGARU5_FWD(args)...);
		}
	};
	
	/**
	 * This class is a service definition for a single service managed by an external system.
	 * 
	 * It hold and injects the service as a shared pointer to the supplied instance.
	 */
	template<typename Type>
	struct extern_shared_service : shared_service<Type>, supplied {
		using shared_service<Type>::shared_service;
		
		static constexpr auto construct(std::shared_ptr<Type> instance) -> inject_result<std::shared_ptr<Type>> {
			return inject(std::move(instance));
		}
	};
	
	/**
	 * This class is a abstract service that a kgr::single_service can override.
	 * 
	 * It cannot be constructed, but only overrided.
	 */
	template<typename T>
	struct abstract_service : abstract {
		auto forward() -> T&;
	};
	
	/**
	 * This class is an abstract service that can be overrided by kgr::shared_service
	 * 
	 * As it is abstract, a service that overrides it must be instanciated by the container before usage.
	 */
	template<typename T>
	struct abstract_shared_service : abstract {
		auto forward() -> std::shared_ptr<T>;
	};
}

namespace kangaru::compatibility_v4::detail {
	struct cached_source {
	private:
		template<injectable T>
		struct mapping {};
		
		template<injectable T> requires(v4_source<T>)
		struct mapping<polymorphic_v4_source<T>> {
			using service = default_or_service_t<T>;
			using type = with_polymorphic_v4_cast<
				with_cast_from<
					v4_source_wrapper<service>,
					service_type<service>
				>,
				service_type<service>
			>&;
		};
		
	public:
		template<injectable T>
		using source_for = typename mapping<T>::type;
	};
	
	struct predicate_not_single {
		template<injectable T>
		consteval bool operator()() const {
			return not std::derived_from<T, single>;
		}
	};
	
	struct get_recursing_source {
		constexpr auto operator()(forwarded_source auto&& source) const  {
			return KANGARU5_FWD(source).source.source.source.source.source.alternative;
		}
	};
	
	using container_cache_type = map_insert_as_assign<polymorphic_map<std::unordered_map<type_id, any_source_of_one_ref>>>;
	using container_heap_storage = default_heap_storage;
	using container_construction = basic_unsafe_exhaustive_construction<make_v4_source_injector_function>;
	
	struct throw_if_service_not_found {
		template<typename Service>
			requires(
				(
					   std::derived_from<v4_source_wrapper_type_t<Service>, supplied>
					or std::derived_from<v4_source_wrapper_type_t<Service>, abstract>
				) and not requires{ typename v4_source_wrapper_type_t<Service>::default_service; }
			)
		constexpr auto provide() const -> Service {
			if constexpr (std::derived_from<v4_source_wrapper_type_t<Service>, supplied>) {
				throw supplied_not_found{};
			} else {
				throw abstract_not_found{};
			}
		}
	};
	
	inline constexpr auto container_base() {
		return make_container_base(allow_assume_cached, throw_if_service_not_found{}, none_source{}).source;
	}
	
	using container_base_t = decltype(container_base());

	template<typename T>
	struct overrides_types {
		using type = std::tuple<>;
	};
}

KANGARU5_EXPORT namespace kangaru::compatibility_v4 {
	struct container;
	
	struct container_service {
	private:
		container* c;
		
	public:
		explicit constexpr container_service(in_place_t, container& c) : c{std::addressof(c)} {}
		
		static constexpr auto construct(container& container) {
			return inject(container);
		}
		
		constexpr auto forward() const -> container& {
			return *c;
		}
	};
	
	struct container {
		constexpr container(detail::container_cache_type cache = {}, detail::container_heap_storage storage = {}) :
			state{
				KANGARU5_NO_ADL(make_source_with_cache_asymmetric<detail::cached_source::source_for>)(
					with_dereference{
						with_heap_storage{
							KANGARU5_NO_ADL(make_source_with_source_wrapping)(
								KANGARU5_NO_ADL(make_source_with_source_wrapping)(
									KANGARU5_NO_ADL(make_source_with_construction)(
										KANGARU5_NO_ADL(seal_source)(detail::container_base()),
										KANGARU5_NO_ADL(make_construction_with_two_step_init_if<detail::predicate_not_single>)(
											detail::container_construction{},
											noop_second_step{}
										)
									)
								)
							),
							std::move(storage),
						},
					},
					std::move(cache)
				),
			} {}
		
	private:
		with_cache_asymmetric<
			with_dereference<
				with_heap_storage<
					with_source_wrapping<
						with_source_wrapping<
							with_construction<
								sealed_source<detail::container_base_t>,
								construction_with_two_step_init_if<
									detail::container_construction,
									noop_second_step,
									detail::predicate_not_single
								>
							>
						>
					>,
					detail::container_heap_storage
				>
			>,
			detail::container_cache_type,
			detail::cached_source::source_for
		> state;
		
		template<typename S>
		constexpr auto container_source(S&& source) {
			auto rebound_state = std::remove_cvref_t<S>::rebind(
				KANGARU5_FWD(source),
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source)
			);
			
			return with_recursion{
				with_alternative{
					KANGARU5_NO_ADL(make_source_with_passthrough<5>)(
						cache_with_two_step_init{
							rebound_state,
							call_second_step_with_transformed_source{
								noop_second_step{},
								detail::get_recursing_source{},
							}
						}
					),
					composed_source{
						external_reference_source{*this},
						KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source.source.source.source)
					},
				},
			};
		}
		
		template<source Source, detail::map_list Map>
		struct with_service_map_lookup {
			Source source;
			
			template<injectable T, forwarded<with_service_map_lookup> Self>
				requires(
					    not detail::v4_source<T>
					and requires{ typename mapped_service_t<T, Map>; }
					and std::same_as<T, service_type<inject_t<mapped_service_t<T, Map>>>>
					and wrapping_source_of<Self, inject_t<mapped_service_t<T, Map>>>
				)
			constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
				return kangaru::provide<inject_t<mapped_service_t<T, Map>>>(KANGARU5_FWD(source).source).forward();
			}
		};
		
		template<detail::map_list Map, forwarded_source Source>
		static constexpr auto make_source_with_service_map_lookup(Source&& source) {
			return with_service_map_lookup<deduced_source_type<Source>, Map>{KANGARU5_FWD(source)};
		}
		
	public:
		template<injectable T> requires(detail::service_map_impl::mapped<T>)
		constexpr auto provide() -> T {
			return service<mapped_service_t<T>>();
		}
		
		template<detail::v4_source Service>
			requires(std::derived_from<Service, single>)
		constexpr auto service() -> service_type<Service> {
			return kangaru::provide<service_type<Service>>(kangaru::provide<detail::polymorphic_v4_source<Service>>(container_source(state)));
		}
		
		template<detail::v4_source Service, typename... Args>
			requires(not std::derived_from<Service, single>)
		constexpr auto service(Args&&... args) -> service_type<Service> {
			auto service = detail::inject_constructor_with_additional_args<detail::v4_source_wrapper<Service>>(
				detail::make_v4_source_injector(container_source(state)),
				KANGARU5_FWD(args)...
			);
			// second step
			return kangaru::provide<service_type<Service>>(std::move(service));
		}
		
		template<detail::map_list Map = map<>, typename Function, typename... Args>
		constexpr auto invoke(Function&& function, Args&&... args) {
			return make_spread_injector(make_source_with_service_map_lookup<Map>(container_source(state)))(
				[&](deducer auto... deduce) -> decltype(auto) requires(callable<Function, decltype(deduce)..., Args&&...>) {
					return KANGARU5_FWD(function)(deduce..., KANGARU5_FWD(args)...);
				}
			);
		}
		
		template<detail::map_list Map, typename Function, typename... Args>
		constexpr auto invoke(Map, Function&& function, Args&&... args) -> decltype(invoke<Map>(KANGARU5_FWD(function), KANGARU5_FWD(args)...)) {
			return invoke<Map>(KANGARU5_FWD(function), KANGARU5_FWD(args)...);
		}
		
		template<detail::v4_source... Services, typename Function, typename... Args>
			requires(callable<Function&&, service_type<Services>..., Args&&...>)
		constexpr auto invoke(Function&& function, Args&&... args) {
			return KANGARU5_FWD(function)(service<Services>()..., KANGARU5_FWD(args)...);
		}
		
		template<detail::v4_source Service>
			requires(
				cache_map_stores<detail::container_cache_type, detail::with_polymorphic_v4_cast<with_cast_from<detail::v4_source_wrapper<Service>, service_type<Service>>, service_type<Service>>>
			)
		bool contains() {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<detail::polymorphic_v4_source<Service>>)();
			auto& cache = state;
			return cache.contains(id);
		}
		
		template<detail::v4_source Service, typename... Args>
			requires(
				cache_map_stores<detail::container_cache_type, detail::with_polymorphic_v4_cast<with_cast_from<detail::v4_source_wrapper<Service>, service_type<Service>>, service_type<Service>>>
			)
		constexpr auto emplace(Args&&... args) -> bool {
			return contains<Service>() ? false : (replace<Service>(KANGARU5_FWD(args)...), true);
		}
		
		template<detail::v4_source Service, typename... Args>
			requires(
				cache_map_stores<detail::container_cache_type, detail::with_polymorphic_v4_cast<with_cast_from<detail::v4_source_wrapper<Service>, service_type<Service>>, service_type<Service>>>
			)
		constexpr auto replace(Args&&... args) -> void {
			using contained_type = detail::with_polymorphic_v4_cast<with_cast_from<detail::v4_source_wrapper<Service>, service_type<Service>>, service_type<Service>>;
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<detail::polymorphic_v4_source<Service>>)();
			
			auto& heap_storage = state.source.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.template emplace<contained_type>(
				in_place_construct{[&] {
					return contained_type{
						with_cast_from<detail::v4_source_wrapper<Service>, service_type<Service>>{
							detail::inject_constructor_with_additional_args<detail::v4_source_wrapper<Service>>(
								detail::make_v4_source_injector(container_source(state)),
								KANGARU5_FWD(args)...
							)
						},
					};
				}}
			);
			
			cache.insert_or_assign(id, *ptr);
		}
		
		constexpr auto fork() const& -> container {
			auto cache = detail::container_cache_type{};
			cache.insert(state.begin(), state.end());
			
			return container{
				std::move(cache),
				detail::container_heap_storage{},
			};
		}
		
		constexpr auto onion() {
			return container_source(state);
		}
		
	private:
	};
}

namespace kangaru::compatibility_v4::detail {
	template<typename T>
	auto service_arg_for_service_type(std::shared_ptr<T> const&) -> std::decay<T>;
	template<typename T>
	auto service_arg_for_service_type(std::unique_ptr<T> const&) -> std::decay<T>;
	template<typename T>
	auto service_arg_for_service_type(T const&) -> std::decay<T>;
	
	template<template<typename, typename> class, typename, std::size_t>
	struct autowire_map;
	
	template<template<typename, typename> class service_type, typename... Maps, std::size_t max_dependencies>
	struct autowire_map<service_type, map<Maps...>, max_dependencies> {
		template<typename T>
		using mapped_service = service_type<
			typename decltype(KANGARU5_NO_ADL(service_arg_for_service_type)(std::declval<T>()))::type,
			autowire_map<service, map<Maps...>, max_dependencies>
		>;
	};
	
	template<map_list Map = map<>, std::size_t max_dependencies = 8>
	using autowire_tag = detail::autowire_map<service, Map, max_dependencies>;
}

namespace kangaru::compatibility_v4 {
	template<in_place_constructible Type, detail::map_list Map, std::size_t max_dependencies>
	struct service<Type, detail::autowire_tag<Map, max_dependencies>> : generic_service<Type> {
		template<typename... Args>
			requires(
				detail::inject_constructor_with_additional_args_callable<
					Type,
					kangaru::detail::call_result_t<
						detail::make_basic_autowire_injector_function<basic_deducer, Map, max_dependencies>,
						ref_result_t<container&>
					>,
					Args&&...
				>
			)
		constexpr service(in_place_t, container& container, Args&&... args) :
			generic_service<Type>{in_place,
				in_place_construct{[&] {
					return detail::inject_constructor_with_additional_args<Type>(
						detail::make_basic_autowire_injector<basic_deducer, Map, max_dependencies>(KANGARU5_NO_ADL(ref)(container)),
						KANGARU5_FWD(args)...
					);
				}}
			} {}
		
		template<typename... Args>
			requires(
				detail::inject_constructor_with_additional_args_callable<
					Type,
					kangaru::detail::call_result_t<
						detail::make_basic_autowire_injector_function<basic_deducer, Map, max_dependencies>,
						ref_result_t<container&>
					>,
					Args&&...
				>
			)
		constexpr void emplace(container& container, Args&&... args) {
			generic_service<Type>::emplace(
				in_place_construct{[&]() {
					return detail::inject_constructor_with_additional_args<Type>(
						detail::make_basic_autowire_injector<basic_deducer, Map, max_dependencies>(KANGARU5_NO_ADL(ref)(container)),
						KANGARU5_FWD(args)...
					);
				}}
			);
		}
		
		auto forward() -> Type {
			return std::move(generic_service<Type>::instance());
		}
		
		template<typename... Args>
		static constexpr auto construct(inject_t<container_service> container_service, Args&&... args) {
			return inject(container_service.forward(), KANGARU5_FWD(args)...);
		}
	};
	
	template<typename Type, detail::map_list Map, std::size_t max_dependencies>
	struct single_service<Type, detail::autowire_tag<Map, max_dependencies>> : generic_service<Type>, single {
		template<typename... Args>
			requires(
				detail::inject_constructor_with_additional_args_callable<
					Type,
					kangaru::detail::call_result_t<
						detail::make_basic_autowire_injector_function<basic_deducer, Map, max_dependencies>,
						ref_result_t<container&>
					>,
					Args&&...
				>
			)
		constexpr single_service(in_place_t, container& container, Args&&... args) :
			generic_service<Type>{in_place,
				in_place_construct{[&] {
					return detail::inject_constructor_with_additional_args<Type>(
						detail::make_basic_autowire_injector<basic_deducer, Map, max_dependencies>(KANGARU5_NO_ADL(ref)(container)),
						KANGARU5_FWD(args)...
					);
				}}
			} {}
		
		template<typename... Args>
			requires(
				detail::inject_constructor_with_additional_args_callable<
					Type,
					kangaru::detail::call_result_t<
						detail::make_basic_autowire_injector_function<basic_deducer, Map, max_dependencies>,
						ref_result_t<container&>
					>,
					Args&&...
				>
			)
		constexpr void emplace(container& container, Args&&... args) {
			generic_service<Type>::emplace(
				in_place_construct{[&]() {
					return detail::inject_constructor_with_additional_args<Type>(
						detail::make_basic_autowire_injector<basic_deducer, Map, max_dependencies>(KANGARU5_NO_ADL(ref)(container)),
						KANGARU5_FWD(args)...
					);
				}}
			);
		}
		
		auto forward() -> Type& {
			return generic_service<Type>::instance();
		}
		
		template<typename... Args>
		static constexpr auto construct(inject_t<container_service> container_service, Args&&... args) {
			return inject(container_service.forward(), KANGARU5_FWD(args)...);
		}
	};
	
	template<typename Type, detail::map_list Map, std::size_t max_dependencies>
	struct unique_service<Type, detail::autowire_tag<Map, max_dependencies>> : unique_service<Type> {
		template<typename... Args>
		static auto construct(inject_t<container_service> container_service, Args&&... args)
			-> inject_result<std::unique_ptr<Type>>
		{
			auto& container = container_service.forward();
			return KANGARU5_NO_ADL(make_basic_spread_injector<basic_deducer, max_dependencies>)(KANGARU5_NO_ADL(ref)(container))(
				[&](deducer auto... deduce) requires(constructor_callable<Type, decltype(deduce)..., Args&&...>) {
					return KANGARU5_NO_ADL(inject)(
						std::make_unique<Type>(KANGARU5_NO_ADL(construct_in_place<Type>)(deduce..., KANGARU5_FWD(args)...))
					);
				}
			);
		}
	};
	
	template<typename Type, detail::map_list Map, std::size_t max_dependencies>
	struct shared_service<Type, detail::autowire_tag<Map, max_dependencies>> : shared_service<Type> {
		using shared_service<Type>::shared_service;
		
		template<typename... Args>
		static auto construct(inject_t<container_service> container_service, Args&&... args) {
			auto& container = container_service.forward();
			return KANGARU5_NO_ADL(make_basic_spread_injector<basic_deducer, max_dependencies>)(container)(
				[&](deducer auto... deduce) requires(constructor_callable<Type, decltype(deduce)..., Args&&...>) {
					return KANGARU5_NO_ADL(inject)(
						std::make_shared<Type>(KANGARU5_NO_ADL(construct_in_place<Type>)(deduce..., KANGARU5_FWD(args)...))
					);
				}
			);
		}
	};
	
	using autowire = detail::autowire_map<service, map<>, 8>;
	using autowire_single = detail::autowire_map<single_service, map<>, 8>;
	using autowire_unique = detail::autowire_map<unique_service, map<>, 8>;
	using autowire_shared = detail::autowire_map<shared_service, map<>, 8>;
}

namespace kangaru::compatibility_v4 {
	template<detail::map_list Map, std::size_t max_dependencies = 8>
	using mapped_autowire = detail::autowire_map<service, Map, max_dependencies>;
	
	template<typename T, detail::map_list Map = map<>, std::size_t max_dependencies = 8>
	using autowire_service = service<T, mapped_autowire<Map, max_dependencies>>;
	
	template<typename T, detail::map_list Map = map<>, std::size_t max_dependencies = 8>
	using autowire_single_service = single_service<T, mapped_autowire<Map, max_dependencies>>;
	
	template<typename T, detail::map_list Map = map<>, std::size_t max_dependencies = 8>
	using autowire_unique_service = unique_service<T, mapped_autowire<Map, max_dependencies>>;
	
	template<typename T, detail::map_list Map = map<>, std::size_t max_dependencies = 8>
	using autowire_shared_service = shared_service<T, mapped_autowire<Map, max_dependencies>>;
}


KANGARU5_EXPORT namespace kgr = kangaru::compatibility_v4;

#include "detail/undef.hpp"

#endif // KANGARU5_COMPATIBILITY_V4_HPP
