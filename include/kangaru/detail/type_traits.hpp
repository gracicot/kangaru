#ifndef KANGARU_DETAIL_TYPE_TRAITS_HPP
#define KANGARU_DETAIL_TYPE_TRAITS_HPP

#ifndef KANGARU5_MODULES
#include <utility>
#endif

#include "define.hpp"

namespace kangaru::detail {
	// Faster conditional implementation
	template<bool b>
	struct conditional;

	template<>
	struct conditional<true> {
		template<typename A, typename>
		using type = A;
	};

	template<>
	struct conditional<false> {
		template<typename, typename B>
		using type = B;
	};
	
	template<bool condition, typename A, typename B>
	using conditional_t = typename conditional<condition>::template type<A, B>;
	
	template<typename F, typename... Args>
	using call_result_t = decltype(std::declval<F>()(std::declval<Args>()...));
	
	template<typename F, typename T, typename... Args>
	using call_result_template_1t_t = decltype(std::declval<F>().template operator()<T>(std::declval<Args>()...));
}

#include "undef.hpp"

#endif // KANGARU_DETAIL_TYPE_TRAITS_HPP
