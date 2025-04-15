#ifndef KANGARU5_DETAIL_DEFINE_HPP
#define KANGARU5_DETAIL_DEFINE_HPP

// We include version to check __cpp_explicit_this_parameter
#include <version>

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
#define KANGARU5_NO_ADL(...) (__VA_ARGS__)

#if KANGARU5_IS_CLANG() == 1
	#define KANGARU5_NO_UNIQUE_ADDRESS [[no_unique_address]]
	#define KANGARU5_INLINE [[gnu::always_inline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() 0
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() 1
	#define KANGARU5_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
	
	// Helper macros to detect compiler features
	#define KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() 1
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() (__cpp_constexpr >= 202306L)
	#ifdef __cpp_explicit_this_parameter
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() (__cpp_explicit_this_parameter >= 202110L)
	#else
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() 0
	#endif
#elif KANGARU5_IS_GNU() == 1
	#define KANGARU5_NO_UNIQUE_ADDRESS [[no_unique_address]]
	#define KANGARU5_INLINE [[gnu::always_inline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() 1
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() 0
	#define KANGARU5_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() (__cpp_constexpr >= 202306L)
	
	// Helper macros to detect compiler features
	#define KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() 1
	#if defined(__cpp_explicit_this_parameter)
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() (__cpp_explicit_this_parameter >= 202110L)
	#else
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() 0
	#endif
#elif KANGARU5_IS_MSVC() == 1
	#define KANGARU5_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
	#define KANGARU5_INLINE [[msvc::forceinline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() 0
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() 1
	#define KANGARU5_FUNCTION_SIGNATURE __FUNCSIG__
	
	// Helper macros to detect compiler features
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() 0
	#define KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() 0
	#if _MSC_VER >= 1943
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() 1
	#else
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() 0
	#endif
#else
	#error "Unrecognized compiler"
#endif

#undef KANGARU5_IS_MSVC
#undef KANGARU5_IS_GNU
#undef KANGARU5_IS_CLANG

#if KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() == 1
	#define KANGARU5_CONSTEVAL_PLACEHOLDER consteval
#else
	#define KANGARU5_CONSTEVAL_PLACEHOLDER constexpr
#endif

#define KANGARU5_UNSAFE /* left undefined */
#define KANGARU5_UNSAFE_BLOCK /* left undefined */

#if KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() == 1
	#define KANGARU5_CONSTEXPR_VOIDSTAR constexpr
#else
	#define KANGARU5_CONSTEXPR_VOIDSTAR
#endif

#if KANGARU5_DEDUCING_THIS_SUPPORTED() == 1
	#define KANGARU5_PROVIDE_FUNCTION_DECL(...) auto provide(this __VA_ARGS__)
#else
	#define KANGARU5_PROVIDE_FUNCTION_DECL(...) friend auto provide(__VA_ARGS__)
#endif

#undef KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED
#undef KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED
#undef KANGARU5_DEDUCING_THIS_SUPPORTED

#else
	#error "A previous header seem to have leaked macros"
#endif // KANGARU5_DETAIL_DEFINE_HPP
