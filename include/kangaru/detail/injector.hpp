#ifndef KANGARU5_DETAIL_INJECTOR_HPP
#define KANGARU5_DETAIL_INJECTOR_HPP

#include "source.hpp"
#include "deducer.hpp"
#include "concepts.hpp"
#include "utility.hpp"
#include "source_reference_wrapper.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <utility>
#include <concepts>
#endif

#include "define.hpp"

namespace kangaru::detail::injector_private {
	// TODO: Report issue to clang
	template<typename Function, std::size_t... S>
	inline constexpr auto callable_workaround_for_clang =
		callable<Function, detail::expand<kangaru::placeholder_deducer, S>...>;
	
	template<typename Function, typename>
	struct parameter_sequence_impl {};
	
	template<typename Function>
		requires(callable<Function>)
	struct parameter_sequence_impl<Function, std::index_sequence<>> {
		using type = std::index_sequence<>;
		using return_type = decltype(std::declval<Function>()());
	};
	
	template<typename Function, std::size_t head, std::size_t... tail>
		requires(callable<Function, kangaru::placeholder_deducer, detail::expand<kangaru::placeholder_deducer, tail>...>)
	struct parameter_sequence_impl<Function, std::index_sequence<head, tail...>> {
		using type = std::index_sequence<head, tail...>;
		using return_type = decltype(
			call_with_deducers(
				std::declval<Function>(),
				std::declval<kangaru::placeholder_deducer>(),
				(tail, std::declval<kangaru::placeholder_deducer>())...
			)
		);
	};
	
	template<typename Function, std::size_t head, std::size_t... tail>
	struct parameter_sequence_impl<Function, std::index_sequence<head, tail...>> :
		parameter_sequence_impl<Function, std::index_sequence<(tail - 1)...>> {};
	
	template<typename F, std::size_t max>
	using parameter_sequence = parameter_sequence_impl<F, std::make_index_sequence<max>>;
	
	template<typename F, std::size_t max>
	using parameter_sequence_t = typename parameter_sequence<F, max>::type;
	
	template<template<typename Deducer, std::size_t nth> typename Expand, typename Function, typename Deducer, typename, typename = std::index_sequence<>>
	struct injectable_sequence {};
	
	template<template<typename Deducer, std::size_t nth> typename Expand, typename Function, typename Deducer, std::size_t... drop>
		requires(callable<Function, Expand<kangaru::placeholder_deducer, drop>...>)
	struct injectable_sequence<Expand, Function, Deducer, std::index_sequence<>, std::index_sequence<drop...>> {
		using type = std::index_sequence<>;
	};
	
	template<template<typename Deducer, std::size_t nth> typename Expand, typename Function, typename Deducer, std::size_t head, std::size_t... tail, std::size_t... drop>
		requires(callable<Function, Deducer, Expand<Deducer, tail>..., detail::expand<kangaru::placeholder_deducer, drop>...>)
	struct injectable_sequence<Expand, Function, Deducer, std::index_sequence<head, tail...>, std::index_sequence<drop...>> {
		using type = std::index_sequence<head, tail...>;
	};
	
	template<template<typename Deducer, std::size_t nth> typename Expand, typename Function, typename Deducer, std::size_t head, std::size_t... tail, std::size_t... drop>
	struct injectable_sequence<Expand, Function, Deducer, std::index_sequence<head, tail...>, std::index_sequence<drop...>> :
		injectable_sequence<Expand, Function, Deducer, std::index_sequence<(tail - 1)...>, std::index_sequence<drop..., sizeof...(drop)>> {};
	
	template<typename Function, typename Deducer, std::size_t max>
	using spread_sequence_t = typename injectable_sequence<detail::expand, Function, Deducer, parameter_sequence_t<Function, max>>::type;
	
	template<typename F, kangaru::deducer Deducer, typename>
	inline constexpr auto callable_with_deducer_sequence_v = false;
	
	template<typename F, std::size_t... S, kangaru::deducer Deducer>
	inline constexpr auto callable_with_deducer_sequence_v<F, Deducer, std::index_sequence<S...>> =
		callable_with_deducers<F, detail::expand<Deducer, S>...>;
} // namespace kangaru::detail::injector_private

KANGARU5_EXPORT namespace kangaru {
	template<typename T>
	concept injector = function_object<T>;
	
	template<typename F>
	concept make_injector = copiable_object<F> and function_object<F>;
	
	template<typename T>
	concept forwarded_injector = injector<std::remove_cvref_t<T>>;
	
	template<typename F, std::size_t max>
	concept reflectable_function = (forwarded_function_object<F> or std::is_function_v<std::remove_pointer_t<F>>) and requires {
		typename detail::injector_private::parameter_sequence_impl<F, std::make_index_sequence<max>>::type;
	};
	
	template<typename F, std::size_t max> requires reflectable_function<F, max>
	using reflected_return_type = typename detail::injector_private::parameter_sequence_impl<F, std::make_index_sequence<max>>::return_type;
	
	template<source Source, template<source_ref> typename Deducer, std::size_t N>
	struct basic_fixed_injector {
		explicit constexpr basic_fixed_injector(Source source) : source(std::move(source)) {}
		
		template<allows_construction_of<Source> S>
			requires(not std::same_as<std::remove_cvref_t<S>, Source>)
		explicit constexpr basic_fixed_injector(S&& source) : source(KANGARU5_FWD(source)) {}
		
		constexpr auto operator()(auto&& function) & -> decltype(auto)
			requires detail::injector_private::callable_with_deducer_sequence_v<decltype(function), Deducer<Source&>, std::make_index_sequence<N>>
		{
			return expand_deducers(std::make_index_sequence<N>{}, KANGARU5_FWD(function), source);
		}
		
		constexpr auto operator()(auto&& function) const& -> decltype(auto)
			requires detail::injector_private::callable_with_deducer_sequence_v<decltype(function), Deducer<Source const&>, std::make_index_sequence<N>>
		{
			return expand_deducers(std::make_index_sequence<N>{}, KANGARU5_FWD(function), source);
		}
		
		constexpr auto operator()(auto&& function) && -> decltype(auto)
			requires detail::injector_private::callable_with_deducer_sequence_v<decltype(function), Deducer<Source&&>, std::make_index_sequence<N>>
		{
			return expand_deducers(std::make_index_sequence<N>{}, KANGARU5_FWD(function), std::move(source));
		}
		
		constexpr auto operator()(auto&& function) const&& -> decltype(auto)
			requires detail::injector_private::callable_with_deducer_sequence_v<decltype(function), Deducer<Source const&&>, std::make_index_sequence<N>>
		{
			return expand_deducers(std::make_index_sequence<N>{}, KANGARU5_FWD(function), std::move(source));
		}
		
	private:
		template<std::size_t... s>
		static constexpr auto expand_deducers(std::index_sequence<s...>, auto&& function, auto&& source) -> decltype(auto) {
			using deducer = Deducer<decltype(source)>;
			return KANGARU5_NO_ADL(call_with_deducers)(KANGARU5_FWD(function), (void(s), deducer{KANGARU5_FWD(source)})...);
		}
		
		Source source;
	};
	
	template<source Source, std::size_t max>
	using fixed_injector = basic_fixed_injector<Source, basic_deducer, max>;
	
	template<source Source>
	using simple_injector = fixed_injector<Source, 1>;
	
	struct make_simple_injector_function {
		template<typename Source> requires source<std::remove_cvref_t<Source>>
		inline constexpr auto operator()(Source&& source) const {
			return simple_injector<deduced_source_type<Source>>{KANGARU5_FWD(source)};
		}
	};
	
	inline constexpr auto make_simple_injector = make_simple_injector_function{};
	
	template<std::size_t N>
	struct make_fixed_injector_function {
		template<typename Source> requires source<std::remove_cvref_t<Source>>
		inline constexpr auto operator()(Source&& source) const {
			return fixed_injector<deduced_source_type<Source>, N>{KANGARU5_FWD(source)};
		}
	};
	
	template<std::size_t N>
	inline constexpr auto make_fixed_injector = make_fixed_injector_function<N>{};
	
	template<source Source, std::size_t max>
	using strict_fixed_injector = basic_fixed_injector<Source, strict_deducer, max>;
	
	template<source Source>
	using strict_simple_injector = strict_fixed_injector<Source, 1>;
	
	struct make_strict_simple_injector_function {
		template<typename Source> requires source<std::remove_cvref_t<Source>>
		inline constexpr auto operator()(Source&& source) const {
			return strict_simple_injector<deduced_source_type<Source>>{KANGARU5_FWD(source)};
		}
	};
	
	inline constexpr auto make_strict_simple_injector = make_strict_simple_injector_function{};
	
	template<std::size_t N>
	struct make_strict_fixed_injector_function {
		template<typename Source> requires source<std::remove_cvref_t<Source>>
		inline constexpr auto operator()(Source&& source) const {
			return strict_fixed_injector<deduced_source_type<Source>, N>{KANGARU5_FWD(source)};
		}
	};
	
	template<std::size_t N>
	inline constexpr auto make_strict_fixed_injector = make_strict_fixed_injector_function<N>{};
	
	template<source Source, template<source_ref> typename Deducer, std::size_t max>
	struct basic_spread_injector {
		explicit constexpr basic_spread_injector(Source source) : source(std::move(source)) {}
		
		template<allows_construction_of<Source> S>
			requires(not std::same_as<std::remove_cvref_t<S>, Source>)
		explicit constexpr basic_spread_injector(S&& source) : source(KANGARU5_FWD(source)) {}
		
		template<reflectable_function<max> F, typename..., typename Seq = detail::injector_private::spread_sequence_t<F, Deducer<Source&>, max>>
		constexpr auto operator()(F&& function) & -> decltype(auto) requires detail::injector_private::callable_with_deducer_sequence_v<F, Deducer<Source&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<reflectable_function<max> F, typename..., typename Seq = detail::injector_private::spread_sequence_t<F, Deducer<Source&&>, max>>
		constexpr auto operator()(F&& function) && -> decltype(auto) requires detail::injector_private::callable_with_deducer_sequence_v<F, Deducer<Source&&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
		template<reflectable_function<max> F, typename..., typename Seq = detail::injector_private::spread_sequence_t<F, Deducer<Source const&>, max>>
		constexpr auto operator()(F&& function) const& -> decltype(auto) requires detail::injector_private::callable_with_deducer_sequence_v<F, Deducer<Source const&>, Seq> {
			return expand_deducers(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<reflectable_function<max> F, typename..., typename Seq = detail::injector_private::spread_sequence_t<F, Deducer<Source const&&>, max>>
		constexpr auto operator()(F&& function) const&& -> decltype(auto) requires detail::injector_private::callable_with_deducer_sequence_v<F, Deducer<Source const&&>, Seq> {
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
	
	template<source Source>
	using fast_spread_injector = basic_spread_injector<Source, basic_deducer, 4>;
	
	template<source Source>
	using spread_injector = basic_spread_injector<Source, basic_deducer, 8>;
	
	template<source Source>
	using slow_spread_injector = basic_spread_injector<Source, basic_deducer, 16>;
	
	template<source Source>
	using optional_injector = basic_spread_injector<Source, basic_deducer, 1>;
	
	template<template<typename> typename Deducer, std::size_t max>
	struct make_basic_spread_injector_function {
		template<forwarded_source Source>
		inline constexpr auto operator()(Source&& source) const {
			return basic_spread_injector<deduced_source_type<Source>, Deducer, max>{
				KANGARU5_FWD(source)
			};
		}
	};
	
	template<template<typename> typename Deducer, std::size_t max>
	inline constexpr auto make_basic_spread_injector = make_basic_spread_injector_function<Deducer, max>{};
	
	using make_fast_spread_injector_function = make_basic_spread_injector_function<basic_deducer, 4>;
	using make_spread_injector_function = make_basic_spread_injector_function<basic_deducer, 8>;
	using make_slow_spread_injector_function = make_basic_spread_injector_function<basic_deducer, 16>;
	using make_optional_injector_function = make_basic_spread_injector_function<basic_deducer, 1>;
	
	inline constexpr auto make_fast_spread_injector = make_fast_spread_injector_function{};
	inline constexpr auto make_spread_injector = make_spread_injector_function{};
	inline constexpr auto make_slow_spread_injector = make_slow_spread_injector_function{};
	inline constexpr auto make_optional_injector = make_optional_injector_function{};
	
	template<source Source>
	using strict_fast_spread_injector = basic_spread_injector<Source, strict_deducer, 4>;
	
	template<source Source>
	using strict_spread_injector = basic_spread_injector<Source, strict_deducer, 8>;
	
	template<source Source>
	using strict_slow_spread_injector = basic_spread_injector<Source, strict_deducer, 16>;
	
	template<source Source>
	using strict_optional_injector = basic_spread_injector<Source, strict_deducer, 1>;
	
	using make_strict_fast_spread_injector_function = make_basic_spread_injector_function<strict_deducer, 4>;
	using make_strict_spread_injector_function = make_basic_spread_injector_function<strict_deducer, 8>;
	using make_strict_slow_spread_injector_function = make_basic_spread_injector_function<strict_deducer, 16>;
	using make_strict_optional_injector_function = make_basic_spread_injector_function<strict_deducer, 1>;
	
	inline constexpr auto make_strict_spread_injector = make_strict_spread_injector_function{};
	inline constexpr auto make_strict_slow_spread_injector = make_strict_slow_spread_injector_function{};
	inline constexpr auto make_strict_fast_spread_injector = make_strict_fast_spread_injector_function{};
	inline constexpr auto make_strict_optional_injector = make_strict_optional_injector_function{};
	
	template<injector Injector1, injector Injector2>
	struct concatenated_injector {
		// TODO: Support immovable sources
		explicit constexpr concatenated_injector(Injector1 injector1, Injector2 injector2) noexcept :
			injector1{std::move(injector1)}, injector2{std::move(injector2)} {}
		
		constexpr static auto call_function(auto&& function, deducer auto... deduce_first) {
			// Workaround for clang bug: https://github.com/llvm/llvm-project/issues/61589
			using helper = decltype_helper<decltype(function), decltype(deduce_first)...>;
			return [&function, deduce_first...](deducer auto... deduce_second) -> decltype(std::declval<helper>()(deduce_second...)) {
				return KANGARU5_FWD(function)(detail::decay_copy(deduce_first)..., deduce_second...);
			};
		}
		
		template<typename Injector, typename Function>
		struct inner_injector {
			Injector injector;
			Function function;
			
			constexpr auto operator()(deducer auto... deduce_first) -> decltype(KANGARU5_FWD(injector)(call_function(KANGARU5_FWD(function), deduce_first...))) {
				return KANGARU5_FWD(injector)(call_function(KANGARU5_FWD(function), deduce_first...));
			}
		};
		
		template<typename F>
		constexpr auto operator()(F&& function) & -> decltype(std::declval<Injector1&>()(inner_injector<Injector2&, F&&>{std::declval<Injector2&>(), KANGARU5_FWD(function)})) {
			return injector1(inner_injector<Injector2&, F&&>{injector2, KANGARU5_FWD(function)});
		}
		
		template<typename F>
		constexpr auto operator()(F&& function) const& -> decltype(std::declval<Injector1 const&>()(inner_injector<Injector2 const&, F&&>{std::declval<Injector2 const&>(), KANGARU5_FWD(function)})) {
			return injector1(inner_injector<Injector2 const&, F&&>{injector2, KANGARU5_FWD(function)});
		}
		
		template<typename F>
		constexpr auto operator()(F&& function) && -> decltype(std::declval<Injector1&&>()(inner_injector<Injector2&&, F&&>{std::declval<Injector2&&>(), KANGARU5_FWD(function)})) {
			return std::move(injector1)(inner_injector<Injector2&&, F&&>{std::move(injector2), KANGARU5_FWD(function)});
		}
		
		template<typename F>
		constexpr auto operator()(F&& function) const&& -> decltype(std::declval<Injector1 const&&>()(inner_injector<Injector2 const&&, F&&>{std::declval<Injector2 const&&>(), KANGARU5_FWD(function)})) {
			return std::move(injector1)(inner_injector<Injector2 const&&, F&&>{std::move(injector2), KANGARU5_FWD(function)});
		}
		
	private:
		template<typename F, typename... DeduceFirst>
		struct decltype_helper {
			template<typename... Args>
			auto operator()(Args...) -> decltype(std::declval<F>()(std::declval<DeduceFirst>()..., std::declval<Args>()...));
		};
		
		Injector1 injector1;
		Injector2 injector2;
	};
	
	template<forwarded_injector Injector1, forwarded_injector... Injectors>
	inline constexpr auto concat(Injector1&& first, Injectors&&... rest) {
		if constexpr (sizeof...(rest) > 1) {
			return concatenated_injector{KANGARU5_FWD(first), concat(KANGARU5_FWD(rest)...)};
		} else {
			return concatenated_injector{KANGARU5_FWD(first), KANGARU5_FWD(rest)...};
		}
	}
	
	// TODO: While this implementation is simpler, it makes GCC cry
	/* auto compose(auto first, auto second) {
		constexpr auto call_function = [](auto function, auto... args_first) {
			return [function, args_first...](auto... args_second) -> decltype(function(args_first..., args_second...)) {
				return function(args_first..., args_second...);
			};
		};
		
		auto inner_injector = [second, call_function](auto function) {
			return [call_function, function, second](auto... args_first) -> decltype(second(call_function(function, args_first...))) {
				return second(call_function(function, args_first...));
			};
		};
		
		return [inner_injector, first](auto function) -> decltype(first(inner_injector(function))) {
			return first(inner_injector(function));
		};
	} */
	
	template<function_object Function, function_object MakeInjector>
	struct call_with_injector {
	private:
		template<injectable T, forwarded<Function> F>
		struct template_call {
			KANGARU5_NO_UNIQUE_ADDRESS
			F&& func;
			
			template<deducer... Deducers>
			constexpr auto operator()(Deducers... deduce) const
				-> detail::call_result_t<F&&, exclude_deducer<T, decltype(deduce)>...>
			// TODO: It seems we need to check the return type here and I can't understand why.
			//       Skipping this check or putting this check anywhere else completely fails.
			requires(
				callable_returns<
					T,
					F&&,
					exclude_deducer<T, decltype(deduce)>...
				>
			) {
				return KANGARU5_FWD(func)(KANGARU5_NO_ADL(exclude_deduction<T>)(deduce)...);
			}
			
			// TODO: We can't enforce exclude deducer properly if we want to generally call the template
			//
			// constexpr auto operator()(deducer auto... deduce) const -> decltype(auto)
			// requires callable_template_1t<
			// 	F&&,
			// 	T,
			// 	exclude_deducer<T, decltype(deduce)>...
			// > {
			// 	return KANGARU5_FWD(func).template operator()<T>(KANGARU5_NO_ADL(exclude_deduction<T>)(deduce)...);
			// }
		};
		
	public:
		constexpr call_with_injector()
			requires(std::default_initializable<Function> and std::default_initializable<MakeInjector>) :
			function{},
			make_injector{} {}
		
		explicit constexpr call_with_injector(Function function)
			requires(std::default_initializable<MakeInjector>) :
			function{std::move(function)},
			make_injector{} {}
		
		constexpr call_with_injector(Function function, MakeInjector make_injector) :
			function{std::move(function)},
			make_injector{std::move(make_injector)} {}
		
		template<forwarded_source Source> requires(callable<detail::call_result_t<MakeInjector const&, fwd_ref_result_t<Source&&>>, Function&>)
		constexpr auto operator()(Source&& source) & -> decltype(auto) {
			return std::as_const(make_injector)(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)))(function);
		}
		
		template<forwarded_source Source> requires(callable<detail::call_result_t<MakeInjector const&, fwd_ref_result_t<Source&&>>, Function&&>)
		constexpr auto operator()(Source&& source) && -> decltype(auto) {
			return std::as_const(make_injector)(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)))(std::move(function));
		}
		
		template<forwarded_source Source> requires(callable<detail::call_result_t<MakeInjector const&, fwd_ref_result_t<Source&&>>, Function const&>)
		constexpr auto operator()(Source&& source) const& -> decltype(auto) {
			return std::as_const(make_injector)(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)))(function);
		}
		
		template<forwarded_source Source> requires(callable<detail::call_result_t<MakeInjector const&, fwd_ref_result_t<Source&&>>, Function const&&>)
		constexpr auto operator()(Source&& source) const&& -> decltype(auto) {
			return std::as_const(make_injector)(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)))(std::move(function));
		}
		
		template<injectable T, forwarded_source Source> requires(callable<detail::call_result_t<MakeInjector const&, fwd_ref_result_t<Source&&>>, template_call<T, Function&>>)
		constexpr auto operator()(Source&& source) & -> decltype(auto) {
			return std::as_const(make_injector)(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)))(template_call<T, Function&>{function});
		}
		
		template<injectable T, forwarded_source Source> requires(callable<detail::call_result_t<MakeInjector const&, fwd_ref_result_t<Source&&>>, template_call<T, Function&&>>)
		constexpr auto operator()(Source&& source) && -> decltype(auto) {
			return std::as_const(make_injector)(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)))(template_call<T, Function&&>{std::move(function)});
		}
		
		template<injectable T, forwarded_source Source> requires(callable<detail::call_result_t<MakeInjector const&, fwd_ref_result_t<Source&&>>, template_call<T, Function const&>>)
		constexpr auto operator()(Source&& source) const& -> decltype(auto) {
			return std::as_const(make_injector)(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)))(template_call<T, Function const&>{function});
		}
		
		template<injectable T, forwarded_source Source> requires(callable<detail::call_result_t<MakeInjector const&, fwd_ref_result_t<Source&&>>, template_call<T, Function const&&>>)
		constexpr auto operator()(Source&& source) const&& -> decltype(auto) {
			return std::as_const(make_injector)(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)))(template_call<T, Function const&&>{std::move(function)});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		Function function;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_INJECTOR_HPP
