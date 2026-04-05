#ifndef KANGARU5_DETAIL_INCREMENTAL_SOURCE_HPP
#define KANGARU5_DETAIL_INCREMENTAL_SOURCE_HPP

#include "source.hpp"
#include "source_types.hpp"
#include "utility.hpp"

#ifndef KANGARU5_MODULES
#include <utility>
#endif

#include "define.hpp"

namespace kangaru::detail::incremental_source_private {
	template<typename Function, typename Context>
	concept callable_incremental_source_initializer =
		    source<Context>
		and function_object<Function>
		and callable<Function&&, Context>
		and source<detail::call_result_t<Function&&, Context>>;
	
	template<source Constructed, function_object... Modules>
	inline constexpr auto incremental_source_complete_v = false;
	
	template<source... Constructed, function_object Head, function_object... Tail>
		requires(callable_incremental_source_initializer<Head, composed_source<Constructed...>>)
	inline constexpr auto incremental_source_complete_v<composed_source<Constructed...>, Head, Tail...> =
		    callable_incremental_source_initializer<Head, composed_source<Constructed...>>
		and incremental_source_complete_v<composed_source<Constructed..., source_reference_wrapper<detail::call_result_t<Head, composed_source<Constructed...>>>>, Tail...>;
	
	template<source... Constructed>
	inline constexpr auto incremental_source_complete_v<composed_source<Constructed...>> = true;
} // namespace kangaru::detail::incremental_source_private

KANGARU5_EXPORT namespace kangaru {
	template<typename... Functions>
		requires(
			   detail::incremental_source_private::incremental_source_complete_v<Functions...>
			or detail::incremental_source_private::incremental_source_complete_v<composed_source<>, Functions...>
		)
	struct incremental_source;
	
	template<source... Constructed, function_object MakeSource, function_object... Next>
		requires detail::incremental_source_private::incremental_source_complete_v<composed_source<Constructed...>, MakeSource, Next...>
	struct incremental_source<composed_source<Constructed...>, MakeSource, Next...> {
	private:
		using source_t = detail::call_result_t<MakeSource, composed_source<Constructed...>>;
		using constructed_t = composed_source<Constructed..., source_reference_wrapper<source_t>>;
		using next_t = incremental_source<constructed_t, Next...>;
		
	public:
		incremental_source(incremental_source const&) = delete;
		auto operator=(incremental_source const&) -> incremental_source& = delete;
		incremental_source(incremental_source&&) = delete;
		auto operator=(incremental_source&&) -> incremental_source& = delete;
		~incremental_source() = default;
		
		constexpr incremental_source(composed_source<Constructed...> accumulated, MakeSource make_source, Next... next) :
			source{std::move(make_source)(accumulated)},
			next{KANGARU5_NO_ADL(composed_source_cat)(accumulated, KANGARU5_NO_ADL(tie)(source)), next...} {}
		
		template<injectable T, forwarded<incremental_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<incremental_source> Self> requires source_of<detail::forward_like_t<Self, next_t>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).next);
		}
		
		source_t source;
		next_t next;
	};
	
	template<function_object MakeSource, function_object... Next>
		requires detail::incremental_source_private::incremental_source_complete_v<composed_source<>, MakeSource, Next...>
	struct incremental_source<MakeSource, Next...> {
	private:
		using source_t = detail::call_result_t<MakeSource, composed_source<>>;
		using constructed_t = tied_source<source_t>;
		using next_t = incremental_source<constructed_t, Next...>;
		
	public:
		incremental_source(incremental_source const&) = delete;
		auto operator=(incremental_source const&) -> incremental_source& = delete;
		incremental_source(incremental_source&&) = delete;
		auto operator=(incremental_source&&) -> incremental_source& = delete;
		~incremental_source() = default;
		
		constexpr incremental_source(MakeSource make_source, Next... next) :
			source{std::move(make_source)(tie())},
			next{KANGARU5_NO_ADL(tie)(source), std::move(next)...} {}
		
		template<injectable T, forwarded<incremental_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<incremental_source> Self> requires source_of<detail::forward_like_t<Self, next_t>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).next);
		}
		
		source_t source;
		next_t next;
	};
	
	template<source... Constructed, function_object MakeSource>
		requires detail::incremental_source_private::incremental_source_complete_v<composed_source<Constructed...>, MakeSource>
	struct incremental_source<composed_source<Constructed...>, MakeSource> {
	private:
		using source_t = detail::call_result_t<MakeSource, composed_source<Constructed...>>;
		
	public:
		constexpr incremental_source(composed_source<Constructed...> accumulated, MakeSource make_head) :
			source{std::move(make_head)(accumulated)} {}
		
		template<injectable T, forwarded<incremental_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		source_t source;
	};
	
	template<function_object Head>
		requires detail::incremental_source_private::incremental_source_complete_v<composed_source<>, Head>
	struct incremental_source<Head> {
	private:
		using source_t = detail::call_result_t<Head, composed_source<>>;
		
	public:
		explicit constexpr incremental_source(Head make_head) :
			source{std::move(make_head)(tie())} {}
		
		template<injectable T, forwarded<incremental_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		source_t source;
	};
	
	template<source... Constructed>
	struct incremental_source<composed_source<Constructed...>> {
		explicit constexpr incremental_source(composed_source<Constructed...>) {}
	};
	
	template<>
	struct incremental_source<> {};
	
	template<typename... Functions>
	incremental_source(Functions...) -> incremental_source<Functions...>;
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU5_DETAIL_INCREMENTAL_SOURCE_HPP
