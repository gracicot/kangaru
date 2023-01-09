#ifndef KANGARU_DETAIL_DEDUCER_HPP
#define KANGARU_DETAIL_DEDUCER_HPP

#include "concepts.hpp"
#include "source.hpp"

#include <utility>
#include <type_traits>

#include "define.hpp"

namespace kangaru {
	template<typename Deducer, typename T>
	concept deducer_for = requires(Deducer deducer) {
		{ deducer.operator T() } -> std::same_as<T>;
	};
	
	template<typename Source>
	struct deducer {
		explicit constexpr deducer(detail::concepts::forwarded<std::remove_cvref_t<Source>> auto&& source) noexcept :
			source{std::addressof(source)} {}
		
		template<typename T> requires source_of<Source, T>
		operator T() const& {
			return provide(provide_tag<T>, static_cast<Source&&>(*source));
		}
		
		template<typename T> requires source_of<Source, T&>
		operator T&() & {
			return provide(provide_tag<T&>, static_cast<Source&&>(*source));
		}
		
		template<typename T> requires source_of<Source, T&&>
		operator T&&() & {
			return provide(provide_tag<T&&>, static_cast<Source&&>(*source));
		}
		
		template<typename S>
		operator deducer<S>() = delete;
		
	private:
		std::remove_reference_t<Source>* source;
	};
	
	template<typename Exclude, typename Deducer>
	struct exclude_deducer {
		explicit constexpr exclude_deducer(Deducer deducer) noexcept :
			deducer{deducer} {}
		
		template<typename T>
		requires (detail::concepts::different_from<Exclude, T> and deducer_for<Deducer, T>)
		operator T() const& {
			return deducer.operator T();
		}
		
		template<typename T>
		requires (detail::concepts::different_from<Exclude, T&> and deducer_for<Deducer, T&>)
		operator T&() & {
			return deducer.operator T&();
		}
		
		template<typename T>
		requires (detail::concepts::different_from<Exclude, T&&> and deducer_for<Deducer, T&&>)
		operator T&&() & {
			return deducer.operator T&&();
		}
		
	private:
		Deducer deducer;
	};

	template<typename T>
	auto exclude_deduction(auto deducer) {
		return exclude_deducer<T, decltype(deducer)>{deducer};
	}
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU_DETAIL_DEDUCER_HPP
