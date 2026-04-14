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

namespace kangaru::detail::container_common_private {
	// TODO: Prevent duplicate?
	template<injectable, source>
	struct enumerated_select_source_of {};
	
	template<injectable T, source Source, source... Sources>
		requires(requires { typename select_source_of<T, Sources...>; })
	struct enumerated_select_source_of<T, enumerated_source_of<Source, Sources...>> {
		using type = select_source_of<T, Sources...>;
	};
	
	template<injectable T, source EnumeratedSource>
	using enumerated_select_source_of_t = typename enumerated_select_source_of<T, EnumeratedSource>::type;
	
	template<injectable T>
	struct default_type_to_source_mapping {};
	
	template<object T>
	struct default_type_to_source_mapping<T&> {
		using type = reference_source<T>;
	};
	
	template<object T>
	struct default_type_to_source_mapping<T&&> {
		using type = rvalue_source<T>;
	};
	
	template<kangaru::object T>
	struct default_type_to_source_mapping<std::shared_ptr<T>> {
		using type = shared_pointer_source<T>;
	};
	
	template<injectable T>
		requires(unqualified_object<T>)
	struct default_type_to_source_mapping<T> {
		using type = object_source<T>;
	};
	
	// Banned types. References to shared pointer are not allowed in the default mapping.
	template<kangaru::object T>
	struct default_type_to_source_mapping<std::shared_ptr<T>&> {};
	
	template<kangaru::object T>
	struct default_type_to_source_mapping<std::shared_ptr<T> const&> {};
	
	template<kangaru::object T>
	struct default_type_to_source_mapping<std::shared_ptr<T>&&> {};
	
	template<kangaru::object T>
	struct default_type_to_source_mapping<std::shared_ptr<T> const&&> {};
}

KANGARU5_EXPORT namespace kangaru {
	template<source Source> requires(unqualified_object<Source>)
	struct mapping_with_base_source {
		template<injectable T>
		using source_for = typename detail::container_common_private::default_type_to_source_mapping<T>::type;
	};
	
	// Unconstrained typename since we rely on enumerated_source_of for constrains
	template<typename Source, typename... Enumerated>
	struct mapping_with_base_source<enumerated_source_of<Source, Enumerated...>> {
	private:
		template<injectable T>
		struct mapping : detail::container_common_private::default_type_to_source_mapping<T> {};
		
		template<injectable T>
			requires(
				requires{ typename select_source_of<T, Enumerated...>; }
			)
		struct mapping<T> {
			using type = select_source_of<T, Enumerated...>;
		};
		
	public:
		template<injectable T>
		using source_for = typename mapping<T>::type;
	};
	
	template<injectable T> requires(allow_runtime_caching_v<T>)
	using cached_source_mapping = typename detail::container_common_private::default_type_to_source_mapping<T>::type;
	
	template<injectable T> requires(allow_runtime_caching_v<T>)
	using cached_reference_to_source_mapping = cached_source_mapping<T>&;
	
	template<source_ref Source>
	struct cached_source_mapping_using {
	private:
		template<injectable T>
		using unconstrained =
			typename mapping_with_base_source<
				std::remove_cvref_t<Source>
			>::template source_for<T>;
		
	public:
		template<injectable T>
			requires(
				   allow_runtime_caching_v<T>
				or source_of<Source, unconstrained<T>>
			)
		using source_for = unconstrained<T>;
	};
	
	template<source_ref Source, injectable T>
	using cached_source_mapping_using_t = typename cached_source_mapping_using<Source>::template source_for<T>;
	
	template<source_ref Source>
	struct cached_reference_to_source_mapping_using {
		template<injectable T>
		using source_for = cached_source_mapping_using_t<Source, T>&;
	};
	
	template<source_ref Source, injectable T>
	using cached_reference_to_source_mapping_using_t = cached_source_mapping_using_t<Source, T>&;
	
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
		reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base_source(Source&& source, Lambdas&&... lambdas) {
		return KANGARU5_NO_ADL(enumerate_source<reflected_return_type<Lambdas, max>...>)(
			with_function_call{
				KANGARU5_FWD(source),
				call_with_injector{
					KANGARU5_FWD(lambdas),
					make_basic_spread_injector<Deducer, max>,
				}...,
			}
		);
	}
	
	template<
		std::size_t max = 8,
		make_injector MakeInjector,
		forwarded_source Source,
		reflectable_function<max>... Lambdas
	> requires(not reflectable_function<Source, max>)
	inline constexpr auto make_container_base_source(MakeInjector make_injector, Source&& source, Lambdas&&... lambdas) {
		return KANGARU5_NO_ADL(enumerate_source<reflected_return_type<Lambdas, max>...>)(
			with_function_call{
				KANGARU5_FWD(source),
				call_with_injector{
					KANGARU5_FWD(lambdas),
					make_injector,
				}...,
			}
		);
	}
	
	template<
		std::size_t max = 8,
		reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base_source(Lambdas&&... lambdas) {
		return KANGARU5_NO_ADL(enumerate_source<reflected_return_type<Lambdas, max>...>)(
			with_function_call{
				none_source{},
				call_with_injector{
					KANGARU5_FWD(lambdas),
					make_basic_spread_injector<basic_deducer, max>,
				}...,
			}
		);
	}
	
	template<
		std::size_t max = 8,
		forwarded_source IfNotFound,
		forwarded_source Source,
		reflectable_function<max>... Lambdas
	> requires(not reflectable_function<IfNotFound, max> and not reflectable_function<Source, max>)
	inline constexpr auto make_container_base_source(
		allow_assume_cached_t,
		IfNotFound&& source_if_not_found,
		Source&& source,
		Lambdas&&... lambdas
	) {
		return KANGARU5_NO_ADL(enumerate_source<
			reflected_return_type<Lambdas, max>...
		>)(
			with_alternative{
				with_function_call{
					KANGARU5_FWD(source),
					call_with_injector{
						KANGARU5_FWD(lambdas),
						make_basic_spread_injector<basic_deducer, max>,
					}...,
				},
				KANGARU5_FWD(source_if_not_found),
			}
		);
	}
	
	template<
		std::size_t max = 8,
		make_injector MakeInjector,
		forwarded_source IfNotFound,
		forwarded_source Source,
		reflectable_function<max>... Lambdas
	> requires(not reflectable_function<IfNotFound, max> and not reflectable_function<Source, max>)
	inline constexpr auto make_container_base_source(
		allow_assume_cached_t,
		MakeInjector make_injector,
		IfNotFound&& source_if_not_found,
		Source&& source,
		Lambdas&&... lambdas
	) {
		return KANGARU5_NO_ADL(enumerate_source<
			reflected_return_type<Lambdas, max>...
		>)(
			with_alternative{
				with_function_call{
					KANGARU5_FWD(source),
					call_with_injector{
						KANGARU5_FWD(lambdas),
						make_injector,
					}...,
				},
				KANGARU5_FWD(source_if_not_found),
			}
		);
	}
	
	template<
		std::size_t max = 8,
		forwarded_source IfNotFound,
		reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base_source(
		allow_assume_cached_t,
		IfNotFound&& source_if_not_found,
		Lambdas&&... lambdas
	) {
		if constexpr (sizeof...(Lambdas) == 0) {
			return KANGARU5_FWD(source_if_not_found);
		} else {
			return KANGARU5_NO_ADL(enumerate_source<
				reflected_return_type<Lambdas, max>...
			>)(
				with_alternative{
					with_function_call{
						none_source{},
						call_with_injector{
							KANGARU5_FWD(lambdas),
							make_basic_spread_injector<basic_deducer, max>,
						}...,
					},
					KANGARU5_FWD(source_if_not_found),
				}
			);
		}
	}
	
	inline constexpr auto make_container_base_source(allow_assume_cached_t) {
		return throw_if_not_found{};
	}
}

#include "undef.hpp"

#endif // # KANGARU5_DETAIL_SOURCE_TYPES_HPP
