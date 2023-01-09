#ifndef KANGARU5_DETAIL_INJECTOR_HPP
#define KANGARU5_DETAIL_INJECTOR_HPP

#include "source.hpp"
#include "source_types.hpp"
#include "deducer.hpp"
#include "concepts.hpp"

#include <utility>

#include "define.hpp"

namespace kangaru::detail::injector {
	struct decl_source {
		template<typename T>
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<decl_source> auto&&) -> T;
	};
	
	template<typename T, std::size_t>
	using expand = T;
	
	template<typename F, typename T, std::size_t... s>
	concept invocable_with_sequence_expanded = std::invocable<F, expand<T, s>...>;
	
	template<typename Function, typename>
	struct fill_sequence_impl { using type = std::index_sequence<>; };
	
	template<typename Function, std::size_t head, std::size_t... tail>
	requires invocable_with_sequence_expanded<Function, deducer<decl_source>, head, tail...>
	struct fill_sequence_impl<Function, std::index_sequence<head, tail...>> {
		using type = std::index_sequence<head, tail...>;
	};
	
	template<typename Function, std::size_t head, std::size_t... tail>
	struct fill_sequence_impl<Function, std::index_sequence<head, tail...>> : fill_sequence_impl<Function, std::index_sequence<(tail - 1)...>> {};

	template<typename F, std::size_t max>
	using fill_sequence = fill_sequence_impl<F, std::make_index_sequence<max>>;

	template<typename F, std::size_t max>
	using fill_sequence_t = typename fill_sequence<F, max>::type;

	template<typename F, typename T, typename S>
	concept invocable_with_sequence = requires(F function, S sequence) {
		[]<std::size_t... s>(std::index_sequence<s...>) requires invocable_with_sequence_expanded<F, T, s...> {}(sequence);
	};
}

namespace kangaru {
	template<typename Source>
	struct simple_injector {
		explicit constexpr simple_injector(Source source) noexcept : source{std::move(source)} {}
		
		constexpr auto operator()(std::invocable<deducer<Source&>> auto&& function) & -> decltype(auto) {
			return KANGARU5_FWD(function)(deducer<Source&>{source});
		}
		
		constexpr auto operator()(std::invocable<deducer<Source const&>> auto&& function) const& -> decltype(auto) {
			return KANGARU5_FWD(function)(deducer<Source const&>{source});
		}
		
		constexpr auto operator()(std::invocable<deducer<Source&&>> auto&& function) && -> decltype(auto) {
			return KANGARU5_FWD(function)(deducer<Source&&>{std::move(source)});
		}
		
		constexpr auto operator()(std::invocable<deducer<Source const&&>> auto&& function) const&& -> decltype(auto) {
			return KANGARU5_FWD(function)(deducer<Source const&&>{std::move(source)});
		}
		
	private:
		Source source;
	};
	
	template<typename Source, std::size_t max>
	struct basic_fill_injector {
		explicit constexpr basic_fill_injector(Source source) noexcept : source{std::move(source)} {}
		
		template<typename F, typename Seq = detail::injector::fill_sequence_t<F, max>>
		constexpr auto operator()(F&& function) & requires detail::injector::invocable_with_sequence<F, deducer<Source&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename Seq = detail::injector::fill_sequence_t<F, max>>
		constexpr auto operator()(F&& function) && requires detail::injector::invocable_with_sequence<F, deducer<Source&&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
		template<typename F, typename Seq = detail::injector::fill_sequence_t<F, max>>
		constexpr auto operator()(F&& function) const& requires detail::injector::invocable_with_sequence<F, deducer<Source const&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), source);
		}
		
		template<typename F, typename Seq = detail::injector::fill_sequence_t<F, max>>
		constexpr auto operator()(F&& function) const&& requires detail::injector::invocable_with_sequence<F, deducer<Source const&&>, Seq> {
			return fill_impl(Seq{}, KANGARU5_FWD(function), std::move(source));
		}
		
	private:
		template<typename F, std::size_t... s>
		static constexpr auto fill_impl(std::index_sequence<s...>, F&& function, auto&& source) -> decltype(auto) {
			using Deducer = deducer<decltype(source)>;
			return KANGARU5_FWD(function)((void(s), Deducer{KANGARU5_FWD(source)})...);
		}
		
		Source source;
	};
	
	template<typename Source>
	using fast_fill_injector = basic_fill_injector<Source, 4>;
	
	template<typename Source>
	using fill_injector = basic_fill_injector<Source, 8>;
	
	template<typename Source>
	using slow_fill_injector = basic_fill_injector<Source, 16>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_INJECTOR_HPP
