#ifndef KANGARU5_DETAIL_SOURCE_TRAITS_HPP
#define KANGARU5_DETAIL_SOURCE_TRAITS_HPP

#include "source.hpp"

#ifndef KANGARU5_MODULES

#endif

#include "define.hpp"

namespace kangaru {
	namespace detail::source_traits {
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
	}
	
	template<injectable T, forwarded_source... Sources>
	using select_source_of = typename detail::source_traits::select_source_of_impl<0, T, Sources...>::type;
	
	template<injectable T, forwarded_source... Sources>
	inline constexpr auto select_source_of_index = detail::source_traits::select_source_of_impl<0, T, Sources...>::index;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TRAITS_HPP
