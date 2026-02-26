#ifndef KANGARU5_DETAIL_CONTAINER_COMMON_HPP
#define KANGARU5_DETAIL_CONTAINER_COMMON_HPP

#include "attributes.hpp"
#include "source_types.hpp"
#include "source.hpp"
#include "exceptions.hpp"
#include "source_traits.hpp"

#ifndef KANGARU5_MODULES
#include <cstdlib>
#include <exception>
#endif

#include "define.hpp"

namespace kangaru::detail::container_common {
	// TODO: Prevent duplicate?
	template<injectable, kangaru::source>
	struct enumerated_select_source_of {};
	
	template<injectable T, kangaru::source Source, kangaru::source... Sources>
		requires(requires { typename select_source_of<T, Sources...>; })
	struct enumerated_select_source_of<T, enumerated_source_of<Source, Sources...>> {
		using type = select_source_of<T, Sources...>;
	};
	
	template<injectable T, kangaru::source EnumeratedSource>
	using enumerated_select_source_of_t = typename enumerated_select_source_of<T, EnumeratedSource>::type;
}

KANGARU5_EXPORT namespace kangaru {
	struct throw_if_not_found {
		template<injectable T> requires(assume_runtime_cached_v<T>)
		[[noreturn]]
		constexpr auto provide() const -> T {
			throw not_found_exception{};
		}
	};
	
	struct abort_if_not_found {
		template<injectable T> requires(assume_runtime_cached_v<T>)
		[[noreturn]]
		constexpr auto provide() const -> T {
			std::abort();
		}
	};
	
	struct terminate_if_not_found {
		template<injectable T> requires(assume_runtime_cached_v<T>)
		[[noreturn]]
		constexpr auto provide() const -> T {
			std::terminate();
		}
	};
}

#include "undef.hpp"

#endif // # KANGARU5_DETAIL_SOURCE_TYPES_HPP
