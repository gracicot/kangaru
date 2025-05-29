#ifndef KANGARU5_DETAIL_CONTAINER_HPP
#define KANGARU5_DETAIL_CONTAINER_HPP

#include "cache_types.hpp"
#include "recursive_source.hpp"
#include "cache.hpp"
#include "heap_storage.hpp"

#include <unordered_map>
#include <concepts>

#include "define.hpp"


namespace kangaru {
	template<injectable T>
	using cached_reference_to_reference_source =
		detail::utility::ttype_t<cached_reference_to_source<reference_source>, T>;
	
	template<
		rebindable_source Source,
		cache_map Cache = std::unordered_map<std::size_t, void*>,
		heap_storage Storage = default_heap_storage
	>
	struct container {
		explicit constexpr container(Source source) noexcept :
			state{
				with_cache{
					with_heap_storage{
						KANGARU5_NO_ADL(make_source_with_exhaustive_construction)(
							std::move(source)
						)
					},
					Cache{}
				}
			} {}
		
		constexpr container() noexcept requires std::default_initializable<Source> :
			container{Source{}} {}
		
	private:
		with_cache<
			with_heap_storage<
				with_exhaustive_construction<Source>,
				Storage
			>,
			Cache
		> state;
		
		template<typename Self, typename S>
		static constexpr auto container_source(Self&& self, S&& source) {
			return with_recursion{
				with_passthrough{
					KANGARU5_NO_ADL(make_source_with_exhaustive_construction)(
						with_alternative{
							with_recursion{
								KANGARU5_NO_ADL(make_source_with_cache_using_source<cached_reference_to_reference_source>)(
									with_dereference{
										KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source))
									}
								)
							},
							KANGARU5_NO_ADL(concat)(
								external_reference_source{self}, 
								KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source))
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
	};
}

#include "undef.hpp"

#endif
