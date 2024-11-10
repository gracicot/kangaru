#ifndef KANGARU5_DETAIL_SOURCE_HPP
#define KANGARU5_DETAIL_SOURCE_HPP

#include "concepts.hpp"
#include "utility.hpp"

#include <concepts>

#include "define.hpp"

namespace kangaru {
	template<typename T>
	concept injectable = object<T> or reference<T>;
	
	template<typename T>
	concept source = object<T> and std::move_constructible<T> and std::is_class_v<T>;
	
	template<typename T>
	concept source_ref = reference<T> and source<std::remove_cvref_t<T>>;
	
	template<typename T>
	concept forwarded_source = source<std::remove_reference_t<T>>;
	
	namespace detail::source {
		template<typename = void>
		auto provide(auto&&) requires false = delete;
		
		template<typename Source, typename T>
		concept adl_nonmember_source_of =
			    kangaru::source<std::remove_cvref_t<Source>>
			and kangaru::injectable<T>
			and requires(Source&& source) {
				{ provide(KANGARU5_FWD(source)) } -> std::same_as<T>;
			};
		
		template<typename Source, typename T>
		concept adl_nonmember_template_source_of =
			    kangaru::source<std::remove_cvref_t<Source>>
			and kangaru::injectable<T>
			and requires(Source&& source) {
				{ provide<T>(KANGARU5_FWD(source)) } -> std::same_as<T>;
			};
		
		template<typename Source, typename T>
		concept member_template_source_of =
			    kangaru::source<std::remove_cvref_t<Source>>
			and kangaru::injectable<T>
			and requires(Source&& source) {
				{ KANGARU5_FWD(source).template provide<T>() } -> std::same_as<T>;
			};
		
		template<typename Source, typename T>
		concept member_source_of =
			    kangaru::source<std::remove_cvref_t<Source>>
			and kangaru::injectable<T>
			and requires(Source&& source) {
				{ KANGARU5_FWD(source).provide() } -> std::same_as<T>;
			};
		
		template<kangaru::injectable T>
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
			template<kangaru::injectable T>
			inline constexpr auto provide = detail::source::provide_function<T>{};
		}
	}
	
	inline namespace niebloid {
		using namespace detail::source::niebloid;
	}
	
	template<typename Source, typename T>
	concept source_of =
		   detail::source::member_source_of<Source, T>
		or detail::source::member_template_source_of<Source, T>
		or detail::source::adl_nonmember_source_of<Source, T>
		or detail::source::adl_nonmember_template_source_of<Source, T>;
	
	template<typename Source>
	concept weak_wrapping_source =
		    source<std::remove_reference_t<Source>>
		and requires(Source source) {
			{ source.source };
		};
	
	template<typename Source> requires weak_wrapping_source<std::remove_reference_t<Source>>
	using wrapped_source_t = std::remove_reference_t<decltype(std::declval<Source&&>().source)>;
	
	template<typename Source>
	concept wrapping_source =
		    weak_wrapping_source<Source>
		and source<wrapped_source_t<Source>>;
	
	struct none_source {};
	static_assert(source<none_source>);
	
	template<wrapping_source Source>
	using forwarded_wrapped_source_t = detail::utility::forward_like_t<Source, wrapped_source_t<Source>>;
	
	template<typename Source, typename T>
	concept wrapping_source_of =
		    wrapping_source<std::remove_reference_t<Source>>
		and source_of<forwarded_wrapped_source_t<Source>, T>;
	
	
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_HPP
