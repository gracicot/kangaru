#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP

#include "void_t.hpp"

#include <type_traits>
#include <utility>

namespace kgr {
namespace detail {

/*
 * Type trait that check if a particular type T indeed have a forward function, and ensure it doesn't return void.
 */
template<typename T, typename = void>
struct has_forward : std::false_type {};

/*
* Specialization of has_non_void_forward when the type T has a forward member function.
*/
template<typename T>
struct has_forward<T, void_t<decltype(std::declval<T>().forward())>> : std::integral_constant<bool,
	!std::is_same<decltype(std::declval<T>().forward()), void>::value
> {};

/*
 * Type trait that extract the return type of the forward function.
 */
template<typename, typename = void>
struct service_type_helper {};

/*
 * Specialization of ServiceTypeHelper when T has a valid forward function callable without parameter.
 * Makes an alias to the return type of the forward function.
 */
template<typename T>
struct service_type_helper<T, typename std::enable_if<has_forward<T>::value>::type> {
	using type = decltype(std::declval<T>().forward());
};

/*
 * Variable sent as parameter for in_place_t.
 * 
 * Used by the container to desambiguate between contructor that construct the service, and those that doesnt.
 * 
 * We defined the variable before the type because the variable is private, but the type is public.
 */
struct {} constexpr in_place{};

namespace nostd {
// Implementation of std::invoke
// Inspired by https://stackoverflow.com/questions/33462729/how-does-stdinvokec1z-work
template <typename F, typename... Args>
inline auto invoke(F&& f, Args&&... args)
noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
-> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
{
	return std::forward<F>(f)(std::forward<Args>(args)...);
}

template <typename Base, typename T, typename Derived>
inline auto invoke(T Base::*pmd, Derived&& ref) noexcept ->
decltype((std::forward<Derived>(ref).*pmd))
{
	return std::forward<Derived>(ref).*pmd;
}

template <typename Base, typename T, typename Derived>
inline auto invoke(T Base::*pmd, std::reference_wrapper<Derived> ref) noexcept ->
decltype((ref.get().*pmd))
{
	return ref.get().*pmd;
}

template <typename PMD, typename Pointer>
inline auto invoke(PMD pmd, Pointer&& ptr) noexcept ->
decltype(((*std::forward<Pointer>(ptr)).*pmd))
{
	return (*std::forward<Pointer>(ptr)).*pmd;
}

template <typename Base, typename T, typename Derived, typename... Args>
inline auto invoke(T Base::*pmf, Derived&& ref, Args&&... args)
noexcept(noexcept((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...)))
-> decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...))
{
	return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
}

template <typename Base, typename T, typename Derived, typename... Args>
inline auto invoke(T Base::*pmf, std::reference_wrapper<Derived> ref, Args&&... args)
noexcept(noexcept((ref.get().*pmf)(std::forward<Args>(args)...)))
-> decltype((ref.get().*pmf)(std::forward<Args>(args)...))
{
	return (ref.get().*pmf)(std::forward<Args>(args)...);
}

template <typename PMF, typename Pointer, typename... Args>
inline auto invoke(PMF pmf, Pointer&& ptr, Args&&... args)
noexcept(noexcept(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...)))
-> decltype(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...))
{
	return ((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...);
}

template<typename... Args>
using invoke_result_t = decltype(invoke(std::declval<Args>()...));

} // namespace nostd
} // namespace detail

/**
 * This type is the return type of the forward function of the service T.
 */
template<typename T>
using service_type = typename detail::service_type_helper<T>::type;

/*
 * This is a tag type to mark a constructor as the one the container should use to construct a definition.
 */
using in_place_t = decltype(detail::in_place);

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_UTILS_HPP
