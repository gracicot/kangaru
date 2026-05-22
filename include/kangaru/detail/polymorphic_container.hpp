#ifndef KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP

#include "concepts.hpp"
#include "two_step_init.hpp"
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
		template<source_ref Source>
		struct cached_source {
			template<injectable T>
			struct mapping {};
			
			template<injectable T> requires (requires{ typename cached_source_mapping_using_t<Source, T>; })
			struct mapping<kangaru::any_source_of_ref<T>> {
				using type = with_polymorphic_cast<
					with_cast_from<
						cached_source_mapping_using_t<Source, T>,
						T
					>,
					T
				>&;
			};
			
			template<injectable T>
			using source_for = typename mapping<T>::type;
		};
	}
	
	KANGARU5_EXPORT template<
		rebindable_source Source = none_source,
		dereferenceable_cache_map Cache = polymorphic_map<std::unordered_map<type_id, any_source_of_one_ref>>,
		dereferenceable_heap_storage Storage = default_heap_storage,
		construction Construction = exhaustive_construction
	>
	struct polymorphic_container {
		template<allows_construction_of<Source> S>
		constexpr polymorphic_container(S&& source, Cache cache, Storage storage, Construction construction) :
			state{
				KANGARU5_NO_ADL(make_source_with_cache_asymmetric<
					detail::never_type_identity
				>)(
					with_dereference{
						with_heap_storage{
							KANGARU5_NO_ADL(make_source_with_source_wrapping)(
								KANGARU5_NO_ADL(make_source_with_source_wrapping)(
									KANGARU5_NO_ADL(make_source_with_construction)(
										seal_source(with_exclude_mapping<Source, cached_source_mapping_using_t>{KANGARU5_FWD(source)}),
										construction
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
		
		template<allows_construction_of<Source> S>
		constexpr polymorphic_container(S&& source, Cache cache, Storage storage)
			requires std::default_initializable<Construction> :
			polymorphic_container{KANGARU5_FWD(source), std::move(cache), std::move(storage), Construction{}} {}
		
		template<allows_construction_of<Source> S>
		constexpr polymorphic_container(S&& source, Cache cache)
			requires(
				    std::default_initializable<Storage>
				and std::default_initializable<Construction>
			) :
			polymorphic_container{KANGARU5_FWD(source), std::move(cache), Storage{}, Construction{}} {}
		
		template<allows_construction_of<Source> S>
		explicit constexpr polymorphic_container(S&& source)
			requires(
				    std::default_initializable<Cache>
				and std::default_initializable<Storage>
				and std::default_initializable<Construction>
			) :
			polymorphic_container{KANGARU5_FWD(source), Cache{}, Storage{}, Construction{}} {}
		
		constexpr polymorphic_container()
			requires(
				    std::default_initializable<Source>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
				and std::default_initializable<Construction>
			) :
			polymorphic_container{Source{}, Cache{}, Storage{}, Construction{}} {}
		
	private:
		with_cache_asymmetric<
			with_dereference<
				with_heap_storage<
					with_source_wrapping<
						with_source_wrapping<
							with_construction<
								sealed_source<with_exclude_mapping<Source, cached_source_mapping_using_t>>,
								Construction
							>
						>
					>,
					Storage
				>
			>,
			Cache,
			// Since we want to always properly support forwarding from the providing source,
			// we pass a template type alias that never exists by default and change it in container_source.
			detail::never_type_identity
		> state;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
		
		template<injectable T>
		using polymorphic_source = any_source_of_ref<T>;
		
		using unwrapped_cache = std::remove_cvref_t<maybe_unwrap_result_t<Cache>>;
		
		template<typename S>
		constexpr auto container_source(S&& source) {
			auto rebound_state = std::remove_cvref_t<S>::template rebind<
				detail::polymorphic_container_private::cached_source<
					detail::forward_like_t<S, Source>
				>::template source_for
			>(
				KANGARU5_FWD(source),
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source)
			);
			
			return with_recursion{
				KANGARU5_NO_ADL(make_source_with_passthrough<1>)(
					KANGARU5_NO_ADL(make_source_with_two_step_construction)(
						with_alternative{
							KANGARU5_NO_ADL(make_source_with_passthrough<6>)(
								KANGARU5_NO_ADL(make_source_with_provide_using_source<
									polymorphic_source
								>)(
									cache_with_two_step_init{
										std::move(rebound_state),
										second_step_from_attribute{},
									}
								)
							),
							composed_source{
								external_reference_source{*this},
								KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source.source.source.source.source.source.wrapped_source().source)
							},
						},
						std::as_const(construction)
					)
				),
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
		
		constexpr auto scoped() const& -> polymorphic_container<ref_result_t<Source const&>, Cache, Storage, Construction>
		requires(
			    not reference_wrapper<Cache>
			and std::default_initializable<Cache>
			and std::default_initializable<Storage>
		) {
			auto cache = Cache{};
			cache.insert(state.begin(), state.end());
			
			return polymorphic_container<ref_result_t<Source const&>, Cache>{
				KANGARU5_NO_ADL(ref)(state.source.source.source.source.source.source.wrapped_source().source),
				std::move(cache),
				Storage{},
				construction,
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
			
			auto const ptr = heap_storage.emplace_from([&] {
				return contained_type{
					with_cast_from<std::remove_cvref_t<S>, T>{
						KANGARU5_FWD(source)
					},
				};
			});
			
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
			
			auto const ptr = heap_storage.emplace_from([&] {
				return contained_type{
					with_cast_from<source, T>{
						std::move(in_place)
					},
				};
			});
			
			cache.insert_or_assign(id, *ptr);
			return kangaru::provide<T>(*ptr);
		}
		
		template<injectable T>
		constexpr void erase() {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<any_source_of_ref<T>>)();
			state.erase(id);
		}
	};
	
	template<typename Source, typename Cache, typename Storage, typename Construction>
		requires(not deducer<std::remove_cvref_t<Source>>)
	polymorphic_container(
		Source&& source,
		Cache const& cache,
		Storage const& storage,
		Construction const& construction
	) -> polymorphic_container<deduced_source_type<Source>, Cache, Storage, Construction>;
	
	template<typename Source, typename Cache, typename Storage>
		requires(not deducer<std::remove_cvref_t<Source>>)
	polymorphic_container(
		Source&& source,
		Cache const& cache,
		Storage const& storage
	) -> polymorphic_container<deduced_source_type<Source>, Cache, Storage>;
	
	template<typename Source, typename Cache>
		requires(not deducer<std::remove_cvref_t<Source>>)
	polymorphic_container(
		Source&& source,
		Cache const& cache
	) -> polymorphic_container<deduced_source_type<Source>, Cache>;
	
	template<typename Source>
		requires(not deducer<std::remove_cvref_t<Source>>)
	polymorphic_container(
		Source&& source
	) -> polymorphic_container<deduced_source_type<Source>>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
