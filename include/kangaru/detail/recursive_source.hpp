#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "kangaru/detail/utility.hpp"
#include "source_types.hpp"
#include "constructor.hpp"
#include "injector.hpp"

#include <type_traits>

#include "define.hpp"

struct Test;
struct Camera;
struct Model;

struct Scene;

struct RecTest;


namespace kangaru::sources {
	template<typename Source>
	struct recursive_source {
		constexpr explicit recursive_source(Source source) noexcept : source{std::move(source)} {}
		
		template<detail::concepts::object T>
		struct recurse_construct {
			auto operator()(auto deduce1, auto... deduce) -> decltype(constructor<T>()(kangaru::exclude_deduction<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...)) {
				return constructor<T>()(kangaru::exclude_deduction<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
			}
		};
		
		template<detail::concepts::object T> requires (not source_of<Source, T>)
		struct provide_recursive {
			auto operator()(detail::concepts::forwarded<recursive_source> auto&& source) -> decltype(spread_injector<decltype(kangaru::ref(source))>{kangaru::ref(source)}(recurse_construct<T>{})) {
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
		
		template<detail::concepts::object T> requires (not source_of<Source, T>)
		friend constexpr auto provide(provide_tag_t<T>, detail::concepts::forwarded<recursive_source> auto&& source) -> decltype(provide_recursive<T>{}(KANGARU5_FWD(source))) {
			return provide_recursive<T>{}(KANGARU5_FWD(source));
		}
		
	private:
		Source source;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

