#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "utility.hpp"
#include "source_types.hpp"
#include "constructor.hpp"
#include "injector.hpp"

#include <type_traits>

#include "define.hpp"

namespace kangaru::sources {
	template<source Source>
	struct recursive_source {
		constexpr explicit recursive_source(Source source) noexcept : source{std::move(source)} {}
		
		template<detail::concepts::object T>
		struct recurse_construct {
			auto operator()(deducer auto deduce1, deducer auto... deduce) -> decltype(constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...)) {
				return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
		};
		
		template<detail::concepts::object T, typename S> requires (not source_of<S, T>)
		struct provide_recursive {
			auto operator()(detail::concepts::forwarded<recursive_source> auto&& source) -> decltype(spread_injector<decltype(kangaru::ref(source))>{kangaru::ref(source)}(recurse_construct<T>{})) {
				auto self = ref(source);
				auto injector = spread_injector<decltype(self)>{self};
				auto make = recurse_construct<T>{};
				return injector(make);
			}
		};
		
		template<typename T, detail::concepts::forwarded<recursive_source> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag_t<T>, Self&& source) -> T {
			return kangaru::provide(provide_tag<T>, KANGARU5_FWD(source).source);
		}
		
		template<detail::concepts::object T, detail::concepts::forwarded<recursive_source> Self, typename S = detail::utility::forward_like_t<Self, Source>>
			requires (not source_of<S, T>)
		friend constexpr auto provide(provide_tag_t<T>, Self&& source) -> decltype(provide_recursive<T, S>{}(KANGARU5_FWD(source))) {
			return provide_recursive<T, S>{}(KANGARU5_FWD(source));
		}
		
	private:
		Source source;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

