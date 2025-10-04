#ifndef KANGARU5_DETAIL_SOURCE_REBIND_HPP
#define KANGARU5_DETAIL_SOURCE_REBIND_HPP

#include "source.hpp"
#include "source_reference_wrapper.hpp"
#include "utility.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <concepts>
#endif

#include "define.hpp"

namespace kangaru {
	namespace detail::source_rebind {
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
	} // namespace detail::source_rebind
	
	KANGARU5_EXPORT template<typename Source>
	concept transparent_rebindable_wrapping_source =
		    wrapping_source<Source>
		and requires(Source source) {
			// Here we need to used ttype_t instead of directly using ::ttype<...>::type because GCC 12 has issues with it.
			typename detail::utility::ttype_t<
				detail::source_rebind::rebind_wrapper<std::remove_cv_t<Source>>,
				std::decay_t<decltype(source.source)>
			>;
		};
	
	KANGARU5_EXPORT template<typename Source>
	concept stateful_rebindable_wrapping_source =
		    wrapping_source<Source>
		and requires(Source source) {
			std::decay_t<Source>::rebind(source, [](auto&&) -> none_source { return {}; });
		};
	
	KANGARU5_EXPORT template<typename Source>
	concept rebindable_wrapping_source =
		   transparent_rebindable_wrapping_source<Source>
		or stateful_rebindable_wrapping_source<Source>;
	
	namespace detail::source_rebind {
		struct rebind_function {
			template<forwarded_source Wrapper> requires (rebindable_wrapping_source<std::remove_reference_t<Wrapper>> and not forwarded_reference_wrapper<Wrapper>)
			constexpr auto operator()(Wrapper&& source, forwarded_object auto&& new_leaf) const noexcept {
				if constexpr (stateful_rebindable_wrapping_source<std::remove_reference_t<Wrapper>>) {
					return std::decay_t<Wrapper>::rebind(source, new_leaf);
				} else if constexpr (transparent_rebindable_wrapping_source<std::remove_reference_t<Wrapper>>) {
					using rebound = typename detail::utility::ttype_t<
						detail::source_rebind::rebind_wrapper<std::remove_cvref_t<Wrapper>>,
						decltype(operator()(source.source, KANGARU5_FWD(new_leaf)))
					>;
					return rebound{
						operator()(source.source, KANGARU5_FWD(new_leaf))
					};
				} else {
					static_assert(not std::same_as<Wrapper, Wrapper>, "exhaustive");
				}
			}
			
			template<forwarded_reference_wrapper Wrapper>
			constexpr auto operator()(Wrapper&& wrapper, forwarded_object auto&& new_leaf) const noexcept {
				decltype(auto) unwrapped = KANGARU5_FWD(wrapper).unwrap();
				return operator()(KANGARU5_FWD(unwrapped), KANGARU5_FWD(new_leaf));
			}
			
			template<forwarded_source Leaf> requires (
				    not forwarded_reference_wrapper<Leaf>
				and not rebindable_wrapping_source<std::remove_reference_t<Leaf>>
				and not forwarded_wrapping_source<Leaf>
			)
			constexpr auto operator()(Leaf&& leaf, forwarded_object auto&& new_leaf) const noexcept {
				// We do not forward new_leaf here, since it may be called multiple times
				return new_leaf(KANGARU5_FWD(leaf));
			}
		};
		
		template<kangaru::source Source>
		inline constexpr auto is_rebindable_v = false;
		
		template<kangaru::source Wrapper> requires reference_wrapper<Wrapper>
		inline constexpr auto is_rebindable_v<Wrapper> = is_rebindable_v<source_reference_wrapped_type<Wrapper>>;
		
		template<kangaru::source Wrapper> requires (rebindable_wrapping_source<Wrapper> and not reference_wrapper<Wrapper>)
		inline constexpr auto is_rebindable_v<Wrapper> = is_rebindable_v<wrapped_source_t<Wrapper>>;
		
		template<kangaru::source Leaf> requires (not reference_wrapper<Leaf> and not rebindable_wrapping_source<Leaf> and not wrapping_source<Leaf>)
		inline constexpr auto is_rebindable_v<Leaf> = true;
		
		namespace niebloid {
			inline constexpr auto rebind = detail::source_rebind::rebind_function{};
		}
	} // namespace detail::source_rebind
	
	KANGARU5_EXPORT inline namespace niebloid {
		using namespace detail::source_rebind::niebloid;
	}
	
	KANGARU5_EXPORT template<typename Source>
	concept rebindable_source = source<Source> and detail::source_rebind::is_rebindable_v<Source>;
	
	KANGARU5_EXPORT template<rebindable_source Source, forwarded_source Leaf>
	using rebind_result_t = decltype(kangaru::rebind(std::declval<Source>(), std::declval<Leaf>()));
	
	KANGARU5_EXPORT template<forwarded_wrapping_source Source, forwarded_source Leaf>
	using wrapped_source_rebind_result_t = decltype(kangaru::rebind(std::declval<forwarded_wrapped_source_t<Source>>(), std::declval<Leaf>()));
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_REBIND_HPP
