#ifndef KGR_INCLUDE_KANGARU_DETAIL_SINGLE_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_SINGLE_HPP

#include "traits.hpp"

#include <tuple>

namespace kgr {

struct Single {
	Single() = default;
	~Single() = default;
	Single(const Single&) = delete;
	Single& operator=(const Single&) = delete;
	Single(Single&&) = default;
	Single& operator=(Single&&) = default;
};

namespace detail {

template<typename, typename = void>
struct parent_type_helper;

template<typename, typename = void>
struct default_type_helper;

}

template<typename T>
struct Default {
private:
	template<typename, typename> friend struct detail::default_type_helper;
	using DefaultService = T;
};

template<typename... Types>
struct Overrides {
private:
	template<typename, typename> friend struct detail::parent_type_helper;
	using ParentTypes = std::tuple<Types...>;
};

namespace detail {

template<typename, typename>
struct parent_type_helper {
	using ParentTypes = std::tuple<>;
};

template<typename T>
struct parent_type_helper<T, void_t<typename T::ParentTypes>> {
	using ParentTypes = typename T::ParentTypes;
};

template<typename T>
using parent_types = typename parent_type_helper<T>::ParentTypes;

template<typename, typename>
struct default_type_helper : std::false_type {
	using has_default = std::false_type;
};

template<typename T>
struct default_type_helper<T, void_t<typename T::DefaultService>> {
	using has_default = std::true_type;
	using Service = typename T::DefaultService;
};

template<typename T>
using default_type = typename default_type_helper<T>::Service;

template<typename T>
using has_default = typename default_type_helper<T>::has_default;

template<typename T>
using is_single = std::is_base_of<Single, T>;

template<typename Service, typename Overrider>
struct is_overriden_by_helper {
private:
	
	template<typename T, std::size_t I>
	static std::is_same<T, typename std::tuple_element<I, parent_types<Overrider>>::type> element_test();
	
	template<typename T>
	static std::false_type test_helper(seq<>);
	
	template<typename T, std::size_t first, std::size_t... tail, typename Element = decltype(element_test<T, first>()), typename Next = decltype(test_helper<T>(seq<tail...>{}))>
	static std::integral_constant<bool, (Element::value || Next::value)> test_helper(seq<first, tail...>);
	
	template<typename T>
	static auto test(int) -> decltype(test_helper<T>(tuple_seq<parent_types<Overrider>>{}));
	
	template<typename T>
	static std::false_type test(...);
	
public:
	using type = decltype(test<Service>(0));
};

template<typename Service, typename Overrider>
using is_overriden_by = typename is_overriden_by_helper<Service, Overrider>::type;

} // namespace detail
} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_SINGLE_HPP
