#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFINE
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFINE

#ifndef KGR_KANGARU_CXX17_NOEXCEPT
#if defined(__cpp_noexcept_function_type) || __cplusplus >= 201703L || _MSVC_LANG > 201402L
#define KGR_KANGARU_CXX17_NOEXCEPT_FPTR
#define KGR_KANGARU_CXX17_NOEXCEPT noexcept
#else
#define KGR_KANGARU_CXX17_NOEXCEPT
#endif
#endif

#ifndef KGR_KANGARU_USE_ALTERNATE_MAP_PROBE
#if ( \
	!(defined(__clang__) && __clang_major__ < 7 && !defined(__APPLE__)) && \
	!(defined(_MSC_VER) && !defined(__clang__)) && \
	!(defined(__APPLE__) && defined(__clang__) && __clang_major__ == 10 && __clang_patchlevel__ < 1) && \
	!(defined(__APPLE__) && defined(__clang__) && __clang_major__ < 10) \
)
#define KGR_KANGARU_USE_ALTERNATE_MAP_PROBE
#endif
#endif

#ifndef KGR_KANGARU_MSVC_DISABLE_VALIDATION_AUTOWIRE
#if defined(_MSC_VER) && _MSC_VER <= 1900
// MSVC 2015 cannot properly validate autowired argument to services.
// It will generate bad code and cause crashes
#define KGR_KANGARU_MSVC_DISABLE_VALIDATION_AUTOWIRE
#endif
#endif

// These preprocessor directives allow kangaru to work with exceptions disabled.
#ifndef KGR_KANGARU_THROW
#if (defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)) && !defined(KGR_KANGARU_NOEXCEPTION)
	#define KGR_KANGARU_THROW(exception) throw exception
#else
	#ifndef KGR_KANGARU_NOEXCEPTION
		#define KGR_KANGARU_NOEXCEPTION
	#endif
	#define KGR_KANGARU_THROW(exception) std::abort()
#endif
#endif

#ifndef KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
#if defined(_MSC_VER) && _MSC_VER <= 1900
#ifndef __clang__
// MSVC has a defect that makes the use of the template keyword an error in some corner cases.
#define KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
#endif // !__clang__
#endif // _MSC_VER
#endif // KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD

#ifndef KGR_KANGARU_MSVC_EXACT_DECLTYPE
#if _MSC_VER
#ifndef __clang__
// MSVC has a defect that makes decltype with the address of a
// generic lambda not possible unless sending the address to a function.
#define KGR_KANGARU_MSVC_EXACT_DECLTYPE
#endif // !__clang__
#endif // _MSC_VER
#endif // KGR_KANGARU_MSVC_EXACT_DECLTYPE

#ifndef KGR_KANGARU_EMPTY_BASES
#ifdef _MSC_VER
#define KGR_KANGARU_EMPTY_BASES __declspec(empty_bases)
#else
#define KGR_KANGARU_EMPTY_BASES
#endif // _MSC_VER
#endif // KGR_KANGARU_EMPTY_BASES

#ifndef KGR_KANGARU_FUNCTION_SIGNATURE
#if defined(_MSC_VER)
#define KGR_KANGARU_FUNCTION_SIGNATURE __FUNCSIG__
#else
#define KGR_KANGARU_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif // _MSC_VER
#endif // KGR_KANGARU_FUNCTION_SIGNATURE

#ifndef KGR_KANGARU_NONCONST_TYPEID
#ifdef _MSC_VER
#ifndef __clang__
#define KGR_KANGARU_NONCONST_TYPEID
#endif // !__clang__
#endif // _MSC_VER
#endif // KGR_KANGARU_NONCONST_TYPEID

#ifndef KGR_KANGARU_HASH_EXTENDED_CONSTEXPR
#if (defined(_MSC_VER) && _MSC_VER >= 1910) || __cplusplus >= 201402L
#define KGR_KANGARU_HASH_EXTENDED_CONSTEXPR
#endif
#endif // KGR_KANGARU_HASH_EXTENDED_CONSTEXPR

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_DEFINE
