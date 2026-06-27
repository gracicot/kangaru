#ifndef KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP

#include "concepts.hpp"
#include "two_step_init.hpp"
#include "optional.hpp"
#include "source_reference_wrapper.hpp"
#include "source_traits.hpp"
#include "utility.hpp"
#include "source_types.hpp"
#include "cache_types.hpp"
#include "recursive_source.hpp"
#include "polymorphic_source.hpp"
#include "cache.hpp"
#include "heap_storage.hpp"
#include "container_common.hpp"

#ifndef KANGARU5_MODULES
#include <unordered_map>
#include <concepts>
#include <memory>
#include <utility>
#endif

#include "define.hpp"

namespace kangaru {
	namespace detail::polymorphic_container_private {
		template<template<typename> typename Mapping>
		struct cached_source {
		private:
			template<injectable T>
			struct mapping {};
			
			template<injectable T> requires (requires{ typename Mapping<T>; })
			struct mapping<kangaru::any_source_of_ref<T>> {
				using type = with_polymorphic_cast<
					with_cast_from<
						Mapping<T>,
						T
					>,
					T
				>&;
			};
			
		public:
			template<injectable T>
			using source_for = typename mapping<T>::type;
		};
	}
	
	KANGARU5_EXPORT template<
		source Source = none_source,
		construction Construction = exhaustive_construction,
		template<typename> typename CacheMapping = default_source_mapping_runtime_cached,
		dereferenceable_cache_map Cache = polymorphic_map<std::unordered_map<type_id, any_source_of_one_ref>>,
		dereferenceable_heap_storage Storage = default_heap_storage
	>
	struct polymorphic_container {
		constexpr polymorphic_container(Source source, Construction construction, Cache cache = {}, Storage storage = {}) :
			state{
				KANGARU5_NO_ADL(make_source_with_cache_asymmetric<
					cache_mapping::template source_for
				>)(
					with_dereference{
						with_heap_storage{
							KANGARU5_NO_ADL(make_source_with_source_wrapping)(
								KANGARU5_NO_ADL(make_source_with_source_wrapping)(
									KANGARU5_NO_ADL(make_source_with_construction)(
										KANGARU5_NO_ADL(seal_source)(
											KANGARU5_NO_ADL(filter_if)(
												KANGARU5_FWD(source),
												predicate_not_mapped{}
											)
										),
										KANGARU5_NO_ADL(make_construction_with_two_step_init_if<predicate_not_mapped>)(
											KANGARU5_NO_ADL(make_construction_with_unique_ptr)(construction),
											second_step_from_attribute{}
										)
									)
								)
							),
							std::move(storage),
						},
					},
					std::move(cache)
				),
			},
			construction{construction} {}
		
		constexpr polymorphic_container(container_base<Source, Construction, CacheMapping> base, Cache cache, Storage storage = {}) :
			polymorphic_container{std::move(base).source, std::move(base).construction, std::move(cache), std::move(storage)} {}
		
		explicit constexpr polymorphic_container(container_base<Source, Construction, CacheMapping> base)
			requires(
				    std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			polymorphic_container{std::move(base).source, std::move(base).construction, Cache{}, Storage{}} {}
		
		explicit constexpr polymorphic_container(Source source)
			requires(
				    std::default_initializable<Construction>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			polymorphic_container{std::move(source), Construction{}, Cache{}, Storage{}} {}
		
		constexpr polymorphic_container()
			requires(
				    std::default_initializable<Source>
				and std::default_initializable<Construction>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			polymorphic_container{Source{}, Construction{}, Cache{}, Storage{}} {}
		
	private:
		using cache_mapping = detail::polymorphic_container_private::cached_source<
			CacheMapping
		>;
		
		struct predicate_not_mapped {
			template<injectable T>
			consteval bool operator()() const {
				return not allow_runtime_caching_v<T>;
			}
		};
		
		with_cache_asymmetric<
			with_dereference<
				with_heap_storage<
					with_source_wrapping<
						with_source_wrapping<
							with_construction<
								sealed_source<
									filter_if_source<Source, predicate_not_mapped>
								>,
								construction_with_two_step_init_if<
									construction_with_unique_ptr<Construction>,
									second_step_from_attribute,
									predicate_not_mapped
								>
							>
						>
					>,
					Storage
				>
			>,
			Cache,
			cache_mapping::template source_for
		> state;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
		
		template<injectable T>
		using polymorphic_source = any_source_of_ref<T>;
		
		using unwrapped_cache = std::remove_cvref_t<maybe_unwrap_result_t<Cache>>;
		
		template<typename S>
		constexpr auto container_source(S&& source) {
			auto rebound_state = std::remove_cvref_t<S>::rebind(
				KANGARU5_FWD(source),
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source)
			);
			
			return with_recursion{
				with_alternative{
					KANGARU5_NO_ADL(make_source_with_passthrough<6>)(
						KANGARU5_NO_ADL(make_source_with_provide_using_source<
							polymorphic_source
						>)(
							cache_with_two_step_init{
								rebound_state,
								second_step_from_attribute{},
							}
						)
					),
					composed_source{
						external_reference_source{*this},
						KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source.source.source.source.source.source.wrapped_source().source)
					},
				},
			};
		}
		
	public:
		template<injectable T>
		constexpr auto provide() & -> T requires(source_of<decltype(container_source(state)), T>) {
			return kangaru::provide<T>(
				container_source(state)
			);
		}
		
		template<injectable T>
		constexpr auto provide() && -> T requires(source_of<decltype(container_source(std::move(state))), T>) {
			return kangaru::provide<T>(
				container_source(std::move(state))
			);
		}
		
		constexpr auto scoped() const& -> polymorphic_container<ref_result_t<Source const&>, Construction, CacheMapping, Cache, Storage>
		requires(
			    not reference_wrapper<Cache>
			and std::default_initializable<Cache>
			and std::default_initializable<Storage>
		) {
			auto cache = Cache{};
			cache.insert(state.begin(), state.end());
			
			return polymorphic_container<ref_result_t<Source const&>, Construction, CacheMapping, Cache>{
				KANGARU5_NO_ADL(ref)(state.source.source.source.source.source.source.wrapped_source().source),
				construction,
				std::move(cache),
			};
		}
		
		template<injectable T>
			requires(
				    cache_map_stores<unwrapped_cache, any_source_of_ref<T>>
				and source_of<polymorphic_container&, T>
			)
		constexpr auto has_in_cache() -> bool {
			return state.contains(KANGARU5_NO_ADL(type_id_for<any_source_of_ref<T>>)());
		}
		
		template<injectable T, source_of<T> S>
			requires(
				    cache_map_stores<unwrapped_cache, any_source_of_ref<T>>
				and source_of<polymorphic_container&, T>
			)
		constexpr auto replace(S&& source) -> T {
			using contained_type = with_polymorphic_cast<with_cast_from<std::remove_cvref_t<S>, T>, T>;
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<any_source_of_ref<T>>)();
			
			auto& heap_storage = state.source.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.template emplace<contained_type>(
				in_place_construct{[&] {
					return contained_type{
						with_cast_from<std::remove_cvref_t<S>, T>{
							KANGARU5_FWD(source)
						},
					};
				}}
			);
			
			cache.insert_or_assign(id, *ptr);
			return kangaru::provide<T>(*ptr);
		}
		
		template<injectable T, callable F>
			requires(
				    cache_map_stores<unwrapped_cache, any_source_of_ref<T>>
				and source_of<polymorphic_container&, T>
				and source_of<detail::call_result_t<F>, T>
			)
		constexpr auto replace(in_place_construct<F> in_place) -> T {
			using source = detail::call_result_t<F>;
			using contained_type = with_polymorphic_cast<with_cast_from<source, T>, T>;
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<any_source_of_ref<T>>)();
			
			auto& heap_storage = state.source.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.template emplace<contained_type>(
				in_place_construct{[&] {
					return contained_type{
						with_cast_from<source, T>{
							std::move(in_place)
						},
					};
				}}
			);
			
			cache.insert_or_assign(id, *ptr);
			return kangaru::provide<T>(*ptr);
		}
		
		template<injectable T>
			requires(cache_map_stores<unwrapped_cache, any_source_of_ref<T>>)
		constexpr void erase() {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<any_source_of_ref<T>>)();
			state.erase(id);
		}
		
		template<unqualified_object T>
			requires(cache_map_stores<unwrapped_cache, any_source_of_ref<T>>)
		constexpr auto get_from_cache() const -> optional<T> {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<any_source_of_ref<T>>)();
			if (auto const it = state.find(id); it != state.end()) {
				return kangaru::provide<T>(static_cast<any_source_of_ref<T>>(it->second));
			}
			
			return {};
		}
		
		template<reference T>
			requires(cache_map_stores<unwrapped_cache, any_source_of_ref<T>>)
		constexpr auto get_from_cache() const -> optional<T&> {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<any_source_of_ref<T>>)();
			if (auto const it = state.find(id); it != state.end()) {
				return static_cast<T&>(kangaru::provide<T>(static_cast<any_source_of_ref<T>>(it->second)));
			}
			
			return {};
		}
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
