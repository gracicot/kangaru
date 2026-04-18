#ifndef KANGARU5_DETAIL_ATTRIBUTES_HPP
#define KANGARU5_DETAIL_ATTRIBUTES_HPP

#include "concepts.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <tuple>
#endif

#include "define.hpp"

namespace kangaru::detail::attribute_private {
	auto attribute(auto&&) -> void requires false;
	
	template<template<typename> typename Attribute, weak_injectable T, typename Default>
	struct attribute_function {
		template<typename A>
		static constexpr auto lookup(int) -> decltype(attribute(std::declval<Attribute<A>>()));
		
		template<typename A>
		static constexpr auto lookup(void*) -> Default;
	public:
		using type = decltype(lookup<T>(0));
	};
	
	template<template<typename> typename Attribute, weak_injectable T, typename Default>
	using attribute_function_t = typename attribute_function<Attribute, T, Default>::type;
	
	template<template<typename> typename Attribute, weak_injectable T>
	struct evaluate_attribute {
		using type = Attribute<T>;
	};
	
	template<template<typename> typename Attribute, weak_injectable T>
		requires requires{ typename Attribute<T>::template ttype<T>; }
	struct evaluate_attribute<Attribute, T> {
		using type = typename Attribute<T>::template ttype<T>;
	};
	
	template<template<weak_injectable> typename Trait, weak_injectable T>
	using evaluate_attribute_t = typename evaluate_attribute<Trait, T>::type;
}

KANGARU5_EXPORT namespace kangaru {
	template<typename T>
	struct allow_runtime_caching {
		template<weak_injectable A> requires(std::same_as<T, A>)
		using ttype = typename detail::attribute_private::attribute_function_t<allow_runtime_caching, A, std::false_type>;
	};
	
	template<weak_injectable T>
	inline constexpr auto allow_runtime_caching_v = detail::attribute_private::evaluate_attribute_t<allow_runtime_caching, T>::value;
	
	template<typename T>
	struct allow_empty_injection {
		template<weak_injectable A> requires(std::same_as<T, A>)
		using ttype = typename detail::attribute_private::attribute_function_t<allow_empty_injection, A, std::false_type>;
	};
	
	template<weak_injectable T>
	inline constexpr auto allow_empty_injection_v = detail::attribute_private::evaluate_attribute_t<allow_empty_injection, T>::value;
	
	// Consider having one type by default and a special list of types if necessary.
	template<typename T>
	struct overrides_types_in_cache {
		template<weak_injectable A> requires(std::same_as<T, A>)
		struct ttype {
			using type = detail::attribute_private::attribute_function_t<overrides_types_in_cache, A, std::tuple<>>;
		};
	};
	
	template<typename T>
	using overrides_types_in_cache_t = typename detail::attribute_private::evaluate_attribute_t<overrides_types_in_cache, T>::type;
	
	template<typename T>
	struct allow_injection_using {
		template<weak_injectable A> requires(std::same_as<T, A>)
		using ttype = typename detail::attribute_private::attribute_function_t<allow_injection_using, A, std::true_type>;
	};
	
	template<weak_injectable T>
	inline constexpr auto allow_injection_using_v = detail::attribute_private::evaluate_attribute_t<allow_injection_using, T>::value;
	
	template<typename T>
	struct assume_runtime_cached {
		template<weak_injectable A> requires(std::same_as<T, A>)
		using ttype = typename detail::attribute_private::attribute_function_t<assume_runtime_cached, A, std::false_type>;
	};
	
	template<weak_injectable T>
	inline constexpr auto assume_runtime_cached_v = detail::attribute_private::evaluate_attribute_t<assume_runtime_cached, T>::value;
	
	// forward declaration to avoid circular dependency
	struct noop_second_step;
	
	template<typename T>
	struct second_step_init {
		template<weak_injectable A> requires(std::same_as<T, A>)
		struct ttype {
			using type = typename detail::attribute_private::attribute_function_t<second_step_init, A, noop_second_step>;
		};
	};
	
	template<weak_injectable T>
	using second_step_init_t = detail::attribute_private::evaluate_attribute_t<second_step_init, T>::type;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_ATTRIBUTES_HPP
