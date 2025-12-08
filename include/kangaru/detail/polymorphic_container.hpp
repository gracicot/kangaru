#ifndef KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP

#include "source_types.hpp"
#include "cache_types.hpp"
#include "recursive_source.hpp"
#include "polymorphic_source.hpp"
#include "cache.hpp"
#include "heap_storage.hpp"

#ifndef KANGARU5_MODULES
#include <unordered_map>
#include <concepts>
#endif

#include "define.hpp"

namespace kangaru {
	namespace detail::polymorphic_container {
		template<kangaru::source>
		struct polymorphic_to_concrete {};
		
		template<injectable T>
		struct polymorphic_to_concrete<kangaru::polymorphic_source<T&>> {
			using type = with_polymorphic_cast<with_cast_from<reference_source<T>, T&>, T&>&;
		};
		
		template<kangaru::source T>
		using polymorphic_to_concrete_t = typename polymorphic_to_concrete<T>::type;
	}
	
	KANGARU5_EXPORT template<
		rebindable_source Source,
		dereferenceable_cache_map Cache = polymorphic_map<std::unordered_map<std::size_t, type_erased_source_reference>>,
		dereferenceable_heap_storage Storage = default_heap_storage
	>
	struct polymorphic_container {
		constexpr polymorphic_container(Source source, Cache cache, Storage storage) noexcept :
			state{
				KANGARU5_NO_ADL(make_source_with_cache_asymmetric<detail::polymorphic_container::polymorphic_to_concrete_t>)(
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
			) :
			polymorphic_container{std::move(source), Cache{}, Storage{}} {}
		
		constexpr polymorphic_container() noexcept
			requires(
				    std::default_initializable<Source>
				and std::default_initializable<Cache>
				and std::default_initializable<Storage>
			) :
			polymorphic_container{Source{}, Cache{}, Storage{}} {}
		
	private:
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
			detail::polymorphic_container::polymorphic_to_concrete_t
		> state;
		
		template<typename Self, typename S>
		static constexpr auto container_source(Self&& self, S&& source) {
			return with_recursion{
				with_passthrough{
					KANGARU5_NO_ADL(make_source_with_exhaustive_construction)(
						with_alternative{
							with_recursion{
								KANGARU5_NO_ADL(make_source_with_cache_using_source<polymorphic_source>)(
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
			if constexpr (reference<T>) {
				return state.contains(detail::ctti::type_id_for<polymorphic_source<T>>());
			} else {
				return false;
			}
		}
		
		template<callable F>
			requires(
				    std::move_constructible<F>
				and object<detail::type_traits::call_result_t<F>>
				and source_of<polymorphic_container&, detail::type_traits::call_result_t<F>&>
			)
		constexpr auto replace(F function) -> detail::type_traits::call_result_t<F>& {
			using result_type = detail::type_traits::call_result_t<F>;
			using contained_type = with_polymorphic_cast<with_cast_from<reference_source<result_type>, result_type&>, result_type&>;
			constexpr auto id = detail::ctti::type_id_for<polymorphic_source<result_type&>>();
			
			auto const ptr = state.source.source.emplace_from([&] {
				return contained_type{
					with_cast_from<reference_source<result_type>, result_type&>{
						reference_source<result_type>{
							construct_from_call{std::move(function)}
						}
					},
				};
			});
			
			state.insert_or_assign(id, *ptr);
			return kangaru::provide<result_type&>(ptr->source.source);
		}
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
