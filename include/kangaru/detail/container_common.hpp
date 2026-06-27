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
	
	// Banning source of a mapped source, a bit like provide_using source
	template<kangaru::object T>
	struct default_type_to_source_mapping<reference_source<T>> {};
	
	template<kangaru::object T>
	struct default_type_to_source_mapping<rvalue_source<T>> {};
	
	template<kangaru::object T>
	struct default_type_to_source_mapping<shared_pointer_source<T>> {};
	
	template<kangaru::object T>
	struct default_type_to_source_mapping<object_source<T>> {};
	
	template<template<typename> typename Mapping, source Source, std::size_t max, reflectable_function<max>... Functions>
	struct container_base_mapping {
	private:
		template<injectable T>
		struct mapping {};
		
		template<injectable T>
			requires(
				    requires{ typename Mapping<T>; }
				and source_of<Source, Mapping<T>>
				and not allow_runtime_caching_v<T>
				and not requires{ typename select_source_of<T, reflected_return_type<Functions, max>...>; }
			)
		struct mapping<T> {
			using type = Mapping<T>;
		};
		
		template<injectable T>
			requires(
				    requires{ typename Mapping<T>; }
				and allow_runtime_caching_v<T>
				and not requires{ typename select_source_of<T, reflected_return_type<Functions, max>...>; }
			)
		struct mapping<T> {
			using type = Mapping<T>;
		};
		
		template<injectable T>
			requires(
				requires{ typename select_source_of<T, reflected_return_type<Functions, max>...>; }
			)
		struct mapping<T> {
			using type = select_source_of<T, reflected_return_type<Functions, max>...>;
		};
		
	public:
		template<injectable T>
		using source_for = typename mapping<T>::type;
	};
}

KANGARU5_EXPORT namespace kangaru {
	template<injectable T>
	using default_source_mapping = typename detail::container_common_private::default_type_to_source_mapping<T>::type;
	
	template<injectable T> requires(allow_runtime_caching_v<T>)
	using default_source_mapping_runtime_cached = default_source_mapping<T>;
	
	template<template<typename> typename Mapping>
	struct source_mapping_with_reference {
		template<injectable T>
		using source_for = Mapping<T>&;
	};
	
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

	template<typename Source, template<typename> typename Mapping>
	struct not_found_with_provide_mapped {
		template<injectable T, forwarded<not_found_with_provide_mapped> Self>
			requires(
				    wrapping_source_of<Self, T>
				and not requires {
					typename Mapping<T>;
					requires wrapping_source_of<Self, Mapping<T>>;
				}
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<template<typename> typename Mapping, forwarded_source Source>
	inline constexpr auto make_source_not_found_with_provide_mapped(Source&& source) {
		return not_found_with_provide_mapped<deduced_source_type<Source>, Mapping>{KANGARU5_FWD(source)};
	}
	
	template<source Source, construction Construction, template<typename> typename Mapping>
	struct container_base {
		Source source;
		Construction construction;
		
		template<injectable T>
		using source_for = Mapping<T>;
	};
}

namespace kangaru::detail::container_common_private {
	template<template<typename> typename Mapping, std::size_t max, typename... Lambdas, forwarded_source Source, construction Construction>
	auto make_container_base_impl(Source&& source, Construction const& construction) {
		using mapping = detail::container_common_private::container_base_mapping<Mapping, deduced_source_type<Source>, max, Lambdas...>;
		return container_base<deduced_source_type<Source>, Construction, mapping::template source_for>{
			KANGARU5_FWD(source),
			construction
		};
	}
}

KANGARU5_EXPORT namespace kangaru {
	struct allow_assume_cached_t { constexpr allow_assume_cached_t() = default; }
	inline constexpr allow_assume_cached{};
	
	template<
		std::size_t max = 8,
		template<typename> typename Mapping = default_source_mapping,
		forwarded_source Source,
		reflectable_function<max>... Lambdas
	> requires(not reflectable_function<Source, max>)
	inline constexpr auto make_container_base(Source&& source, Lambdas&&... lambdas) {
		return detail::container_common_private::make_container_base_impl<Mapping, max, Lambdas...>(
			filter<reflected_return_type<Lambdas, max>...>(
				KANGARU5_FWD(source)
			),
			construct_with_alternative{
				select_function_for_type{
					call_with_injector{
						KANGARU5_FWD(lambdas),
						make_basic_spread_injector_function<basic_deducer, max>{},
					}...,
				},
				exhaustive_construction{},
			}
		);
	}
	
	template<
		std::size_t max = 8,
		template<typename> typename Mapping = default_source_mapping,
		make_injector MakeInjector,
		forwarded_source Source,
		reflectable_function<max>... Lambdas
	> requires(not reflectable_function<Source, max>)
	inline constexpr auto make_container_base(MakeInjector make_injector, Source&& source, Lambdas&&... lambdas) {
		return detail::container_common_private::make_container_base_impl<Mapping, max, Lambdas...>(
			filter<reflected_return_type<Lambdas, max>...>(
				KANGARU5_FWD(source)
			),
			construct_with_alternative{
				select_function_for_type{
					call_with_injector{
						KANGARU5_FWD(lambdas),
						make_injector,
					}...,
				},
				basic_exhaustive_construction{make_injector},
			}
		);
	}
	
	template<
		std::size_t max = 8,
		template<typename> typename Mapping = default_source_mapping,
		reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base(Lambdas&&... lambdas) {
		return detail::container_common_private::make_container_base_impl<Mapping, max, Lambdas...>(
			none_source{},
			construct_with_alternative{
				select_function_for_type{
					call_with_injector{
						KANGARU5_FWD(lambdas),
						make_basic_spread_injector<basic_deducer, max>,
					}...,
				},
				exhaustive_construction{},
			}
		);
	}
	
	template<
		std::size_t max = 8,
		template<typename> typename Mapping = default_source_mapping,
		forwarded_source IfNotFound,
		forwarded_source Source,
		reflectable_function<max>... Lambdas
	> requires(not reflectable_function<IfNotFound, max> and not reflectable_function<Source, max>)
	inline constexpr auto make_container_base(
		allow_assume_cached_t,
		IfNotFound&& source_if_not_found,
		Source&& source,
		Lambdas&&... lambdas
	) {
		return detail::container_common_private::make_container_base_impl<Mapping, max, Lambdas...>(
			filter<reflected_return_type<Lambdas, max>...>(with_alternative{
				KANGARU5_FWD(source),
				make_source_not_found_with_provide_mapped<Mapping>(KANGARU5_FWD(source_if_not_found)),
			}),
			construct_with_alternative{
				select_function_for_type{
					call_with_injector{
						KANGARU5_FWD(lambdas),
						make_basic_spread_injector<basic_deducer, max>,
					}...,
				},
				exhaustive_construction{},
			}
		);
	}
	
	template<
		std::size_t max = 8,
		template<typename> typename Mapping = default_source_mapping,
		make_injector MakeInjector,
		forwarded_source IfNotFound,
		forwarded_source Source,
		reflectable_function<max>... Lambdas
	> requires(not reflectable_function<IfNotFound, max> and not reflectable_function<Source, max>)
	inline constexpr auto make_container_base(
		allow_assume_cached_t,
		MakeInjector make_injector,
		IfNotFound&& source_if_not_found,
		Source&& source,
		Lambdas&&... lambdas
	) {
		return detail::container_common_private::make_container_base_impl<Mapping, max, Lambdas...>(
			filter<reflected_return_type<Lambdas, max>...>(with_alternative{
				KANGARU5_FWD(source),
				make_source_not_found_with_provide_mapped<Mapping>(KANGARU5_FWD(source_if_not_found))
			}),
			construct_with_alternative{
				select_function_for_type{
					call_with_injector{
						KANGARU5_FWD(lambdas),
						make_injector,
					}...,
				},
				basic_exhaustive_construction{make_injector},
			}
		);
	}
	
	template<
		std::size_t max = 8,
		template<typename> typename Mapping = default_source_mapping,
		forwarded_source IfNotFound,
		reflectable_function<max>... Lambdas
	>
	inline constexpr auto make_container_base(
		allow_assume_cached_t,
		IfNotFound&& source_if_not_found,
		Lambdas&&... lambdas
	) {
		return detail::container_common_private::make_container_base_impl<Mapping, max, Lambdas...>(
			filter<reflected_return_type<Lambdas, max>...>(
				make_source_not_found_with_provide_mapped<Mapping>(KANGARU5_FWD(source_if_not_found))
			),
			construct_with_alternative{
				select_function_for_type{
					call_with_injector{
						KANGARU5_FWD(lambdas),
						make_basic_spread_injector<basic_deducer, max>,
					}...,
				},
				exhaustive_construction{},
			}
		);
	}
	
	template<template<typename> typename Mapping = default_source_mapping>
	inline constexpr auto make_container_base(allow_assume_cached_t) {
		return detail::container_common_private::make_container_base_impl<Mapping, 0>(
			make_source_not_found_with_provide_mapped<Mapping>(throw_if_not_found{}),
			exhaustive_construction{}
		);
	}
}

#include "undef.hpp"

#endif // # KANGARU5_DETAIL_SOURCE_TYPES_HPP
