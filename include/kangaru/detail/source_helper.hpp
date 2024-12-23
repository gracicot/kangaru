#ifndef KANGARU5_DETAIL_SOURCE_HELPER_HPP
#define KANGARU5_DETAIL_SOURCE_HELPER_HPP

#include "source.hpp"
#include "source_reference_wrapper.hpp"

#include "define.hpp"
#include <type_traits>

namespace kangaru {
	namespace detail::source_helper {
		template<wrapping_source>
		struct rebind_wrapper {};
		
		template<template<typename> typename Branch, kangaru::source Source>
		struct rebind_wrapper<Branch<Source>> {
			template<kangaru::source NewSource> requires requires { typename Branch<NewSource>; }
			struct ttype {
				using type = Branch<NewSource>;
			};
		};
		
		template<template<typename> typename Branch, kangaru::source Source>
		struct rebind_wrapper<Branch<Source> const> {
			template<kangaru::source NewSource> requires requires { typename Branch<NewSource>; }
			struct ttype {
				using type = Branch<NewSource>;
			};
		};
	} // namespace detail::source_helper
	
	template<typename Source>
	concept transparent_rebindable_wrapping_source =
		    wrapping_source<Source>
		and requires(Source source) {
			typename std::type_identity_t<
				typename detail::source_helper::rebind_wrapper<Source>::template ttype<
					std::decay_t<decltype(source.source)>
				>::type
			>;
			requires std::constructible_from<
				typename detail::source_helper::rebind_wrapper<Source>::template ttype<std::decay_t<decltype(source.source)>>::type,
				std::decay_t<decltype(source.source)>
			>;
		};
	
	template<typename Source>
	concept stateful_rebindable_wrapping_source =
		    wrapping_source<Source>
		and requires(Source source) {
			Source::rebind(source, none_source{});
		};
	
	template<typename Source>
	concept rebindable_wrapping_source =
		   transparent_rebindable_wrapping_source<Source>
		or stateful_rebindable_wrapping_source<Source>;
	
	namespace detail::source_helper {
		// TODO: Design an interface where rebinding might branch off in composed sources
		// and allow rebinding with a transformed leaf instead of ignoring it.
		
		// Forward declaration because otherwise the top overloads cannot find the bottom ones.
		template<kangaru::source Leaf> requires (not reference_wrapper<Leaf> and not rebindable_wrapping_source<Leaf>)
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Leaf&) noexcept;
		
		// Forward declaration because otherwise the top overloads cannot find the bottom ones.
		template<rebindable_wrapping_source Wrapper> requires (not reference_wrapper<Wrapper>)
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Wrapper& source) noexcept;
		
		// Forward declaration because otherwise the top overloads cannot find the bottom ones.
		template<reference_wrapper Wrapper>
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Wrapper wrapper) noexcept;
		
		template<reference_wrapper Wrapper>
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Wrapper wrapper) noexcept {
			// TODO: Do we need to use forwarding refs for wrappers?
			auto&& unwrapped = wrapper.unwrap();
			return KANGARU5_NO_ADL(rebind_source_tree)(KANGARU5_FWD(new_leaf), unwrapped);
		}
		
		template<kangaru::source Leaf> requires (not reference_wrapper<Leaf> and not rebindable_wrapping_source<Leaf>)
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Leaf&) noexcept {
			return KANGARU5_FWD(new_leaf);
		}
		
		template<rebindable_wrapping_source Wrapper> requires (not reference_wrapper<Wrapper>)
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Wrapper& source) noexcept {
			if constexpr (stateful_rebindable_wrapping_source<Wrapper>) {
				return Wrapper::rebind(source, KANGARU5_NO_ADL(rebind_source_tree)(KANGARU5_FWD(new_leaf), source.source));
			} else if constexpr (transparent_rebindable_wrapping_source<Wrapper>) {
				using rebound = typename detail::source_helper::rebind_wrapper<Wrapper>::template ttype<
					decltype(KANGARU5_NO_ADL(rebind_source_tree)(KANGARU5_FWD(new_leaf), source.source))
				>::type;
				return rebound{
					KANGARU5_NO_ADL(rebind_source_tree)(KANGARU5_FWD(new_leaf), source.source)
				};
			} else {
				static_assert(not std::same_as<Wrapper, Wrapper>, "exhaustive");
			}
		}
	} // namespace detail::source_helper
} // namespace kangaru

#include "undef.hpp"

#endif
