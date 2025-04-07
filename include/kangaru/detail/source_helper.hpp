#ifndef KANGARU5_DETAIL_SOURCE_HELPER_HPP
#define KANGARU5_DETAIL_SOURCE_HELPER_HPP

#include "source.hpp"
#include "source_reference_wrapper.hpp"

#include <type_traits>
#include <concepts>

#include "define.hpp"

namespace kangaru {
	namespace detail::source_helper {
		template<wrapping_source>
		struct rebind_wrapper {};
		
		template<template<typename> typename Branch, kangaru::source Source>
			requires std::constructible_from<Branch<Source>, Source&&>
		struct rebind_wrapper<Branch<Source>> {
			template<kangaru::source NewSource> requires requires { typename Branch<NewSource>; }
			struct ttype {
				using type = Branch<NewSource>;
			};
		};
		
		template<template<typename, typename> typename Branch, kangaru::source Source, typename Param>
			requires (
				    std::constructible_from<Branch<Source, Param>, Source&&>
				and not std::constructible_from<Branch<Source, Param>, Source&&, Param>
			)
		struct rebind_wrapper<Branch<Source, Param>> {
			template<kangaru::source NewSource> requires requires { typename Branch<NewSource, Param>; }
			struct ttype {
				using type = Branch<NewSource, Param>;
			};
		};
		
		template<template<typename, template<typename> typename> typename Branch, kangaru::source Source, template<typename> typename Param>
			requires (
				std::constructible_from<Branch<Source, Param>, Source&&>
			)
		struct rebind_wrapper<Branch<Source, Param>> {
			template<kangaru::source NewSource> requires requires { typename Branch<NewSource, Param>; }
			struct ttype {
				using type = Branch<NewSource, Param>;
			};
		};
	} // namespace detail::source_helper
	
	template<typename Source>
	concept transparent_rebindable_wrapping_source =
		    wrapping_source<Source>
		and requires(Source source) {
			typename detail::source_helper::rebind_wrapper<std::remove_cv_t<Source>>::template ttype<
				std::decay_t<decltype(source.source)>
			>::type;
		};
	
	template<typename Source>
	concept stateful_rebindable_wrapping_source =
		    wrapping_source<Source>
		and requires(Source source) {
			std::decay_t<Source>::rebind(source, [](auto&&) -> none_source { return {}; });
		};
	
	template<typename Source>
	concept rebindable_wrapping_source =
		   transparent_rebindable_wrapping_source<Source>
		or stateful_rebindable_wrapping_source<Source>;
	
	namespace detail::source_helper {
		struct rebind_function {
			template<forwarded_source Wrapper> requires (rebindable_wrapping_source<std::remove_reference_t<Wrapper>> and not forwarded_reference_wrapper<Wrapper>)
			constexpr auto operator()(Wrapper&& source, forwarded_source auto&& new_leaf) const noexcept {
				if constexpr (stateful_rebindable_wrapping_source<std::remove_reference_t<Wrapper>>) {
					return std::decay_t<Wrapper>::rebind(source, new_leaf);
				} else if constexpr (transparent_rebindable_wrapping_source<std::remove_reference_t<Wrapper>>) {
					using rebound = typename detail::source_helper::rebind_wrapper<std::remove_cvref_t<Wrapper>>::template ttype<
						decltype(operator()(source.source, KANGARU5_FWD(new_leaf)))
					>::type;
					return rebound{
						operator()(source.source, KANGARU5_FWD(new_leaf))
					};
				} else {
					static_assert(not std::same_as<Wrapper, Wrapper>, "exhaustive");
				}
			}
			
			template<forwarded_reference_wrapper Wrapper>
			constexpr auto operator()(Wrapper&& wrapper, forwarded_source auto&& new_leaf) const noexcept {
				// TODO: Do we need to use forwarding refs for wrappers?
				// TODO: Do we need to unwrap references at all? Does it even make sense?
				decltype(auto) unwrapped = KANGARU5_FWD(wrapper).unwrap();
				return operator()(KANGARU5_FWD(unwrapped), KANGARU5_FWD(new_leaf));
			}
			
			template<forwarded_source Leaf> requires (
				    not forwarded_reference_wrapper<Leaf>
				and not rebindable_wrapping_source<std::remove_reference_t<Leaf>>
				and not forwarded_wrapping_source<Leaf>
			)
			constexpr auto operator()(Leaf&& leaf, forwarded_source auto&& new_leaf) const noexcept {
				// We do not forward new_leaf here, since it may be called multiple times
				return new_leaf(KANGARU5_FWD(leaf));
			}
		};
		
		template<kangaru::source Source>
		inline constexpr auto is_rebindable_v = false;
		
		template<kangaru::source Wrapper> requires reference_wrapper<Wrapper>
		inline constexpr auto is_rebindable_v<Wrapper> = is_rebindable_v<source_reference_wrapped_type<Wrapper>>;
		
		template<kangaru::source Wrapper> requires (not reference_wrapper<Wrapper> and rebindable_wrapping_source<Wrapper>)
		inline constexpr auto is_rebindable_v<Wrapper> = is_rebindable_v<wrapped_source_t<Wrapper>>;
		
		template<kangaru::source Leaf> requires (not reference_wrapper<Leaf> and not rebindable_wrapping_source<Leaf>)
		inline constexpr auto is_rebindable_v<Leaf> = true;
		
		namespace niebloid {
			inline constexpr auto rebind = detail::source_helper::rebind_function{};
		}
	} // namespace detail::source_helper
	
	inline namespace niebloid {
		using namespace detail::source_helper::niebloid;
	}
	
	template<typename Source>
	concept rebindable_source = source<Source> and detail::source_helper::is_rebindable_v<Source>;
	
	template<source Source, forwarded_source Leaf>
	using rebind_result_t = decltype(kangaru::rebind(std::declval<Source>(), std::declval<Leaf>()));
	
	template<forwarded_wrapping_source Source, forwarded_source Leaf>
	using rebind_wrapped_source_result_t = decltype(kangaru::rebind(std::declval<forwarded_wrapped_source_t<Source>>(), std::declval<Leaf>()));
} // namespace kangaru

#include "undef.hpp"

#endif
