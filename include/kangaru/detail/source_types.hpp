#ifndef KANGARU5_DETAIL_SOURCE_TYPES_HPP
#define KANGARU5_DETAIL_SOURCE_TYPES_HPP

#include "deducer.hpp"
#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"
#include "constructor.hpp"

#include <tuple>
#include <concepts>
#include <algorithm>
#include <type_traits>

#include "define.hpp"

namespace kangaru {
	template<source... Sources>
	struct composed_source {
		explicit constexpr composed_source(Sources... sources) noexcept : sources{std::move(sources)...} {}
		
		template<typename T>
		friend constexpr auto provide(provide_tag<T>, forwarded<composed_source> auto&& source) -> T
		requires (((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> ? 1 : 0) + ...) == 1) {
			constexpr auto index = index_of<T, decltype(source)>(std::index_sequence_for<Sources...>{});
			return provide(provide_tag_v<T>, std::get<index>(KANGARU5_FWD(source).sources));
		}
		
	private:
		template<typename T, typename Self, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			return ((source_of<detail::utility::forward_like_t<Self, Sources>, T> ? S : 0) + ...);
		}
		
		std::tuple<Sources...> sources;
	};
	
	inline constexpr auto concat(auto&&... sources) requires(... and source<std::remove_cvref_t<decltype(sources)>>) {
		return composed_source{KANGARU5_FWD(sources)...};
	}
	
	template<source... Sources>
	struct select_first_source {
		explicit constexpr select_first_source(std::tuple<Sources...> sources) noexcept : sources{std::move(sources)} {}
		
		template<typename T>
		friend constexpr auto provide(provide_tag<T>, forwarded<select_first_source> auto&& source) -> T
		requires ((source_of<detail::utility::forward_like_t<decltype(source), Sources>, T> or ...)) {
			constexpr auto index = index_of<T, decltype(source)>(std::index_sequence_for<Sources...>{});
			return provide(provide_tag_v<T>, std::get<index>(KANGARU5_FWD(source).sources));
		}
		
	private:
		template<typename T, typename Self, std::size_t... S>
		constexpr static auto index_of(std::index_sequence<S...>) {
			auto const source_handles = std::array{std::pair{S, source_of<detail::utility::forward_like_t<Self, Sources>, T>}...};
			
			auto const it = std::find_if(source_handles.begin(), source_handles.end(), [](std::pair<std::size_t, bool> source) {
				return source.second;
			});
			
			return std::distance(source_handles.begin(), it);
		}
		
		std::tuple<Sources...> sources;
	};
	
	template<object... Ts>
	struct tuple_source {
		explicit constexpr tuple_source(std::tuple<Ts...> objects) noexcept : objects{std::move(objects)} {}
		
		template<typename T> requires (... or std::same_as<T, Ts>)
		friend constexpr auto provide(provide_tag<T>, forwarded<tuple_source> auto&& source) -> T {
			return std::get<T>(KANGARU5_FWD(source).objects);
		}
		
	private:
		std::tuple<Ts...> objects;
	};
	
	template<unqualified_object F> requires std::invocable<F>
	struct function_source {
		explicit constexpr function_source(F function) noexcept : function{std::move(function)} {}
	
	private:
		using T = decltype(std::declval<F>()());
		
		friend constexpr auto provide(provide_tag<T>, forwarded<function_source> auto&& source) -> T {
			return KANGARU5_FWD(source).function();
		}
		
		F function;
	};
	
	template<unqualified_object T>
	struct object_source {
		explicit constexpr object_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(provide_tag<T>, forwarded<object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_object_source {
		injectable_object_source(deducer auto... deduce) requires std::constructible_from<T, decltype(deduce)...> :
			object{deduce...} {}
		
		friend constexpr auto provide(provide_tag<T>, forwarded<injectable_object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_reference_source {
		template<deducer... Deducer> requires std::constructible_from<T, Deducer...>
		injectable_reference_source(Deducer... deduce) :
			object{deduce...} {}
		
		friend constexpr auto provide(provide_tag<T&>, injectable_reference_source& source) -> T& {
			return source.object;
		}
		
		friend constexpr auto provide(provide_tag<T&>, injectable_reference_source&& source) -> T& {
			return source.object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_rvalue_source {
		injectable_rvalue_source(deducer auto... deduce) requires std::constructible_from<T, decltype(deduce)...> :
			object{deduce...} {}
		
		friend constexpr auto provide(provide_tag<T&&>, injectable_rvalue_source& source) -> T&& {
			return std::move(source).object;
		}
		
		friend constexpr auto provide(provide_tag<T&&>, injectable_rvalue_source&& source) -> T&& {
			return std::move(source).object;
		}
		
	private:
		T object;
	};
	
	template<object T>
	struct rvalue_source {
		explicit constexpr rvalue_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(provide_tag<T&&>, rvalue_source& source) -> T&& {
			return std::move(source.object);
		}
		
		friend constexpr auto provide(provide_tag<T&&>, rvalue_source&& source) -> T&& {
			return std::move(source.object);
		}
		
	private:
		T object;
	};
	
	template<object T>
	struct external_rvalue_source {
		explicit constexpr external_rvalue_source(T&& reference) noexcept : reference{std::addressof(reference)} {}
		
		friend constexpr auto provide(provide_tag<T&&>, external_rvalue_source const& source) -> T&& {
			return std::move(*source.reference);
		}
		
	private:
		T* reference;
	};
	
	template<object T>
	struct external_reference_source {
		explicit constexpr external_reference_source(T& reference) noexcept : reference{std::addressof(reference)} {}
		
		friend constexpr auto provide(provide_tag<T&>, external_reference_source const& source) -> T& {
			return *source.reference;
		}
		
	private:
		T* reference;
	};
	
	template<object T>
	struct reference_source {
		explicit constexpr reference_source(T object) noexcept : object{std::move(object)} {}
		
		friend constexpr auto provide(provide_tag<T&>, reference_source& source) -> T& {
			return source.object;
		}
		
		friend constexpr auto provide(provide_tag<T&>, reference_source&& source) -> T& {
			return source.object;
		}
		
	private:
		T object;
	};
	
	template<wrapping_source Source, typename Filtered>
	struct with_filter_passthrough {
		explicit constexpr with_filter_passthrough(Source source) noexcept : source{std::move(source)} {}
		
		template<different_from<Filtered> T, forwarded<with_filter_passthrough> Self> requires source_of<wrapped_source_t<Self>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, KANGARU5_FWD(source).source);
		}
		
		template<forwarded<with_filter_passthrough> Self> requires wrapping_source_of<detail::utility::forward_like_t<Self, Source>, Filtered>
		friend constexpr auto provide(provide_tag<Filtered> tag, Self&& source) -> Filtered {
			return provide(tag, KANGARU5_FWD(source).source.source);
		}
		
		Source source;
	};
	
	template<typename T, typename Source> requires source<std::remove_reference_t<Source>>
	constexpr auto make_source_with_filter_passthrough(Source&& source) {
		return with_filter_passthrough<std::remove_cvref_t<Source>, T>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct source_reference_wrapper {
		constexpr source_reference_wrapper(Source& source) noexcept : source{std::addressof(source)} {}
		
		template<typename T>
		friend constexpr auto provide(provide_tag<T>, source_reference_wrapper const& source) -> T
		requires source_of<Source&, T> {
			return provide(provide_tag_v<T>, *source.source);
		}
		
		[[nodiscard]]
		constexpr auto unwrap() const noexcept -> Source& {
			return *source;
		}
		
	private:
		Source* source;
	};
	
	template<source Source>
	struct source_const_reference_wrapper {
		constexpr source_const_reference_wrapper(Source const& source) noexcept : source{std::addressof(source)} {}
		constexpr source_const_reference_wrapper(source_reference_wrapper<Source> const& source) noexcept : source{source.unwrap()} {}
		
		template<typename T> requires source_of<Source const&, T>
		friend constexpr auto provide(provide_tag<T>, source_const_reference_wrapper const& source) -> T {
			return provide(provide_tag_v<T>, *source.source);
		}
		
		[[nodiscard]]
		constexpr auto unwrap() const noexcept -> Source const& {
			return *source;
		}
		
	private:
		Source const* source;
	};
	
	template<source Source, typename Type>
	struct filter_source {
		constexpr filter_source(Source source) noexcept : source{std::move(source)} {}
		
	private:
		template<different_from<Type> T>
		friend constexpr auto provide(provide_tag<T>, forwarded<filter_source> auto const& source) -> T {
			return provide(provide_tag_v<T>, source.source);
		}
		
		Source source;
	};
	
	template<source Source, std::default_initializable Filter>
	struct filter_if_source {
		constexpr filter_if_source(Source source) noexcept : source{std::move(source)} {}
		
	private:
		template<typename T> requires(requires { requires Filter{}.template operator()<T>(); })
		friend constexpr auto provide(provide_tag<T>, forwarded<filter_if_source> auto const& source) -> T {
			return provide(provide_tag_v<T>, source.source);
		}
		
		Source source;
	};
	
	template<typename T, typename Source>
	inline constexpr auto filter(Source&& source) -> filter_source<std::decay_t<Source>, T> {
		return filter_source<std::decay_t<Source>, T>{KANGARU5_FWD(source)};
	}
	
	template<typename Source, typename Filter>
	inline constexpr auto filter_if(Source&& source, [[maybe_unused]] Filter filter) -> filter_if_source<std::decay_t<Source>, Filter> {
		return filter_if_source<std::decay_t<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct source_reference_wrapper_for {
		using type = source_reference_wrapper<Source>;
	};
	
	template<source Source>
	struct source_reference_wrapper_for<Source const> {
		using type = source_const_reference_wrapper<Source>;
	};
	
	template<source Source>
	using source_reference_wrapper_for_t = typename source_reference_wrapper_for<Source>::type;
	
	template<typename T>
	concept reference_wrapper = source<T> and requires(T ref) {
		{ ref.unwrap() } -> reference;
	};
	
	template<typename Wrapper, typename T>
	concept reference_wrapper_for = requires(Wrapper const& ref) {
		{ ref.unwrap() } -> std::same_as<T&>;
	};
	
	template<reference_wrapper Wrapper>
	using source_reference_wrapped_type = std::remove_reference_t<decltype(std::declval<Wrapper>().unwrap())>;
	
	template<typename Source> requires source<std::remove_const_t<Source>>
	inline constexpr auto ref(Source& source) -> source_reference_wrapper_for_t<Source> {
		static_assert(reference_wrapper_for<source_reference_wrapper_for_t<Source>, Source>);
		return source_reference_wrapper_for_t<Source>{source};
	}
	
	inline constexpr auto tie(source auto&... sources) {
		return kangaru::concat(kangaru::ref(sources)...);
	}
	
	template<source Source>
	struct with_reference_passthrough {
		explicit constexpr with_reference_passthrough(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<object T, forwarded<with_reference_passthrough> Self>
			requires source_of<detail::utility::forward_like_t<Self, Source>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, KANGARU5_FWD(source).source);
		}
		
		template<reference T, forwarded<with_reference_passthrough> Self>
			requires source_of<detail::utility::forward_like_t<Self, decltype(std::declval<Source>().source)>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			return provide(tag, KANGARU5_FWD(source).source.source);
		}
	};
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	inline constexpr auto make_source_with_reference_passthrough(Source&& source) noexcept {
		return with_reference_passthrough<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_dereference {
		explicit constexpr with_dereference(Source source) noexcept : source{std::move(source)} {}
		
		Source source;
		
		template<typename T> requires source_of<Source, T*>
		friend constexpr auto provide(provide_tag<T&>, forwarded<with_dereference> auto&& source) -> T& {
			return *provide(provide_tag_v<T*>, KANGARU5_FWD(source).source);
		}
		
		template<typename T> requires (not std::is_pointer_v<T> and source_of<Source, T>)
		friend constexpr auto provide(provide_tag<T>, forwarded<with_dereference> auto&& source) -> T {
			return provide(provide_tag_v<T>, KANGARU5_FWD(source).source);
		}
	};
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	inline constexpr auto make_source_with_dereference(Source&& source) {
		return with_dereference<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct basic_wrapping_source {
		template<typename T, forwarded<basic_wrapping_source> Self> requires source_of<wrapped_source_t<Self>, T>
		friend constexpr auto provide(provide_tag<T>, Self&& source) -> T {
			return provide(provide_tag_v<T>, std::forward<Self>(source).source);
		}
		
		Source source;
	};
	
	template<typename Source> requires source<std::remove_cvref_t<Source>>
	constexpr auto wrap_source(Source&& source) {
		return basic_wrapping_source<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TYPES_HPP
