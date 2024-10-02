#ifndef KANGARU5_DETAIL_DEFINE_HPP
#define KANGARU5_DETAIL_DEFINE_HPP

#if defined(_MSC_VER) && !defined(__clang__)
	#define KANGARU5_IS_MSVC() 1
#else
	#define KANGARU5_IS_MSVC() 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
	#define KANGARU5_IS_GNU() 1
#else
	#define KANGARU5_IS_GNU() 0
#endif

#if defined(__clang__)
	#define KANGARU5_IS_CLANG() 1
#else
	#define KANGARU5_IS_CLANG() 0
#endif

#define KANGARU5_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#define KANGARU5_CONSTRUCTOR_T(...) decltype(::kangaru::constructor<__VA_ARGS__>())

#if KANGARU5_IS_CLANG() == 1
	#define KANGARU5_NO_UNIQUE_ADDRESS [[no_unique_address]]
	#define KANGARU5_INLINE [[gnu::always_inline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() 0
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() 1
	#define KANGARU5_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

#if KANGARU5_IS_GNU() == 1
	#define KANGARU5_NO_UNIQUE_ADDRESS [[no_unique_address]]
	#define KANGARU5_INLINE [[gnu::always_inline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() 1
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() 0
	#define KANGARU5_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

#if KANGARU5_IS_MSVC() == 1
	#define KANGARU5_FUNCTION_SIGNATURE __FUNCSIG__
	#define KANGARU5_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
	#define KANGARU5_INLINE [[msvc::forceinline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() 0
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() 1
#endif

#if KANGARU5_IS_CLANG() == 1
	#if __clang_major__ < 14
		#define KANGARU5_CONSTEVAL_SUPPORTED() 0
	#else
		#define KANGARU5_CONSTEVAL_SUPPORTED() 1
	#endif
#else
	#define KANGARU5_CONSTEVAL_SUPPORTED() 1
#endif

#if KANGARU5_CONSTEVAL_SUPPORTED() == 1 && KANGARU5_IS_MSVC() == 1
	#define KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() 0
#else
	#define KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() KANGARU5_CONSTEVAL_SUPPORTED()
#endif

#if KANGARU5_IS_CLANG() == 1
	#if __clang_major__ >= 17
		#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() 1
	#else
		#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() 0
	#endif
#elif KANGARU5_IS_GNU() == 1
	#if __GNUC__ >= 14
		#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() 1
	#else
		#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() 0
	#endif
#elif KANGARU5_IS_MSVC() == 1
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() 0
#elif __cpp_constexpr >= 202306L // C++26 constexpr
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() 1
#else
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() 0
#endif

#undef KANGARU5_IS_MSVC
#undef KANGARU5_IS_GNU
#undef KANGARU5_IS_CLANG

#if KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() == 1
	#define KANGARU5_CONSTEVAL_PLACEHOLDER consteval
#else
	#define KANGARU5_CONSTEVAL_PLACEHOLDER constexpr
#endif

#if KANGARU5_CONSTEVAL_SUPPORTED() == 1
	#define KANGARU5_CONSTEVAL consteval
#else
	#define KANGARU5_CONSTEVAL constexpr
#endif

#define KANGARU5_UNSAFE /* left undefined */
#define KANGARU5_UNSAFE_BLOCK /* left undefined */

#if KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() == 1
	#define KANGARU5_CONSTEXPR_VOIDSTAR constexpr
#else
	#define KANGARU5_CONSTEXPR_VOIDSTAR
#endif

#undef KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED
#undef KANGARU5_CONSTEVAL_SUPPORTED
#undef KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED

#else
	#error "A previous header seem to have leaked macros"
#endif // KANGARU5_DETAIL_DEFINE_HPP
