#ifndef KANGARU5_DETAIL_SOURCE_HPP
#define KANGARU5_DETAIL_SOURCE_HPP

#include "concepts.hpp"

#include <concepts>
#include <cstdint>

#include "define.hpp"

namespace kangaru {
	template<typename>
	struct provide_tag {};
	
	template<typename T>
	inline constexpr auto provide_tag_v = provide_tag<T>{}; 
	
	template<typename T>
	concept source = object<T> and std::move_constructible<T>;
	
	template<typename T>
	concept source_ref = std::is_reference_v<T> and source<std::remove_cvref_t<T>>;
	
	namespace detail::source {
		auto provide(provide_tag<struct poison>, auto&&) = delete;
		
		template<typename Source, typename T>
		concept adl_nonmember_source_of =
			    kangaru::source<std::remove_cvref_t<Source>>
			and requires(provide_tag<T> tag, Source&& source) {
				{ provide(tag, KANGARU5_FWD(source)) } -> std::same_as<T>;
			};
		
		template<typename Source, typename T>
		concept member_template_source_of =
			    kangaru::source<std::remove_cvref_t<Source>>
			and requires(Source&& source) {
				{ KANGARU5_FWD(source).template provide<T>() } -> std::same_as<T>;
			};
		
		template<typename Source, typename T>
		concept member_source_of =
			    kangaru::source<std::remove_cvref_t<Source>>
			and requires(Source&& source) {
				{ KANGARU5_FWD(source).provide() } -> std::same_as<T>;
			};
		
		struct provide_function {
			template<typename T, typename Source> requires adl_nonmember_source_of<Source, T>
			KANGARU5_INLINE constexpr auto operator()(provide_tag<T> tag, Source&& source) const -> T {
				return provide(tag, KANGARU5_FWD(source));
			}
			
			template<typename T, typename Source> requires member_template_source_of<Source, T>
			KANGARU5_INLINE constexpr auto operator()(provide_tag<T>, Source&& source) const -> T {
				return KANGARU5_FWD(source).template provide<T>();
			}
			
			template<typename T, typename Source> requires member_source_of<Source, T>
			KANGARU5_INLINE constexpr auto operator()(provide_tag<T>, Source&& source) const -> T {
				return KANGARU5_FWD(source).provide();
			}
		};
	}
	
	inline namespace neibloid {
		inline constexpr auto provide = detail::source::provide_function{};
	}
	
	template<typename Source, typename T>
	concept source_of =
		   detail::source::adl_nonmember_source_of<Source, T>
		or detail::source::member_template_source_of<Source, T>
		or detail::source::member_source_of<Source, T>;
	
	template<typename T>
	concept wrapping_source = source<T> and source<std::decay_t<decltype(std::declval<T>().source)>>;
	
	struct noop_source {};
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_HPP
