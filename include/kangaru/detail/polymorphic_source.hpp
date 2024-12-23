#ifndef KANGARU5_DETAIL_POLYMORPHIC_SOURCE_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_SOURCE_HPP

#include "source_reference_wrapper.hpp"
#include "source.hpp"
#include "utility.hpp"

#include <memory>
#include <type_traits>

#include "define.hpp"

namespace kangaru {
	template<injectable T>
	struct polymorphic_source {
		explicit constexpr polymorphic_source(source_of<T> auto& source) noexcept requires (not std::is_const_v<decltype(source)>) :
			source{std::addressof(source)},
			provide_function{provide_function_for<std::remove_cvref_t<decltype(source)>>()} {}
		
		explicit constexpr polymorphic_source(reference_wrapper auto&& source) noexcept
			requires (
				    source_of<decltype(source), T>
				and not std::is_rvalue_reference_v<decltype(source.unwrap())>
				and not std::is_const_v<source_reference_wrapped_type<decltype(source)>>
			) :
				source{std::addressof(source.unwrap())},
				provide_function{provide_function_for<source_reference_wrapped_type<decltype(source)>>()} {}
		
		/**
		 * @brief Unsafe constructor that allows construction from a type erased source and a function pointer to call
		 * provide on.
		 * 
		 * @details The reason why this function is unsafe is that there is no way to verify at this point if calling
		 * the function pointer with the source void pointer parameter will result in undefined behavior or not. It it
		 * meant to be called only if the source is known to be the type the provide function expects.
		 * 
		 * @warning Unsafe
		 */
		KANGARU5_UNSAFE
		constexpr polymorphic_source(
			void* source,
			detail::utility::function_pointer_t<auto(void*) -> T> provide_function
		) noexcept :
			source{source},
			provide_function{provide_function} {}
		
		KANGARU5_CONSTEXPR_VOIDSTAR auto provide() const& -> T {
			return provide_function(source);
		}
		
	private:
		template<source Source>
		static constexpr auto provide_function_for() -> std::invocable<void*> auto {
			return [](void* s) KANGARU5_CONSTEXPR_VOIDSTAR -> T {
				return kangaru::provide<T>(*static_cast<Source*>(s));
			};
		}
		
		void* source;
		detail::utility::function_pointer_t<auto(void*) -> T> provide_function;
	};
	
	/**
	 * @brief A type erased source which completely hides the source type and provide type.
	 */
	struct type_erased_source_reference {
		/**
		 * @details Not constexpr since it needs to construct a function_container<T> in the storage byte array.
		 */
		template<injectable T, source_of<T> S> requires (not std::is_const_v<S>)
		explicit type_erased_source_reference(S& source) noexcept : source{std::addressof(source)} {
			static_assert(
				sizeof(dummy_function_container) == sizeof(function_container<T>),
				"function container has a different size for type T"
			);
			
			std::construct_at(
				static_cast<function_container<T>*>(static_cast<void*>(function_container_type_erased)),
				[](void* source) -> T {
					return kangaru::provide<T>(*static_cast<S*>(source));
				}
			);
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
			using provide_t = auto(*)(void*) -> T;
			provide_t provide_function;
		};
		
		using dummy_function_container = function_container<int>;
		void* source;
		
		alignas(alignof(dummy_function_container))
		std::byte function_container_type_erased[sizeof(dummy_function_container)];
	};
}

#include "undef.hpp"

#endif
