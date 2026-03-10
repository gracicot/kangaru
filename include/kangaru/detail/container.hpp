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
		dereferenceable_cache_map Cache = std::unordered_map<std::size_t, void*>,
		dereferenceable_heap_storage Storage = default_heap_storage
	>
	struct container {
		constexpr container(Source source, Cache cache, Storage storage) noexcept :
			state{
				with_cache{
					with_heap_storage{
						KANGARU5_NO_ADL(make_source_with_exhaustive_construction)(
							std::move(source)
						),
						std::move(storage),
					},
					std::move(cache),
				}
			} {}
		
		constexpr container(Source source, Cache cache) noexcept
			requires std::default_initializable<Storage> :
			container{std::move(source), std::move(cache), Storage{}} {}
		
		explicit constexpr container(Source source) noexcept
			requires(
				    std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			container{std::move(source), Cache{}, Storage{}} {}
		
		constexpr container() noexcept
			requires(
				    std::default_initializable<Source>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			container{Source{}, Cache{}, Storage{}} {}
		
	private:
		using state_type = with_cache<
			with_heap_storage<
				with_exhaustive_construction<Source>,
				Storage
			>,
			Cache
		>;
		
		state_type state;
		
		template<typename Self, typename S>
		static constexpr auto container_source(Self&& self, S&& source) {
			return with_recursion{
				with_passthrough{
					KANGARU5_NO_ADL(make_source_with_exhaustive_construction)(
						with_alternative{
							with_recursion{
								KANGARU5_NO_ADL(make_source_with_provide_using_source<
									cached_reference_to_source_mapping_using<
										detail::utility::forward_like_t<Self, Source>
									>::template source_for
								>)(
									with_dereference{
										KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source))
									}
								)
							},
							external_reference_source{self},
						}
					)
				}
			};
		}
		
		template<typename Self>
		using container_source_t = decltype(
			container_source(std::declval<Self>(), std::declval<Self>().state)
		);
		
	public:
		template<injectable T>
		constexpr auto provide() & -> T requires source_of<container_source_t<container&>, T> {
			return kangaru::provide<T>(
				container_source(*this, state)
			);
		}
		
		template<injectable T>
		constexpr auto provide() && -> T requires source_of<container_source_t<container&&>, T> {
			return kangaru::provide<T>(
				container_source(std::move(*this), std::move(state))
			);
		}
		
		constexpr auto scoped() const& -> container<ref_result_t<Source const&>, Cache, Storage>
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
			};
		}
		
		template<injectable T> requires(source_of<container&, T>)
		constexpr auto has_in_cache() -> bool {
			return state.contains(detail::ctti::type_id_for<typename mapping_with_base_source<Source>::template source_for<T>*>());
		}
		
		template<injectable T, source_of<T> S>
			requires(source_of<container&, T> and std::same_as<std::remove_cvref_t<S>, typename mapping_with_base_source<Source>::template source_for<T>>)
		constexpr auto replace(S&& source) -> T {
			using contained_type = std::remove_cvref_t<S>;
			constexpr auto id = detail::ctti::type_id_for<contained_type*>();
			
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
				source_of<container&, T>
				and std::same_as<detail::type_traits::call_result_t<F>, typename mapping_with_base_source<Source>::template source_for<T>>
			)
		constexpr auto replace(in_place_construct<F> in_place) -> T {
			using contained_type = detail::type_traits::call_result_t<F>;
			constexpr auto id = detail::ctti::type_id_for<contained_type*>();
			
			auto& heap_storage = state.source;
			auto& cache = state;
			
			auto const ptr = heap_storage.emplace_from([&] {
				return contained_type{std::move(in_place)};
			});
			
			cache.insert_or_assign(id, ptr);
			return kangaru::provide<T>(*ptr);
		}
	};
}

#include "undef.hpp"

#endif
