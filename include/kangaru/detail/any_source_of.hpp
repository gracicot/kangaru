#ifndef KANGARU5_DETAIL_ANY_SOURCE_OF_HPP
#define KANGARU5_DETAIL_ANY_SOURCE_OF_HPP

#include "source.hpp"
#include "concepts.hpp"

#ifndef KANGARU5_MODULES
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
		template<forwarded_source Source> requires(not_self<Source, any_source_of> and ... and source_of<Source&, Types>)
		constexpr any_source_of(Source&& source) :
			source{new Source{KANGARU5_FWD(source)}},
			vtable{std::addressof(vtable_instance<std::remove_cvref_t<Source>>)} {}
		
		template<forwarded_source Source> requires(not_self<std::remove_cvref_t<Source>, any_source_of> and ... and source_of<Source&, Types>)
		auto operator=(Source&& rhs) -> any_source_of& {
			if (source) {
				vtable->destroy(source);
			}
			
			source = new Source{KANGARU5_FWD(rhs)};
			vtable = std::addressof(vtable_instance<std::remove_cvref_t<Source>>);
			return *this;
		}
		
		any_source_of(any_source_of const&) = delete;
		auto operator=(any_source_of const&) -> any_source_of& = delete;
		
		any_source_of(any_source_of&& other) noexcept : source{std::exchange(other.source, nullptr)}, vtable{std::exchange(other.vtable, nullptr)} {}
		
		auto operator=(any_source_of&& rhs) -> any_source_of& {
			std::swap(source, rhs.source);
			std::swap(vtable, rhs.vtable);
			return *this;
		}
		
		~any_source_of() noexcept {
			if (source) {
				vtable->destroy(source);
			}
		}
		
		template<injectable T, forwarded<any_source_of> Self> requires(... || std::same_as<Types, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return std::get<provide_function_ptr<T>>(KANGARU5_FWD(source).vtable->provide)(KANGARU5_FWD(source).source);
		}
		
	private:
		template<injectable T>
		using provide_function_ptr = auto(*)(void*) -> T;
		
		struct vtable_t {
			std::tuple<provide_function_ptr<Types>...> provide;
			auto(*destroy)(void const*) -> void;
			
			template<source Source>
			static constexpr auto init_for() -> vtable_t {
				return vtable_t{
					.provide = {
						[](void* source) KANGARU5_CONSTEXPR_VOIDSTAR -> Types {
							return kangaru::provide<Types>(*static_cast<Source*>(source));
						}...
					},
					.destroy = [](void const* source) {
						delete static_cast<Source const*>(source);
					}
				};
			}
		};
		
		template<source Source>
		inline static constexpr auto vtable_instance = vtable_t::template init_for<Source>();
		
		void* source;
		vtable_t const* vtable;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_ANY_SOURCE_OF_HPP
