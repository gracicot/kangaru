#ifndef KANGARU5_DETAIL_NORETURN_HPP
#define KANGARU5_DETAIL_NORETURN_HPP

namespace kangaru::detail::noreturn {
	[[noreturn]]
	auto noreturn() -> void;
}

#endif // KANGARU5_DETAIL_NORETURN_HPP
