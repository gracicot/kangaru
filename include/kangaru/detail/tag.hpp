#ifndef KANGARU5_DETAIL_TAG_HPP
#define KANGARU5_DETAIL_TAG_HPP

#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru::detail::tag {
	auto config(auto&&) -> void requires false;
	
	template<injectable T, typename Default, template<typename> typename Tag>
	struct tag_function {
		template<typename A>
		static constexpr auto lookup(int) -> decltype(config(std::declval<Tag<A>&>()));
		
		template<typename A>
		static constexpr auto lookup(void*) -> Default;
	public:
		using type = decltype(lookup<T>(0));
	};
	
	template<injectable T, typename Default, template<typename> typename Tag>
	using tag_function_t = typename tag_function<T, Default, Tag>::type;
	
	template<template<injectable> typename Trait, injectable T>
	struct evaluate_tag {
		using type = Trait<T>;
	};
	
	template<template<injectable> typename Trait, injectable T>
		requires requires{ typename Trait<T>::template ttype<T>; }
	struct evaluate_tag<Trait, T> {
		using type = typename Trait<T>::template ttype<T>;
	};
	
	template<template<injectable> typename Trait, injectable T>
	using evaluate_tag_t = typename evaluate_tag<Trait, T>::type;
}

KANGARU5_EXPORT namespace kangaru {
	template<injectable T>
	struct allow_runtime_caching {
		template<injectable A>
		using ttype = typename detail::tag::tag_function_t<A, std::false_type, allow_runtime_caching>;
	};
	
	template<injectable T>
	inline constexpr auto allow_runtime_caching_v = detail::tag::evaluate_tag_t<allow_runtime_caching, T>::value;
	
	template<injectable T>
	struct allow_empty_injection {
		template<injectable A>
		using ttype = typename detail::tag::tag_function_t<A, std::false_type, allow_empty_injection>;
	};
	
	template<injectable T>
	inline constexpr auto allow_empty_injection_v = detail::tag::evaluate_tag_t<allow_empty_injection, T>::value;
	
	template<injectable T>
	struct overrides_types_in_cache {
		template<injectable A>
		struct ttype {
			using type = detail::tag::tag_function_t<A, std::tuple<>, overrides_types_in_cache>;
		};
	};
	
	template<typename T>
	using overrides_types_in_cache_t = typename detail::tag::evaluate_tag_t<overrides_types_in_cache, T>::type;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_TAG_HPP
