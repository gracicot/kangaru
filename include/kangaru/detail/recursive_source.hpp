#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "utility.hpp"
#include "constructor.hpp"
#include "injector.hpp"
#include "tag.hpp"
#include "source.hpp"
#include "source_reference_wrapper.hpp"
#include "source_types.hpp"
#include "source_helper.hpp"

#include <type_traits>
#include <cstdlib>

#include "define.hpp"

namespace kangaru {
	template<typename T>
	concept construction = movable_object<T> and requires {
		detail::utility::template_type_identity<T::template construct>{};
	};
	
	template<movable_object MakeInjector>
	struct basic_non_empty_construction {
		constexpr basic_non_empty_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_non_empty_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
		};
		
		template<unqualified_object T, typename Source>
			requires (
				    source<std::remove_cvref_t<Source>>
				and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>
			)
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using non_empty_construction = basic_non_empty_construction<make_spread_injector_function>;
	
	static_assert(construction<non_empty_construction>);
	
	template<movable_object MakeInjector>
	struct basic_unsafe_exhaustive_construction {
		constexpr basic_unsafe_exhaustive_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_unsafe_exhaustive_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
			
			constexpr auto operator()() const -> T requires constructor_callable<T> {
				return constructor<T>()();
			}
		};
		
		template<unqualified_object T, typename Source>
			requires (source<std::remove_cvref_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>)
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}
	
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using unsafe_exhaustive_construction = basic_unsafe_exhaustive_construction<make_spread_injector_function>;
	
	static_assert(construction<unsafe_exhaustive_construction>);
	
	template<movable_object MakeInjector>
	struct basic_exhaustive_construction {
		constexpr basic_exhaustive_construction() requires std::default_initializable<MakeInjector> = default;
		
		explicit constexpr basic_exhaustive_construction(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<unqualified_object T>
		struct construct {
			constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires constructor_callable<
				T,
				exclude_special_constructors_deducer<T, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			> {
				return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
			
			constexpr auto operator()() const -> T requires (constructor_callable<T> and is_empty_injection_constructible_v<T>) {
				return constructor<T>()();
			}
		};
		
		template<unqualified_object T, typename Source>
			requires (source<std::remove_reference_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source&&>, construct<T>>)
		constexpr auto operator()(Source&& source) const {
			return make_injector(KANGARU5_FWD(source))(construct<T>{});
		}
		
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using exhaustive_construction = basic_exhaustive_construction<make_spread_injector_function>;
	
	static_assert(construction<exhaustive_construction>);
	
	template<typename Type, movable_object MakeInjector>
	struct basic_placeholder_construct_except {
		KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construct_except() requires std::default_initializable<MakeInjector> = default;
		
		explicit KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construct_except(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<typename T>
		struct construct {
			[[noreturn]]
			auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires (different_from<T, Type> and constructor_callable<
				std::decay_t<T>,
				exclude_special_constructors_deducer<std::decay_t<T>, decltype(deduce1)>,
				exclude_deducer<T, decltype(deduce)>...
			>);
			
			[[noreturn]]
			auto operator()() const -> T requires (different_from<T, Type> and constructor_callable<std::decay_t<T>>);
		};
		
		template<typename T, typename Source>
			requires (source<std::remove_cvref_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>)
		auto operator()(Source&& source) const -> T;
	
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<typename Type>
	using placeholder_construct_except = basic_placeholder_construct_except<Type, make_spread_injector_function>;
	
	static_assert(construction<placeholder_construct_except<int>>);
	
	template<movable_object MakeInjector>
	struct basic_placeholder_construct {
		KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construct() requires std::default_initializable<MakeInjector> = default;
		
		explicit KANGARU5_CONSTEVAL_PLACEHOLDER basic_placeholder_construct(MakeInjector make_injector) noexcept :
			make_injector{std::move(make_injector)} {}
		
		template<typename T>
		struct construct {
			[[noreturn]]
			auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
			requires (not unqualified_object<T> and constructor_callable<
				std::decay_t<T>,
				exclude_special_constructors_deducer<std::decay_t<T>, decltype(deduce1)>,
				exclude_deducer<std::decay_t<T>, decltype(deduce)>...
			>);
			
			[[noreturn]]
			auto operator()() const -> T requires constructor_callable<T>;
		};
		
		template<typename T, typename Source>
			requires (source<std::remove_cvref_t<Source>> and callable<std::invoke_result_t<MakeInjector const&, Source>, construct<T>>)
		auto operator()(Source&& source) const -> T;
	
	private:
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	using placeholder_construct = basic_placeholder_construct<make_spread_injector_function>;
	
	static_assert(construction<placeholder_construct>);
	
	template<source Source, construction Construction>
	struct with_construction {
	private:
		using construction_type = maybe_wrapped_t<Construction>;
		
	public:
		explicit constexpr with_construction(Source source) noexcept
			requires std::default_initializable<Construction> :
			source{std::move(source)} {}
		
		constexpr with_construction(Source source, Construction construction) noexcept :
			source{std::move(source)},
			construction{std::move(construction)} {}
		
		template<typename T, forwarded<with_construction> Self> requires wrapping_source_of<Self, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
		template<typename T, forwarded<with_construction> Self>
			requires (callable_template1<construction_type const&, T, wrapped_source_t<Self>> and not wrapping_source_of<Self, T>)
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			if constexpr (reference_wrapper<Source>) {
				return std::as_const(source.construction).template operator()<T>(KANGARU5_FWD(source).source);
			} else {
				return std::as_const(source.construction).template operator()<T>(kangaru::fwd_ref(KANGARU5_FWD(source).source));
			}
		}
		
		Source source;
		
	private:
		Construction construction;
	};
	
	
	template<typename Source, typename Construct> requires (source<std::remove_cvref_t<Source>> and construction<std::remove_cvref_t<Construct>>)
	inline constexpr auto make_source_with_construction(Source&& source, Construct&& construct) {
		return with_construction<std::remove_cvref_t<Source>, std::remove_cvref_t<Construct>>{KANGARU5_FWD(source), KANGARU5_FWD(construct)};
	}
	
	template<source Source>
	using with_non_empty_construction = with_construction<Source, non_empty_construction>;
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	inline constexpr auto make_source_with_non_empty_construction(Source&& source) {
		return with_construction<std::remove_cvref_t<Source>, non_empty_construction>{KANGARU5_FWD(source), non_empty_construction{}};
	}
	
	template<source Source>
	using with_exhaustive_construction = with_construction<Source, exhaustive_construction>;
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	inline constexpr auto make_source_with_exhaustive_construction(Source&& source) {
		return with_construction<std::remove_cvref_t<Source>, exhaustive_construction>{KANGARU5_FWD(source), exhaustive_construction{}};
	}
	
	template<source Source>
	using with_unsafe_exhaustive_construction = with_construction<Source, unsafe_exhaustive_construction>;
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	inline constexpr auto make_source_with_unsafe_exhaustive_construction(Source&& source) {
		return with_construction<std::remove_cvref_t<Source>, unsafe_exhaustive_construction>{KANGARU5_FWD(source), unsafe_exhaustive_construction{}};
	}
	
	template<source Source>
	struct with_recursion;

	namespace detail::recursive_source {

	}
	
	template<source Source>
	struct with_recursion {
	private:
		template<typename T>
		static constexpr auto rebound_leaf_for(auto&& self) requires wrapping_source<std::remove_reference_t<decltype(self)>> {
			using S = forwarded_wrapped_source_t<decltype(self)>;
			if constexpr (source_of<std::remove_reference_t<S>, T> and reference_wrapper<std::remove_cvref_t<S>>) {
				return self.source;
			} else if constexpr (source_of<std::remove_reference_t<S>, T>) {
				return kangaru::ref(self.source);
			} else if constexpr (reference_wrapper<std::remove_cvref_t<S>>) {
				return make_source_with_filter_passthrough<T>(self);
			} else {
				return make_source_with_filter_passthrough<T>(with_recursion<source_reference_wrapper<std::remove_reference_t<S>>>{kangaru::ref(self.source)});
			}
		}
		
		template<forwarded<with_recursion> Self, typename T>
		using rebound_source_t = decltype(
			detail::source_helper::rebind_source_tree(
				rebound_leaf_for<T>(std::declval<Self>()),
				std::declval<Self&>().source
			)
		);
		
	public:
		explicit constexpr with_recursion(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<typename T, forwarded<with_recursion> Self> requires (not wrapping_source_of<Self, T>)
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T requires source_of<rebound_source_t<Self, T>, T> {
			return provide(
				tag,
				detail::source_helper::rebind_source_tree(
					rebound_leaf_for<T>(KANGARU5_FWD(source)),
					source.source
				)
			);
		}
		
		template<typename T, forwarded<with_recursion> Self> requires wrapping_source_of<Self, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, KANGARU5_FWD(source).source);
		}
	};
	
	template<typename Source> requires (source<std::remove_cvref_t<Source>>)
	inline constexpr auto make_source_with_recursion(Source&& source) {
		return with_recursion<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<typename Tree, typename Type>
	concept construction_tree_needs = not source_of<
		with_recursion<
			with_construction<
				noop_source,
				basic_placeholder_construct_except<Type, make_strict_spread_injector_function>
			>
		>,
		Tree
	>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
