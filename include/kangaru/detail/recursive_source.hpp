#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "utility.hpp"
#include "source_types.hpp"
#include "constructor.hpp"
#include "injector.hpp"
#include "tag.hpp"
#include "noreturn.hpp"

#include <type_traits>
#include <cstdlib>

#include "define.hpp"

namespace kangaru {
	struct make_spread_injector_function {
		template<typename Source> requires source<std::remove_cvref_t<Source>>
		constexpr auto operator()(Source&& source) const noexcept {
			return make_spread_injector(KANGARU5_FWD(source));
		}
	};
	
	struct make_strict_spread_injector_function {
		template<typename Source> requires source<std::remove_cvref_t<Source>>
		constexpr auto operator()(Source&& source) const noexcept {
			return make_strict_spread_injector(KANGARU5_FWD(source));
		}
	};
	
	template<movable_object MakeInjector>
	struct basic_non_empty_construction {
		constexpr basic_non_empty_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_non_empty_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}

		template<unqualified_object T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires callable<
				KANGARU5_CONSTRUCTOR_T(T),
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
		};
		
		template<unqualified_object T, typename Source> requires (source<std::remove_cvref_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>)
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}

	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using non_empty_construction = basic_non_empty_construction<make_spread_injector_function>;
	
	template<movable_object MakeInjector>
	struct basic_unsafe_exhaustive_construction {
		constexpr basic_unsafe_exhaustive_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_unsafe_exhaustive_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}

		template<unqualified_object T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires callable<
				KANGARU5_CONSTRUCTOR_T(T),
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
			
			constexpr auto operator()() const -> T requires callable<KANGARU5_CONSTRUCTOR_T(T)> {
				return constructor<T>()();
			}
		};
		
		template<unqualified_object T, typename Source> requires (source<std::remove_cvref_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>)
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}

	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using unsafe_exhaustive_construction = basic_unsafe_exhaustive_construction<make_spread_injector_function>;
	
	template<movable_object MakeInjector>
	struct basic_exhaustive_construction {
		constexpr basic_exhaustive_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_exhaustive_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}

		template<typename T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires callable<
				KANGARU5_CONSTRUCTOR_T(T),
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
			
			constexpr auto operator()() const -> T requires (callable<KANGARU5_CONSTRUCTOR_T(T)> and is_empty_injection_constructible_v<T>) {
				return constructor<T>()();
			}
		};
		
		template<typename T, typename Source> requires (source<std::remove_reference_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>)
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}

	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using exhaustive_construction = basic_exhaustive_construction<make_spread_injector_function>;
	
	template<typename Type, movable_object MakeInjector>
	struct placeholder_construct_except {
		constexpr placeholder_construct_except() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr placeholder_construct_except(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}

		template<typename T>
		struct construct {
			// auto operator()(deducer auto...) const -> T requires std::same_as<Type, T> = delete;
			
			[[noreturn]]
			auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires (different_from<T, Type> and callable<
				KANGARU5_CONSTRUCTOR_T(std::decay_t<T>),
				exclude_special_constructors_deducer<std::decay_t<T>, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			>);
			
			[[noreturn]]
			auto operator()() const -> T requires (different_from<T, Type> and callable<KANGARU5_CONSTRUCTOR_T(std::decay_t<T>)>);
		};
		
		template<typename T, typename Source> requires (source<std::remove_cvref_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>)
		auto operator()(Source&& source) const -> T;

	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<movable_object MakeInjector>
	struct basic_placeholder_construct {
		constexpr basic_placeholder_construct() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_placeholder_construct(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}

		template<unqualified_object T>
		struct construct {
			[[noreturn]]
			auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires (not unqualified_object<T> and callable<
				KANGARU5_CONSTRUCTOR_T(std::decay_t<T>),
				exclude_special_constructors_deducer<std::decay_t<T>, decltype(deduce1)>,
				exclude_deducer<std::decay_t<T>, decltype(deduce)>...
			>);
			
			[[noreturn]]
			auto operator()() const -> T requires callable<KANGARU5_CONSTRUCTOR_T(T)>;
		};

		template<typename T, typename Source> requires (source<std::remove_cvref_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>)
		auto operator()(Source&& source) const -> T;

	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using placeholder_construct = basic_placeholder_construct<make_spread_injector_function>;
	
	template<source Source, movable_object Construct>
	struct with_recursion {
		constexpr explicit with_recursion(Source source) noexcept
			requires (std::default_initializable<Construct>) :
			source{std::move(source)}, construct{} {}
		
		constexpr with_recursion(Source source, Construct construct) noexcept :
			source{std::move(source)}, construct{std::move(construct)} {}
		
		Source source;
		
	private:
		template<typename T, forwarded<with_recursion> Self>
			requires wrapping_source_of<Self, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
		template<typename T, forwarded<with_recursion> Self>
			requires (not wrapping_source_of<Self, T> and callable_template1<Construct const&, T, source_reference_wrapper_for_t<std::remove_reference_t<Self>>>)
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return source.construct.template operator()<T>(ref(source));
		}
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construct construct;
	};
	
	template<source Source>
	using with_recursive_construct = with_recursion<Source, non_empty_construction>;
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_source_with_recursive_construct(Source&& source) {
		return with_recursive_construct<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	using with_unsafe_exhaustive_recursive_construct = with_recursion<Source, unsafe_exhaustive_construction>;
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_source_with_unsafe_exhaustive_recursive_construct(Source&& source) {
		return with_unsafe_exhaustive_recursive_construct<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	using with_exhaustive_recursive_construct = with_recursion<Source, exhaustive_construction>;
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_source_with_exhaustive_recursive_construct(Source&& source) {
		return with_exhaustive_recursive_construct<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<typename Tree, typename Type>
	concept construction_tree_needs = not source_of<with_recursion<noop_source, placeholder_construct_except<Type, make_strict_spread_injector_function>>, Tree>;
	
	// deep
	template<wrapping_source>
	struct rebind_wrapper {};
	
	template<template<typename> typename Branch, source Source>
	struct rebind_wrapper<Branch<Source>> {
		template<source NewSource>
		using ttype = Branch<NewSource>;
	};
	
	template<template<typename, typename> typename Branch, source Source, typename State>
	struct rebind_wrapper<Branch<Source, State>> {
		template<source NewSource, typename NewState = State>
		using ttype = Branch<NewSource, NewState>;
	};
	
	template<typename Source>
	concept rebindable_wrapping_source =
		    wrapping_source<Source>
		and requires(Source source) {
			typename std::type_identity<typename rebind_wrapper<Source>::template ttype<std::decay_t<decltype(source.source)>>>::type;
			requires std::constructible_from<
				typename rebind_wrapper<Source>::template ttype<std::decay_t<decltype(source.source)>>,
				std::decay_t<decltype(source.source)>
			>;
		};
	
	template<typename Source>
	concept stateful_rebindable_wrapping_source =
		    rebindable_wrapping_source<Source>
		and requires(Source source) {
			typename std::type_identity<typename rebind_wrapper<Source>::template ttype<std::decay_t<decltype(source.source)>, decltype(ref(source))>>::type;
			requires std::constructible_from<
				typename rebind_wrapper<Source>::template ttype<std::decay_t<decltype(source.source)>, decltype(ref(source))>,
				std::decay_t<decltype(source.source)>,
				decltype(ref(source))
			>;
		};

	template<source Source>
	struct with_tree_recursion {
		Source source;
		
		auto test() {
			return rebind_tree(source);
		}
		
	private:
		template<kangaru::source Leaf>
		constexpr auto rebind_tree(Leaf& source) noexcept -> auto {
			return with_tree_recursion<decltype(ref(source))>{ref(source)};
		}
		
		template<rebindable_wrapping_source Wrapper>
		constexpr auto rebind_tree(Wrapper& source) -> auto {
			return typename rebind_wrapper<Wrapper>::template ttype<decltype(rebind_tree(source.source))>{rebind_tree(source.source)};
		}
		
		template<stateful_rebindable_wrapping_source Wrapper>
		constexpr auto rebind_tree(Wrapper& source) -> auto {
			return typename rebind_wrapper<Wrapper>::template ttype<decltype(rebind_tree(source.source)), decltype(ref(source))>{
				rebind_tree(source.source),
				ref(source),
			};
		}
		
		template<typename T> requires source_of<Source, T>
		friend constexpr auto provide(provide_tag<T> tag, forwarded<with_tree_recursion> auto&& source) {
			return provide(tag, KANGARU5_FWD(source).source);
		}
		
		template<typename T> requires (not source_of<Source, T>)
		friend constexpr auto provide(provide_tag<T> tag, forwarded<with_tree_recursion> auto&& source) {
			return provide(tag, source.rebind_tree(source.source));
		}
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
