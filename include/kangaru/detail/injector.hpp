#ifndef KANGARU5_DETAIL_INJECTOR_HPP
#define KANGARU5_DETAIL_INJECTOR_HPP

#include "source.hpp"
#include "source_types.hpp"
#include "deducer.hpp"
#include "concepts.hpp"

#include <utility>

#include "define.hpp"

namespace kangaru::detail::injector {
	struct match_any {
		template<typename T>
		friend auto provide(provide_tag_t<T>, detail::concepts::forwarded<match_any> auto&&) -> T;
	};
	
	template<typename T, std::size_t>
	using expand = T;
	
	template<typename Function, typename>
	struct parameter_sequence_impl {};

	template<typename Function>
	requires detail::concepts::callable<Function>
	struct parameter_sequence_impl<Function, std::index_sequence<>> {
		using type = std::index_sequence<>;
	};
	
	template<typename Function, std::size_t head, std::size_t... tail>
	requires detail::concepts::callable<Function, kangaru::deducer<match_any>, expand<kangaru::deducer<match_any>, tail>...>
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
	
	namespace constraints {
		template<typename F, typename T>
		struct invocable_with_sequence_test {
			template<std::size_t... s> requires detail::concepts::callable<F, expand<T, s>...>
			inline constexpr auto operator()(std::index_sequence<s...>) -> void {}
		};
	}
	
	template<typename F, typename T, typename S>
	concept invocable_with_sequence = detail::concepts::callable<constraints::invocable_with_sequence_test<F, T>, S>;
	
	template<typename Function, typename Source, typename, typename = std::index_sequence<>>
	struct injectable_sequence {};

	template<typename Function, typename Source, std::size_t... drop>
	requires detail::concepts::callable<Function, expand<kangaru::deducer<match_any>, drop>...>
	struct injectable_sequence<Function, Source, std::index_sequence<>, std::index_sequence<drop...>> {
		using type = std::index_sequence<>;
	};
	
	template<typename Function, typename Source, std::size_t head, std::size_t... tail, std::size_t... drop>
	requires detail::concepts::callable<Function, kangaru::deducer<Source>, expand<kangaru::deducer<Source>, tail>..., expand<kangaru::deducer<match_any>, drop>...>
	struct injectable_sequence<Function, Source, std::index_sequence<head, tail...>, std::index_sequence<drop...>> {
		using type = std::index_sequence<head, tail...>;
	};
	
	template<typename Function, typename Source, std::size_t head, std::size_t... tail, std::size_t... drop>
	struct injectable_sequence<Function, Source, std::index_sequence<head, tail...>, std::index_sequence<drop...>> :
		injectable_sequence<Function, Source, std::index_sequence<(tail - 1)...>, std::index_sequence<drop..., sizeof...(drop)>> {};
	
	template<typename Function, typename Source, std::size_t max>
	using spread_sequence_t = typename injectable_sequence<Function, Source, parameter_sequence_t<Function, max>>::type;
}

namespace kangaru {
	template<typename Source>
	struct simple_injector {
		explicit constexpr simple_injector(Source source) noexcept : source{std::move(source)} {}
		
		constexpr auto operator()(auto&& function) & -> decltype(auto) requires detail::concepts::callable<decltype(function), deducer<Source&>> {
			return KANGARU5_FWD(function)(deducer<Source&>{source});
		}
		
		constexpr auto operator()(detail::concepts::callable<deducer<Source const&>> auto&& function) const& -> decltype(auto) requires detail::concepts::callable<decltype(function), deducer<Source const&>> {
			return KANGARU5_FWD(function)(deducer<Source const&>{source});
		}
		
		constexpr auto operator()(detail::concepts::callable<deducer<Source&&>> auto&& function) && -> decltype(auto) {
			return KANGARU5_FWD(function)(deducer<Source&&>{std::move(source)});
		}
		
		constexpr auto operator()(detail::concepts::callable<deducer<Source const&&>> auto&& function) const&& -> decltype(auto) {
			return KANGARU5_FWD(function)(deducer<Source const&&>{std::move(source)});
		}
		
	private:
		Source source;
	};
	
	template<typename Source, std::size_t max>
	struct basic_spread_injector {
		explicit constexpr basic_spread_injector(Source source) noexcept : source{std::move(source)} {}
		
		template<typename F, typename Seq = detail::injector::spread_sequence_t<F, Source, max>>
		constexpr auto operator()(F&& function) & -> decltype(auto) requires detail::injector::invocable_with_sequence<F, deducer<Source&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename Seq = detail::injector::spread_sequence_t<F, Source, max>>
		constexpr auto operator()(F&& function) && -> decltype(auto) requires detail::injector::invocable_with_sequence<F, deducer<Source&&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
		template<typename F, typename Seq = detail::injector::spread_sequence_t<F, Source, max>>
		constexpr auto operator()(F&& function) const& -> decltype(auto) requires detail::injector::invocable_with_sequence<F, deducer<Source const&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename Seq = detail::injector::spread_sequence_t<F, Source, max>>
		constexpr auto operator()(F&& function) const&& -> decltype(auto) requires detail::injector::invocable_with_sequence<F, deducer<Source const&&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
	private:
		template<std::size_t... s>
		static constexpr auto fill_impl(std::index_sequence<s...>, auto&& function, auto&& source) -> decltype(auto) {
			using Deducer = deducer<decltype(source)>;
			return KANGARU5_FWD(function)((void(s), Deducer{KANGARU5_FWD(source)})...);
		}
		
		Source source;
	};
	
	template<typename Source>
	using fast_spread_injector = basic_spread_injector<Source, 6>;
	
	template<typename Source>
	using spread_injector = basic_spread_injector<Source, 12>;
	
	template<typename Source>
	using slow_spread_injector = basic_spread_injector<Source, 24>;
	
	template<typename Injector1, typename Injector2>
	struct composed_injector {
		explicit constexpr composed_injector(Injector1 injector1, Injector2 injector2) noexcept :
			injector1{std::move(injector1)}, injector2{std::move(injector2)} {}
		
		constexpr static auto call_function(auto&& function, auto... deduce_first) {
			return [&function, deduce_first...](auto... deduce_second) -> decltype(KANGARU5_FWD(function)(deduce_first..., deduce_second...)) {
				return KANGARU5_FWD(function)(deduce_first..., deduce_second...);
			};
		}
		
		template<typename Injector, typename Function>
		struct inner_injector {
			Injector injector;
			Function function;
			
			constexpr auto operator()(auto... deduce_first) -> decltype(KANGARU5_FWD(injector)(call_function(KANGARU5_FWD(function), deduce_first...))) {
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
		Injector1 injector1;
		Injector2 injector2;
	};
	
	inline constexpr auto compose(auto&& injector) -> decltype(KANGARU5_FWD(injector)) {
		return KANGARU5_FWD(injector);
	}
	
	inline constexpr auto compose(auto&& first, auto&& second, auto&&... rest) {
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
