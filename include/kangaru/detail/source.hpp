#ifndef KANGARU5_DETAIL_SOURCE_HPP
#define KANGARU5_DETAIL_SOURCE_HPP

#include "concepts.hpp"
#include "utility.hpp"
#include "attributes.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#include <type_traits>
#include <utility>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<typename T>
	concept injectable = weak_injectable<T> and allow_injection_using_v<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept source = object<T> and std::is_class_v<T> and not pointer<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept source_ref = reference<T> and source<std::remove_reference_t<T>>;
	
	KANGARU5_EXPORT template<typename T>
	concept forwarded_source = source<std::remove_reference_t<T>>;
	
	namespace detail::source_private {
		template<typename = void>
		auto provide(auto&&) requires false = delete;
		
		template<typename Source, typename T>
		concept adl_nonmember_source_of =
			    source<std::remove_cvref_t<Source>>
			and injectable<T>
			and requires(Source&& source) {
				{ provide(KANGARU5_FWD(source)) } -> std::same_as<T>;
			};
		
		template<typename Source, typename T>
		concept adl_nonmember_template_source_of =
			    source<std::remove_cvref_t<Source>>
			and injectable<T>
			and requires(Source&& source) {
				{ provide<T>(KANGARU5_FWD(source)) } -> std::same_as<T>;
			};
		
		template<typename Source, typename T>
		concept member_template_source_of =
			    source<std::remove_cvref_t<Source>>
			and injectable<T>
			and requires(Source&& source) {
				{ KANGARU5_FWD(source).template provide<T>() } -> std::same_as<T>;
			};
		
		template<typename Source, typename T>
		concept member_source_of =
			    source<std::remove_cvref_t<Source>>
			and injectable<T>
			and requires(Source&& source) {
				{ KANGARU5_FWD(source).provide() } -> std::same_as<T>;
			};
		
		template<injectable T>
		struct provide_function {
			template<typename Source> requires adl_nonmember_template_source_of<Source, T>
			KANGARU5_INLINE constexpr auto operator()(Source&& source) const -> T {
				return provide<T>(KANGARU5_FWD(source));
			}
			
			template<typename Source> requires adl_nonmember_source_of<Source, T>
			KANGARU5_INLINE constexpr auto operator()(Source&& source) const -> T {
				return provide(KANGARU5_FWD(source));
			}
			
			template<typename Source> requires member_template_source_of<Source, T>
			KANGARU5_INLINE constexpr auto operator()(Source&& source) const -> T {
				return KANGARU5_FWD(source).template provide<T>();
			}
			
			template<typename Source> requires member_source_of<Source, T>
			KANGARU5_INLINE constexpr auto operator()(Source&& source) const -> T {
				return KANGARU5_FWD(source).provide();
			}
		};
		
		namespace niebloid {
			template<injectable T>
			inline constexpr auto provide = detail::source_private::provide_function<T>{};
		}
		
		template<typename T>
		struct deduced_source_type_impl {
			using type = T;
		};
		
		template<typename F>
		struct deduced_source_type_impl<in_place_construct<F>> {
			using type = detail::call_result_t<F>;
		};
	}
	
	KANGARU5_EXPORT inline namespace niebloid {
		using namespace detail::source_private::niebloid;
	}
	
	KANGARU5_EXPORT template<typename Source, typename T>
	concept source_of =
		    forwarded_source<Source>
		and injectable<T>
		and (
			   detail::source_private::member_source_of<Source, T>
			or detail::source_private::member_template_source_of<Source, T>
			or detail::source_private::adl_nonmember_source_of<Source, T>
			or detail::source_private::adl_nonmember_template_source_of<Source, T>
		);
	
	template<typename T, typename F, typename... Args>
	concept callable_returns_source_of =
		    callable<F, Args...>
		and requires(F&& f, Args&&... args) {
			{ KANGARU5_FWD(f)(KANGARU5_FWD(args)...) } -> source_of<T>;
		};
	
	template<typename T, typename F, typename TParam, typename... Args>
	concept callable_template_1t_returns_source_of =
		    callable_template_1t<F, TParam, Args...>
		and requires(F&& f, Args&&... args) {
			{ KANGARU5_FWD(f).template operator()<TParam>(KANGARU5_FWD(args)...) } -> source_of<T>;
		};
	
	KANGARU5_EXPORT template<typename Source>
	concept weak_wrapping_source =
		    source<std::remove_reference_t<Source>>
		and requires(Source source) {
			{ source.source };
		};
	
	KANGARU5_EXPORT template<typename Source> requires weak_wrapping_source<std::remove_reference_t<Source>>
	using wrapped_source_t = std::remove_reference_t<decltype((std::declval<Source&&>().source))>;
	
	KANGARU5_EXPORT template<typename Source>
	concept wrapping_source =
		    weak_wrapping_source<Source>
		and source<wrapped_source_t<Source>>;
	
	KANGARU5_EXPORT template<typename Source>
	concept forwarded_wrapping_source = wrapping_source<std::remove_reference_t<Source>>;
	
	KANGARU5_EXPORT struct none_source {};
	static_assert(source<none_source>);
	
	KANGARU5_EXPORT template<forwarded_wrapping_source Source>
	using forwarded_wrapped_source_t = detail::forward_like_t<Source, wrapped_source_t<Source>>;
	
	KANGARU5_EXPORT template<typename Source, typename T>
	concept wrapping_source_of =
		    forwarded_wrapping_source<Source>
		and source_of<forwarded_wrapped_source_t<Source>, T>;
	
	KANGARU5_EXPORT template<injectable T, source Source>
	struct provide_using {
		template<allows_construction_of<Source> S>
		explicit constexpr provide_using(S&& source) noexcept : source(KANGARU5_FWD(source)) {}
		
		constexpr auto operator()() & -> T requires source_of<Source&, T> {
			return kangaru::provide<T>(source);
		}
		
		constexpr auto operator()() const& -> T requires source_of<Source const&, T> {
			return kangaru::provide<T>(source);
		}
		
		constexpr auto operator()() && -> T requires source_of<Source&&, T> {
			return kangaru::provide<T>(std::move(source));
		}
		
		constexpr auto operator()() const&& -> T requires source_of<Source const&&, T> {
			return kangaru::provide<T>(std::move(source));
		}
		
	private:
		Source source;
	};
	
	KANGARU5_EXPORT template<typename T>
	using deduced_source_type = typename detail::source_private::deduced_source_type_impl<std::decay_t<T>>::type;
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_HPP
