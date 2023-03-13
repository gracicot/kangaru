#ifndef KANGARU5_DETAIL_SOURCE_HPP
#define KANGARU5_DETAIL_SOURCE_HPP

#include "concepts.hpp"

#include <concepts>
#include <cstdint>

#include "define.hpp"

namespace kangaru {
	template<typename>
	struct provide_tag_t {};
	
	template<typename T>
	inline constexpr auto provide_tag = provide_tag_t<T>{}; 
	
	template<typename T>
	concept source = detail::concepts::prvalue<T> and std::move_constructible<T>;
	
	template<typename T>
	concept source_ref = std::is_reference_v<T> and source<std::remove_cvref_t<T>>;
	
	namespace detail::source {
		auto provide(provide_tag_t<struct poison>, auto&&) = delete;

		template<typename Source, typename T>
		concept adl_nonmember_source_of =
			    kangaru::source<std::remove_cvref_t<T>>
			and requires(provide_tag_t<T> tag, Source&& source) {
				{ provide(tag, KANGARU5_FWD(source)) } -> std::same_as<T>;
			};
		
		struct provide_function {
			template<typename T, typename Source> requires adl_nonmember_source_of<Source, T>
			constexpr auto operator()(provide_tag_t<T> tag, Source&& source) const -> T {
				return provide(tag, KANGARU5_FWD(source));
			}
		};
	}
	
	inline constexpr auto provide = detail::source::provide_function{};
	
	template<typename Source, typename T>
	concept source_of = detail::source::adl_nonmember_source_of<Source, T>;
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_HPP
