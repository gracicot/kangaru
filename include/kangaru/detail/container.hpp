#ifndef KANGARU5_DETAIL_CONTAINER_HPP
#define KANGARU5_DETAIL_CONTAINER_HPP

#include "recursive_source.hpp"
#include "cache.hpp"
#include "heap_storage.hpp"
#include "source_from_tag.hpp"

#include <unordered_map>
#include <concepts>

#include "define.hpp"

namespace kangaru {
	namespace detail::container {
		template<typename Self, typename Source>
		inline constexpr auto rebound_source_tree(Self&& self, Source&& source) {
			return with_recursion{
				make_source_with_exhaustive_construction(
					with_alternative{
						with_recursion{
							with_source_from_tag{
								KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source))
							}
						},
						external_reference_source{self}
					}
				)
			};
		}
	} // namespace detail::container
	
	template<source Source>
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
				with_exhaustive_construction<Source>
			>,
			std::unordered_map<std::size_t, void*>
		> source;
		
		template<typename Self>
		using rebound_source_tree_t = decltype(
			detail::container::rebound_source_tree(std::declval<Self>(), std::declval<Self>().source)
		);
		
	public:
		template<injectable T>
		constexpr auto provide() & -> T requires source_of<rebound_source_tree_t<dynamic_container&>, T> {
			return kangaru::provide<T>(
				detail::container::rebound_source_tree(*this, source)
			);
		}
		
		template<injectable T>
		constexpr auto provide() && -> T requires source_of<rebound_source_tree_t<dynamic_container&&>, T> {
			return kangaru::provide<T>(
				detail::container::rebound_source_tree(std::move(*this), std::move(source))
			);
		}
	};
}

#include "undef.hpp"

#endif
