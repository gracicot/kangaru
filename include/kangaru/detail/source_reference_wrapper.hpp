#ifndef KANGARU5_DETAIL_SOURCE_REFERENCE_WRAPPER_HPP
#define KANGARU5_DETAIL_SOURCE_REFERENCE_WRAPPER_HPP

#include "source.hpp"
#include "concepts.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <memory>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<typename T>
	concept reference_wrapper = source<T> and requires(T ref) {
		{ ref.unwrap() } -> reference;
	};
	
	KANGARU5_EXPORT template<object Source> requires (source<std::remove_const_t<Source>> and not reference_wrapper<std::remove_const_t<Source>>)
	struct source_reference_wrapper {
		explicit constexpr source_reference_wrapper(Source& source) noexcept : source{std::addressof(source)} {}
		
		template<injectable T> requires source_of<Source&, T>
		constexpr auto provide() const& -> T {
			return kangaru::provide<T>(*source);
		}
		
		[[nodiscard]]
		constexpr auto unwrap() const noexcept -> Source& {
			return *source;
		}
		
	private:
		Source* source;
	};
	
	// TODO: It seems unsafe to expose rvalues through an lvalue. Is there a way to fix usage?
	KANGARU5_EXPORT template<source_ref Source> requires (not reference_wrapper<std::remove_cvref_t<Source>>)
	struct source_forwarding_reference_wrapper {
		explicit constexpr source_forwarding_reference_wrapper(Source source) noexcept : source{std::addressof(source)} {}
		
		template<injectable T> requires source_of<Source, T>
		constexpr auto provide() const& -> T {
			return kangaru::provide<T>(detail::utility::forward_like<Source>(*source));
		}
		
		[[nodiscard]]
		constexpr auto unwrap() const& noexcept -> Source {
			return detail::utility::forward_like<Source>(*source);
		}
		
	private:
		std::remove_reference_t<Source>* source;
	};
	
	KANGARU5_EXPORT template<forwarded_source Source>
	source_forwarding_reference_wrapper(Source&&) -> source_forwarding_reference_wrapper<Source&&>;
	
	KANGARU5_EXPORT template<typename T>
	concept forwarded_reference_wrapper = reference_wrapper<std::remove_reference_t<T>>;
	
	KANGARU5_EXPORT template<reference_wrapper Wrapper>
	using source_reference_wrapped_type = std::remove_reference_t<decltype(std::declval<Wrapper>().unwrap())>;
	
	KANGARU5_EXPORT inline constexpr auto maybe_unwrap(forwarded_reference_wrapper auto&& ref) noexcept -> decltype(auto) {
		return KANGARU5_FWD(ref).unwrap();
	}
	
	KANGARU5_EXPORT inline constexpr auto maybe_unwrap(auto&& any) -> decltype(any) {
		return KANGARU5_FWD(any);
	}
	
	KANGARU5_EXPORT template<typename MaybeWrapper>
	using maybe_unwrap_result_t = decltype(KANGARU5_NO_ADL(maybe_unwrap)(std::declval<MaybeWrapper>()));
	
	KANGARU5_EXPORT template<source Source> requires (not reference_wrapper<std::remove_cvref_t<Source>>)
	inline constexpr auto ref(Source& source) -> source_reference_wrapper<Source> {
		return source_reference_wrapper<Source>{source};
	}
	
	KANGARU5_EXPORT template<source Source> requires (not reference_wrapper<Source>)
	inline constexpr auto ref(source_reference_wrapper<Source> source) -> source_reference_wrapper<Source> {
		return source;
	}
	
	KANGARU5_EXPORT template<source_ref Source> requires (not reference_wrapper<std::remove_cvref_t<Source>>)
	inline constexpr auto ref(source_forwarding_reference_wrapper<Source> source) -> source_reference_wrapper<std::remove_reference_t<Source>> {
		return source_reference_wrapper<std::remove_reference_t<Source>>{source.unwrap()};
	}
	
	KANGARU5_EXPORT template<forwarded_source Source> requires (not forwarded_reference_wrapper<Source>)
	inline constexpr auto fwd_ref(Source&& source) -> source_forwarding_reference_wrapper<Source&&> {
		return source_forwarding_reference_wrapper<Source&&>{KANGARU5_FWD(source)};
	}
	
	KANGARU5_EXPORT template<forwarded_source Source> requires (not reference_wrapper<std::remove_cvref_t<Source>>)
	inline constexpr auto fwd_ref(source_forwarding_reference_wrapper<Source> source) -> source_forwarding_reference_wrapper<Source&&> {
		return source;
	}
	
	KANGARU5_EXPORT template<forwarded_source Source> requires (not reference_wrapper<std::remove_cvref_t<Source>>)
	inline constexpr auto fwd_ref(source_reference_wrapper<Source> source) -> source_forwarding_reference_wrapper<Source&> {
		return source_forwarding_reference_wrapper<Source&>{source.unwrap()};
	}
	
	KANGARU5_EXPORT template<typename Source> requires(source_ref<Source> or reference_wrapper<Source>)
	using ref_result_t = decltype(KANGARU5_NO_ADL(ref)(std::declval<Source>()));
	
	KANGARU5_EXPORT template<typename Source> requires(source_ref<Source> or reference_wrapper<Source>)
	using fwd_ref_result_t = decltype(KANGARU5_NO_ADL(fwd_ref)(std::declval<Source>()));
	
	KANGARU5_EXPORT template<source Source>
	struct with_source_reference_wrapping {
		Source source;
		
		template<reference_wrapper T, forwarded<with_source_reference_wrapping> Self>
			requires(
				    std::same_as<ref_result_t<T&>, T>
				and wrapping_source_of<Self, decltype(std::declval<T>().unwrap())>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return KANGARU5_NO_ADL(ref)(kangaru::provide<decltype(std::declval<T>().unwrap())>(KANGARU5_FWD(source).source));
		}
	};
	
	KANGARU5_EXPORT template<typename Source>
	with_source_reference_wrapping(Source const&) -> with_source_reference_wrapping<Source>;
	
	KANGARU5_EXPORT template<source Source>
	struct with_source_forwarding_reference_wrapping {
		Source source;
		
		template<reference_wrapper T, forwarded<with_source_forwarding_reference_wrapping> Self>
			requires(
				    std::same_as<fwd_ref_result_t<T&&>, T>
				and wrapping_source_of<Self, decltype(std::declval<T>().unwrap())&&>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return KANGARU5_NO_ADL(fwd_ref)(kangaru::provide<decltype(std::declval<T>().unwrap())&&>(KANGARU5_FWD(source).source));
		}
	};
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_REFERENCE_WRAPPER_HPP
