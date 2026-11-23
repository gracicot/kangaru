#ifndef KANGARU5_DETAIL_ATTRIBUTES_HPP
#define KANGARU5_DETAIL_ATTRIBUTES_HPP
// IWYU pragma: private, include <kangaru/kangaru.hpp>

#include "concepts.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <tuple>
#endif

#include "define.hpp"

namespace kangaru {
	template<typename... Overrides>
	struct override_type_list {};
}

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
	
	template<typename T>
	struct wrap_in_type_list {
		using type = override_type_list<T>;
	};
	
	template<typename... Overrides>
	struct wrap_in_type_list<override_type_list<Overrides...>> {
		using type = override_type_list<Overrides...>;
	};
	
	template<typename T>
	using wrap_in_type_list_t = typename wrap_in_type_list<T>::type;
	
	template<std::size_t, typename>
	struct type_list_element;
	
	template<std::size_t I>
	struct type_list_element<I, override_type_list<>> {};
	
	template<typename Head, typename... Tail>
	struct type_list_element<0, override_type_list<Head, Tail...>> {
		using type = Head;
	};
	
	template<std::size_t I, typename Head, typename... Tail>
	struct type_list_element<I, override_type_list<Head, Tail...>> : type_list_element<I - 1, override_type_list<Tail...>> {};
}

template<typename... Overrides>
struct std::tuple_size<kangaru::override_type_list<Overrides...>> :
	std::integral_constant<std::size_t, sizeof...(Overrides)> {};

template<typename... Overrides, std::size_t N>
struct std::tuple_element<N, kangaru::override_type_list<Overrides...>> :
	kangaru::detail::attribute_private::type_list_element<N, kangaru::override_type_list<Overrides...>> {};

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
	
	template<typename T>
	struct overrides_in_cache {
		template<weak_injectable A> requires(std::same_as<T, A>)
		struct ttype {
			using type = detail::attribute_private::attribute_function_t<overrides_in_cache, A, override_type_list<>>;
		};
	};
	
	template<typename T>
	using overrides_in_cache_t = detail::attribute_private::wrap_in_type_list_t<typename detail::attribute_private::evaluate_attribute_t<overrides_in_cache, T>::type>;
	
	template<typename T>
	struct allow_injection_using {
		template<weak_injectable A> requires(std::same_as<T, A>)
		using ttype = typename detail::attribute_private::attribute_function_t<allow_injection_using, A, std::true_type>;
	};
	
	template<typename... Overrides>
	struct allow_injection_using<override_type_list<Overrides...>> : std::false_type {};
	
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
