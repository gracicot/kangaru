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
		// TODO: Allow immovable sources
		explicit(sizeof...(Sources) == 1) constexpr composed_source(Sources... sources) requires(... and std::move_constructible<Sources>) : sources{std::move(sources)...} {}
		
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
		
		explicit constexpr composed_source(detail::source_types_private::from_tuple_t, std::tuple<Sources...>&& sources) :
			sources{std::move(sources)} {}
		
		std::tuple<Sources...> sources;
	};
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
		return composed_source<std::decay_t<decltype(sources)>...>{KANGARU5_FWD(sources)...};
	}
	
	template<object... Ts>
	struct tuple_source {
		// TODO: Allow immovable objects
		explicit constexpr tuple_source(std::tuple<Ts...> objects) : objects{std::move(objects)} {}
		
		template<injectable T> requires (... or std::same_as<T, Ts>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS forwarded<tuple_source> auto&& source) -> T {
			return std::get<T>(KANGARU5_FWD(source).objects);
		}
		
	private:
		std::tuple<Ts...> objects;
	};
	
	template<callable F> requires (unqualified_object<F> and std::move_constructible<F> and injectable<detail::call_result_t<F>>)
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
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr object_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires constructor_callable<T, Args&&...>
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
	object_source(T&&) -> object_source<std::decay_t<T>>;
	
	template<object T>
	struct rvalue_source {
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr rvalue_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires constructor_callable<T, Args&&...>
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
	rvalue_source(T&&) -> rvalue_source<std::decay_t<T>>;
	
	template<object T>
	struct reference_source {
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr reference_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires constructor_callable<T, Args&&...>
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
	reference_source(T&&) -> reference_source<std::decay_t<T>>;
	
	template<object T>
	struct shared_pointer_source {
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr shared_pointer_source(From&& object) :
			object{
				std::make_shared<T>(
					KANGARU5_FWD(object)
				)
			} {}
		
		template<typename... Args> requires constructor_callable<T, Args&&...>
		constexpr shared_pointer_source(Args&&... args) :
			object{
				std::make_shared<T>(
					KANGARU5_NO_ADL(make_in_place<T>)(KANGARU5_FWD(args)...)
				)
			} {}
		
		constexpr auto provide() const& -> std::shared_ptr<T> {
			return object;
		}
		
		// TODO: Should we move the shared pointer?
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
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr derived_reference_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires constructor_callable<T, Args&&...>
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
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr derived_pointer_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires(constructible_in_place<T, Args&&...>)
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
		template<typename From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr derived_shared_pointer_source(From&& object) :
			object{
				std::make_shared<T>(
					KANGARU5_FWD(object)
				)
			} {}
		
		template<typename... Args> requires(constructible_in_place<T, Args&&...>)
		constexpr derived_shared_pointer_source(Args&&... args) :
			object{
				std::make_shared<T>(
					KANGARU5_NO_ADL(make_in_place<T>)(KANGARU5_FWD(args)...)
				)
			} {}
		
		constexpr auto provide() const& -> std::shared_ptr<Base> {
			return object;
		}
		
		// TODO: Should we move the shared pointer?
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
		constexpr with_alternative(Source source, Alternative alternative) : source{std::move(source)}, alternative{std::move(alternative)} {}
		
		Source source;
		
		template<injectable T, forwarded<with_alternative> Self> requires (not wrapping_source_of<Self, T> and source_of<detail::forward_like_t<Self, Alternative&&>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).alternative);
		}
		
		template<injectable T, forwarded<with_alternative> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<forwarded<with_alternative> Original, forwarded_source NewLeaf>
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) -> with_alternative<wrapped_source_rebind_result_t<Original, NewLeaf>, fwd_ref_result_t<detail::forward_like_t<Original, Alternative>>> {
			return with_alternative<wrapped_source_rebind_result_t<Original, NewLeaf>, fwd_ref_result_t<detail::forward_like_t<Original, Alternative>>>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_NO_ADL(fwd_ref)(maybe_unwrap(KANGARU5_FWD(original).alternative))
			};
		}
		
		Alternative alternative;
	};
	
	template<forwarded_source Wrapped, forwarded_source Alternative>
	inline constexpr auto make_source_with_alternative(Wrapped&& wrapped, Alternative&& alternative) {
		return with_alternative<std::decay_t<Wrapped>, std::decay_t<Alternative>>{KANGARU5_FWD(wrapped), KANGARU5_FWD(alternative)};
	}
	
	template<source Source, injectable Type>
	struct filter_source {
		constexpr filter_source(Source source) : source{std::move(source)} {}
		
		template<injectable T, forwarded<filter_source> Self>
			requires(different_from<Type, T> and wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<source Source, std::default_initializable Filter>
	struct filter_if_source {
		explicit constexpr filter_if_source(Source source) : source{std::move(source)} {}
		constexpr filter_if_source(Source source, Filter) : source{std::move(source)} {}
		
		template<injectable T, forwarded<filter_if_source> Self>
			requires(Filter{}.template operator()<T>() and source_of<detail::forward_like_t<Self, Source>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
	private:
		Source source;
	};
	
	template<source Filter, forwarded_source Source>
	inline constexpr auto filter(Source&& source) {
		return filter_source<std::decay_t<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
	template<forwarded_source Source, std::default_initializable Filter>
	inline constexpr auto filter_if(Source&& source, [[maybe_unused]] Filter filter) {
		return filter_if_source<std::decay_t<Source>, Filter>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_passthrough {
		explicit constexpr with_passthrough(Source source) : source{std::move(source)} {}
		
		Source source;
		
		template<injectable T, forwarded<with_passthrough> Self>
			requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<injectable T, forwarded<with_passthrough> Self>
			requires(not wrapping_source_of<Self, T> and wrapping_source_of<wrapped_source_t<Self>, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source.source);
		}
	};
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_passthrough(Source&& source) {
		return with_passthrough<std::decay_t<Source>>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_dereference {
		// TODO: Add a const& and && + conversion constructor?
		explicit constexpr with_dereference(Source source) : source{std::move(source)} {}
		
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
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_dereference(Source&& source) {
		return with_dereference<std::remove_cvref_t<Source>>{KANGARU5_FWD(source)};
	}
	
	// TODO: Allow enumerated source and variadic From
	template<source Source, injectable From> requires source_of<Source, From>
	struct with_cast_from {
		Source source;
		
		template<injectable T, forwarded<with_cast_from> Self> requires(wrapping_source_of<Self, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		template<different_from<From> T, forwarded<with_cast_from> Self> requires(injectable<T> and not wrapping_source_of<Self, T> and safe_convertible_to<From, T>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			decltype(auto) result = kangaru::provide<From>(KANGARU5_FWD(source).source);
			return static_cast<T>(result);
		}
		
		template<kangaru::source S, injectable F>
		friend auto attribute(overrides_types_in_cache<with_cast_from<S, F>>) -> overrides_types_in_cache<S>;
		
		template<kangaru::source S, injectable F>
		friend auto attribute(second_step_init<with_cast_from<S, F>>) -> call_second_step_from_attribute_on_wrapped_source;
	};
	
	template<injectable From>
	inline constexpr auto make_source_with_cast_from(forwarded_source auto&& source) -> with_cast_from<std::decay_t<decltype(source)>, From> {
		return with_cast_from<std::decay_t<decltype(source)>, From>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct with_source_wrapping {
		Source source;
		
		template<wrapping_source T, forwarded<with_source_wrapping> Self>
			requires(
				    wrapping_source_of<Self, wrapped_source_t<T>>
				and constructor_callable<T, wrapped_source_t<T>>
			)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return KANGARU5_NO_ADL(constructor<T>)(kangaru::provide<wrapped_source_t<T>>(KANGARU5_FWD(source).source));
		}
	};
	
	template<forwarded_source Source>
	inline constexpr auto make_source_with_source_wrapping(Source&& source) {
		return with_source_wrapping<std::decay_t<Source>>{KANGARU5_FWD(source)};
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
		
		// TODO: Report clang issue. We shouldn't need to define this function. detail::source_rebind::rebind_wrapper is broken?
		template<forwarded<with_provide_using_source> Original, forwarded_source NewLeaf>
			requires(
				std::constructible_from<
					Source,
					detail::forward_like_t<Original, Source>
				>
			)
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) ->
			with_provide_using_source<wrapped_source_rebind_result_t<Original, NewLeaf>, SourceFor>
		{
			return with_provide_using_source<wrapped_source_rebind_result_t<Original, NewLeaf>, SourceFor> {
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
			};
		}
		
		Source source;
	};
	
	template<template<typename> typename SourceFor>
	inline constexpr auto make_source_with_provide_using_source(forwarded_source auto&& source) {
		return with_provide_using_source<std::decay_t<decltype(source)>, SourceFor>{KANGARU5_FWD(source)};
	}
	
	template<source Source>
	struct basic_wrapping_source {
		template<injectable T, forwarded<basic_wrapping_source> Self> requires wrapping_source_of<Self, T>
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			return kangaru::provide<T>(KANGARU5_FWD(source).source);
		}
		
		Source source;
	};
	
	template<forwarded_source Source>
	inline constexpr auto wrap_source(Source&& source) {
		return basic_wrapping_source<std::decay_t<Source>>{KANGARU5_FWD(source)};
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
		
		template<forwarded<enumerated_source_of> Original, forwarded_source NewLeaf>
			requires(
				std::constructible_from<
					Source,
					detail::forward_like_t<Original, Source>
				>
			)
		static constexpr auto rebind(Original&& original, NewLeaf&& new_leaf) ->
			enumerated_source_of<wrapped_source_rebind_result_t<Original, NewLeaf>, Types...>
		{
			return enumerated_source_of<wrapped_source_rebind_result_t<Original, NewLeaf>, Types...> {
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
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
		return enumerated_source_of<std::decay_t<Source>, Types...>{KANGARU5_FWD(source)};
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
