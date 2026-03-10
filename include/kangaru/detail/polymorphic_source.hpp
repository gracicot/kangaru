#ifndef KANGARU5_DETAIL_POLYMORPHIC_SOURCE_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_SOURCE_HPP

#include "any_source_of.hpp"
#include "source_reference_wrapper.hpp"
#include "source.hpp"
#include "utility.hpp"
#include "attributes.hpp"

#ifndef KANGARU5_MODULES
#include <memory>
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru {
	// TODO: Check if we need that alias. We now have a more generic any_source_of_ref
	KANGARU5_EXPORT template<injectable T>
	using polymorphic_source = any_source_of_ref<T>;
	
	/**
	 * @brief A type erased source which completely hides the source type and provide type.
	 */
	KANGARU5_EXPORT struct type_erased_source_reference {
		template<injectable T>
		explicit constexpr type_erased_source_reference(polymorphic_source<T> const& source) : source{source.source} {
			static_assert(
				sizeof(dummy_function_container) == sizeof(function_container<T>),
				"function container has a different size for type T"
			);
			
			std::construct_at(
				static_cast<function_container<T>*>(static_cast<void*>(function_container_type_erased)),
				std::get<0>(source.source_vtable.provide)
			);
		}
		
		template<injectable T>
		constexpr auto operator=(polymorphic_source<T> const& rhs) -> type_erased_source_reference& {
			source = rhs.source;
			
			std::construct_at(
				static_cast<function_container<T>*>(static_cast<void*>(function_container_type_erased)),
				std::get<0>(rhs.source_vtable.provide)
			);
			
			return *this;
		}
		
		/**
		 * @warning Unsafe
		 * 
		 * @details The reason why this function is unsafe is to let users of this type store type information in a way
		 * that makes sense for their use cases. For example, one could store the type information as a key in a map and
		 * associate it with the right type erased value.
		 */
		template<injectable T> KANGARU5_UNSAFE
		explicit operator polymorphic_source<T> () const noexcept {
			auto const [provide_function] = *static_cast<function_container<T> const*>(
				static_cast<void const*>(function_container_type_erased)
			);
			
			return polymorphic_source<T>{source, provide_function};
		}
		
	private:
		template<injectable T>
		struct function_container {
			detail::utility::function_pointer_t<auto(void*) -> T> provide_function;
		};
		
		using dummy_function_container = function_container<int>;
		void* source;
		
		alignas(alignof(dummy_function_container))
		std::byte function_container_type_erased[sizeof(dummy_function_container)];
	};
	
	namespace detail::polymorphic_source {
		template<typename>
		struct override_polymorphic {};
		
		template<typename... Types>
		struct override_polymorphic<std::tuple<Types...>> {
			 using type = std::tuple<kangaru::polymorphic_source<Types>...>;
		};
		
		template<typename Tuple>
		using override_polymorphic_t = typename override_polymorphic<Tuple>::type;
	}
	
	KANGARU5_EXPORT template<source Source, injectable Primary>
	struct with_polymorphic_cast {
		Source source;
		
		template<injectable T, forwarded<with_polymorphic_cast> Self> requires(wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		explicit constexpr operator type_erased_source_reference() noexcept {
			return type_erased_source_reference{polymorphic_source<Primary>{source}};
		}
		
		template<injectable T> requires source_of<Source&, T>
		explicit constexpr operator polymorphic_source<T>() & {
			return polymorphic_source<T>{source};
		}
		
		template<injectable T> requires source_of<Source const&, T>
		explicit constexpr operator polymorphic_source<T>() const& {
			return polymorphic_source<T>{source};
		}
		
		template<kangaru::source S, injectable P>
		friend auto attribute(overrides_types_in_cache<with_polymorphic_cast<S, P>>)
			-> detail::polymorphic_source::override_polymorphic_t<overrides_types_in_cache_t<P>>;
	};
}

#include "undef.hpp"

#endif
