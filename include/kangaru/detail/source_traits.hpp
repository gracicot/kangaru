#ifndef KANGARU5_DETAIL_SOURCE_TRAITS_HPP
#define KANGARU5_DETAIL_SOURCE_TRAITS_HPP

#include "source_reference_wrapper.hpp"
#include "source.hpp"

#ifndef KANGARU5_MODULES
#include <cstddef>
#endif

#include "define.hpp"

namespace kangaru {
	namespace detail::source_traits_private {
		template<std::size_t index, injectable T, forwarded_source... Sources>
		struct select_source_of_impl {};
		
		template<std::size_t idx, injectable T, forwarded_source Source, forwarded_source... Sources>
			requires(source_of<Source, T>)
		struct select_source_of_impl<idx, T, Source, Sources...> {
			using type = Source;
			static constexpr auto index = idx;
		};
		
		template<std::size_t index, injectable T, forwarded_source Source, forwarded_source... Sources>
			requires(not source_of<Source, T>)
		struct select_source_of_impl<index, T, Source, Sources...> : select_source_of_impl<index + 1, T, Sources...> {};
		
		template<std::size_t index, injectable T>
		struct select_source_of_impl<index, T> {};
		
		template<injectable T, forwarded_source... Sources>
		inline constexpr auto has_one_result = (0 + ... + (source_of<Sources, T> ? 1 : 0)) == 1;
	}
	
	template<injectable T, forwarded_source... Sources>
		requires(detail::source_traits_private::has_one_result<T, Sources...>)
	using select_source_of = typename detail::source_traits_private::select_source_of_impl<0, T, Sources...>::type;
	
	template<injectable T, forwarded_source... Sources>
	inline constexpr auto select_source_of_index = detail::source_traits_private::select_source_of_impl<0, T, Sources...>::index;
	
	namespace detail::source_traits_private {
		template<std::size_t level, source Source>
		struct get_nested_wrapped_source {};
		
		template<typename Source>
		struct get_nested_wrapped_source<0, Source> {
			using type = Source;
		};
		
		template<std::size_t level, source Source>
			requires(level > 0 and forwarded_wrapping_source<maybe_unwrap_result_t<Source&&>>)
		struct get_nested_wrapped_source<level, Source> :
			get_nested_wrapped_source<
				level - 1,
				wrapped_source_t<std::remove_cvref_t<maybe_unwrap_result_t<Source>>>
			>{};
	} // namespace detail::source_traits_private
	
	template<std::size_t level, source Source>
	using get_nested_wrapped_source_t = typename detail::source_traits_private::get_nested_wrapped_source<level, Source>::type;
	
	template<std::size_t level, forwarded_source Source>
		requires(level == 0 or forwarded_wrapping_source<maybe_unwrap_result_t<Source>>)
	static constexpr auto get_nested_wrapped_source(Source&& source) -> detail::forward_like_t<
		Source,
		get_nested_wrapped_source_t<level, std::remove_cvref_t<maybe_unwrap_result_t<Source>>>
	> {
		if constexpr (level > 0) {
			return KANGARU5_NO_ADL(get_nested_wrapped_source<level - 1>)(KANGARU5_NO_ADL(maybe_unwrap)(KANGARU5_FWD(source)).source);
		} else {
			return KANGARU5_FWD(source);
		}
	}
	
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TRAITS_HPP
