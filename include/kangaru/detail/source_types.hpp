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
		injectable_object_source(deducer auto&&... deduce) requires std::constructible_from<T, decltype(deduce)...> :
			object{deduce...} {}
		
		friend constexpr auto provide(provide_tag<T>, forwarded<injectable_object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
	};
	
	template<unqualified_object T>
	struct injectable_reference_source {
		injectable_reference_source(deducer auto&&... deduce) requires std::constructible_from<T, decltype(deduce)...> :
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
		injectable_rvalue_source(deducer auto&&... deduce) requires std::constructible_from<T, decltype(deduce)...> :
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
	
	template<typename Source, typename Type>
	struct filter_source {
		constexpr filter_source(Source source) noexcept : source{std::move(source)} {}
		
	private:
		template<different_from<Type> T>
		friend constexpr auto provide(provide_tag<T>, forwarded<filter_source> auto const& source) -> T {
			return provide(provide_tag_v<T>, source.source);
		}
		
		Source source;
	};
	
	template<typename Source, std::default_initializable Filter>
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
	
	inline constexpr auto concat(auto&&... sources) requires(... and source<std::remove_cvref_t<decltype(sources)>>) {
		return composed_source{KANGARU5_FWD(sources)...};
	}
	
	template<source Source>
	struct source_reference_wrapper_for {
		using type = source_reference_wrapper<Source>;
	};
	
	template<source Source>
	using source_reference_wrapper_for_t = typename source_reference_wrapper_for<Source>::type;
	
	template<source Source>
	inline constexpr auto ref(Source& source) -> source_reference_wrapper_for_t<Source> {
		static_assert(requires(source_reference_wrapper_for_t<Source>& ref) {
			{ ref.unwrap() } -> std::same_as<Source&>;
		});
		return source_reference_wrapper_for_t<Source>{source};
	}
	
	inline constexpr auto tie(source auto&... sources) {
		return concat(ref(sources)...);
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TYPES_HPP
