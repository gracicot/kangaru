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
	template<source Source>
	struct dynamic_container {
		constexpr dynamic_container() noexcept requires std::default_initializable<Source> : source{make_container_source(Source{})} {}
		explicit constexpr dynamic_container(Source source) noexcept : source{make_container_source(std::move(source))} {}
		
	private:
		static constexpr auto make_container_source(Source&& source) noexcept {
			return kangaru::make_source_with_cache(
				kangaru::make_source_with_heap_storage(
					kangaru::make_source_with_exhaustive_construction(
						std::move(source)
					)
				),
				std::unordered_map<std::size_t, void*>{}
			);
		}
		
		using container_source_t = decltype(make_container_source(std::declval<Source>()));
		
		container_source_t source;
		
		static constexpr auto rebound_source_tree(auto&& source) {
			return with_recursion{
				kangaru::make_source_with_exhaustive_construction(
					with_alternative{
						with_recursion{
							with_source_from_tag{
								KANGARU5_NO_ADL(ref)(source.source)
							}
						},
						external_reference_source<dynamic_container>{source}
					}
				)
			};
		}
		
		template<typename Self>
		using rebound_source_tree_t = decltype(rebound_source_tree(std::declval<Self>()));
		
		template<typename T>
		static constexpr auto do_provide(auto&& source) -> T {
			return kangaru::provide(
				provide_tag_v<T>, rebound_source_tree(KANGARU5_FWD(source))
			);
		}
		
	public:
		template<typename T> requires source_of<rebound_source_tree_t<dynamic_container&>, T>
		constexpr auto provide() & -> T {
			return do_provide<T>(*this);
		}
		
		template<typename T> requires source_of<rebound_source_tree_t<dynamic_container&&>, T>
		constexpr auto provide() && -> T {
			return do_provide<T>(std::move(*this));
		}
	};
}

#include "undef.hpp"

#endif
