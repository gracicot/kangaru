#ifndef KANGARU5_DETAIL_INJECTOR_HPP
#define KANGARU5_DETAIL_INJECTOR_HPP

#include "source.hpp"
#include "source_types.hpp"
#include "deducer.hpp"
#include "concepts.hpp"
#include "utility.hpp"

#include <type_traits>
#include <utility>

#include "define.hpp"

namespace kangaru::detail::injector {
	template<typename Function, typename>
	struct parameter_sequence_impl {};
	
	template<typename Function>
		requires callable<Function>
	struct parameter_sequence_impl<Function, std::index_sequence<>> {
		using type = std::index_sequence<>;
	};
	
	template<typename Function, std::size_t head, std::size_t... tail>
		requires callable<Function, kangaru::placeholder_deducer, detail::utility::expand<kangaru::placeholder_deducer, tail>...>
	struct parameter_sequence_impl<Function, std::index_sequence<head, tail...>> {
		using type = std::index_sequence<head, tail...>;
	};
	
	template<typename Function, std::size_t head, std::size_t... tail>
	struct parameter_sequence_impl<Function, std::index_sequence<head, tail...>> :
		parameter_sequence_impl<Function, std::index_sequence<(tail - 1)...>> {};
	
	template<typename F, std::size_t max>
	using parameter_sequence = parameter_sequence_impl<F, std::make_index_sequence<max>>;
	
	template<typename F, std::size_t max>
	using parameter_sequence_t = typename parameter_sequence<F, max>::type;
	
	template<typename F, typename Deducer, typename S>
	concept invocable_with_strict_deducer_sequence = requires(F&& function, Deducer deduce, S seq) {
		detail::deducer::invoke_with_strict_deducer_sequence(seq, KANGARU5_FWD(function), deduce);
	};
	
	template<typename F, typename Deducer, typename S>
	concept invocable_with_deducer_sequence = requires(F&& function, Deducer deduce, S seq) {
		detail::deducer::invoke_with_deducer_sequence(seq, KANGARU5_FWD(function), deduce);
	};
	
	template<template<typename Deducer, std::size_t nth> typename Expand, typename Function, typename Deducer, typename, typename = std::index_sequence<>>
	struct injectable_sequence {};
	
	template<template<typename Deducer, std::size_t nth> typename Expand, typename Function, typename Deducer, std::size_t... drop>
		requires callable<Function, Expand<kangaru::placeholder_deducer, drop>...>
	struct injectable_sequence<Expand, Function, Deducer, std::index_sequence<>, std::index_sequence<drop...>> {
		using type = std::index_sequence<>;
	};
	
	template<template<typename Deducer, std::size_t nth> typename Expand, typename Function, typename Deducer, std::size_t head, std::size_t... tail, std::size_t... drop>
		requires callable<Function, Deducer, Expand<Deducer, tail>..., detail::utility::expand<kangaru::placeholder_deducer, drop>...>
	struct injectable_sequence<Expand, Function, Deducer, std::index_sequence<head, tail...>, std::index_sequence<drop...>> {
		using type = std::index_sequence<head, tail...>;
	};
	
	template<template<typename Deducer, std::size_t nth> typename Expand, typename Function, typename Deducer, std::size_t head, std::size_t... tail, std::size_t... drop>
	struct injectable_sequence<Expand, Function, Deducer, std::index_sequence<head, tail...>, std::index_sequence<drop...>> :
		injectable_sequence<Expand, Function, Deducer, std::index_sequence<(tail - 1)...>, std::index_sequence<drop..., sizeof...(drop)>> {};
	
	template<typename Function, typename Deducer, std::size_t max>
	using spread_sequence_t = typename injectable_sequence<detail::utility::expand, Function, Deducer, parameter_sequence_t<Function, max>>::type;
	
	template<typename Function, std::size_t max>
	struct make_strict_expander {
		template<typename Deducer, std::size_t nth>
		using ttype = detail::deducer::filtered_value_category_deducer_for<Function, Deducer, nth, max>;
	};
	
	template<typename Function, typename Deducer, std::size_t max>
	using strict_spread_sequence_t = typename injectable_sequence<make_strict_expander<Function, max>::template ttype, Function, Deducer, parameter_sequence_t<Function, max>>::type;
}

namespace kangaru {
	template<typename T>
	concept injector = object<T> and std::move_constructible<T>;
	
	template<typename F, std::size_t max>
	concept reflectable_function = requires {
		requires detail::injector::parameter_sequence_impl<F, std::make_index_sequence<max>>::type;
	};
	
	template<source Source>
	struct simple_injector {
		explicit constexpr simple_injector(Source source) noexcept : source{std::move(source)} {}
		
		constexpr auto operator()(auto&& function) & -> decltype(auto) requires callable<decltype(function), basic_deducer<Source&>> {
			return kangaru::invoke_with_deducers(KANGARU5_FWD(function), basic_deducer<Source&>{source});
		}
		
		constexpr auto operator()(callable<basic_deducer<Source const&>> auto&& function) const& -> decltype(auto) requires callable<decltype(function), basic_deducer<Source const&>> {
			return kangaru::invoke_with_deducers(KANGARU5_FWD(function), basic_deducer<Source const&>{source});
		}
		
		constexpr auto operator()(callable<basic_deducer<Source&&>> auto&& function) && -> decltype(auto) {
			return kangaru::invoke_with_deducers(KANGARU5_FWD(function), basic_deducer<Source&&>{std::move(source)});
		}
		
		constexpr auto operator()(callable<basic_deducer<Source const&&>> auto&& function) const&& -> decltype(auto) {
			return kangaru::invoke_with_deducers(KANGARU5_FWD(function), basic_deducer<Source const&&>{std::move(source)});
		}
		
	private:
		Source source;
	};
	
	template<source Source, std::size_t max>
	struct basic_spread_injector {
		explicit constexpr basic_spread_injector(Source source) noexcept : source{std::move(source)} {}
		
		template<typename F, typename Seq = detail::injector::spread_sequence_t<F, basic_deducer<Source&>, max>>
		constexpr auto operator()(F&& function) & -> decltype(auto) requires detail::injector::invocable_with_deducer_sequence<F, basic_deducer<Source&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename Seq = detail::injector::spread_sequence_t<F, basic_deducer<Source&&>, max>>
		constexpr auto operator()(F&& function) && -> decltype(auto) requires detail::injector::invocable_with_deducer_sequence<F, basic_deducer<Source&&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
		template<typename F, typename Seq = detail::injector::spread_sequence_t<F, basic_deducer<Source const&>, max>>
		constexpr auto operator()(F&& function) const& -> decltype(auto) requires detail::injector::invocable_with_deducer_sequence<F, basic_deducer<Source const&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename Seq = detail::injector::spread_sequence_t<F, basic_deducer<Source const&&>, max>>
		constexpr auto operator()(F&& function) const&& -> decltype(auto) requires detail::injector::invocable_with_deducer_sequence<F, basic_deducer<Source const&&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
	private:
		template<std::size_t... s>
		static constexpr auto fill_impl(std::index_sequence<s...>, auto&& function, auto&& source) -> decltype(auto) {
			using Deducer = basic_deducer<decltype(source)>;
			return kangaru::invoke_with_deducers(KANGARU5_FWD(function), (void(s), Deducer{KANGARU5_FWD(source)})...);
		}
		
		Source source;
	};
	
	template<source Source, std::size_t max>
	struct basic_strict_spread_injector {
		explicit constexpr basic_strict_spread_injector(Source source) noexcept : source{std::move(source)} {}
		
		template<typename F, typename Seq = detail::injector::strict_spread_sequence_t<F, strict_deducer<Source&>, max>>
		constexpr auto operator()(F&& function) & -> decltype(auto) requires detail::injector::invocable_with_strict_deducer_sequence<F, strict_deducer<Source&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename Seq = detail::injector::strict_spread_sequence_t<F, strict_deducer<Source&&>, max>>
		constexpr auto operator()(F&& function) && -> decltype(auto) requires detail::injector::invocable_with_strict_deducer_sequence<F, strict_deducer<Source&&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
		template<typename F, typename Seq = detail::injector::strict_spread_sequence_t<F, strict_deducer<Source const&>, max>>
		constexpr auto operator()(F&& function) const& -> decltype(auto) requires detail::injector::invocable_with_strict_deducer_sequence<F, strict_deducer<Source const&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename Seq = detail::injector::strict_spread_sequence_t<F, strict_deducer<Source const&&>, max>>
		constexpr auto operator()(F&& function) const&& -> decltype(auto) requires detail::injector::invocable_with_strict_deducer_sequence<F, strict_deducer<Source const&&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
	private:
		template<std::size_t... s>
		static constexpr auto fill_impl(std::index_sequence<s...>, auto&& function, auto&& source) -> decltype(auto) {
			using Deducer = strict_deducer<decltype(source)>;
			return kangaru::invoke_with_strict_deducers(KANGARU5_FWD(function), (void(s), Deducer{KANGARU5_FWD(source)})...);
		}
		
		Source source;
	};
	
	template<source Source>
	using fast_spread_injector = basic_spread_injector<Source, 4>;
	
	template<source Source>
	using spread_injector = basic_spread_injector<Source, 8>;
	
	template<source Source>
	using slow_spread_injector = basic_spread_injector<Source, 16>;
	
	template<source Source>
	using strict_fast_spread_injector = basic_strict_spread_injector<Source, 4>;
	
	template<source Source>
	using strict_spread_injector = basic_strict_spread_injector<Source, 8>;
	
	template<source Source>
	using strict_slow_spread_injector = basic_strict_spread_injector<Source, 16>;
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_fast_spread_injector(Source&& source) {
		return fast_spread_injector<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_spread_injector(Source&& source) {
		return spread_injector<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_slow_spread_injector(Source&& source) {
		return slow_spread_injector<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
		
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_strict_fast_spread_injector(Source&& source) {
		return strict_fast_spread_injector<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_strict_spread_injector(Source&& source) {
		return strict_spread_injector<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_strict_slow_spread_injector(Source&& source) {
		return strict_slow_spread_injector<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<injector Injector1, injector Injector2>
	struct composed_injector {
		explicit constexpr composed_injector(Injector1 injector1, Injector2 injector2) noexcept :
			injector1{std::move(injector1)}, injector2{std::move(injector2)} {}
		
		constexpr static auto call_function(auto&& function, deducer auto... deduce_first) {
			// Workaround for clang bug: https://github.com/llvm/llvm-project/issues/61589
			using helper = decltype_helper<decltype(function), decltype(deduce_first)...>;
			return [&function, deduce_first...](deducer auto... deduce_second) -> decltype(std::declval<helper>()(deduce_second...)) {
				return KANGARU5_FWD(function)(detail::utility::decay_copy(deduce_first)..., deduce_second...);
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
	
	template<typename Injector> requires(injector<std::remove_cvref_t<Injector>>)
	inline constexpr auto compose(Injector&& injector) -> decltype(KANGARU5_FWD(injector)) {
		return KANGARU5_FWD(injector);
	}
	
	template<typename Injector1, typename Injector2, typename... Injectors>
		requires(
			    injector<std::remove_cvref_t<Injector1>>
			and injector<std::remove_cvref_t<Injector2>>
			and (... and injector<std::remove_cvref_t<Injectors>>)
		)
	inline constexpr auto compose(Injector1&& first, Injector2&& second, Injectors&&... rest) {
		return composed_injector{KANGARU5_FWD(first), compose(KANGARU5_FWD(second), KANGARU5_FWD(rest)...)};
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
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_INJECTOR_HPP
