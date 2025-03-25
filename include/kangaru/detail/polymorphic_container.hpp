#ifndef KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP

#include "kangaru/detail/utility.hpp"
#include "source_types.hpp"
#include "cache_types.hpp"
#include "recursive_source.hpp"
#include "polymorphic_source.hpp"
#include "cache.hpp"
#include "heap_storage.hpp"

#include <unordered_map>
#include <concepts>

#include "define.hpp"

namespace kangaru {
	namespace detail::polymorphic_container {
		template<kangaru::source>
		struct polymorphic_to_concrete {};
		
		template<injectable T>
		struct polymorphic_to_concrete<polymorphic_source<T&>> {
			using type = with_polymorphic_cast<with_cast_from<injectable_reference_source<T>, T&>, T&>&;
		};
		
		template<kangaru::source T>
		using polymorphic_to_concrete_t = typename polymorphic_to_concrete<T>::type;
	}
	
	template<source Source, cache_map Cache = polymorphic_map<std::unordered_map<std::size_t, type_erased_source_reference>>, heap_storage Storage = default_heap_storage>
	struct polymorphic_container {
		explicit constexpr polymorphic_container(Source source) noexcept :
			source{
				make_source_with_cache_asymmetric<detail::polymorphic_container::polymorphic_to_concrete_t>(
					make_source_with_dereference(
						make_source_with_heap_storage(
							make_source_with_source_wrapping(
								make_source_with_source_wrapping(
									make_source_with_exhaustive_construction(
										std::move(source)
									)
								)
							)
						)
					),
					Cache{}
				)
			} {}
		
		constexpr polymorphic_container() noexcept requires std::default_initializable<Source> :
			polymorphic_container{Source{}} {}
		
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
		> source;
		
		template<typename Self, typename S>
		static constexpr auto rebound_source_tree(Self&& self, S&& source) {
			return with_recursion{
				make_source_with_exhaustive_construction(
					with_alternative{
						with_recursion{
							make_source_with_cache_using_source<polymorphic_source>(
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
		constexpr auto provide() & -> T requires source_of<rebound_source_tree_t<polymorphic_container&>, T> {
			return kangaru::provide<T>(
				rebound_source_tree(*this, source)
			);
		}
		
		template<injectable T>
		constexpr auto provide() && -> T requires source_of<rebound_source_tree_t<polymorphic_container&&>, T> {
			return kangaru::provide<T>(
				rebound_source_tree(std::move(*this), std::move(source))
			);
		}
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_POLYMORPHIC_CONTAINER_HPP
