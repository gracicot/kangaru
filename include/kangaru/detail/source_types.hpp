#ifndef KANGARU5_DETAIL_SOURCE_TYPES_HPP
#define KANGARU5_DETAIL_SOURCE_TYPES_HPP

#include "deducer.hpp"
#include "exceptions.hpp"
#include "two_step_init.hpp"
#include "source_traits.hpp"
#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"
#include "constructor.hpp"
#include "source_reference_wrapper.hpp"
#include "attributes.hpp"
#include "source_rebind.hpp"

#ifndef KANGARU5_MODULES
#include <tuple>
#include <concepts>
#include <algorithm>
#include <type_traits>
#include <functional>
#endif

#include "define.hpp"

namespace kangaru::detail::source_types_private {
	struct from_tuple_t {
		explicit constexpr from_tuple_t() = default;
	} inline constexpr from_tuple{};
	
	struct composed_source_access;
}

namespace kangaru {
	template<>
	struct allow_injection_using<detail::source_types_private::from_tuple_t> : std::false_type {};
	
	KANGARU5_EXPORT template<source... Sources>
	struct composed_source {
		// Constructor only needed for GCC.
		explicit(sizeof...(Sources) == 1)
		constexpr composed_source(Sources... sources)
		requires(
			... and (
				    not detail::is_specialisation_of_v<in_place_construct, Sources>
				and not deducer<Sources>
				and std::move_constructible<Sources>
			)
		) :
			sources{std::move(sources)...} {}
		
		template<typename... S>
			requires(
				    sizeof...(S) == sizeof...(Sources)
				and (... and std::constructible_from<Sources, S&&>)
			)
		explicit(sizeof...(S) == 1)
		constexpr composed_source(S&&... sources) : sources{std::forward_as_tuple(KANGARU5_FWD(sources)...)} {}
		
		template<injectable T, forwarded<composed_source> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T
		requires (
			((source_of<detail::forward_like_t<Self, Sources>, T> ? 1 : 0) + ... + 0) == 1
		) {
			constexpr auto index = select_source_of_index<T, detail::forward_like_t<Self, Sources>...>;
			return kangaru::provide<T>(std::get<index>(KANGARU5_FWD(source).sources));
		}
		
		template<injectable T, forwarded<composed_source> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T
		requires ("Ambiguous source resolution: One or more source can provide type T",
			((source_of<detail::forward_like_t<Self, Sources>, T> ? 1 : 0) + ... + 0) > 1
		) = delete;
		
	private:
		friend detail::source_types_private::composed_source_access;
		
		template<typename... S>
			requires(
				sizeof...(S) == sizeof...(Sources)
				and (... and std::constructible_from<Sources, S&&>)
			)
		explicit constexpr composed_source(detail::source_types_private::from_tuple_t, std::tuple<S...>&& sources) :
			sources{std::move(sources)} {}
		
		std::tuple<Sources...> sources;
	};
	
	template<typename... Sources>
		requires(... and (not deducer<std::remove_cvref_t<Sources>>))
	composed_source(Sources&&...) -> composed_source<deduced_source_type<Sources>...>;
}

namespace kangaru::detail::source_types_private {
	struct composed_source_access {
		template<source... SourcesLhs, source... SourcesRhs>
		static constexpr auto composed_source_cat(composed_source<SourcesLhs...> const& lhs, composed_source<SourcesRhs...> const& rhs) -> composed_source<SourcesLhs..., SourcesRhs...> {
			return composed_source<SourcesLhs..., SourcesRhs...>{from_tuple, std::tuple_cat(lhs.sources, rhs.sources)};
		}
		
		template<source... SourcesLhs, source... SourcesRhs>
		static constexpr auto composed_source_cat(composed_source<SourcesLhs...> const& lhs, composed_source<SourcesRhs...>&& rhs) -> composed_source<SourcesLhs..., SourcesRhs...> {
			return composed_source<SourcesLhs..., SourcesRhs...>{from_tuple, std::tuple_cat(lhs.sources, std::move(rhs).sources)};
		}
		
		template<source... SourcesLhs, source... SourcesRhs>
		static constexpr auto composed_source_cat(composed_source<SourcesLhs...>&& lhs, composed_source<SourcesRhs...> const& rhs) -> composed_source<SourcesLhs..., SourcesRhs...> {
			return composed_source<SourcesLhs..., SourcesRhs...>{from_tuple, std::tuple_cat(std::move(lhs).sources, rhs.sources)};
		}
		
		template<source... SourcesLhs, source... SourcesRhs>
		static constexpr auto composed_source_cat(composed_source<SourcesLhs...>&& lhs, composed_source<SourcesRhs...>&& rhs) -> composed_source<SourcesLhs..., SourcesRhs...> {
			return composed_source<SourcesLhs..., SourcesRhs...>{from_tuple, std::tuple_cat(std::move(lhs).sources, std::move(rhs).sources)};
		}
	};
}

KANGARU5_EXPORT namespace kangaru {
	template<source... SourcesLhs, source... SourcesRhs>
	inline constexpr auto composed_source_cat(composed_source<SourcesLhs...> const& lhs, composed_source<SourcesRhs...> const& rhs) -> composed_source<SourcesLhs..., SourcesRhs...> {
		return detail::source_types_private::composed_source_access::composed_source_cat(lhs, rhs);
	}
	
	template<source... SourcesLhs, source... SourcesRhs>
	inline constexpr auto composed_source_cat(composed_source<SourcesLhs...> const& lhs, composed_source<SourcesRhs...>&& rhs) -> composed_source<SourcesLhs..., SourcesRhs...> {
		return detail::source_types_private::composed_source_access::composed_source_cat(lhs, std::move(rhs));
	}
	
	template<source... SourcesLhs, source... SourcesRhs>
	inline constexpr auto composed_source_cat(composed_source<SourcesLhs...>&& lhs, composed_source<SourcesRhs...> const& rhs) -> composed_source<SourcesLhs..., SourcesRhs...> {
		return detail::source_types_private::composed_source_access::composed_source_cat(std::move(lhs), rhs);
	}
	
	template<source... SourcesLhs, source... SourcesRhs>
	inline constexpr auto composed_source_cat(composed_source<SourcesLhs...>&& lhs, composed_source<SourcesRhs...>&& rhs) -> composed_source<SourcesLhs..., SourcesRhs...> {
		return detail::source_types_private::composed_source_access::composed_source_cat(std::move(lhs), std::move(rhs));
	}
	
	template<source Lhs, source Rhs>
	using composed_source_cat_t = decltype(KANGARU5_NO_ADL(composed_source_cat)(std::declval<Lhs>(), std::declval<Rhs>()));
	
	inline constexpr auto compose(forwarded_source auto&&... sources) {
		return composed_source<deduced_source_type<decltype(sources)>...>{KANGARU5_FWD(sources)...};
	}
	
	template<object... Ts>
	struct tuple_source {
		explicit constexpr tuple_source(std::tuple<Ts...> objects) : objects{std::move(objects)} {}
		
		template<typename... From>
			requires(
				    sizeof...(From) == sizeof...(Ts)
				and (... and std::convertible_to<From&&, Ts>)
			)
		explicit constexpr tuple_source(std::tuple<From...> objects) : objects{std::move(objects)} {}
		
		template<injectable T> requires(... or std::same_as<T, Ts>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<tuple_source> auto&& source) -> T {
			return std::get<T>(KANGARU5_FWD(source).objects);
		}
		
	private:
		std::tuple<Ts...> objects;
	};
	
	template<callable F> requires(unqualified_object<F> and function_object<F> and injectable<detail::call_result_t<F>>)
	struct function_source {
		constexpr function_source() requires(std::default_initializable<F>) : function{} {}
		
		explicit constexpr function_source(F function) : function{std::move(function)} {}
		
		template<forwarded<function_source> Self>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> detail::call_result_t<detail::forward_like_t<Self, F>> {
			return KANGARU5_FWD(source).function();
		}
		
	private:
		F function;
	};
	
	template<injectable T>
	using dynamic_function_source = function_source<std::function<T()>>;
	
	template<unqualified_object T>
	struct object_source {
		template<not_self<object_source> From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr object_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires((... and not_self<Args, object_source>) and constructor_callable<T, Args&&...>)
		constexpr object_source(Args&&... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<object_source> auto&& source) -> T {
			return KANGARU5_FWD(source).object;
		}
		
	private:
		T object;
		
		template<unqualified_object U>
		friend auto attribute(allow_empty_injection<object_source<U>>) -> std::true_type;
		
		template<unqualified_object U>
		friend auto attribute(allow_runtime_caching<object_source<U>>) -> std::bool_constant<allow_runtime_caching_v<U>>;
		
		template<unqualified_object U>
		friend auto attribute(assume_runtime_cached<object_source<U>>) -> std::bool_constant<assume_runtime_cached_v<U>>;
		
		template<kangaru::object U>
		friend auto attribute(second_step_init<object_source<U>>) -> call_second_step_from_attribute_on_member<&object_source<U>::object>;
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	object_source(T&&) -> object_source<deduced_source_type<T>>;
	
	template<object T>
	struct rvalue_source {
		template<not_self<rvalue_source> From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr rvalue_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires((... and not_self<Args, rvalue_source>) and constructor_callable<T, Args&&...>)
		constexpr rvalue_source(Args&&... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr auto provide() & -> T&& {
			return std::move(object);
		}
		
		constexpr auto provide() && -> T&& {
			return std::move(object);
		}
		
	private:
		T object;
		
		template<kangaru::object U>
		friend auto attribute(allow_empty_injection<rvalue_source<U>>) -> std::true_type;
		
		template<kangaru::object U>
		friend auto attribute(allow_runtime_caching<rvalue_source<U>>) -> std::bool_constant<allow_runtime_caching_v<U&&>>;
		
		template<kangaru::object U>
		friend auto attribute(assume_runtime_cached<rvalue_source<U>>) -> std::bool_constant<assume_runtime_cached_v<U&&>>;
		
		template<kangaru::object U>
		friend auto attribute(second_step_init<rvalue_source<U>>) -> call_second_step_from_attribute_on_member<&rvalue_source<U>::object, U&&>;
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	rvalue_source(T&&) -> rvalue_source<deduced_source_type<T>>;
	
	template<object T>
	struct reference_source {
		template<not_self<reference_source> From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr reference_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires((... and not_self<Args, reference_source>) and constructor_callable<T, Args&&...>)
		constexpr reference_source(Args&&... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr auto provide() & -> T& {
			return object;
		}
		
		constexpr auto provide() && -> T& {
			return object;
		}
		
	private:
		T object;
		
		template<kangaru::object U>
		friend auto attribute(allow_empty_injection<reference_source<U>>) -> std::true_type;
		
		template<kangaru::object U>
		friend auto attribute(allow_runtime_caching<reference_source<U>>) -> std::bool_constant<allow_runtime_caching_v<U&>>;
		
		template<kangaru::object U>
		friend auto attribute(assume_runtime_cached<reference_source<U>>) -> std::bool_constant<assume_runtime_cached_v<U&>>;
		
		template<kangaru::object U>
		friend auto attribute(second_step_init<reference_source<U>>) -> call_second_step_from_attribute_on_member<&reference_source<U>::object, U&>;
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	reference_source(T&&) -> reference_source<deduced_source_type<T>>;
	
	template<object T>
	struct shared_pointer_source {
		template<not_self<shared_pointer_source> From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr shared_pointer_source(From&& object) :
			object{
				std::make_shared<T>(
					KANGARU5_FWD(object)
				)
			} {}
		
		template<typename... Args> requires((... and not_self<Args, shared_pointer_source>) and constructor_callable<T, Args&&...>)
		constexpr shared_pointer_source(Args&&... args) :
			object{
				std::make_shared<T>(
					KANGARU5_NO_ADL(construct_in_place<T>)(KANGARU5_FWD(args)...)
				)
			} {}
		
		constexpr auto provide() const& -> std::shared_ptr<T> {
			return object;
		}
		
		constexpr auto provide() && -> std::shared_ptr<T> {
			return object;
		}
		
	private:
		std::shared_ptr<T> object;
		
		template<kangaru::object U>
		friend auto attribute(allow_empty_injection<shared_pointer_source<U>>) -> std::true_type;
		
		template<kangaru::object U>
		friend auto attribute(allow_runtime_caching<shared_pointer_source<U>>) -> std::bool_constant<allow_runtime_caching_v<std::shared_ptr<U>>>;
		
		template<kangaru::object U>
		friend auto attribute(assume_runtime_cached<shared_pointer_source<U>>) -> std::bool_constant<assume_runtime_cached_v<std::shared_ptr<U>>>;
		
		template<kangaru::object U>
		friend auto attribute(second_step_init<shared_pointer_source<U>>) -> call_second_step_from_attribute_on_member<&shared_pointer_source<U>::object>;
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	shared_pointer_source(T&&) -> shared_pointer_source<deduced_source_type<T>>;
	
	template<object T>
	struct external_reference_source {
		explicit constexpr external_reference_source(T& reference) : reference{std::addressof(reference)} {}
		
		constexpr auto provide() const& -> T& {
			return *reference;
		}
		
	private:
		T* reference;
	};
	
	template<object T>
	struct external_rvalue_source {
		explicit constexpr external_rvalue_source(T&& reference) : reference{std::addressof(reference)} {}
		
		constexpr auto provide() const& -> T&& {
			return std::move(*reference);
		}
		
	private:
		T* reference;
	};
	
	template<object Base, std::derived_from<Base> T>
	struct derived_reference_source {
		template<not_self<derived_reference_source> From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr derived_reference_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires((... and not_self<Args, derived_reference_source>) and constructor_callable<T, Args&&...>)
		constexpr derived_reference_source(Args&&... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr auto provide() & -> Base& {
			return object;
		}
		
		constexpr auto provide() && -> Base& {
			return object;
		}
		
	private:
		T object;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(allow_empty_injection<derived_reference_source<B, U>>) -> std::true_type;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(allow_runtime_caching<derived_reference_source<B, U>>) -> std::bool_constant<allow_runtime_caching_v<B*>>;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(assume_runtime_cached<derived_reference_source<B, U>>) -> std::bool_constant<assume_runtime_cached_v<B*>>;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(second_step_init<derived_reference_source<B, U>>) -> call_second_step_from_attribute_on_member<&derived_reference_source<B, U>::object>;
	};
	
	template<object Base, std::derived_from<Base> T>
	struct derived_pointer_source {
		template<not_self<derived_pointer_source> From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr derived_pointer_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires((... and not_self<Args, derived_pointer_source>) and constructor_callable<T, Args&&...>)
		constexpr derived_pointer_source(Args&&... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr auto provide() & -> Base* {
			return pointer();
		}
		
		constexpr auto provide() && -> Base* {
			return pointer();
		}
		
	private:
		T object;
		
		constexpr auto pointer() -> Base* {
			return std::addressof(object);
		}
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(allow_empty_injection<derived_pointer_source<B, U>>) -> std::true_type;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(allow_runtime_caching<derived_pointer_source<B, U>>) -> std::bool_constant<allow_runtime_caching_v<B*>>;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(assume_runtime_cached<derived_pointer_source<B, U>>) -> std::bool_constant<assume_runtime_cached_v<B*>>;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(second_step_init<derived_pointer_source<B, U>>) -> call_second_step_from_attribute_on_member<&derived_pointer_source<B, U>::pointer>;
	};
	
	template<object Base, std::derived_from<Base> T>
	struct derived_shared_pointer_source {
		template<not_self<derived_shared_pointer_source> From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr derived_shared_pointer_source(From&& object) :
			object{
				std::make_shared<T>(
					KANGARU5_FWD(object)
				)
			} {}
		
		template<typename... Args> requires((... and not_self<Args, derived_shared_pointer_source>) and constructor_callable<T, Args&&...>)
		constexpr derived_shared_pointer_source(Args&&... args) :
			object{
				std::make_shared<T>(
					KANGARU5_NO_ADL(construct_in_place<T>)(KANGARU5_FWD(args)...)
				)
			} {}
		
		constexpr auto provide() const& -> std::shared_ptr<Base> {
			return object;
		}
		
		constexpr auto provide() && -> std::shared_ptr<Base> {
			return object;
		}
		
	private:
		std::shared_ptr<T> object;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(allow_empty_injection<derived_shared_pointer_source<B, U>>) -> std::true_type;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(allow_runtime_caching<derived_shared_pointer_source<B, U>>) -> std::bool_constant<allow_runtime_caching_v<B*>>;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(assume_runtime_cached<derived_shared_pointer_source<B, U>>) -> std::bool_constant<assume_runtime_cached_v<B*>>;
		
		template<kangaru::object B, std::derived_from<B> U>
		friend auto attribute(second_step_init<derived_shared_pointer_source<B, U>>) -> call_second_step_from_attribute_on_member<&derived_shared_pointer_source<B, U>::object>;
	};
	
	template<source Source, source Alternative>
	struct with_alternative {
		Source source;
		Alternative alternative;
		
		template<injectable T, forwarded<with_alternative> Self> requires (not wrapping_source_of<Self, T> and source_of<detail::forward_like_t<Self, Alternative&&>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).alternative);
		}
		
		template<injectable T, forwarded<with_alternative> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		// TODO: Explore rebind alternative with same leaf
		template<forwarded<with_alternative> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_alternative<deduced_source_type<NewSource>, fwd_ref_result_t<detail::forward_like_t<Original, Alternative>>>
		{
			return with_alternative<deduced_source_type<NewSource>, fwd_ref_result_t<detail::forward_like_t<Original, Alternative>>>{
				KANGARU5_FWD(new_source),
				KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(original).alternative),
			};
		}
	};
	
	template<typename Source, typename Alternative>
		requires(not deducer<std::remove_cvref_t<Source>> and not deducer<std::remove_cvref_t<Alternative>>)
	with_alternative(Source&&, Alternative&&) -> with_alternative<deduced_source_type<Source>, deduced_source_type<Alternative>>;
	
	template<forwarded_source Wrapped, forwarded_source Alternative>
	inline constexpr auto make_source_with_alternative(Wrapped&& wrapped, Alternative&& alternative) {
		return with_alternative<deduced_source_type<Wrapped>, deduced_source_type<Alternative>>{KANGARU5_FWD(wrapped), KANGARU5_FWD(alternative)};
	}
	
	template<source Source, injectable Type>
	struct filter_source {
		template<allows_construction_of<Source> S>
		constexpr filter_source(S&& source) : source(KANGARU5_FWD(source)) {}
		
		template<injectable T, forwarded<filter_source> Self>
			requires(different_from<Type, T> and wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<source Source, std::default_initializable Filter>
	struct filter_if_source {
		template<allows_construction_of<Source> S>
		explicit constexpr filter_if_source(S&& source) : source(KANGARU5_FWD(source)) {}
		
		template<allows_construction_of<Source> S>
		constexpr filter_if_source(S&& source, Filter) : source(KANGARU5_FWD(source)) {}
		
		template<injectable T, forwarded<filter_if_source> Self>
			requires(Filter{}.template operator()<T>() and source_of<detail::forward_like_t<Self, Source>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
	private:
		Source source;
	};
	
	template<typename Source, typename Filter>
		requires(not deducer<std::remove_cvref_t<Source>>)
	filter_if_source(Source&&, Filter const&) -> filter_if_source<deduced_source_type<Source>, Filter>;
	
	template<injectable T, forwarded_source Source>
	inline constexpr auto filter(Source&& source) {
		return filter_source<deduced_source_type<Source>, T>{KANGARU5_FWD(source)};
	}
	
	template<forwarded_source Source, std::default_initializable Filter>
	inline constexpr auto filter_if(Source&& source, [[maybe_unused]] Filter filter) {
		return filter_if_source<deduced_source_type<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
	template<std::size_t level, source Source>
	struct with_passthrough {
	private:
		template<std::size_t l = level, forwarded_source S>
			requires(l == 0 or forwarded_wrapping_source<maybe_unwrap_result_t<S>>)
		static constexpr auto target_source(S&& source) -> auto&& {
			if constexpr (l > 0) {
				return target_source<l - 1>(maybe_unwrap(KANGARU5_FWD(source)).source);
			} else {
				return KANGARU5_FWD(source);
			}
		}
		
		template<typename S>
		using target_source_t = decltype(target_source<level>(std::declval<forwarded_wrapped_source_t<S>>()));
		
	public:
		Source source;
		
		template<forwarded<with_passthrough> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> with_passthrough<level, deduced_source_type<NewSource>>
		{
			return with_passthrough<level, deduced_source_type<NewSource>>{
				KANGARU5_FWD(new_source),
			};
		}
		
		template<injectable T, forwarded<with_passthrough> Self>
			requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<with_passthrough> Self>
			requires(not wrapping_source_of<Self, T> and source_of<target_source_t<Self>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(target_source(KANGARU5_FWD(source).source));
		}
	};
	
	template<std::size_t level, forwarded_source Source>
	inline constexpr auto make_source_with_passthrough(Source&& source) {
		return with_passthrough<level, deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_dereference {
		Source source;
		
		template<reference T, forwarded<with_dereference> Self> requires wrapping_source_of<Self, std::remove_reference_t<T>*>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return *kangaru::provide<std::remove_reference_t<T>*>(KANGARU5_FWD(source).source);
		}
		
		template<object T, forwarded<with_dereference> Self> requires (not pointer<T> and wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<with_dereference> auto&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	with_dereference(T&&) -> with_dereference<deduced_source_type<T>>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_dereference(Source&& source) {
		return with_dereference<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source, injectable... From>
	struct with_cast_from {
		Source source;
		
		template<injectable T, forwarded<with_cast_from> Self> requires(wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<with_cast_from> Self>
			requires(
				not wrapping_source_of<Self, T> and
				((
					    different_from<T, From>
					and safe_convertible_to<From, T>
					and wrapping_source_of<Self, From> ? 1 : 0
				) + ... + 0) == 1
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			constexpr auto index = index_of<T>(std::index_sequence_for<From...>{});
			using F = std::tuple_element_t<index, std::tuple<From...>>;
			return static_cast<T>(kangaru::provide<F>(KANGARU5_FWD(source).source));
		}
		
	private:
		template<typename T, std::size_t... S>
		static constexpr auto index_of(std::index_sequence<S...>) {
			return ((different_from<T, From> and safe_convertible_to<From, T> ? S : 0) + ... + 0);
		}
		
		template<kangaru::source S, injectable F>
		friend auto attribute(overrides_types_in_cache<with_cast_from<S, F>>) -> overrides_types_in_cache<S>;
		
		template<kangaru::source S, injectable F>
		friend auto attribute(second_step_init<with_cast_from<S, F>>) -> call_second_step_from_attribute_on_wrapped_source;
	};
	
	template<injectable From>
	inline constexpr auto make_source_with_cast_from(forwarded_source auto&& source) -> with_cast_from<deduced_source_type<decltype(source)>, From> {
		return with_cast_from<deduced_source_type<decltype(source)>, From>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_source_wrapping {
		Source source;
		
		template<wrapping_source T, forwarded<with_source_wrapping> Self>
			requires(
				    wrapping_source_of<Self, wrapped_source_t<T>>
				and std::constructible_from<T, wrapped_source_t<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return T(kangaru::provide<wrapped_source_t<T>>(KANGARU5_FWD(source).source));
		}
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	with_source_wrapping(T&&) -> with_source_wrapping<deduced_source_type<T>>;
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_source_wrapping(Source&& source) {
		return with_source_wrapping<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source, template<typename> typename SourceFor>
	struct with_provide_using_source {
		template<injectable T, forwarded<with_provide_using_source> Self>
			requires (
				// We prevent instanciation of this function with T as a SourceFor<...> to prevent
				// recursive constaint evaluation
				    not detail::is_specialisation_of_v<SourceFor, T>
				and requires { typename SourceFor<T>; }
				and source_of<SourceFor<T>, T>
				and wrapping_source_of<Self, SourceFor<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			decltype(auto) source_for_t = kangaru::provide<SourceFor<T>>(KANGARU5_FWD(source).source);
			return kangaru::provide<T>(KANGARU5_FWD(source_for_t));
		}
		
		Source source;
	};
	
	template<template<typename> typename SourceFor>
	inline constexpr auto make_source_with_provide_using_source(forwarded_source auto&& source) {
		return with_provide_using_source<deduced_source_type<decltype(source)>, SourceFor>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct sealed_source {
		template<allows_construction_of<Source> S>
		explicit constexpr sealed_source(S&& source) : source(KANGARU5_FWD(source)) {}
		
		template<injectable T, forwarded<sealed_source> Self> requires source_of<detail::forward_like_t<Self, Source&&>, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		constexpr auto wrapped_source() & -> Source& {
			return source;
		}
		
		constexpr auto wrapped_source() && -> Source&& {
			return std::move(source);
		}
		
		constexpr auto wrapped_source() const& -> Source const& {
			return source;
		}
		
		constexpr auto wrapped_source() const&& -> Source const&& {
			return std::move(source);
		}
		
	private:
		Source source;
	};
	
	template<typename Source>
		requires(not deducer<std::remove_cvref_t<Source>>)
	sealed_source(Source&&) -> sealed_source<deduced_source_type<Source>>;
	
	template<forwarded_source Source>
	inline constexpr auto seal_source(Source&& source) {
		return sealed_source<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct basic_wrapping_source {
		template<injectable T, forwarded<basic_wrapping_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	basic_wrapping_source(T&&) -> basic_wrapping_source<deduced_source_type<T>>;
	
	template<forwarded_source Source>
	inline constexpr auto wrap_source(Source&& source) {
		return basic_wrapping_source<deduced_source_type<Source>>{KANGARU5_FWD(source)};
	}

	/**
	 * Wrapping source that enable other source to select which types can be provided by a particular source.
	 */
	template<source Source, injectable... Types>
	struct enumerated_source_of {
		template<injectable T, forwarded<enumerated_source_of> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<forwarded<enumerated_source_of> Original, forwarded_source NewSource>
		static constexpr auto rebind(Original&& original, NewSource&& new_source)
			-> enumerated_source_of<deduced_source_type<NewSource>, Types...>
		{
			return enumerated_source_of<deduced_source_type<NewSource>, Types...>{
				KANGARU5_FWD(new_source),
			};
		}
		
		Source source;
	};
	
	template<typename Source> requires(false)
	enumerated_source_of(Source const&) -> enumerated_source_of<Source>;
	
	template<typename T>
	enumerated_source_of(object_source<T> const&) -> enumerated_source_of<object_source<T>, T>;
	
	template<typename T>
	enumerated_source_of(reference_source<T> const&) -> enumerated_source_of<reference_source<T>, T&>;
	
	template<typename T>
	enumerated_source_of(external_reference_source<T> const&) -> enumerated_source_of<external_reference_source<T>, T&>;
	
	template<typename Base, typename T>
	enumerated_source_of(derived_reference_source<Base, T> const&) -> enumerated_source_of<derived_reference_source<Base, T>, T&>;
	
	template<typename T>
	enumerated_source_of(rvalue_source<T> const&) -> enumerated_source_of<rvalue_source<T>, T&&>;
	
	template<typename T>
	enumerated_source_of(external_rvalue_source<T> const&) -> enumerated_source_of<external_rvalue_source<T>, T&&>;
	
	template<typename Base, typename T>
	enumerated_source_of(derived_shared_pointer_source<Base, T> const&) -> enumerated_source_of<derived_shared_pointer_source<Base, T>, std::shared_ptr<T>>;
	
	template<typename T>
	enumerated_source_of(shared_pointer_source<T> const&) -> enumerated_source_of<shared_pointer_source<T>, std::shared_ptr<T>>;
	
	template<typename Base, typename T>
	enumerated_source_of(derived_pointer_source<Base, T> const&) -> enumerated_source_of<derived_pointer_source<Base, T>, T*>;
	
	template<injectable... Types, forwarded_source Source>
	inline constexpr auto enumerate_source(Source&& source) {
		return enumerated_source_of<deduced_source_type<Source>, Types...>{KANGARU5_FWD(source)};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(object_source<T> const& source) -> enumerated_source_of<object_source<T>, T> {
		return enumerated_source_of<object_source<T>, T>{source};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(reference_source<T> const& source) -> enumerated_source_of<reference_source<T>, T&> {
		return enumerated_source_of<reference_source<T>, T&>{source};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(external_reference_source<T> const& source) -> enumerated_source_of<external_reference_source<T>, T&> {
		return enumerated_source_of<external_reference_source<T>, T&>{source};
	}
	
	template<typename Base, typename T>
	inline constexpr auto enumerate_source(derived_reference_source<Base, T> const& source) -> enumerated_source_of<derived_reference_source<Base, T>, T&> {
		return enumerated_source_of<derived_reference_source<Base, T>, T&>{source};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(rvalue_source<T> const& source) -> enumerated_source_of<rvalue_source<T>, T&&> {
		return enumerated_source_of<rvalue_source<T>, T&&>{source};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(external_rvalue_source<T> const& source) -> enumerated_source_of<external_rvalue_source<T>, T&&> {
		return enumerated_source_of<external_rvalue_source<T>, T&&>{source};
	}
	
	template<typename Base, typename T>
	inline constexpr auto enumerate_source(derived_shared_pointer_source<Base, T> const& source) -> enumerated_source_of<derived_shared_pointer_source<Base, T>, std::shared_ptr<T>> {
		return enumerated_source_of<derived_shared_pointer_source<Base, T>, std::shared_ptr<T>>{source};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(shared_pointer_source<T> const& source) -> enumerated_source_of<shared_pointer_source<T>, std::shared_ptr<T>> {
		return enumerated_source_of<shared_pointer_source<T>, std::shared_ptr<T>>{source};
	}
	
	template<typename Base, typename T>
	inline constexpr auto enumerate_source(derived_pointer_source<Base, T> const& source) -> enumerated_source_of<derived_pointer_source<Base, T>, T*> {
		return enumerated_source_of<derived_pointer_source<Base, T>, T*>{source};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(object_source<T>&& source) -> enumerated_source_of<object_source<T>, T> {
		return enumerated_source_of<object_source<T>, T>{std::move(source)};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(reference_source<T>&& source) -> enumerated_source_of<reference_source<T>, T&> {
		return enumerated_source_of<reference_source<T>, T&>{std::move(source)};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(external_reference_source<T>&& source) -> enumerated_source_of<external_reference_source<T>, T&> {
		return enumerated_source_of<external_reference_source<T>, T&>{std::move(source)};
	}
	
	template<typename Base, typename T>
	inline constexpr auto enumerate_source(derived_reference_source<Base, T>&& source) -> enumerated_source_of<derived_reference_source<Base, T>, T&> {
		return enumerated_source_of<derived_reference_source<Base, T>, T&>{std::move(source)};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(rvalue_source<T>&& source) -> enumerated_source_of<rvalue_source<T>, T&&> {
		return enumerated_source_of<rvalue_source<T>, T&&>{std::move(source)};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(external_rvalue_source<T>&& source) -> enumerated_source_of<external_rvalue_source<T>, T&&> {
		return enumerated_source_of<external_rvalue_source<T>, T&&>{std::move(source)};
	}
	
	template<typename Base, typename T>
	inline constexpr auto enumerate_source(derived_shared_pointer_source<Base, T>&& source) -> enumerated_source_of<derived_shared_pointer_source<Base, T>, std::shared_ptr<T>> {
		return enumerated_source_of<derived_shared_pointer_source<Base, T>, std::shared_ptr<T>>{std::move(source)};
	}
	
	template<typename T>
	inline constexpr auto enumerate_source(shared_pointer_source<T>&& source) -> enumerated_source_of<shared_pointer_source<T>, std::shared_ptr<T>> {
		return enumerated_source_of<shared_pointer_source<T>, std::shared_ptr<T>>{std::move(source)};
	}
	
	template<typename Base, typename T>
	inline constexpr auto enumerate_source(derived_pointer_source<Base, T>&& source) -> enumerated_source_of<derived_pointer_source<Base, T>, T*> {
		return enumerated_source_of<derived_pointer_source<Base, T>, T*>{std::move(source)};
	}
	
	template<source... Sources>
	using tied_source = composed_source<source_reference_wrapper<Sources>...>;
	
	inline constexpr auto tie(source auto&... sources) {
		return KANGARU5_NO_ADL(compose)(KANGARU5_NO_ADL(ref)(sources)...);
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TYPES_HPP
