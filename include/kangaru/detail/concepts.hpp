#ifndef KANGARU5_DETAIL_CONCEPTS_HPP
#define KANGARU5_DETAIL_CONCEPTS_HPP

#include <concepts>
#include <type_traits>

namespace kangaru::detail::concepts {
	template<typename T, typename U>
	concept different_from = not std::same_as<T, U>;

	template<typename Forwarded, typename T>
	concept forwarded = std::same_as<T, std::remove_cvref_t<Forwarded>>;

	template<typename T>
	concept object = std::is_object_v<T>;
}

#endif // KANGARU5_DETAIL_CONCEPTS_HPP
