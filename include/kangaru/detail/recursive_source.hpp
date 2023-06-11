#ifndef KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
#define KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP

#include "utility.hpp"
#include "source_types.hpp"
#include "constructor.hpp"
#include "injector.hpp"

#include <type_traits>

#include "define.hpp"

namespace kangaru::sources {
	template<source Source, std::move_constructible Construct>
	struct with_recursion {
		constexpr explicit with_recursion(Source source) noexcept requires std::default_initializable<Construct> : construct{}, source{std::move(source)} {}
		constexpr with_recursion(Source source, Construct construct) noexcept : construct{std::move(construct)}, source{std::move(source)} {}
		
	private:
		template<typename Self>
		using injector_type = spread_injector<source_reference_wrapper<std::remove_reference_t<Self>>>;
		
		template<typename T>
		struct call_construct_function {
			Construct const* construct;
			
			constexpr auto operator()(deducer auto... deduce) const -> T requires callable_template1<Construct const&, T, decltype(deduce)...> {
				return construct->template operator()<T>(deduce...);
			}
		};
		
		template<typename T, forwarded<with_recursion> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return kangaru::provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
		
		template<typename T, forwarded<with_recursion> Self, typename S = detail::utility::forward_like_t<Self, Source>>
			requires (not source_of<S, T> and callable<injector_type<Self>, call_construct_function<T>>)
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			auto self = ref(source);
			auto injector = spread_injector<decltype(self)>{self};
			
			return injector(call_construct_function<T>{std::addressof(source.construct)});
		}
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construct construct;
		Source source;
	};
	
	struct prvalue_construction {
		template<unqualified_object T>
		constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
		requires callable<
			KANGARU5_CONSTRUCTOR_T(T),
			exclude_special_constructors_deducer<T, decltype(deduce1)>,
			exclude_deducer<T, decltype(deduce)>...
		> {
			return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
		}
	};
	
	template<source Source>
	using with_recursive_construct = with_recursion<Source, prvalue_construction>;
	
	template<typename Source> requires source<std::decay_t<Source>>
	inline constexpr auto make_source_with_recursive_construct(Source&& source) {
		return with_recursive_construct<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	struct unsafe_exhaustive_construction {
		template<unqualified_object T>
		constexpr auto operator()(deducer auto deduce1, deducer auto... deduce) const -> T
		requires callable<
			KANGARU5_CONSTRUCTOR_T(T),
			exclude_special_constructors_deducer<T, decltype(deduce1)>,
			exclude_deducer<T, decltype(deduce)>...
		> {
			return constructor<T>()(kangaru::exclude_special_constructors_for<T>(deduce1), kangaru::exclude_deduction<T>(deduce)...);
		}
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_RECURSIVE_SOURCE_HPP
