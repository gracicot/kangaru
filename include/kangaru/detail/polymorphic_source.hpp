#ifndef KANGARU5_DETAIL_POLYMORPHIC_SOURCE_HPP
#define KANGARU5_DETAIL_POLYMORPHIC_SOURCE_HPP

#include "type_traits.hpp"
#include "constructor.hpp"
#include "concepts.hpp"
#include "two_step_init.hpp"
#include "source_reference_wrapper.hpp"
#include "source.hpp"
#include "utility.hpp"
#include "attributes.hpp"

#ifndef KANGARU5_MODULES
#include <memory>
#include <cstddef>
#include <new>
#include <utility>
#include <tuple>
#include <concepts>
#endif

#include "define.hpp"

namespace kangaru::detail::polymorphic_source_private {
	template<injectable... Types>
	struct any_source_of_base {
		template<injectable T, forwarded<any_source_of_base> Self> requires(... || std::same_as<Types, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return std::get<provide_function_ptr<T>>(source.vtable().provide)(source.source);
		}
		
	protected:
		template<injectable T>
		using provide_function_ptr = detail::function_pointer_t<auto(void*) -> T>;
		
		struct vtable_t {
			std::tuple<provide_function_ptr<Types>...> provide;
			auto(*destroy)(void const*) -> void;
			
			template<kangaru::source Source>
			static consteval auto init_for() -> vtable_t {
				return vtable_t{
					.provide = {
						[](void* source) KANGARU5_CONSTEXPR_VOIDSTAR -> Types {
							return kangaru::provide<Types>(*static_cast<Source*>(source));
						}...
					},
					.destroy = [](void const* source) KANGARU5_CONSTEXPR_VOIDSTAR noexcept {
						delete static_cast<Source const*>(source);
					}
				};
			}
		};
		
		static constexpr auto should_inline_vtable = sizeof...(Types) < 4;
		using vtable_type = detail::conditional_t<should_inline_vtable, vtable_t, vtable_t const*>;
		
		template<kangaru::source Source>
		constexpr any_source_of_base(Source* source) noexcept :
			// This const_cast is safe. The only way this pointer can be used is
			// through the `provide` functions from the vtable. Those functions in turn
			// adds back the const
			source_vtable{get_vtable_for<Source>()}, source{const_cast<std::remove_const_t<Source>*>(source)} {}
		
		constexpr any_source_of_base(vtable_type source_vtable, void* source) noexcept :
			source_vtable{source_vtable}, source{source} {}
		
		template<kangaru::source Source>
		static consteval auto get_vtable_for() noexcept {
			if constexpr (should_inline_vtable) {
				return vtable_t::template init_for<Source>();
			} else {
				return std::addressof(vtable_instance<Source>);
			}
		}
		
		template<kangaru::source Source>
		static constexpr auto vtable_instance = vtable_t::template init_for<Source>();
		
		constexpr auto vtable() -> vtable_t const& {
			if constexpr (should_inline_vtable) {
				return source_vtable;
			} else {
				return *source_vtable;
			}
		}
		
		vtable_type source_vtable;
		void* source;
	};
}

namespace kangaru {
	struct any_source_of_one_ref;
	/**
	 * Wraps any sources that can provide Types and type erase it.
	 *
	 * It uses raw new and delete instead of unique pointers with a custom deleter because
	 * we need to place the deleter in the vtable which is held separately from the pointer.
	 */
	template<injectable... Types> requires pack_distinct<Types...>
	struct any_source_of : detail::polymorphic_source_private::any_source_of_base<Types...> {
	private:
		using base = detail::polymorphic_source_private::any_source_of_base<Types...>;
		using base::vtable;
		using base::source;
		using base::source_vtable;
		
	public:
		template<callable Function>
			requires(
				    not_self<detail::call_result_t<Function>, any_source_of>
				and kangaru::source<detail::call_result_t<Function>>
				and (... and source_of<detail::call_result_t<Function>, Types>)
			)
		explicit(false) constexpr any_source_of(in_place_construct<Function> source) noexcept :
			base{new detail::call_result_t<Function>(std::move(source))} {}
		
		template<not_self<any_source_of> Source>
			requires(
				    forwarded_source<Source>
				and (... and source_of<std::remove_cvref_t<Source>&, Types>)
			)
		explicit(false) constexpr any_source_of(Source&& source) :
			base{new std::decay_t<Source>(KANGARU5_FWD(source))} {
			static_assert(std::constructible_from<std::decay_t<Source>, Source>);
		}
		
		template<not_self<any_source_of> Source>
			requires(
				    forwarded_source<Source>
				and std::constructible_from<std::decay_t<Source>, Source>
				and (... and source_of<std::remove_cvref_t<Source>&, Types>)
			)
		constexpr auto operator=(Source&& rhs) -> any_source_of& {
			auto s = std::make_unique<std::decay_t<Source>>(KANGARU5_FWD(rhs));
			if (source) {
				vtable().destroy(source);
			}
			
			source_vtable = base::template get_vtable_for<std::remove_cvref_t<Source>>();
			source = s.release();
			return *this;
		}
		
		any_source_of(any_source_of const&) = delete;
		auto operator=(any_source_of const&) -> any_source_of& = delete;
		
		constexpr any_source_of(any_source_of&& other) noexcept :
			base{std::exchange(other.source_vtable, {}), std::exchange(other.source, nullptr)} {}
		
		constexpr auto operator=(any_source_of&& rhs) noexcept -> any_source_of& {
			if (this == &rhs) return *this;
			std::swap(source_vtable, rhs.source_vtable);
			std::swap(source, rhs.source);
			return *this;
		}
		
		constexpr ~any_source_of() noexcept {
			if (source) {
				vtable().destroy(source);
			}
		}
		
		template<injectable T, forwarded<any_source_of> Self> requires(... || std::same_as<Types, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(static_cast<detail::forward_like_t<Self, base>>(source));
		}
	};
	
	/**
	 * Wraps a reference to any sources that can provide `Types` and type erase it.
	 */
	template<injectable... Types> requires pack_distinct<Types...>
	struct any_source_of_ref : detail::polymorphic_source_private::any_source_of_base<Types...> {
	private:
		using base = detail::polymorphic_source_private::any_source_of_base<Types...>;
		
		// TODO: Ensure the friend is still needed
		friend struct any_source_of_one_ref;
		
		constexpr any_source_of_ref(
			void* source,
			detail::function_pointer_t<auto(void*) -> Types>... provide_function
		) noexcept :
			base{{.provide = {provide_function...}}, source} {}
		
	public:
		template<source Source> requires(... and source_of<Source&, Types>)
		explicit(false) constexpr any_source_of_ref(Source& source) :
			base{std::addressof(source)} {}
		
		template<reference_wrapper Source> requires(not_self<Source, any_source_of_ref>)
		explicit constexpr any_source_of_ref(Source source) noexcept
			requires (
				    not std::is_rvalue_reference_v<decltype(source.unwrap())>
				and (... and source_of<Source, Types>)
			) :
				base{std::addressof(source.unwrap())} {}
		
		template<injectable T, forwarded<any_source_of_ref> Self> requires(... || std::same_as<Types, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(static_cast<detail::forward_like_t<Self, base>>(source));
		}
	};
	/**
	 * A type erased source which completely hides the source type and provide type.
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
		 * The reason why this function is unsafe is to let users of this type store type information in a way
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
