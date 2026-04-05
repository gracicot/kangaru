#ifndef KANGARU5_DETAIL_MODULAR_CONTAINER_HPP
#define KANGARU5_DETAIL_MODULAR_CONTAINER_HPP

#include "source.hpp"
#include "source_types.hpp"
#include "utility.hpp"
#include "incremental_source.hpp"
#include "recursive_source.hpp"
#include "injector.hpp"
#include "constructor.hpp"

#ifndef KANGARU5_MODULES
#include <utility>
#include <type_traits>
#endif

#include "define.hpp"

// TODO: Make amount of parameter customizable
namespace kangaru::detail::modular_container_private {
	// TODO: Deduplicate
	template<source Source>
	using injection_source = with_recursion<with_construction<Source, exhaustive_construction>>;
	
	template<function_object Function>
	struct module_initializer {
		template<forwarded_source Source>
		constexpr auto operator()(Source&& source) && ->
			with_source_reference_wrapping<reference_source<detail::call_result_t<detail::call_result_t<make_strict_spread_injector_function, injection_source<std::remove_reference_t<Source>>>, Function>>>
		requires(
			callable<detail::call_result_t<make_strict_spread_injector_function, injection_source<std::remove_reference_t<Source>>>, Function>
		) {
			auto const injection_source = with_recursion{with_construction{KANGARU5_FWD(source), exhaustive_construction{}}};
			auto injector = KANGARU5_NO_ADL(make_strict_spread_injector)(KANGARU5_NO_ADL(ref)(injection_source));
			using type = decltype(std::move(injector)(std::move(function)));
			auto construct_source = in_place_construct{[&]() -> type { return std::move(injector)(std::move(function)); }};
			return with_source_reference_wrapping{reference_source<type>{std::move(construct_source)}};
		};
		
		Function function;
	};
	
	// This type is to workaround clang 18 and older. If those entities are defined in the private scope of a class,
	// clang will reject this code.
	template<typename...  Modules>
	struct modular_container_base {
		template<typename T, std::size_t... S>
		consteval static auto index_of(std::index_sequence<S...>) {
			return ((source_of<source_reference_wrapper<reflected_return_type<Modules, 8>>, T> ? S : 0) + ...);
		}
		
		struct module_for_type {
			template<injectable T>
			using type = std::tuple_element_t<
				index_of<T>(std::index_sequence_for<Modules...>{}),
				std::tuple<source_reference_wrapper<reflected_return_type<Modules, 8>>...>
			>;
		};
	};
} // namespace kangaru::detail::modular_container_private

KANGARU5_EXPORT namespace kangaru {
	template<function_object... Modules>
		requires(
			std::constructible_from<
				incremental_source<
					detail::modular_container_private::module_initializer<Modules>...
				>,
				detail::modular_container_private::module_initializer<Modules>...
			>
		)
	struct modular_container {
	private:
		using impl_type = incremental_source<detail::modular_container_private::module_initializer<Modules>...>;
		
		template<forwarded<modular_container> Self>
		static auto container_source(Self&& source) {
			return with_recursion{
				with_construction{
					sealed_source{
						make_source_with_provide_using_source<
							detail::modular_container_private::modular_container_base<Modules...>::module_for_type::template type
						>(
							KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).impl)
						)
					}, exhaustive_construction{}
				}
			};
		}
		
		template<forwarded<modular_container> Self>
		using container_source_t = decltype(container_source(std::declval<Self>()));
		
	public:
		explicit(sizeof...(Modules) == 1) constexpr modular_container(Modules... modules) :
			impl{detail::modular_container_private::module_initializer<Modules>{std::move(modules)}...} {}
		
		template<injectable T, forwarded<modular_container> Self> requires(source_of<container_source_t<Self>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(container_source(KANGARU5_FWD(source)));
		}
		
	private:
		impl_type impl;
	};
	
	template<typename... Modules> requires((... and (std::is_function_v<Modules> or function_object<Modules>)))
	using module_dependencies = tied_source<reflected_return_type<std::decay_t<Modules>, 8>...>;
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_MODULAR_CONTAINER_HPP
