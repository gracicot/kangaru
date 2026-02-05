#ifndef KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP

#include "kangaru/detail/concepts.hpp"
#include "kangaru/detail/source_reference_wrapper.hpp"
#include "utility.hpp"
#include "source_types.hpp"
#include "cache_types.hpp"
#include "recursive_source.hpp"
#include "polymorphic_source.hpp"
#include "cache.hpp"
#include "heap_storage.hpp"

#ifndef KANGARU5_MODULES
#include <unordered_map>
#include <concepts>
#include <memory>
#include <utility>
#endif

#include "define.hpp"

namespace kangaru {
	namespace detail::polymorphic_container {
		template<injectable T, kangaru::source Source, typename... Functions, std::size_t... S>
		constexpr auto index_for_source_of(std::index_sequence<S...>) -> std::size_t {
			return ((callable_returns_source_of<T, Functions&, ref_result_t<Source&>> ? S : 0) + ...);
		}
	}
	
	// TODO: Properly enforce const and/or forwarding
	template<kangaru::source Source, kangaru::function_object... Functions>
	struct container_provided_sources {
		explicit(sizeof...(Functions) == 0) constexpr container_provided_sources(Source source, Functions... functions) :
			source{std::move(source)}, functions{std::move(functions)...} {}
		
		template<injectable T, forwarded<container_provided_sources> Self> requires(wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<source T, forwarded<container_provided_sources> Self> requires(((callable_returns<T, Functions&, ref_result_t<Source&>> ? 1 : 0) + ... + 0) == 1)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			constexpr auto index = index_for<T>(std::index_sequence_for<Functions...>{});
			auto& function = std::get<index>(source.functions);
			return function(KANGARU5_NO_ADL(ref)(source.source));
		}
		
		template<source T, forwarded<container_provided_sources> Self> requires("Ambiguous source resolution: One or more callable returns source T",
			((callable_returns<T, Functions&, ref_result_t<Source&>> ? 1 : 0) + ... + 0) > 1
		)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T = delete;
		
		template<forwarded<container_provided_sources> Original, forwarded_source NewLeaf>
			requires(
				std::constructible_from<
					std::tuple<Functions...>,
					detail::utility::forward_like_t<Original, std::tuple<Functions...>>
				>
			)
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept ->
			container_provided_sources<wrapped_source_rebind_result_t<Original, NewLeaf>, Functions...>
		{
			return std::apply(
				[&](auto&&... functions) {
					return container_provided_sources<wrapped_source_rebind_result_t<Original, NewLeaf>, Functions...> {
						kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
						KANGARU5_FWD(functions)...
					};
				},
				KANGARU5_FWD(original).functions
			);
		}
		
		Source source;
		
	private:
		template<kangaru::source T, std::size_t... S>
		static constexpr auto index_for(std::index_sequence<S...>) -> std::size_t {
			return ((callable_returns<T, Functions&, ref_result_t<Source&>> ? S : 0) + ...);
		}
		
		std::tuple<Functions...> functions;
		
	public:
		template<injectable T> requires(((callable_returns_source_of<T, Functions&, ref_result_t<Source&>> ? 1 : 0) + ... + 0) == 1)
		using type = detail::type_traits::call_result_t<std::tuple_element_t<detail::polymorphic_container::index_for_source_of<T, Source, Functions...>(std::index_sequence_for<Functions...>{}), std::tuple<Functions...>>, ref_result_t<Source&>>;
	};
	
	template<source Source, template<typename> typename FallbackSource, function_object MakeInjector>
	struct with_fallback_provided_sources {
		explicit constexpr with_fallback_provided_sources(Source source)
			requires(std::default_initializable<MakeInjector>) : source{std::move(source)} {}
		
		constexpr with_fallback_provided_sources(Source source, MakeInjector make_injector) :
			source{std::move(source)}, make_injector{std::move(make_injector)} {}
		
		Source source;
		
		template<injectable T, forwarded<with_fallback_provided_sources> Self> requires source_of<wrapped_source_t<Self>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<kangaru::source T, forwarded<with_fallback_provided_sources> Self>
			requires(
				    not source_of<wrapped_source_t<Self>, T>
				and detail::utility::is_specialisation_of_v<FallbackSource, T>
				and callable<detail::type_traits::call_result_t<MakeInjector, fwd_ref_result_t<forwarded_wrapped_source_t<Self>>>, constructor_function<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return source.make_injector(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source))(constructor_function<T>{});
		}
		
		template<forwarded<with_fallback_provided_sources> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) noexcept ->
			with_fallback_provided_sources<wrapped_source_rebind_result_t<Original, NewLeaf>, FallbackSource, MakeInjector>
		{
			return with_fallback_provided_sources<wrapped_source_rebind_result_t<Original, NewLeaf>, FallbackSource, MakeInjector>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_FWD(original).make_injector
			};
		}
		
	private:
		template<injectable T>
		struct ttype {};
		
		template<injectable T>
			requires(
				    not requires{ typename Source::template type<T>; }
				and assume_runtime_cached_v<T>
				and requires{ typename FallbackSource<T>; }
				and callable<detail::type_traits::call_result_t<MakeInjector, ref_result_t<Source&>>, constructor_function<FallbackSource<T>>>
			)
		struct ttype<T> {
			using type = FallbackSource<T>;
		};
		
		template<injectable T>
			requires requires{ typename Source::template type<T>; }
		struct ttype<T> {
			using type = typename Source::template type<T>;
		};
		
		KANGARU5_NO_UNIQUE_ADDRESS
		MakeInjector make_injector;
		
	public:
		template<injectable T>
		using type = typename ttype<T>::type;
	};
	
	template<template<typename> typename FallbackSource = throwing_source>
	struct assume_cached_fallback {
		template<injectable T> requires(assume_runtime_cached_v<T>) [[noreturn]]
		constexpr auto operator()(forwarded_source auto&& source) const -> FallbackSource<T> {
			throw throwing_source_exception{};
		}
	};
	
	KANGARU5_EXPORT template<
		rebindable_source Source,
		dereferenceable_cache_map Cache = polymorphic_map<std::unordered_map<std::size_t, type_erased_source_reference>>,
		dereferenceable_heap_storage Storage = default_heap_storage
	>
	struct polymorphic_container {
		constexpr polymorphic_container(Source source, Cache cache, Storage storage)  :
			state{
				KANGARU5_NO_ADL(make_source_with_cache_asymmetric<
					polymorphic_to_concrete<Source>::template type
				>)(
					with_dereference{
						with_heap_storage{
							KANGARU5_NO_ADL(make_source_with_source_wrapping)(
								KANGARU5_NO_ADL(make_source_with_source_wrapping)(
									KANGARU5_NO_ADL(make_source_with_exhaustive_construction)(
										std::move(source)
									)
								)
							),
							std::move(storage),
						}
					},
					std::move(cache)
				)
			} {}
		
		constexpr polymorphic_container(Source source, Cache cache) noexcept
			requires std::default_initializable<Storage> :
			polymorphic_container{std::move(source), std::move(cache), Storage{}} {}
		
		explicit constexpr polymorphic_container(Source source) noexcept
			requires(
				    std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) : polymorphic_container{std::move(source), Cache{}, Storage{}} {}
		
		constexpr polymorphic_container() noexcept
			requires(
				    std::default_initializable<Source>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) : polymorphic_container{Source{}, Cache{}, Storage{}} {}
		
	private:
		template<source Base>
		struct polymorphic_to_concrete {
			template<source>
			struct ttype {};
			
			template<object T>
				requires(allow_runtime_caching_v<T&> or source_of<Base&, reference_source<T>>)
			struct ttype<polymorphic_source<T&>> {
				using type = with_polymorphic_cast<with_cast_from<reference_source<T>, T&>, T&>&;
			};
			
			template<object T>
				requires(allow_runtime_caching_v<std::shared_ptr<T>> or source_of<Base&, shared_pointer_source<T>>)
			struct ttype<kangaru::polymorphic_source<std::shared_ptr<T>>> {
				using type = with_polymorphic_cast<with_cast_from<shared_pointer_source<T>, std::shared_ptr<T>>, std::shared_ptr<T>>&;
			};
			
			template<object T>
				requires requires{ typename Source::template type<T>; }
			struct ttype<kangaru::polymorphic_source<T>> {
				using type = with_polymorphic_cast<with_cast_from<typename Source::template type<T>, T>, T>&;
			};
			
			template<source T>
			using type = typename ttype<T>::type;
		};
		
		with_cache_asymmetric<
			with_dereference<
				with_heap_storage<
					with_source_wrapping<
						with_source_wrapping<
							with_exhaustive_construction<Source>
						>
					>,
					Storage
				>
			>,
			Cache,
			polymorphic_to_concrete<Source>::template type
		> state;
		
		template<typename Self, typename S>
		static constexpr auto container_source(Self&& self, S&& source) {
			return with_recursion{
				with_passthrough{
					KANGARU5_NO_ADL(make_source_with_exhaustive_construction)(
						with_alternative{
							with_recursion{
								KANGARU5_NO_ADL(make_source_with_provide_using_source<
									kangaru::polymorphic_source
								>)(
									KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source))
								)
							},
							KANGARU5_NO_ADL(compose)(
								external_reference_source{self},
								KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source.source.source.source.source)
							)
						}
					)
				}
			};
		}
		
		template<typename Self>
		using container_source_tree_t = decltype(
			container_source(std::declval<Self>(), std::declval<Self>().state)
		);
		
	public:
		template<injectable T>
		constexpr auto provide() & -> T requires source_of<container_source_tree_t<polymorphic_container&>, T> {
			return kangaru::provide<T>(
				container_source(*this, state)
			);
		}
		
		template<injectable T>
		constexpr auto provide() && -> T requires source_of<container_source_tree_t<polymorphic_container&&>, T> {
			return kangaru::provide<T>(
				container_source(std::move(*this), std::move(state))
			);
		}
		
		constexpr auto scoped() const& -> polymorphic_container<ref_result_t<Source const&>, Cache>
		requires(
			    not reference_wrapper<Cache>
			and std::default_initializable<Cache>
			and std::default_initializable<Storage>
		) {
			auto cache = Cache{};
			cache.insert(state.begin(), state.end());
			
			return polymorphic_container<ref_result_t<Source const&>, Cache>{
				KANGARU5_NO_ADL(ref)(state.source.source.source.source.source.source),
				std::move(cache),
			};
		}
		
		template<injectable T> requires(source_of<polymorphic_container&, T>)
		constexpr auto has_in_cache() -> bool {
			return state.contains(detail::ctti::type_id_for<polymorphic_source<T>>());
		}
		
		template<injectable T, source_of<T> S>
			requires(source_of<polymorphic_container&, T>)
		constexpr auto replace(S source) -> T {
			using contained_type = with_polymorphic_cast<with_cast_from<S, T>, T>;
			constexpr auto id = detail::ctti::type_id_for<polymorphic_source<T>>();
			
			auto& heap_storage = state.source.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.emplace_from([&] {
				return contained_type{
					with_cast_from<S, T>{
						std::move(source)
					},
				};
			});
			
			cache.insert_or_assign(id, *ptr);
			return kangaru::provide<T>(*ptr);
		}
		
		template<injectable T, callable F>
			requires(source_of<polymorphic_container&, T> and source_of<detail::type_traits::call_result_t<F>, T>)
		constexpr auto replace(in_place_construct<F> in_place) -> T {
			using S = detail::type_traits::call_result_t<F>;
			using contained_type = with_polymorphic_cast<with_cast_from<S, T>, T>;
			constexpr auto id = detail::ctti::type_id_for<polymorphic_source<T>>();
			
			auto& heap_storage = state.source.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.emplace_from([&] {
				return contained_type{
					with_cast_from<S, T>{
						std::move(in_place)
					},
				};
			});
			
			cache.insert_or_assign(id, *ptr);
			return kangaru::provide<T>(*ptr);
		}
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
