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
		constexpr auto operator()(source auto& source) const noexcept {
			return make_spread_injector(ref(source));
		}
	};
	
	struct make_strict_spread_injector_function {
		constexpr auto operator()(source auto& source) const noexcept {
			return make_strict_spread_injector(ref(source));
		}
	};
	
	template<source Source, movable_object Construct, movable_object MakeInjector = make_spread_injector_function>
	struct with_recursion {
		constexpr explicit with_recursion(Source source) noexcept
			requires (std::default_initializable<Construct> and std::default_initializable<MakeInjector>) :
			source{std::move(source)}, construct{}, make_injector{} {}
		
		constexpr with_recursion(Source source, Construct construct) noexcept
			requires std::default_initializable<MakeInjector> :
			source{std::move(source)}, construct{std::move(construct)}, make_injector{} {}
		
		constexpr with_recursion(Source source, Construct construct, MakeInjector make_injector) noexcept :
			source{std::move(source)}, construct{std::move(construct)}, make_injector{std::move(make_injector)} {}
		
		Source source;
		
	private:
		template<typename Self>
		using injector_type = decltype(std::declval<MakeInjector>()(std::declval<Self&>()));
		
		template<typename T>
		struct call_construct_function {
			Construct const* construct;
			
			constexpr auto operator()(deducer auto... deduce) const -> T
				requires callable_template1<Construct const&, T, decltype(deduce)...>
			{
				return construct->template operator()<T>(deduce...);
			}
		};
		
		template<typename T, forwarded<with_recursion> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return kangaru::provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
		template<typename T, forwarded<with_recursion> Self, typename S = detail::utility::forward_like_t<Self, Source>>
			requires (not source_of<S, T> and callable<injector_type<Self>, call_construct_function<T>>)
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			auto injector = source.make_injector(source);
			return std::move(injector)(call_construct_function<T>{std::addressof(source.construct)});
		}
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construct construct;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	struct non_empty_construction {
		template<unqualified_object T>
		constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
		requires callable<
			KANGARU5_CONSTRUCTOR_T(T),
			exclude_special_constructors_deducer<T, decltype(deduce1)>,
			exclude_deducer<T, decltype(deduce)>...
		> {
			return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
		}
	};
	
	template<source Source>
	using with_recursive_construct = with_recursion<Source, non_empty_construction>;
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_source_with_recursive_construct(Source&& source) {
		return with_recursive_construct<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	struct unsafe_exhaustive_construction {
		template<unqualified_object T>
		constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
		requires callable<
			KANGARU5_CONSTRUCTOR_T(T),
			exclude_special_constructors_deducer<T, decltype(deduce1)>,
			exclude_deducer<T, decltype(deduce)>...
		> {
			return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
		}
		
		template<unqualified_object T>
			requires callable<KANGARU5_CONSTRUCTOR_T(T)> 
		constexpr auto operator()() const -> T {
			return constructor<T>()();
		}
	};
	
	template<typename Type>
	struct placeholder_construct_except {
		template<typename T> requires std::same_as<Type, T>
		auto operator()(deducer auto...) const -> T = delete;
		
		template<different_from<Type> T> [[noreturn]]
		auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
		requires callable<
			KANGARU5_CONSTRUCTOR_T(std::decay_t<T>),
			exclude_special_constructors_deducer<std::decay_t<T>, decltype(deduce1)>,
			exclude_deducer<T, decltype(deduce)>...
		>;
		
		template<different_from<Type> T>
			requires callable<KANGARU5_CONSTRUCTOR_T(std::decay_t<T>)> [[noreturn]]
		auto operator()() const -> T;
	};
	
	struct placeholder_construct {
		template<unqualified_object T> [[noreturn]]
		auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
		requires callable<
			KANGARU5_CONSTRUCTOR_T(T),
			exclude_special_constructors_deducer<T, decltype(deduce1)>,
			exclude_deducer<T, decltype(deduce)>...
		>;
		
		template<typename T> requires (not unqualified_object<T>) [[noreturn]]
		auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
		requires callable<
			KANGARU5_CONSTRUCTOR_T(std::decay_t<T>),
			exclude_special_constructors_deducer<std::decay_t<T>, decltype(deduce1)>,
			exclude_deducer<std::decay_t<T>, decltype(deduce)>...
		>;
		
		template<typename T>
			requires callable<KANGARU5_CONSTRUCTOR_T(T)> [[noreturn]]
		auto operator()() const -> T;
	};
	
	template<typename Tree, typename Type>
	concept construction_tree_needs = not source_of<with_recursion<noop_source, placeholder_construct_except<Type>, make_strict_spread_injector_function>, Tree>;
	
	template<source Source>
	using with_unsafe_exhaustive_recursive_construct = with_recursion<Source, unsafe_exhaustive_construction>;
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_source_with_unsafe_exhaustive_recursive_construct(Source&& source) {
		return with_unsafe_exhaustive_recursive_construct<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	struct exhaustive_construction {
		template<unqualified_object T>
		constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
		requires callable<
			KANGARU5_CONSTRUCTOR_T(T),
			exclude_special_constructors_deducer<T, decltype(deduce1)>,
			exclude_deducer<T, decltype(deduce)>...
		> {
			return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
		}
		
		template<unqualified_object T>
			requires (callable<KANGARU5_CONSTRUCTOR_T(T)> and is_empty_injection_constructible_v<T>)
		constexpr auto operator()() const -> T {
			return constructor<T>()();
		}
	};
	
	template<source Source>
	using with_exhaustive_recursive_construct = with_recursion<Source, exhaustive_construction>;
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_source_with_exhaustive_recursive_construct(Source&& source) {
		return with_exhaustive_recursive_construct<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source, movable_object Lambda, movable_object MakeInjector = make_spread_injector_function>
	struct with_lambda_source {
		with_lambda_source(Source source, Lambda lambda) noexcept requires std::default_initializable<MakeInjector> :
			source{std::move(source)},
			lambda{std::move(lambda)},
			make_injector{} {}
		
		with_lambda_source(Source source, Lambda lambda, MakeInjector make_injector) noexcept :
			source{std::move(source)},
			lambda{std::move(lambda)},
			make_injector{std::move(make_injector)} {}
		
		Source source;
		
	private:
		template<typename Self>
		using injector_type = decltype(std::declval<MakeInjector>()(std::declval<Self&>()));
		
		template<typename T, forwarded<with_lambda_source> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return kangaru::provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
		template<typename T, forwarded<with_lambda_source> Self>
			requires (not source_of<Source, T> and callable<injector_type<Self>, detail::utility::forward_like_t<Self, Lambda>>)
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			auto injector = source.make_injector(source);
			return std::move(injector)(KANGARU5_FWD(source).lambda);
		}
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Lambda lambda;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
	};
	
	template<source Source>
	struct with_pouch_source {
		with_pouch_source(Source source) noexcept :
			source{std::move(source)} {}
		
		template<typename T, forwarded<with_pouch_source> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return kangaru::provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
		template<forwarded<with_pouch_source> Self>
		friend constexpr auto provide(provide_tag<Source>, Self&& source) -> Source {
			return source;
		}
		
	private:
		Source source;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
