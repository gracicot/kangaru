#ifndef KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP

#include "concepts.hpp"
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
	namespace detail::polymorphic_container::file_private {
		template<source_ref Source>
		struct polymorphic_to_concrete {
			template<kangaru::source>
			struct ttype {};
			
			template<object T>
				requires(allow_runtime_caching_v<T&> or source_of<Source, reference_source<T>>)
			struct ttype<kangaru::polymorphic_source<T&>> {
				using type = with_polymorphic_cast<with_cast_from<reference_source<T>, T&>, T&>&;
			};
			
			template<object T>
				requires(allow_runtime_caching_v<std::shared_ptr<T>> or source_of<Source, shared_pointer_source<T>>)
			struct ttype<kangaru::polymorphic_source<std::shared_ptr<T>>> {
				using type = with_polymorphic_cast<with_cast_from<shared_pointer_source<T>, std::shared_ptr<T>>, std::shared_ptr<T>>&;
			};
			
			template<injectable T>
				requires(
					    requires{ typename container_common::enumerated_select_source_of_t<T, std::remove_cvref_t<Source>>; }
					and source_of<Source, container_common::enumerated_select_source_of_t<T, std::remove_cvref_t<Source>>>
				)
			struct ttype<kangaru::polymorphic_source<T>> {
				using selected_source = container_common::enumerated_select_source_of_t<T, std::remove_cvref_t<Source>>;
				using type = with_polymorphic_cast<with_cast_from<selected_source, T>, T>&;
			};
			
			template<kangaru::source T>
			using type = typename ttype<T>::type;
		};
	}
	
	KANGARU5_EXPORT template<
		rebindable_source Source,
		dereferenceable_cache_map Cache = polymorphic_map<std::unordered_map<std::size_t, type_erased_source_reference>>,
		dereferenceable_heap_storage Storage = default_heap_storage
	>
	struct polymorphic_container {
		constexpr polymorphic_container(Source source, Cache cache, Storage storage)  :
			state{
				KANGARU5_NO_ADL(make_source_with_cache_asymmetric<
					detail::utility::never_type_identity
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
			// Since we want to always properly support forwarding from the providing source,
			// we pass a template type alias that never exists by default and change it in container_source.
			detail::utility::never_type_identity
		> state;
		
		template<typename Self, typename S>
		static constexpr auto container_source(Self&& self, S&& source) {
			auto rebound_state = with_cache_asymmetric<
				fwd_ref_result_t<forwarded_wrapped_source_t<S&&>>,
				ref_result_t<S&>,
				detail::polymorphic_container::file_private::polymorphic_to_concrete<
					detail::utility::forward_like_t<Self, Source>
				>::template type
			>{
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source).source), KANGARU5_NO_ADL(ref)(source)
			};
			
			return with_recursion{
				with_passthrough{
					KANGARU5_NO_ADL(make_source_with_exhaustive_construction)(
						with_alternative{
							with_recursion{
								KANGARU5_NO_ADL(make_source_with_provide_using_source<
									kangaru::polymorphic_source
								>)(
									std::move(rebound_state)
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
		using container_source_t = decltype(
			container_source(std::declval<Self>(), std::declval<Self>().state)
		);
		
	public:
		template<injectable T>
		constexpr auto provide() & -> T requires source_of<container_source_t<polymorphic_container&>, T> {
			return kangaru::provide<T>(
				container_source(*this, state)
			);
		}
		
		template<injectable T>
		constexpr auto provide() && -> T requires source_of<container_source_t<polymorphic_container&&>, T> {
			return kangaru::provide<T>(
				container_source(std::move(*this), std::move(state))
			);
		}
		
		// TODO: Support dynamic provided in const overloads
		template<injectable T>
		constexpr auto provide() const& -> T requires source_of<Source const&, T> {
			return kangaru::provide<T>(
				state.source.source.source.source.source.source
			);
		}
		
		template<injectable T>
		constexpr auto provide() const&& -> T requires source_of<Source const&&, T> {
			return kangaru::provide<T>(
				state.source.source.source.source.source.source
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
