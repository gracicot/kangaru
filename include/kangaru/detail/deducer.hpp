#ifndef KANGARU_DETAIL_DEDUCER_HPP
#define KANGARU_DETAIL_DEDUCER_HPP

#include "concepts.hpp"
#include "source.hpp"

#include <utility>
#include <type_traits>

#include "define.hpp"

struct Scene;
struct Camera;

namespace kangaru {
	template<typename Source>
	struct deducer;
	
	template<typename Exclude, typename Deducer>
	struct exclude_deducer;
	
	template<typename Exclude, typename Deducer>
	struct exclude_special_constructors_deducer;
	
	namespace detail::deducer {
		template<typename Deducer, typename T>
		concept deducer_for = requires(Deducer deducer) {
			{ deducer.operator T() } -> std::same_as<T>;
		};
		
		template<typename>
		inline constexpr auto is_any_deducer_v = false;
		
		template<typename Source>
		inline constexpr auto is_any_deducer_v<kangaru::deducer<Source>> = true;
		
		template<typename Exclude, typename Deducer>
		inline constexpr auto is_any_deducer_v<kangaru::exclude_deducer<Exclude, Deducer>> = true;
		
		template<typename Exclude, typename Deducer>
		inline constexpr auto is_any_deducer_v<kangaru::exclude_special_constructors_deducer<Exclude, Deducer>> = true;
		
		// Deducible prvalue cannot deduce a const prvalue
		template<typename T, typename Source>
		concept deducible =
			    detail::concepts::object<T>
			and not std::is_const_v<T>
			and not is_any_deducer_v<std::remove_cv_t<T>>
			and kangaru::source_of<Source, T>;
		
		template<typename T, typename Source>
		concept deducible_lvalue =
			    detail::concepts::object<T>
			and not is_any_deducer_v<std::remove_cv_t<T>>
			and kangaru::source_of<Source, T&>;

		template<typename T, typename Source>
		concept deducible_rvalue =
			    detail::concepts::object<T>
			and not is_any_deducer_v<std::remove_cv_t<T>>
			and kangaru::source_of<Source, T&&>;
	}

	template<typename Source>
	struct deducer {
		explicit constexpr deducer(detail::concepts::forwarded<std::remove_cvref_t<Source>> auto&& source) noexcept :
			source{std::addressof(source)} {}
		
		template<typename T>
		constexpr operator T() const requires detail::deducer::deducible<T, Source> {
			return provide(provide_tag<T>, static_cast<Source&&>(*source));
		}

		template<typename T>
		constexpr operator T&() requires (detail::deducer::deducible_lvalue<T, Source>) {
			return provide(provide_tag<T&>, static_cast<Source&&>(*source));
		}
		
		template<typename T>
		constexpr operator T&&() requires (detail::deducer::deducible_rvalue<T, Source>) {
			return provide(provide_tag<T&&>, static_cast<Source&&>(*source));
		}
		
	private:
		std::remove_reference_t<Source>* source;
	};
	
	template<typename Exclude, typename Deducer>
	struct exclude_deducer {
		explicit constexpr exclude_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<detail::concepts::object T>
		requires (detail::concepts::different_from<Exclude, T> and detail::deducer::deducer_for<Deducer const, T>)
		constexpr operator T() const {
			// Call with const so we can only call the prvalue conversion operator
			return std::as_const(deducer).operator T();
		}
		
		template<detail::concepts::object T>
		requires (detail::concepts::different_from<Exclude, T&> and detail::deducer::deducer_for<Deducer, T&>)
		constexpr operator T&() {
			return deducer.operator T&();
		}
		
		template<detail::concepts::object T>
		requires (detail::concepts::different_from<Exclude, T&&> and detail::deducer::deducer_for<Deducer, T&&>)
		constexpr operator T&&() {
			return deducer.operator T&&();
		}
		
	private:
		Deducer deducer;
	};
	
	template<typename T>
	inline constexpr auto exclude_deduction(auto deducer) {
		return exclude_deducer<T, decltype(deducer)>{deducer};
	}

	template<typename Exclude, typename Deducer>
	struct exclude_special_constructors_deducer {
		explicit constexpr exclude_special_constructors_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<detail::concepts::object T>
		requires (detail::concepts::different_from<Exclude, T> and detail::deducer::deducer_for<Deducer const, T>)
		constexpr operator T() const {
			// Call with const so we can only call the prvalue conversion operator
			return std::as_const(deducer).operator T();
		}
		
		template<detail::concepts::object T>
		requires (detail::concepts::different_from<Exclude, std::remove_cv_t<T>> and detail::deducer::deducer_for<Deducer, T&>)
		constexpr operator T&() {
			return deducer.operator T&();
		}
		
		template<detail::concepts::object T>
		requires (detail::concepts::different_from<Exclude, std::remove_cv_t<T>> and detail::deducer::deducer_for<Deducer, T&&>)
		constexpr operator T&&() {
			return deducer.operator T&&();
		}

	private:
		Deducer deducer;
	};
	
	template<detail::concepts::object T> requires (not std::is_const_v<T>)
	inline constexpr auto exclude_special_constructors_for(auto deducer) {
		return exclude_special_constructors_deducer<T, decltype(deducer)>{deducer};
	}
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU_DETAIL_DEDUCER_HPP
