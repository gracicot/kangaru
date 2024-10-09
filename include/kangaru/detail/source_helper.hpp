#ifndef KANGARU5_DETAIL_SOURCE_HELPER_HPP
#define KANGARU5_DETAIL_SOURCE_HELPER_HPP

#include "source.hpp"
#include "source_reference_wrapper.hpp"

#include "define.hpp"

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
		
		template<template<typename, typename> typename Branch, kangaru::source Source, typename State>
		struct rebind_wrapper<Branch<Source, State>> {
			template<kangaru::source NewSource, typename NewState = State> requires requires { typename Branch<NewSource, NewState>; }
			struct ttype {
				using type = Branch<NewSource, NewState>;
			};
		};
		
		template<template<typename> typename Branch, kangaru::source Source>
		struct rebind_wrapper<Branch<Source> const> {
			template<kangaru::source NewSource> requires requires { typename Branch<NewSource>; }
			struct ttype {
				using type = Branch<NewSource>;
			};
		};
		
		template<template<typename, typename> typename Branch, kangaru::source Source, typename State>
		struct rebind_wrapper<Branch<Source, State> const> {
			template<kangaru::source NewSource, typename NewState = State> requires requires { typename Branch<NewSource, NewState>; }
			struct ttype {
				using type = Branch<NewSource, NewState>;
			};
		};
	} // namespace detail::source_helper
	
	template<typename Source>
	concept rebindable_wrapping_source =
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
		    rebindable_wrapping_source<Source>
		and requires(Source source) {
			typename std::type_identity_t<
				typename detail::source_helper::rebind_wrapper<Source>::template ttype<
					std::decay_t<decltype(source.source)>,
					decltype(kangaru::ref(source))
				>::type
			>;
			requires std::constructible_from<
				typename detail::source_helper::rebind_wrapper<Source>::template ttype<
					std::decay_t<decltype(source.source)>,
					decltype(kangaru::ref(source))
				>::type,
				std::decay_t<decltype(source.source)>,
				decltype(kangaru::ref(source))
			>;
		};

	namespace detail::source_helper {
		template<kangaru::source Leaf> requires (not reference_wrapper<Leaf> and not rebindable_wrapping_source<Leaf>)
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Leaf&) noexcept -> auto {
			return KANGARU5_FWD(new_leaf);
		}
		
		template<rebindable_wrapping_source Wrapper> requires (not reference_wrapper<Wrapper>)
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Wrapper& source) noexcept {
			if constexpr (stateful_rebindable_wrapping_source<Wrapper>) {
				using rebound = typename detail::source_helper::rebind_wrapper<Wrapper>::template ttype<
					decltype(rebind_source_tree(KANGARU5_FWD(new_leaf), source.source)),
					decltype(kangaru::ref(source))
				>::type;
				return rebound{
					rebind_source_tree(KANGARU5_FWD(new_leaf), source.source),
					kangaru::ref(source),
				};
			} else {
				using rebound = typename detail::source_helper::rebind_wrapper<Wrapper>::template ttype<
					decltype(rebind_source_tree(KANGARU5_FWD(new_leaf), source.source))
				>::type;
				return rebound{
					rebind_source_tree(KANGARU5_FWD(new_leaf), source.source)
				};
			}
		}
		
		template<reference_wrapper Wrapper>
		constexpr auto rebind_source_tree(forwarded_source auto&& new_leaf, Wrapper wrapper) {
			return rebind_source_tree(KANGARU5_FWD(new_leaf), wrapper.unwrap());
		}
		
		template<typename Source> requires (forwarded_source<Source> and not wrapping_source<std::remove_reference_t<Source>>)
		constexpr auto source_tree_leaf(Source&& source) -> auto&& {
			return KANGARU5_FWD(source);
		}
		
		template<typename Source> requires (wrapping_source<std::remove_reference_t<Source>>)
		constexpr auto source_tree_leaf(Source&& source) -> auto&& {
			return source_tree_leaf(KANGARU5_FWD(source).source);
		}
	} // namespace detail::source_helper
} // namespace kangaru

#include "undef.hpp"

#endif
