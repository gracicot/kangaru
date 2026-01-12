#ifndef KANGARU5_DETAIL_DEFINE_HPP
#define KANGARU5_DETAIL_DEFINE_HPP

// We include version to check __cpp_explicit_this_parameter
#ifndef KANGARU5_MODULES
#include <version>
#endif

#ifdef KANGARU5_MODULES
	#define KANGARU5_EXPORT export
#else
	#define KANGARU5_EXPORT
#endif

#if defined(_MSC_VER) && !defined(__clang__)
	#define KANGARU5_IS_MSVC() true
#else
	#define KANGARU5_IS_MSVC() false
#endif

#if defined(__GNUC__) && !defined(__clang__)
	#define KANGARU5_IS_GNU() true
#else
	#define KANGARU5_IS_GNU() false
#endif

#if defined(__clang__)
	#define KANGARU5_IS_CLANG() true
#else
	#define KANGARU5_IS_CLANG() false
#endif

#define KANGARU5_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#define KANGARU5_NO_ADL(...) (__VA_ARGS__)

#if KANGARU5_IS_CLANG()
	#define KANGARU5_NO_UNIQUE_ADDRESS [[no_unique_address]]
	#define KANGARU5_INLINE [[clang::always_inline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() false
	#define KANGARU5_RVALUE_CONST_AMBIGUOUS() false
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() true
	#define KANGARU5_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
	#define KANGARU5_VOLATILE_PRVALUE_DETECTION_NEEDED() false
	
	// Helper macros to detect compiler features
	#define KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() true
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() (__cpp_constexpr >= 202306L)
	
	#if __cplusplus >= 202302L
		#if defined(__apple_build_version__)
			#define KANGARU5_DEDUCING_THIS_SUPPORTED() (__apple_build_version__ >= 17000013)
		#else
			#define KANGARU5_DEDUCING_THIS_SUPPORTED() (__clang_major__ >= 18)
		#endif
	#else
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() false
	#endif
#elif KANGARU5_IS_GNU()
	#define KANGARU5_NO_UNIQUE_ADDRESS [[no_unique_address]]
	#define KANGARU5_INLINE [[gnu::always_inline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() true
	#define KANGARU5_RVALUE_CONST_AMBIGUOUS() true
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() false
	#define KANGARU5_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() (__cpp_constexpr >= 202306L)
	#define KANGARU5_VOLATILE_PRVALUE_DETECTION_NEEDED() false
	
	// Helper macros to detect compiler features
	#define KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() true
	#if defined(__cpp_explicit_this_parameter)
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() (__cpp_explicit_this_parameter >= 202110L)
	#else
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() false
	#endif
#elif KANGARU5_IS_MSVC()
	// Use msvc::no_unique_address when feedback item 11026048 is fixed
	#define KANGARU5_NO_UNIQUE_ADDRESS
	#define KANGARU5_INLINE [[msvc::forceinline]]
	#define KANGARU5_RVALUE_AMBIGUOUS() false
	#define KANGARU5_RVALUE_CONST_AMBIGUOUS() false
	#define KANGARU5_LVALUE_CONST_AMBIGUOUS() true
	#define KANGARU5_FUNCTION_SIGNATURE __FUNCSIG__
	#define KANGARU5_VOLATILE_PRVALUE_DETECTION_NEEDED() true
	
	// Helper macros to detect compiler features
	#define KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED() false
	#define KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED() false
	#if _MSC_VER >= 1943 && _MSVC_LANG >= 202302L
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() true
	#else
		#define KANGARU5_DEDUCING_THIS_SUPPORTED() false
	#endif
#else
	#error "Unrecognized compiler"
#endif

#undef KANGARU5_IS_MSVC
#undef KANGARU5_IS_GNU
#undef KANGARU5_IS_CLANG

#if KANGARU5_VOLATILE_PRVALUE_DETECTION_NEEDED()
	#define KANGARU5_VOLATILE_PRVALUE_DETECTION volatile
	#define KANGARU5_VOLATILE_OVERLOAD(...) __VA_ARGS__
	#define KANGARU5_AMBIGUOUS_BASED_PRVALUE_DETECTION() false
#else
	#define KANGARU5_VOLATILE_PRVALUE_DETECTION
	#define KANGARU5_VOLATILE_OVERLOAD(...)
	#define KANGARU5_AMBIGUOUS_BASED_PRVALUE_DETECTION() true
#endif

#if KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED()
	#define KANGARU5_CONSTEVAL_PLACEHOLDER consteval
#else
	#define KANGARU5_CONSTEVAL_PLACEHOLDER constexpr
#endif

#define KANGARU5_UNSAFE /* left undefined */
#define KANGARU5_UNSAFE_BLOCK /* left undefined */

#if KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED()
	#define KANGARU5_CONSTEXPR_VOIDSTAR constexpr
#else
	#define KANGARU5_CONSTEXPR_VOIDSTAR
#endif

#if KANGARU5_DEDUCING_THIS_SUPPORTED()
	#define KANGARU5_PROVIDE_FUNCTION_FRIEND
	#define KANGARU5_PROVIDE_FUNCTION_THIS this
#else
	#define KANGARU5_PROVIDE_FUNCTION_FRIEND friend
	#define KANGARU5_PROVIDE_FUNCTION_THIS
#endif

#undef KANGARU5_CONSTEVAL_PLACEHOLDER_SUPPORTED
#undef KANGARU5_CONSTEXPR_VOIDSTAR_CAST_SUPPORTED
#undef KANGARU5_DEDUCING_THIS_SUPPORTED
#undef KANGARU5_VOLATILE_PRVALUE_DETECTION_NEEDED

#else
	#error "A previous header seem to have leaked macros"
#endif // KANGARU5_DETAIL_DEFINE_HPP
