#ifndef KANGARU5_DETAIL_CONTAINER_HPP
#define KANGARU5_DETAIL_CONTAINER_HPP

#include "cache_types.hpp"
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
	template<injectable T>
	using cached_reference_to_reference_source =
		detail::utility::ttype_t<cached_reference_to_source<reference_source>, T>;
	
	template<
		rebindable_source Source,
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
								KANGARU5_NO_ADL(make_source_with_provide_using_source<cached_reference_to_reference_source>)(
									with_dereference{
										KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source))
									}
								)
							},
							KANGARU5_NO_ADL(compose)(
								external_reference_source{self}, 
								KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source.source.source)
							)
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
			if constexpr (reference<T>) {
				return state.contains(detail::ctti::type_id_for<reference_source<std::remove_reference_t<T>>*>());
			} else {
				return false;
			}
		}
		
		template<callable F>
			requires(
				    std::move_constructible<F>
				and object<detail::type_traits::call_result_t<F>>
				and source_of<container&, detail::type_traits::call_result_t<F>&>
			)
		constexpr auto replace(F function) -> detail::type_traits::call_result_t<F>& {
			using result_type = detail::type_traits::call_result_t<F>;
			
			auto const ptr = state.source.emplace_from([&] {
				return reference_source<result_type>{in_place_construct{std::move(function)}};
			});
			
			state.insert_or_assign(detail::ctti::type_id_for<reference_source<result_type>*>(), ptr);
			return kangaru::provide<result_type&>(*ptr);
		}
	};
}

#include "undef.hpp"

#endif
