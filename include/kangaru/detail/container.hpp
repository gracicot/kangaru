#ifndef KANGARU5_DETAIL_CONTAINER_HPP
#define KANGARU5_DETAIL_CONTAINER_HPP

#include "cache_types.hpp"
#include "optional.hpp"
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
		source Source = none_source,
		construction Construction = exhaustive_construction,
		template<typename> typename CacheMapping = default_source_mapping_runtime_cached,
		dereferenceable_cache_map Cache = std::unordered_map<type_id, void*>,
		dereferenceable_heap_storage Storage = default_heap_storage
	>
	struct container {
		constexpr container(Source source, Construction construction, Cache cache = {}, Storage storage = {}) :
			state{
				with_cache{
					with_heap_storage{
						KANGARU5_NO_ADL(make_source_with_construction)(
							KANGARU5_NO_ADL(seal_source)(filter_if(KANGARU5_FWD(source), predicate_not_mapped{})),
							KANGARU5_NO_ADL(make_construction_with_two_step_init_if<predicate_not_mapped>)(
								KANGARU5_NO_ADL(make_construction_with_unique_ptr)(construction),
								second_step_from_attribute{}
							)
						),
						std::move(storage),
					},
					std::move(cache),
				}
			},
			construction{construction} {}
		
		constexpr container(container_base<Source, Construction, CacheMapping> base, Cache cache, Storage storage = {}) :
			container{std::move(base).source, std::move(base).construction, std::move(cache), std::move(storage)} {}
		
		explicit constexpr container(container_base<Source, Construction, CacheMapping> base)
			requires(
				    std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			container{std::move(base).source, std::move(base).construction, Cache{}, Storage{}} {}
		
		explicit constexpr container(Source source)
			requires(
				    std::default_initializable<Construction>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			container{std::move(source), Construction{}, Cache{}, Storage{}} {}
		
		constexpr container()
			requires(
				    std::default_initializable<Source>
				and std::default_initializable<Construction>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			container{Source{}, Construction{}, Cache{}, Storage{}} {}
		
	private:
		struct predicate_not_mapped {
			template<injectable T>
			consteval bool operator()() const {
				return not allow_runtime_caching_v<T>;
			}
		};
		
		with_cache<
			with_heap_storage<
				with_construction<
					sealed_source<filter_if_source<Source, predicate_not_mapped>>,
					construction_with_two_step_init_if<
						construction_with_unique_ptr<Construction>,
						second_step_from_attribute,
						predicate_not_mapped
					>
				>,
				Storage
			>,
			Cache
		> state;
		
		using unwrapped_cache = std::remove_cvref_t<maybe_unwrap_result_t<Cache>>;
		
		KANGARU5_NO_UNIQUE_ADDRESS
		Construction construction;
		
		template<typename S>
		constexpr auto container_source(S&& source) {
			auto rebound_state = std::remove_cvref_t<S>::rebind(
				KANGARU5_FWD(source),
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source)
			);
			
			return with_recursion{
				with_alternative{
					KANGARU5_NO_ADL(make_source_with_passthrough<4>)(
						KANGARU5_NO_ADL(make_source_with_provide_using_source<
							source_mapping_with_reference<CacheMapping>::template source_for
						>)(
							with_dereference{
								cache_with_two_step_init{
									rebound_state,
									call_second_step_on_dereference{
										call_second_step_from_attribute_on_prvalue{},
									},
								},
							}
						)
					),
					composed_source{
						external_reference_source{*this},
						KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source.source.source.wrapped_source().source),
					},
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
		
		constexpr auto scoped() const& -> container<ref_result_t<Source const&>, Construction, CacheMapping, Cache, Storage>
		requires(
			    not reference_wrapper<Cache>
			and std::default_initializable<Cache>
			and std::default_initializable<Storage>
		) {
			auto cache = Cache{};
			cache.insert(state.begin(), state.end());
			
			return container<ref_result_t<Source const&>, Construction, CacheMapping, Cache, Storage>{
				KANGARU5_NO_ADL(ref)(state.source.source.source.wrapped_source().source),
				construction,
				std::move(cache),
			};
		}
		
		template<injectable T>
			requires(
				    cache_map_stores<unwrapped_cache, CacheMapping<T>*>
				and source_of<container&, T>
			)
		constexpr auto has_in_cache() -> bool {
			return state.contains(KANGARU5_NO_ADL(type_id_for<CacheMapping<T>*>)());
		}
		
		template<injectable T, source_of<T> S>
			requires(
				    std::same_as<std::remove_cvref_t<S>, CacheMapping<T>>
				and cache_map_stores<unwrapped_cache, CacheMapping<T>*>
				and source_of<container&, T>
			)
		constexpr auto replace(S&& source) -> T {
			using contained_type = CacheMapping<T>;
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<contained_type*>)();
			
			auto& heap_storage = state.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.template emplace<contained_type>(
				in_place_construct{[&] {
					return contained_type{
						KANGARU5_FWD(source)
					};
				}}
			);
			
			cache.insert_or_assign(id, ptr);
			return kangaru::provide<T>(*ptr);
		}
		
		template<injectable T, callable F>
			requires(
				    std::same_as<detail::call_result_t<F>, CacheMapping<T>>
				and cache_map_stores<unwrapped_cache, CacheMapping<T>*>
				and source_of<container&, T>
			)
		constexpr auto replace(in_place_construct<F> in_place) -> T {
			using contained_type = detail::call_result_t<F>;
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<contained_type*>)();
			
			auto& heap_storage = state.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.template emplace<contained_type>(
				in_place_construct{[&] {
					return contained_type{std::move(in_place)};
				}}
			);
			
			cache.insert_or_assign(id, ptr);
			return kangaru::provide<T>(*ptr);
		}
		
		template<injectable T>
			requires(cache_map_stores<unwrapped_cache, CacheMapping<T>*>)
		constexpr void erase() {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<CacheMapping<T>*>)();
			state.erase(id);
		}
		
		template<unqualified_object T>
			requires(cache_map_stores<unwrapped_cache, CacheMapping<T>*>)
		constexpr auto get_from_cache() const -> optional<T> {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<CacheMapping<T>*>)();
			if (auto const it = state.find(id); it != state.end()) {
				return kangaru::provide<T>(*static_cast<CacheMapping<T>*>(it->second));
			}
			
			return {};
		}
		
		template<reference T>
			requires(cache_map_stores<unwrapped_cache, CacheMapping<T>*>)
		constexpr auto get_from_cache() const -> optional<T&> {
			constexpr auto id = KANGARU5_NO_ADL(type_id_for<CacheMapping<T>*>)();
			if (auto const it = state.find(id); it != state.end()) {
				return static_cast<T&>(kangaru::provide<T>(*static_cast<CacheMapping<T>*>(it->second)));
			}
			
			return {};
		}
	};
}

#include "undef.hpp"

#endif
