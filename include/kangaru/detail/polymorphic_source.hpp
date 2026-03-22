#ifndef KANGARU5_DETAIL_POLYMORPHIC_SOURCE_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_SOURCE_HPP

#include "any_source_of.hpp"
#include "two_step_init.hpp"
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
	/**
	 * @brief A type erased source which completely hides the source type and provide type.
	 *
	 * Functions of this type cannot be marked as constexpr because it uses a byte buffer based type punning.
	 * Unions cannot be used for this type because we must type erase the return type of the provide function.
	 */
	KANGARU5_EXPORT struct any_source_of_one_ref {
		template<injectable T>
		constexpr explicit any_source_of_one_ref(any_source_of_ref<T> const& source) : source{source.source} {
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
		auto operator=(any_source_of_ref<T> const& rhs) -> any_source_of_one_ref& {
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
		explicit operator any_source_of_ref<T> () const noexcept {
			auto const [provide_function] = *static_cast<function_container<T> const*>(
				static_cast<void const*>(function_container_type_erased)
			);
			
			return any_source_of_ref<T>{source, provide_function};
		}
		
	private:
		template<injectable T>
		struct function_container {
			detail::function_pointer_t<auto(void*) -> T> provide_function;
		};
		
		using dummy_function_container = function_container<int>;
		void* source;
		
		alignas(alignof(dummy_function_container))
		std::byte function_container_type_erased[sizeof(dummy_function_container)];
	};
	
	namespace detail::polymorphic_source_private {
		template<typename>
		struct override_polymorphic {};
		
		template<typename... Types>
		struct override_polymorphic<std::tuple<Types...>> {
			 using type = std::tuple<kangaru::any_source_of_ref<Types>...>;
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
		
		explicit constexpr operator any_source_of_one_ref() noexcept {
			return any_source_of_one_ref{any_source_of_ref<Primary>{source}};
		}
		
		template<injectable T> requires source_of<Source&, T>
		explicit constexpr operator any_source_of_ref<T>() & {
			return any_source_of_ref<T>{source};
		}
		
		template<injectable T> requires source_of<Source const&, T>
		explicit constexpr operator any_source_of_ref<T>() const& {
			return any_source_of_ref<T>{source};
		}
		
		template<kangaru::source S, injectable P>
		friend auto attribute(overrides_types_in_cache<with_polymorphic_cast<S, P>>)
			-> detail::polymorphic_source_private::override_polymorphic_t<overrides_types_in_cache_t<P>>;
		
		template<kangaru::source S, injectable P>
		friend auto attribute(second_step_init<with_polymorphic_cast<S, P>>)
			-> call_second_step_from_attribute_on_wrapped_source;
	};
}

#include "undef.hpp"

#endif
