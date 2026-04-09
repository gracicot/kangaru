#ifndef KANGARU5_DETAIL_TWO_STEP_INIT_HPP
#define KANGARU5_DETAIL_TWO_STEP_INIT_HPP

#include "source_rebind.hpp"
#include "source.hpp"
#include "concepts.hpp"
#include "attributes.hpp"
#include "deducer.hpp"
#include "injector.hpp"

#include <concepts>
#include <functional>

#include "define.hpp"

namespace kangaru::detail::two_step_init_private {
	template<auto memfn, forwarded_object T>
		requires(pointer_to_member_function<decltype(memfn)>)
	struct bound_memfn {
		T&& object;
		
		template<typename... Args> requires(std::invocable<decltype(memfn), T&&, Args&&...>)
		auto operator()(Args&&... args) const -> std::invoke_result_t<decltype(memfn), T&&, Args&&...> {
			return std::invoke(memfn, KANGARU5_FWD(object), KANGARU5_FWD(args)...);
		}
	};
	
	template<auto fn, forwarded_object T>
	struct bound_fn {
		T&& object;
		
		template<typename... Args> requires(std::invocable<decltype(fn), T&&, Args&&...>)
		auto operator()(Args&&... args) const -> detail::call_result_t<decltype(fn), T&&, Args&&...> {
			return fn(KANGARU5_FWD(object), KANGARU5_FWD(args)...);
		}
	};
	
	template<pointer_to_member MemPtrType>
	struct member_type {};
	
	template<typename Type, typename Class>
	struct member_type<Type Class::*> {
		using type = std::invoke_result_t<Type Class::*, Class&>;
	};
	
	template<auto mptr>
	using member_type_for = typename member_type<decltype(mptr)>::type;
}

KANGARU5_EXPORT namespace kangaru {
	template<typename T>
	concept second_step_function = function_object<T> and std::semiregular<T> and not pointer<T>;
	
	struct noop_second_step {
		template<injectable T>
		constexpr auto operator()(T&, forwarded_source auto&&) const -> void {}
	};
	
	template<source Source, second_step_function SecondStep>
	struct with_two_step_init {
		explicit constexpr with_two_step_init(Source source, SecondStep) noexcept :
			source{std::move(source)} {}
		
		constexpr with_two_step_init(Source source) noexcept requires(
			std::default_initializable<SecondStep>
		) :
			source{std::move(source)} {}
		
		Source source;
		
		template<injectable T, forwarded<with_two_step_init> Self>
			requires(wrapping_source_of<Self, T> and callable_template_1t<SecondStep const&, T, T&, forwarded_wrapped_source_t<Self>>)
		constexpr KANGARU5_PROVIDE_FUNCTION_FRIEND auto provide(KANGARU5_PROVIDE_FUNCTION_THIS Self&& source) -> T {
			decltype(auto) result = kangaru::provide<T>(KANGARU5_FWD(source).source);
			void(std::as_const(source).second_step.template operator()<T>(result, KANGARU5_FWD(source).source));
			
			if constexpr (reference<T>) {
				return static_cast<T>(result);
			} else {
				return result;
			}
		}
		
		template<forwarded<with_two_step_init> Original, forwarded_source NewSource>
			requires(std::constructible_from<SecondStep, detail::forward_like_t<Original, SecondStep>>)
		static constexpr auto rebind(Original&& original, NewSource&& new_leaf) noexcept -> with_two_step_init<wrapped_source_rebind_result_t<Original, NewSource>, SecondStep> {
			return with_two_step_init<wrapped_source_rebind_result_t<Original, NewSource>, SecondStep>{
				kangaru::rebind(KANGARU5_FWD(original).source, KANGARU5_FWD(new_leaf)),
				KANGARU5_FWD(original).second_step
			};
		}
		
	private:
		SecondStep second_step;
	};
	
	template<auto f>
	struct call_function {
		template<injectable T, forwarded_source Source> requires(callable<decltype(f), T&, Source&&>)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			f(object, KANGARU5_FWD(source));
		}
		
		template<injectable T, forwarded_source Source> requires(callable<decltype(f), T&>)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			f(object);
		}
	};
	
	template<function_object MakeInjector, auto fn>
	struct call_injected_function_with {
		template<injectable T, forwarded_source Source>
			requires(
				callable<
					detail::call_result_t<MakeInjector, fwd_ref_result_t<Source&&>>,
					detail::two_step_init_private::bound_fn<fn, T&>
				>
			)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			MakeInjector{}(KANGARU5_FWD(source))(
				detail::two_step_init_private::bound_fn<fn, T&>{object}
			);
		}
	};
	
	template<auto fn>
	using call_injected_function = call_injected_function_with<make_spread_injector_function, fn>;
	
	template<function_object MakeInjector, auto... memfn>
		requires(
			    std::default_initializable<MakeInjector>
			and (... and pointer_to_member_function<std::remove_cvref_t<decltype(memfn)>>)
		)
	struct call_injected_member_functions_with {
		template<injectable T, forwarded_source Source>
			requires(
				(... and callable<
					detail::call_result_t<MakeInjector, fwd_ref_result_t<Source&&>>,
					detail::two_step_init_private::bound_memfn<memfn, T&>
				>)
			)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			auto injector = MakeInjector{}(KANGARU5_NO_ADL(fwd_ref)(KANGARU5_FWD(source)));
			(std::move(injector)(detail::two_step_init_private::bound_memfn<memfn, T&>{object}), ...);
		}
	};
	
	template<auto... memfn>
	using call_injected_member_functions = call_injected_member_functions_with<make_spread_injector_function, memfn...>;
	
	struct second_step_from_attribute {
		template<injectable T, forwarded_source Source>
			requires(callable_template_1t<second_step_init_t<T>, T, T&, Source&&>)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			second_step_init_t<T>{}.template operator()<T>(object, KANGARU5_FWD(source));
		}
	};
	
	template<second_step_function SecondStep>
	struct call_second_step_on_dereference {
		constexpr call_second_step_on_dereference() = default;
		explicit constexpr call_second_step_on_dereference(SecondStep second_step) : second_step{std::move(second_step)} {}
		
		template<injectable T, forwarded_source Source>
			requires(
				    requires(T const& ptr) { { *ptr } -> reference; }
				and callable_template_1t<SecondStep const&, decltype(*std::declval<T const&>()), decltype(*std::declval<T const&>()), Source&&>
			)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			auto& ref = *std::as_const(object);
			void(std::as_const(second_step).template operator()<decltype(ref)>(ref, KANGARU5_FWD(source)));
		}
		
	private:
		SecondStep second_step;
	};
	
	using call_second_step_from_attribute_on_dereference = call_second_step_on_dereference<second_step_from_attribute>;
	
	template<second_step_function SecondStep>
	struct call_second_step_on_prvalue {
		constexpr call_second_step_on_prvalue() = default;
		explicit constexpr call_second_step_on_prvalue(SecondStep second_step) : second_step{std::move(second_step)} {}
		
		template<injectable T, forwarded_source Source>
			requires(
				    not std::is_const_v<std::remove_reference_t<T>>
				and callable_template_1t<SecondStep const&, std::remove_cvref_t<T>, T&, Source&&>
			)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			void(std::as_const(second_step).template operator()<std::remove_cvref_t<T>>(object, KANGARU5_FWD(source)));
		}
		
	private:
		SecondStep second_step;
	};
	
	using call_second_step_from_attribute_on_prvalue = call_second_step_on_prvalue<second_step_from_attribute>;
	
	template<second_step_function SecondStep>
	struct call_second_step_on_wrapped_source {
	private:
		template<typename T>
		using injected_type = detail::conditional_t<reference<T>,
			detail::forward_like_t<T, std::remove_cv_t<wrapped_source_t<T>>>&&,
			std::remove_cv_t<wrapped_source_t<T>>
		>;
		
		SecondStep second_step;
		
	public:
		constexpr call_second_step_on_wrapped_source() = default;
		explicit constexpr call_second_step_on_wrapped_source(SecondStep second_step) : second_step{std::move(second_step)} {}
		
		template<injectable T, forwarded_source Source>
			requires(
				    wrapping_source<std::remove_cvref_t<T>>
				and callable_template_1t<SecondStep const&, injected_type<T>, forwarded_wrapped_source_t<T&>, Source&&>
			)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			void(std::as_const(second_step).template operator()<injected_type<T>>(object.source, KANGARU5_FWD(source)));
		}
	};
	
	using call_second_step_from_attribute_on_wrapped_source = call_second_step_on_wrapped_source<second_step_from_attribute>;
	
	template<auto mptr, second_step_function SecondStep, injectable As = detail::two_step_init_private::member_type_for<mptr>>
		requires(
			    pointer_to_member<decltype(mptr)>
			and std::convertible_to<detail::two_step_init_private::member_type_for<mptr>&, As&>
		)
	struct call_second_step_on_member {
		template<injectable T, forwarded_source Source>
			requires(
				    std::convertible_to<std::invoke_result_t<decltype(mptr), T&>, As&>
				and callable_template_1t<SecondStep const&, As, As&, Source&&>
			)
		constexpr auto operator()(T& object, Source&& source) const -> void {
			void(std::as_const(second_step).template operator()<As>(std::invoke(mptr, object), KANGARU5_FWD(source)));
		}
		
		SecondStep second_step;
	};
	
	template<auto mptr, injectable As = detail::two_step_init_private::member_type_for<mptr>>
		requires(
			    pointer_to_member<decltype(mptr)>
			and std::convertible_to<detail::two_step_init_private::member_type_for<mptr>&, As&>
		)
	using call_second_step_from_attribute_on_member = call_second_step_on_member<mptr, second_step_from_attribute, As>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_TWO_STEP_INIT_HPP
