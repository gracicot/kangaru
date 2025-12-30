#ifndef KANGARU5_DETAIL_LAZY_HPP
#define KANGARU5_DETAIL_LAZY_HPP

#include "source.hpp"
#include "optional.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<injectable T, source_of<T> Source>
	struct lazy {
		explicit constexpr lazy(Source source) noexcept : source{std::move(source)} {}
		
		constexpr auto operator*() & -> T {
			ensure_initialized();
			return *object;
		}
		
		constexpr auto operator*() && -> T {
			ensure_initialized();
			return *std::move(object);
		}
		
		constexpr auto operator->() -> std::remove_reference_t<T>* {
			ensure_initialized();
			return object.operator->();
		}
		
		template<typename U> requires std::convertible_to<U&&, T&>
		constexpr auto object_or(U&& default_value) const noexcept -> T& {
			return object.has_value() ? *object : KANGARU5_FWD(default_value);
		}
		
	private:
		KANGARU5_INLINE constexpr auto ensure_initialized() -> void {
			if (not object) {
				object.emplace(kangaru::provide<T>(source));
			}
		}
		
		Source source;
		optional<T> object;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_LAZY_HPP
