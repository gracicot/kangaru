#ifndef KANGARU5_DETAIL_ANY_SOURCE_OF_HPP
#define KANGARU5_DETAIL_ANY_SOURCE_OF_HPP

#include "source.hpp"
#include "concepts.hpp"
#include "type_traits.hpp"
#include "constructor.hpp"

#ifndef KANGARU5_MODULES
#include <new>
#include <utility>
#include <memory>
#include <tuple>
#include <concepts>
#endif

#include "define.hpp"

KANGARU5_EXPORT namespace kangaru {
	/**
	 * Wraps any sources that can provide Types and type erase it.
	 *
	 * It uses raw new and delete instead of unique pointers with a custom deleter because
	 * we need to place the deleter in the vtable which is held separately from the pointer.
	 */
	template<injectable... Types> requires pack_distinct<Types...>
	struct any_source_of {
		template<callable Function>
			requires(
				    not_self<detail::type_traits::call_result_t<Function>, any_source_of>
				and ... and source_of<detail::type_traits::call_result_t<Function>, Types>
			)
		explicit(false) constexpr any_source_of(in_place_construct<Function> source) :
			source_vtable{get_vtable_for<detail::type_traits::call_result_t<Function>>()},
			source{new detail::type_traits::call_result_t<Function>(std::move(source))} {}
		
		template<not_self<any_source_of> Source> requires(forwarded_source<Source> and ... and source_of<Source&, Types>)
		explicit(false) constexpr any_source_of(Source&& source) :
			source_vtable{get_vtable_for<std::remove_cvref_t<Source>>()},
			source{new Source{KANGARU5_FWD(source)}} {}
		
		template<not_self<any_source_of> Source> requires(forwarded_source<Source> and ... and source_of<Source&, Types>)
		constexpr auto operator=(Source&& rhs) -> any_source_of& {
			if (source) {
				vtable().destroy(source);
			}
			
			source = new Source{KANGARU5_FWD(rhs)};
			source_vtable = get_vtable_for<std::remove_cvref_t<Source>>();
			return *this;
		}
		
		any_source_of(any_source_of const&) = delete;
		auto operator=(any_source_of const&) -> any_source_of& = delete;
		
		constexpr any_source_of(any_source_of&& other) noexcept :
			source_vtable{std::exchange(other.source_vtable, {})},
			source{std::exchange(other.source, nullptr)} {}
		
		constexpr auto operator=(any_source_of&& rhs) -> any_source_of& {
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
			return std::get<provide_function_ptr<T>>(source.vtable().provide)(source.source);
		}
		
	private:
		template<injectable T>
		using provide_function_ptr = auto(*)(void*) -> T;
		
		struct vtable_t {
			std::tuple<provide_function_ptr<Types>...> provide;
			auto(*destroy)(void const*) -> void;
			
			template<source Source>
			static consteval auto init_for() -> vtable_t {
				return vtable_t{
					.provide = {
						[](void* source) KANGARU5_CONSTEXPR_VOIDSTAR -> Types {
							return kangaru::provide<Types>(*static_cast<Source*>(source));
						}...
					},
					.destroy = [](void const* source) KANGARU5_CONSTEXPR_VOIDSTAR {
						delete static_cast<Source const*>(source);
					}
				};
			}
		};
		
		static constexpr auto should_inline_vtable = sizeof...(Types) < 4;
		using vtable_type = detail::type_traits::conditional_t<should_inline_vtable, vtable_t, vtable_t const*>;
		
		template<source Source>
		static consteval auto get_vtable_for() {
			if constexpr (should_inline_vtable) {
				return vtable_t::template init_for<Source>();
			} else {
				return std::addressof(vtable_instance<Source>);
			}
		}
		
		template<source Source>
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

#include "undef.hpp"

#endif // KANGARU5_DETAIL_ANY_SOURCE_OF_HPP
