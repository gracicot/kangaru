#ifndef KANGARU5_DETAIL_CONSTRUCTOR_HPP
#define KANGARU5_DETAIL_CONSTRUCTOR_HPP

#include "concepts.hpp"
#include "utility.hpp"

#ifndef KANGARU5_MODULES
#include <utility>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<unqualified_object Type>
	inline constexpr auto constructor() noexcept {
		auto const call_constructor = ::kangaru::detail::utility::overload{
			[](int, auto&&... args)
				noexcept(noexcept(Type(KANGARU5_FWD(args)...)))
				-> decltype(Type(KANGARU5_FWD(args)...)) {
					return Type(KANGARU5_FWD(args)...);
				},
			[](void*, auto&&... args)
				noexcept(noexcept(Type{KANGARU5_FWD(args)...}))
				-> decltype(Type{KANGARU5_FWD(args)...}) {
					return Type{KANGARU5_FWD(args)...};
				},
		};
		using constructor_t = decltype(call_constructor);
		return [call_constructor](auto&&... args)
			noexcept(noexcept(::std::declval<constructor_t>()(0, KANGARU5_FWD(args)...)))
			-> decltype(::std::declval<constructor_t>()(0, KANGARU5_FWD(args)...)) {
				return call_constructor(0, KANGARU5_FWD(args)...);
			};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONSTRUCTOR_HPP
