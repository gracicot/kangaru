#ifndef KANGARU5_DETAIL_CONTAINER_HPP
#define KANGARU5_DETAIL_CONTAINER_HPP

#include "cache_types.hpp"
#include "recursive_source.hpp"
#include "cache.hpp"
#include "heap_storage.hpp"
#include "source_from_tag.hpp"

#include <unordered_map>
#include <concepts>

#include "define.hpp"


namespace kangaru {
	template<source Source, cache_map Cache = std::unordered_map<std::size_t, void*>, heap_storage Storage = default_heap_storage>
	struct dynamic_container {
		explicit constexpr dynamic_container(Source source) noexcept :
			source{
				make_source_with_cache(
					make_source_with_heap_storage(
						make_source_with_exhaustive_construction(
							std::move(source)
						)
					),
					std::unordered_map<std::size_t, void*>{}
				)
			} {}
		
		constexpr dynamic_container() noexcept requires std::default_initializable<Source> :
			dynamic_container{Source{}} {}
		
	private:
		with_cache<
			with_heap_storage<
				with_exhaustive_construction<Source>,
				Storage
			>,
			Cache
		> source;
		
		template<typename Self, typename S>
		static constexpr auto rebound_source_tree(Self&& self, S&& source) {
			return with_recursion{
				make_source_with_exhaustive_construction(
					with_alternative{
						with_recursion{
							make_source_with_cache_using<injectable_reference_source>(
								KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source))
							)
						},
						external_reference_source{self}
					}
				)
			};
		}
		
		template<typename Self>
		using rebound_source_tree_t = decltype(
			rebound_source_tree(std::declval<Self>(), std::declval<Self>().source)
		);
		
	public:
		template<injectable T>
		constexpr auto provide() & -> T requires source_of<rebound_source_tree_t<dynamic_container&>, T> {
			return kangaru::provide<T>(
				rebound_source_tree(*this, source)
			);
		}
		
		template<injectable T>
		constexpr auto provide() && -> T requires source_of<rebound_source_tree_t<dynamic_container&&>, T> {
			return kangaru::provide<T>(
				rebound_source_tree(std::move(*this), std::move(source))
			);
		}
	};
}

#include "undef.hpp"

#endif
