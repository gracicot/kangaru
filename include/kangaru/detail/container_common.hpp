#ifndef KANGARU5_DETAIL_CONTAINER_COMMON_HPP
#define KANGARU5_DETAIL_CONTAINER_COMMON_HPP

#include "attributes.hpp"
#include "concepts.hpp"
#include "injector.hpp"
#include "recursive_source.hpp"
#include "source_types.hpp"
#include "source.hpp"
#include "exceptions.hpp"
#include "source_traits.hpp"

#ifndef KANGARU5_MODULES
#include <cstdlib>
#include <exception>
#endif

#include "define.hpp"

namespace kangaru::detail::container_common {
	// TODO: Prevent duplicate?
	template<injectable, kangaru::source>
	struct enumerated_select_source_of {};
	
	template<injectable T, kangaru::source Source, kangaru::source... Sources>
		requires(requires { typename select_source_of<T, Sources...>; })
	struct enumerated_select_source_of<T, enumerated_source_of<Source, Sources...>> {
		using type = select_source_of<T, Sources...>;
	};
	
	template<injectable T, kangaru::source EnumeratedSource>
	using enumerated_select_source_of_t = typename enumerated_select_source_of<T, EnumeratedSource>::type;
}

KANGARU5_EXPORT namespace kangaru {
	struct throw_if_not_found {
		template<injectable T> requires(assume_runtime_cached_v<T>)
		[[noreturn]]
		constexpr auto provide() const -> T {
			throw not_found_exception{};
		}
	};
	
	struct abort_if_not_found {
		template<injectable T> requires(assume_runtime_cached_v<T>)
		[[noreturn]]
		constexpr auto provide() const -> T {
			std::abort();
		}
	};
	
	struct terminate_if_not_found {
		template<injectable T> requires(assume_runtime_cached_v<T>)
		[[noreturn]]
		constexpr auto provide() const -> T {
			std::terminate();
		}
	};
	
	struct allow_assume_cached_t { constexpr allow_assume_cached_t() = default; }
	inline constexpr allow_assume_cached{};
	
	template<
		std::size_t max = 8,
		template<typename> typename Deducer = basic_deducer,
		forwarded_source Source,
		forwarded_reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base_source(Source&& source, Lambdas&&... lambdas) {
		return enumerate_source<reflected_return_type<Lambdas, max>...>(
			with_function_call{
				KANGARU5_FWD(source),
				call_with_injector{
					lambdas,
					make_basic_spread_injector<Deducer, max>,
				}...,
			}
		);
	}
	
	template<
		std::size_t max = 8,
		template<typename> typename Deducer = basic_deducer,
		forwarded_reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base_source(Lambdas&&... lambdas) {
		return enumerate_source<reflected_return_type<Lambdas, max>...>(
			with_function_call{
				none_source{},
				call_with_injector{
					lambdas,
					make_basic_spread_injector<Deducer, max>,
				}...,
			}
		);
	}
	
	template<
		std::size_t max = 8,
		template<typename> typename Deducer = basic_deducer,
		forwarded_source IfNotFound,
		forwarded_source Source,
		forwarded_reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base_source(
		allow_assume_cached_t,
		IfNotFound&& source_if_not_found,
		Source&& source,
		Lambdas&&... lambdas
	) {
		return enumerate_source<
			reflected_return_type<Lambdas, max>...,
			std::decay_t<IfNotFound>
		>(
			with_alternative{
				with_function_call{
					KANGARU5_FWD(source),
					call_with_injector{
						lambdas,
						make_basic_spread_injector<Deducer, max>,
					}...,
				},
				kangaru::object_source{KANGARU5_FWD(source_if_not_found)},
			}
		);
	}
	
	template<
		std::size_t max = 8,
		template<typename> typename Deducer = basic_deducer,
		forwarded_source IfNotFound,
		forwarded_reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base_source(
		allow_assume_cached_t,
		IfNotFound&& source_if_not_found,
		Lambdas&&... lambdas
	) {
		if constexpr (sizeof...(Lambdas) == 0) {
			return enumerate_source(kangaru::object_source{KANGARU5_FWD(source_if_not_found)});
		} else {
			return enumerate_source<
				reflected_return_type<Lambdas, max>...,
				std::decay_t<IfNotFound>
			>(
				with_alternative{
					with_function_call{
						none_source{},
						call_with_injector{
							lambdas,
							make_basic_spread_injector<Deducer, max>,
						}...,
					},
					kangaru::object_source{KANGARU5_FWD(source_if_not_found)},
				}
			);
		}
	}
	
	inline constexpr auto make_container_base_source(allow_assume_cached_t) {
		return enumerate_source(
			kangaru::object_source{throw_if_not_found{}}
		);
	}
}

#include "undef.hpp"

#endif // # KANGARU5_DETAIL_SOURCE_TYPES_HPP
