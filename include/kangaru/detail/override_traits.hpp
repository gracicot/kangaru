#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_TRAITS_HPP

#include "traits.hpp"
#include "meta_list.hpp"

namespace kgr {
namespace detail {

template<typename T>
struct is_override_service_helper {
private:
	template<typename...>
	static std::false_type test(...);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<is_service<meta_list_element_t<S, parent_types<U>>>::value>...> = 0>
	static std::true_type test(seq<S...>);
	
public:
	using type = decltype(test<T>(tuple_seq<parent_types<T>>{}));
};

template<typename T>
using is_override_service = typename is_override_service_helper<T>::type;

template<typename T>
struct is_override_convertible_helper {
private:
	// This is a workaround for msvc. Expansion in very complex expression
	// leaves the compiler without clues about what's going on.
	template<std::size_t I, typename U>
	struct expander {
		using type = is_explicitly_convertible<ServiceType<U>, ServiceType<meta_list_element_t<I, parent_types<U>>>>;
	};

	template<typename...>
	static std::false_type test(...);

	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<expander<S, U>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<is_service<meta_list_element_t<S, parent_types<U>>>::value>...> = 0>
	static decltype(test_helper<U>(seq<S...>{})) test(seq<S...>);
	
public:
	using type = decltype(test<T>(tuple_seq<parent_types<T>>{}));
};

template<typename T>
using is_override_convertible = typename is_override_convertible_helper<T>::type;

template<typename T>
struct is_override_services_helper {
private:
	// This is a workaround for msvc. Expansion in very complex expression
	// leaves the compiler without clues about what's going on.
	template<std::size_t I, typename U>
	struct expander {
		using type = is_service<meta_list_element_t<I, parent_types<U>>>;
	};
	
	template<typename...>
	static std::false_type test(...);

	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<expander<S, U>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<is_service<meta_list_element_t<S, parent_types<U>>>::value>...> = 0>
	static decltype(test_helper<U>(seq<S...>{})) test(seq<S...>);
	
public:
	using type = decltype(test<T>(tuple_seq<parent_types<T>>{}));
};

template<typename T>
using is_override_services = typename is_override_services_helper<T>::type;

template<typename T>
struct is_override_virtual_helper {
private:
	// This is a workaround for msvc. Expansion in very complex expression
	// leaves the compiler without clues about what's going on.
	template<std::size_t I, typename U>
	struct expander {
		using type = is_virtual<meta_list_element_t<I, parent_types<U>>>;
	};
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename U, std::size_t... S, int_t<enable_if_t<expander<S, U>::type::value>...> = 0>
	static std::true_type test(seq<S...>);
	
public:
	using type = decltype(test<T>(tuple_seq<parent_types<T>>{}));
};

template<typename T>
using is_override_virtual = typename is_override_virtual_helper<T>::type;

template<typename T>
struct is_default_overrides_abstract_helper {
private:
	template<typename>
	static std::false_type test(...);
	
	template<typename U, enable_if_t<has_default<U>::value && is_abstract_service<U>::value, int> = 0>
	static is_overriden_by<U, default_type<U>> test(int);
	
	template<typename U, enable_if_t<!has_default<U>::value, int> = 0>
	static std::true_type test(int);
	
public:
	using type = decltype(test<T>(0));
};

template<typename T>
using is_default_overrides_abstract = typename is_default_overrides_abstract_helper<T>::type;

template<typename T>
struct is_default_convertible_helper {
private:
	template<typename>
	static std::false_type test(...);
	
	template<typename U, enable_if_t<has_default<U>::value, int> = 0>
	static is_explicitly_convertible<ServiceType<default_type<U>>, ServiceType<U>> test(int);
	
	template<typename U, enable_if_t<!has_default<U>::value, int> = 0>
	static std::true_type test(int);
	
public:
	using type = decltype(test<T>(0));
};

template<typename T>
using is_default_convertible = typename is_default_convertible_helper<T>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_OVERRIDE_TRAITS_HPP
