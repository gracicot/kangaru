#ifndef KANGARU5_DETAIL_DEFINE_HPP
#define KANGARU5_DETAIL_DEFINE_HPP

#if defined(_MSC_VER) && !defined(__clang__)
#define KANGARU5_IS_MSVC 1
#else
#define KANGARU5_IS_MSVC 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define KANGARU5_IS_GNU 1
#else
#define KANGARU5_IS_GNU 0
#endif

#if defined(__clang__)
#define KANGARU5_IS_CLANG 1
#else
#define KANGARU5_IS_CLANG 0
#endif

#if KANGARU5_IS_MSVC == 1
#define KANGARU5_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define KANGARU5_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#if KANGARU_IS_MSVC == 1
#define KANGARU5_FUNCTION_SIGNATURE __FUNCSIG__
#else
#define KANGARU5_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

#define KANGARU5_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#define KANGARU5_CONSTRUCTOR_T(...) decltype(::kangaru::constructor<__VA_ARGS__>())

#if KANGARU5_IS_CLANG == 1
#define KANGARU5_INLINE [[gnu::always_inline]]
#define KANGARU5_RVALUE_AMBIGUOUS 0
#define KANGARU5_LVALUE_CONST_AMBIGUOUS 1
#endif

#if KANGARU5_IS_GNU == 1
#define KANGARU5_INLINE [[gnu::always_inline]]
#define KANGARU5_RVALUE_AMBIGUOUS 1
#define KANGARU5_LVALUE_CONST_AMBIGUOUS 0
#endif

#if KANGARU5_IS_MSVC == 1
#define KANGARU5_INLINE [[msvc::forceinline]]
#define KANGARU5_RVALUE_AMBIGUOUS 0
#define KANGARU5_LVALUE_CONST_AMBIGUOUS 1
#endif

// #define KANGARU5_DEBUG_SKIP_PRVALUE_PREPASS

#ifdef KANGARU5_DEBUG_SKIP_PRVALUE_PREPASS
#define KANGARU5_SKIP_PRVALUE_PREPASS 1
#else
#define KANGARU5_SKIP_PRVALUE_PREPASS 0
#endif

#undef KANGARU5_IS_MSVC
#undef KANGARU5_IS_GNU
#undef KANGARU5_IS_CLANG

#define KANGARU5_CONSTEVAL_IF_POSSIBLE consteval

#if __cplusplus >= 202302L
#define KANGARU5_CONSTEXPR_VOIDSTAR constexpr
#else
#define KANGARU5_CONSTEXPR_VOIDSTAR
#endif

#else
#error "A previous header seem to have leaked macros"
#endif // KANGARU5_DETAIL_DEFINE_HPP
