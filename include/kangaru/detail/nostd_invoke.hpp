#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_NOSTD_INVOKE
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_NOSTD_INVOKE

namespace kgr {
namespace detail {
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
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_NOSTD_INVOKE
