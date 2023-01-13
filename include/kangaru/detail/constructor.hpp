#ifndef KANGARU5_DETAIL_CONSTRUCOR_HPP
#define KANGARU5_DETAIL_CONSTRUCOR_HPP

#include "define.hpp"

namespace kangaru {
	template<typename Type>
	auto constructor_for() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONSTRUCOR_HPP
