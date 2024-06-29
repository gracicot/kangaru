#ifndef KANGARU5_DETAIL_INCREMENTAL_SOURCE_HPP
#define KANGARU5_DETAIL_INCREMENTAL_SOURCE_HPP

#include "source.hpp"
#include "type_traits.hpp"
#include "constructor.hpp"
#include "recursive_source.hpp"

namespace kangaru::sources {
	auto incremental(source auto source, auto&& first, auto&&... rest) {
		using result = decltype(kgr::spread_injector{ref(source)}(KANGARU5_FWD(first)));
		using result_source = detail::type_traits::conditional_t<
			std::is_lvalue_reference_v<result>,
			external_reference_source<std::remove_reference_t<result>>,
			reference_source<std::remove_reference_t<result>>
		>;
		
		auto injector = kgr::spread_injector{ref(source)}
		auto first_result = result_source{std::move(injector)(KANGARU5_FWD(first))};
		
		if constexpr (sizeof...(rest) > 0) {
			return incremental(
				concat(std::move(source), std::move(first_result)),
				KANGARU5_FWD(rest)...
			);
		} else {
			return first_result;
		}
	}
}
