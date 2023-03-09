#ifndef KANGARU_DETAIL_TYPE_TRAITS_HPP
#define KANGARU_DETAIL_TYPE_TRAITS_HPP

namespace kangaru::detail::type_traits {
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
}

#endif // KANGARU_DETAIL_TYPE_TRAITS_HPP
