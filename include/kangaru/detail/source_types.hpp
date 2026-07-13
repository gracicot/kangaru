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
				and (... and (not deducer<S>))
				and (... and (in_place_constructible<std::remove_cvref_t<Sources>> and std::constructible_from<Sources, S&&>))
			)
		explicit(sizeof...(S) == 1)
		constexpr composed_source(S&&... sources) : sources{in_place_construct{in_place_construct_function_for<Sources, S&&>{KANGARU5_FWD(sources)}}...} {}
		
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
		
		template<source S, reference From>
			requires(std::constructible_from<std::remove_cvref_t<S>, From&&>)
		struct in_place_construct_function_for {
			explicit in_place_construct_function_for(From&& from) : from(std::addressof(from)) {}
			constexpr auto operator()() const&& {
				return std::remove_cvref_t<S>(static_cast<From&&>(*from));
			}
			
		private:
			std::remove_reference_t<From>* from;
		};
		
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
		friend auto attribute(second_step_init<object_source<U>>) -> call_second_step_from_attribute_on_member<&object_source<U>::object, U>;
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
	struct pointer_source {
		template<not_self<pointer_source> From = T> requires(not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, T>)
		explicit constexpr pointer_source(From&& object) : object(KANGARU5_FWD(object)) {}
		
		template<typename... Args> requires((... and not_self<Args, pointer_source>) and constructor_callable<T, Args&&...>)
		constexpr pointer_source(Args&&... args) : object(KANGARU5_NO_ADL(constructor<T>)(KANGARU5_FWD(args)...)) {}
		
		constexpr auto provide() & -> T* {
			return pointer();
		}
		
		constexpr auto provide() && -> T* {
			return pointer();
		}
		
	private:
		T object;
		
		constexpr auto pointer() -> T* {
			return std::addressof(object);
		}
		
		template<kangaru::object U>
		friend auto attribute(allow_empty_injection<pointer_source<U>>) -> std::true_type;
		
		template<kangaru::object U>
		friend auto attribute(allow_runtime_caching<pointer_source<U>>) -> std::bool_constant<allow_runtime_caching_v<U&>>;
		
		template<kangaru::object U>
		friend auto attribute(assume_runtime_cached<pointer_source<U>>) -> std::bool_constant<assume_runtime_cached_v<U&>>;
		
		template<kangaru::object U>
		friend auto attribute(second_step_init<pointer_source<U>>) -> call_second_step_from_attribute_on_member<&pointer_source<U>::pointer, U*>;
	};
	
	template<typename T> requires(not deducer<std::remove_cvref_t<T>>)
	pointer_source(T&&) -> pointer_source<deduced_source_type<T>>;
	
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
		friend auto attribute(second_step_init<shared_pointer_source<U>>) -> call_second_step_from_attribute_on_member<&shared_pointer_source<U>::object, std::shared_ptr<U>>;
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
		friend auto attribute(second_step_init<derived_reference_source<B, U>>) -> call_second_step_from_attribute_on_member<&derived_reference_source<B, U>::object, U&>;
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
		friend auto attribute(second_step_init<derived_pointer_source<B, U>>) -> call_second_step_from_attribute_on_member<&derived_pointer_source<B, U>::pointer, U*>;
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
		friend auto attribute(second_step_init<derived_shared_pointer_source<B, U>>) -> call_second_step_from_attribute_on_member<&derived_shared_pointer_source<B, U>::object, std::shared_ptr<U>>;
	};
	
	template<source... Sources>
	using tied_source = composed_source<source_reference_wrapper<Sources>...>;
	
	inline constexpr auto tie(source auto&... sources) {
		return KANGARU5_NO_ADL(compose)(KANGARU5_NO_ADL(ref)(sources)...);
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_SOURCE_TYPES_HPP
