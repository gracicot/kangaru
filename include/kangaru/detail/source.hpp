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
	
	namespace detail::source {
		auto provide(provide_tag_t<struct poison>, auto&&) = delete;
		
		struct provide_function {
			template<typename T, typename Source>
			constexpr auto operator()(provide_tag_t<T> tag, Source&& source) const -> decltype(provide(tag, KANGARU5_FWD(source))) {
				return provide(tag, KANGARU5_FWD(source));
			}
		};
	}
	
	inline constexpr auto provide = detail::source::provide_function{};
	
	template<typename Source, typename T>
	concept nonmember_source_of = requires(Source&& source) {
		{ kangaru::provide(provide_tag<T>, KANGARU5_FWD(source)) } -> std::same_as<T>;
	};
	
	template<typename Source, typename T>
	concept source_of = nonmember_source_of<Source, T>;
	
	template<typename T>
	concept source = detail::concepts::object<T> and std::move_constructible<T>;
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_HPP
