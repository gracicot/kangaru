#ifndef KANGARU5_DETAIL_LAZY_HPP
#define KANGARU5_DETAIL_LAZY_HPP

#include "deducer.hpp"
#include "source.hpp"
#include "optional.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#endif

#include "define.hpp"

KANGARU5_EXPORT namespace kangaru {
	template<injectable T, source_of<T> Source>
	struct lazy {
		template<allows_construction_of<Source> S>
		explicit constexpr lazy(S&& source) noexcept : source{KANGARU5_FWD(source)} {}
		
		constexpr auto operator*() & -> T {
			ensure_initialized();
			return static_cast<T>(*object);
		}
		
		constexpr auto operator*() && -> T {
			ensure_initialized();
			return static_cast<T>(*std::move(object));
		}
		
		constexpr auto operator->() -> std::remove_reference_t<T>* {
			ensure_initialized();
			return object.operator->();
		}
		
		template<typename U> requires std::convertible_to<U&&, T&>
		constexpr auto object_or(U&& default_value) const noexcept -> T& {
			return object.has_value() ? static_cast<T>(*object) : KANGARU5_FWD(default_value);
		}
		
	private:
		KANGARU5_INLINE constexpr auto ensure_initialized() -> void {
			if (not object) {
				object.emplace(kangaru::provide<T>(source));
			}
		}
		
		using as_contained = detail::conditional_t<std::is_rvalue_reference_v<T>, T&, T>;
		
		Source source;
		optional<as_contained> object;
	};
	
	template<source Source, injectable Type>
	struct with_lazy_evaluation_of {
		template<allows_construction_of<Source> S>
		explicit constexpr with_lazy_evaluation_of(S&& source) noexcept : source{KANGARU5_FWD(source)} {}
		
		template<forwarded<with_lazy_evaluation_of> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> Type {
			return *KANGARU5_FWD(source).source;
		}
		
	private:
		lazy<Type, Source> source;
	};
	
	template<injectable Type, forwarded_source Source>
	inline constexpr auto make_source_with_lazy_evaluation_of(Source&& source) {
		return with_lazy_evaluation_of<deduced_source_type<Source>, Type>{KANGARU5_FWD(source)};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_LAZY_HPP
