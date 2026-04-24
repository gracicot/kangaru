#ifndef KANGARU5_DETAIL_CONTAINER_HPP
#define KANGARU5_DETAIL_CONTAINER_HPP

#include "cache_types.hpp"
#include "container_common.hpp"
#include "type_traits.hpp"
#include "recursive_source.hpp"
#include "cache.hpp"
#include "heap_storage.hpp"

#ifndef KANGARU5_MODULES
#include <unordered_map>
#include <concepts>
#endif

#include "define.hpp"

KANGARU5_EXPORT namespace kangaru {
	template<
		rebindable_source Source = none_source,
		dereferenceable_cache_map Cache = std::unordered_map<type_id, void*>,
		dereferenceable_heap_storage Storage = default_heap_storage,
		construction Construction = exhaustive_construction
	>
	struct container {
		template<allows_construction_of<Source> S>
		constexpr container(S&& source, Cache cache, Storage storage, Construction construction) :
			state{
				with_cache{
					with_heap_storage{
						KANGARU5_NO_ADL(make_source_with_construction)(
							KANGARU5_FWD(source),
							construction
						),
						std::move(storage),
					},
					std::move(cache),
				}
			},
			construction{construction} {}
		
		template<allows_construction_of<Source> S>
		constexpr container(S&& source, Cache cache, Storage storage)
			requires std::default_initializable<Construction> :
			container{KANGARU5_FWD(source), std::move(cache), std::move(storage), Construction{}} {}
		
		template<allows_construction_of<Source> S>
		constexpr container(S&& source, Cache cache)
			requires(
				    std::default_initializable<Storage>
				and std::default_initializable<Construction>
			) :
			container{KANGARU5_FWD(source), std::move(cache), Storage{}, Construction{}} {}
		
		template<allows_construction_of<Source> S>
		explicit constexpr container(S&& source)
			requires(
				    std::default_initializable<Cache>
				and std::default_initializable<Storage>
				and std::default_initializable<Construction>
			) :
			container{KANGARU5_FWD(source), Cache{}, Storage{}, Construction{}} {}
		
		constexpr container()
			requires(
				    std::default_initializable<Source>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
				and std::default_initializable<Construction>
			) :
			container{Source{}, Cache{}, Storage{}, Construction{}} {}
		
	private:
		using state_type = with_cache<
			with_heap_storage<
				with_construction<Source, Construction>,
				Storage
			>,
			Cache
		>;
		
		using unwrapped_cache = std::remove_cvref_t<maybe_unwrap_result_t<Cache>>;
		
		template<injectable T>
		using contained_mapped_type = typename mapping_with_base_source<Source>::template source_for<T>;
		
		state_type state;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
		
		template<typename S>
		constexpr auto container_source(S&& source) {
			auto rebound_state = with_cache<
				fwd_ref_result_t<forwarded_wrapped_source_t<S&&>>,
				ref_result_t<S&>
			>{
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source), KANGARU5_NO_ADL(ref)(source)
			};
			
			return with_recursion{
				with_passthrough{
					KANGARU5_NO_ADL(make_source_with_two_step_construction)(
						with_alternative{
							with_recursion{
								KANGARU5_NO_ADL(make_source_with_provide_using_source<
									cached_reference_to_source_mapping_using<
										detail::forward_like_t<S, Source>
									>::template source_for
								>)(
									with_dereference{
										cache_with_two_step_init{
											std::move(rebound_state),
											call_second_step_on_dereference{
												call_second_step_from_attribute_on_prvalue{},
											},
										},
									}
								),
							},
							external_reference_source{*this},
						},
						std::as_const(construction)
					),
				},
			};
		}
		
	public:
		template<injectable T>
		constexpr auto provide() & -> T requires source_of<decltype(container_source(state)), T> {
			return kangaru::provide<T>(
				container_source(state)
			);
		}
		
		template<injectable T>
		constexpr auto provide() && -> T requires source_of<decltype(container_source(std::move(state))), T> {
			return kangaru::provide<T>(
				container_source(std::move(state))
			);
		}
		
		constexpr auto scoped() const& -> container<ref_result_t<Source const&>, Cache, Storage, Construction>
		requires(
			    not reference_wrapper<Cache>
			and std::default_initializable<Cache>
			and std::default_initializable<Storage>
		) {
			auto cache = Cache{};
			cache.insert(state.begin(), state.end());
			
			return container<ref_result_t<Source const&>, Cache, Storage>{
				KANGARU5_NO_ADL(ref)(state.source.source.source),
				std::move(cache),
				Storage{},
				construction,
			};
		}
		
		template<injectable T>
			requires(
				    cache_map_stores<unwrapped_cache, contained_mapped_type<T>*>
				and source_of<container&, T>
			)
		constexpr auto has_in_cache() -> bool {
			return state.contains(KANGARU5_NO_ADL(type_id_for<contained_mapped_type<T>*>)());
		}
		
		template<injectable T, source_of<T> S>
			requires(
				    std::same_as<std::remove_cvref_t<S>, contained_mapped_type<T>>
				and cache_map_stores<unwrapped_cache, contained_mapped_type<T>*>
				and source_of<container&, T>
			)
		constexpr auto replace(S&& source) -> T {
			using contained_type = contained_mapped_type<T>;
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<contained_type*>)();
			
			auto& heap_storage = state.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.emplace_from([&] {
				return contained_type{
					KANGARU5_FWD(source)
				};
			});
			
			cache.insert_or_assign(id, ptr);
			return kangaru::provide<T>(*ptr);
		}
		
		template<injectable T, callable F>
			requires(
				    std::same_as<detail::call_result_t<F>, contained_mapped_type<T>>
				and cache_map_stores<unwrapped_cache, contained_mapped_type<T>*>
				and source_of<container&, T>
			)
		constexpr auto replace(in_place_construct<F> in_place) -> T {
			using contained_type = detail::call_result_t<F>;
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<contained_type*>)();
			
			auto& heap_storage = state.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.emplace_from([&] {
				return contained_type{std::move(in_place)};
			});
			
			cache.insert_or_assign(id, ptr);
			return kangaru::provide<T>(*ptr);
		}
		
		template<injectable T>
			requires(cache_map_stores<unwrapped_cache, contained_mapped_type<T>*>)
		constexpr void erase() {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<contained_mapped_type<T>*>)();
			state.erase(id);
		}
	};
	
	template<typename Source, typename Cache, typename Storage, typename Construction>
		requires(not deducer<std::remove_cvref_t<Source>>)
	container(
		Source&& source,
		Cache const& cache,
		Storage const& storage,
		Construction const& construction
	) -> container<deduced_source_type<Source>, Cache, Storage, Construction>;
	
	template<typename Source, typename Cache, typename Storage>
		requires(not deducer<std::remove_cvref_t<Source>>)
	container(
		Source&& source,
		Cache const& cache,
		Storage const& storage
	) -> container<deduced_source_type<Source>, Cache, Storage>;
	
	template<typename Source, typename Cache>
		requires(not deducer<std::remove_cvref_t<Source>>)
	container(
		Source&& source,
		Cache const& cache
	) -> container<deduced_source_type<Source>, Cache>;
	
	template<typename Source>
		requires(not deducer<std::remove_cvref_t<Source>>)
	container(
		Source&& source
	) -> container<deduced_source_type<Source>>;
}

#include "undef.hpp"

#endif
