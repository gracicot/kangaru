#ifndef KANGARU5_DETAIL_ATTRIBUTES_HPP
#define KANGARU5_DETAIL_ATTRIBUTES_HPP

#include "source.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru::detail::attribute {
	auto attribute(auto&&) -> void requires false;
	
	template<template<typename> typename Attribute, injectable T, typename Default>
	struct attribute_function {
		template<typename A>
		static constexpr auto lookup(int) -> decltype(attribute(std::declval<Attribute<A>>()));
		
		template<typename A>
		static constexpr auto lookup(void*) -> Default;
	public:
		using type = decltype(lookup<T>(0));
	};
	
	template<template<typename> typename Attribute, injectable T, typename Default>
	using attribute_function_t = typename attribute_function<Attribute, T, Default>::type;
	
	template<template<typename> typename Attribute, injectable T>
	struct evaluate_attribute {
		using type = Attribute<T>;
	};
	
	template<template<typename> typename Attribute, injectable T>
		requires requires{ typename Attribute<T>::template ttype<T>; }
	struct evaluate_attribute<Attribute, T> {
		using type = typename Attribute<T>::template ttype<T>;
	};
	
	template<template<injectable> typename Trait, injectable T>
	using evaluate_attribute_t = typename evaluate_attribute<Trait, T>::type;
}

KANGARU5_EXPORT namespace kangaru {
	template<injectable T>
	struct allow_runtime_caching {
		template<injectable A>
		using ttype = typename detail::attribute::attribute_function_t<allow_runtime_caching, A, std::false_type>;
	};
	
	template<injectable T>
	inline constexpr auto allow_runtime_caching_v = detail::attribute::evaluate_attribute_t<allow_runtime_caching, T>::value;
	
	template<injectable T>
	struct allow_empty_injection {
		template<injectable A>
		using ttype = typename detail::attribute::attribute_function_t<allow_empty_injection, A, std::false_type>;
	};
	
	template<injectable T>
	inline constexpr auto allow_empty_injection_v = detail::attribute::evaluate_attribute_t<allow_empty_injection, T>::value;
	
	template<injectable T>
	struct overrides_types_in_cache {
		template<injectable A>
		struct ttype {
			using type = detail::attribute::attribute_function_t<overrides_types_in_cache, A, std::tuple<>>;
		};
	};
	
	template<typename T>
	using overrides_types_in_cache_t = typename detail::attribute::evaluate_attribute_t<overrides_types_in_cache, T>::type;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_ATTRIBUTES_HPP
