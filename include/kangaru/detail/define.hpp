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

#if KANGARU5_IS_MSVC == 1
#define KANGARU5_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define KANGARU5_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#define KANGARU5_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

#undef KANGARU5_IS_MSVC
#undef KANGARU5_IS_GNU

#else
#error "A previous header seem to have leaked macros"
#endif // KANGARU5_DETAIL_DEFINE_HPP
