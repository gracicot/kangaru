#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "kangaru/detail/utility.hpp"
#include "source_types.hpp"
#include "constructor.hpp"
#include "injector.hpp"

#include <type_traits>

#include "define.hpp"

namespace kangaru::sources {
	template<typename Source>
	struct recursive_source {
		constexpr explicit recursive_source(Source source) noexcept : source{std::move(source)} {}
		
		/* template<typename T>
		struct recurse_construct {
			auto operator()(auto deduce1, auto... deduce) -> decltype(constructor<T>()(kangaru::exclude_deduction<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...)) {
				return constructor<T>()(kangaru::exclude_deduction<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
		};
		
		template<typename T> requires (not source_of<Source, T>)
		struct provide_recursive {
			auto operator()(detail::concepts::forwarded<recursive_source> auto&& source) -> decltype(auto) {
				auto self = ref(source);
				auto injector = spread_injector<decltype(self)>{self};
				auto make = recurse_construct<T>{};
				return injector(make);
			}
		};
		
		template<typename T> requires source_of<Source, T>
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<recursive_source> auto&& source) -> T {
			return kangaru::provide(provide_tag<T>, KANGARU5_FWD(source).source);
		}
		
		template<typename T> requires detail::concepts::object<T>
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<recursive_source> auto&& source) -> T {
			auto recurse = provide_recursive<T>{};
			return recurse(KANGARU5_FWD(source));
		} */
		
		template<typename T>
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<recursive_source> auto&& source) -> T {
			if constexpr (source_of<Source, T>) {
				return kangaru::provide(provide_tag<T>, KANGARU5_FWD(source).source);
			} else if constexpr(not std::is_reference_v<T>) {
				auto self = ref(source);
				auto injector = spread_injector<std::remove_cvref_t<decltype(self)>>{self};
				auto make = constructor<T>();
				auto lambda = [make](auto deduce1, auto... deduce) -> decltype(make(kangaru::exclude_deduction<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...)) { return make(kangaru::exclude_deduction<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...); };
				return injector(lambda);
			} else {
				throw 0;
			}
		}
		
	private:
		Source source;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

