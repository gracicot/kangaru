#ifndef KANGARU5_DETAIL_OPTIONAL_HPP
#define KANGARU5_DETAIL_OPTIONAL_HPP

#include "concepts.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>
#include <exception>
#include <compare>
#endif

#include "define.hpp"

namespace kangaru::detail::optional_private {
	struct empty {};
	
	template<typename T>
	concept optional_suitable = object<T> or lvalue_reference<T>;
}

KANGARU5_EXPORT namespace kangaru {
	struct in_place_t {} inline constexpr in_place;
	
	template<detail::optional_private::optional_suitable T>
	struct optional;
}

namespace kangaru::detail::optional_private {
	template<typename T, typename Arg>
	concept optional_forwarded_construction_object =
		    different_from<in_place_t, std::decay_t<Arg>>
		and optional_suitable<T>
		and different_from<optional<T>, std::decay_t<Arg>>
		and std::constructible_from<T, Arg&&>;
}

KANGARU5_EXPORT namespace kangaru {
	struct nullopt_t {} inline constexpr nullopt;
	
	struct bad_optional_access : std::exception {
		auto what() const noexcept -> char const* override {
			return "kangaru::optional::value() called on a disengaged optional";
		}
	};
	
	template<object T>
	struct optional<T> {
		using value_type = T;
		
		explicit(false) constexpr optional(nullopt_t) noexcept : engaged{false}, storage{detail::optional_private::empty{}} {}
		constexpr optional() : optional{nullopt} {};
		
		constexpr optional(optional const& other)
			requires(std::is_copy_constructible_v<T> and not (std::is_trivially_copy_constructible_v<T> and std::is_trivially_destructible_v<T>)) :
				engaged{other.engaged}
		{
			if (engaged) {
				std::construct_at(std::addressof(storage.object), other.storage.object);
			}
		}
		
		constexpr optional(optional const& other) noexcept
			requires(std::is_trivially_copy_constructible_v<T> and std::is_trivially_destructible_v<T>) = default;
		
		constexpr optional(optional&& other)
			noexcept(std::is_nothrow_move_constructible_v<T>)
			requires(std::is_move_constructible_v<T> and not (std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>)) :
				engaged{other.engaged}
		{
			if (engaged) {
				std::construct_at(std::addressof(storage.object), std::move(other.storage.object));
			}
		}
		
		constexpr optional(optional&& other) noexcept
			requires(std::is_trivially_move_constructible_v<T> and std::is_trivially_destructible_v<T>) = default;
		
		template<typename U>
		explicit(!std::convertible_to<U&&, T>)
		constexpr optional(optional<U> const& other) : engaged{other.has_value()} {
			if (engaged) {
				new (&storage.object) T(*other);
			}
		}
		
		template<typename U>
		explicit(!std::convertible_to<U&&, T>)
		constexpr optional(optional<U>&& other) : engaged{other.has_value()} {
			if (engaged) {
				new (&storage.object) T(*std::move(other));
			}
		}
		
		template<typename... Args> requires std::constructible_from<T, Args&&...>
		constexpr optional(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) :
			engaged{true}, storage{storage_initialize, KANGARU5_FWD(args)...} {}
		
		template<typename U, typename... Args> requires std::constructible_from<T, std::initializer_list<U>&, Args&&...>
		constexpr explicit optional(in_place_t, std::initializer_list<U> ilist, Args&&... args) : engaged{true}, storage{storage_initialize, ilist, KANGARU5_FWD(args)...} {}
		
		template<typename U>
			requires detail::optional_private::optional_forwarded_construction_object<T, U>
			explicit(!std::convertible_to<U&&, T>)
		constexpr optional(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>) :
			engaged{true}, storage{storage_initialize, KANGARU5_FWD(value)} {}
		
		constexpr auto operator=(optional const& rhs)
			noexcept(std::is_nothrow_copy_constructible_v<T> and std::is_nothrow_copy_assignable_v<T>) -> optional&
			requires(not std::is_trivially_copy_assignable_v<T>)
		{
			if (rhs.engaged) {
				if (engaged) {
					storage.object = rhs.storage.object;
				} else {
					emplace(rhs.storage.object);
				}
			} else {
				reset();
			}
			
			return *this;
		}
		
		constexpr auto operator=(optional const& rhs)
			noexcept(std::is_nothrow_copy_constructible_v<T> and std::is_nothrow_copy_assignable_v<T>) -> optional&
			requires(std::is_trivially_copy_assignable_v<T>) = default;
		
		constexpr auto operator=(optional&& rhs)
			noexcept(std::is_nothrow_move_constructible_v<T> and std::is_nothrow_move_assignable_v<T>) -> optional&
			requires(not std::is_trivially_move_assignable_v<T>)
		{
			if (rhs.engaged) {
				if (engaged) {
					storage.object = std::move(rhs.storage.object);
				} else {
					emplace(std::move(rhs.storage.object));
				}
			} else {
				reset();
			}
			
			return *this;
		}
		
		constexpr auto operator=(optional&& rhs)
			noexcept(std::is_nothrow_move_constructible_v<T> and std::is_nothrow_move_assignable_v<T>) -> optional&
			requires(std::is_trivially_move_assignable_v<T>) = default;
		
		constexpr ~optional() noexcept requires(std::is_trivially_destructible_v<T>) = default;
		
		constexpr ~optional() noexcept {
			if (engaged) {
				std::destroy_at(std::addressof(storage.object));
			}
		}
		
		constexpr auto operator=(nullopt_t) noexcept -> optional& {
			if (engaged) {
				destroy();
			}
			
			return *this;
		}
		
		template<typename U = std::remove_cv_t<T>>
			requires(
				    different_from<optional, std::remove_cvref_t<U>>
				and std::is_constructible_v<T, U>
				and std::is_assignable_v<T&, U>
				and (
					   not std::is_scalar_v<T>
					or different_from<T, std::decay_t<U>>
				)
			)
		constexpr auto operator=(U&& value) -> optional& {
			if (engaged) {
				storage.object = KANGARU5_FWD(value);
			} else {
				emplace(KANGARU5_FWD(value));
			}
			
			return *this;
		}
		
		template<typename U>
			requires(
				    std::is_constructible_v<T, U>
				and std::is_assignable_v<T&, U>
				and not (
					   std::is_constructible_v<T, optional<U>&>
					or std::is_constructible_v<T, optional<U> const&>
					or std::is_constructible_v<T, optional<U>&&>
					or std::is_constructible_v<T, optional<U> const&&>
					or std::is_convertible_v<optional<U>&, T>
					or std::is_convertible_v<optional<U> const&, T>
					or std::is_convertible_v<optional<U>&&, T>
					or std::is_convertible_v<optional<U> const&&, T>
					or std::is_assignable_v<T&, optional<U>&>
					or std::is_assignable_v<T&, optional<U> const&>
					or std::is_assignable_v<T&, optional<U>&&>
					or std::is_assignable_v<T&, optional<U> const&&>
				)
			)
		constexpr auto operator=(optional<U> const& rhs) -> optional& {
			if (rhs.engaged) {
				if (engaged) {
					storage.object = rhs.storage.object;
				} else {
					emplace(rhs.storage.object);
				}
			} else {
				reset();
			}
			
			return *this;
		}
		
		template<typename U>
			requires(
				    std::is_constructible_v<T, U>
				and std::is_assignable_v<T&, U>
				and not (
					   std::is_constructible_v<T, optional<U>&>
					or std::is_constructible_v<T, optional<U> const&>
					or std::is_constructible_v<T, optional<U>&&>
					or std::is_constructible_v<T, optional<U> const&&>
					or std::is_convertible_v<optional<U>&, T>
					or std::is_convertible_v<optional<U> const&, T>
					or std::is_convertible_v<optional<U>&&, T>
					or std::is_convertible_v<optional<U> const&&, T>
					or std::is_assignable_v<T&, optional<U>&>
					or std::is_assignable_v<T&, optional<U> const&>
					or std::is_assignable_v<T&, optional<U>&&>
					or std::is_assignable_v<T&, optional<U> const&&>
				)
			)
		constexpr auto operator=(optional<U>&& rhs) -> optional& {
			if (engaged) {
				storage.object = std::move(rhs).storage.object;
			} else {
				emplace(std::move(rhs).storage.object);
			}
			
			return *this;
		}
		
		constexpr auto swap(optional& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>) -> void {
			if (other.engaged) {
				if (engaged) {
					using std::swap;
					swap(storage.object, other.storage.object);
				} else {
					construct(std::move(other.storage.object));
					other.reset();
				}
			} else {
				other.construct(std::move(storage.object));
				reset();
			}
		}
		
		friend constexpr auto swap(optional& lhs, optional& rhs) -> void {
			lhs.swap(rhs);
		}
		
		constexpr auto reset() noexcept -> void {
			if (engaged) {
				destroy();
			}
		}
		
		template<typename... Args> requires std::constructible_from<T, Args&&...>
		constexpr auto emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) -> T& {
			reset();
			construct(KANGARU5_FWD(args)...);
			return storage.object;
		}
		
		template<typename U, typename... Args> requires std::constructible_from<T, std::initializer_list<U>&, Args&&...>
		constexpr auto emplace(std::initializer_list<U> list, Args&&... args) noexcept(
			std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args&&...>
		) -> T& {
			reset();
			construct(list, KANGARU5_FWD(args)...);
			return storage.object;
		}
		
		constexpr auto value() & -> T& {
			return has_value() ? storage.object : throw bad_optional_access{};
		}
		
		constexpr auto value() const& -> T const& {
			return has_value() ? storage.object : throw bad_optional_access{};
		}
		
		constexpr auto value() && -> T&& {
			return has_value() ? std::move(storage.object) : throw bad_optional_access{};
		}
		
		constexpr auto value() const&& -> T const&& {
			return has_value() ? std::move(storage.object) : throw bad_optional_access{};
		}
		
		constexpr auto operator*() & noexcept -> T& {
			return storage.object;
		}
		
		constexpr auto operator*() const& noexcept -> T const& {
			return storage.object;
		}
		
		constexpr auto operator*() && noexcept -> T&& {
			return std::move(storage.object);
		}
		
		constexpr auto operator*() const&& noexcept -> T const&& {
			return std::move(storage.object);
		}
		
		constexpr auto operator->() noexcept -> T* {
			return std::addressof(storage.object);
		}
		
		constexpr auto operator->() const noexcept -> T const* {
			return std::addressof(storage.object);
		}
		
		constexpr auto has_value() const noexcept -> bool {
			return engaged;
		}
		
		explicit constexpr operator bool() const noexcept {
			return has_value();
		}
		
		template<std::three_way_comparable_with<T> U> requires(unqualified_object<U>)
		constexpr auto operator<=>(optional<U> const& rhs) const -> std::compare_three_way_result_t<T, U> {
			optional const& lhs = *this;
			return lhs and rhs ? *lhs <=> *rhs : lhs.has_value() <=> rhs.has_value();
		}
		
		friend constexpr auto operator<=>(optional const& lhs, nullopt_t) -> std::strong_ordering {
			return lhs.has_value() <=> false;
		}
		
		template<std::three_way_comparable_with<T> U>
		constexpr auto operator<=>(U const& rhs) const -> std::compare_three_way_result_t<T, U> {
			optional const& lhs = *this;
			return lhs.has_value() ? *lhs <=> rhs : std::strong_ordering::less;
		}
		
		friend constexpr auto operator==(optional const& lhs, optional const& rhs) {
			return lhs and rhs ? *lhs == *rhs : lhs.has_value() == rhs.has_value();
		}
		
		template<typename U = std::remove_cv_t<T>> requires(std::is_copy_constructible_v<T> and std::convertible_to<U&&, T>)
		constexpr auto value_or(U&& default_value) const& -> T {
			return has_value() ? **this : static_cast<T>(KANGARU5_FWD(default_value));
		}
		
		template<typename U = std::remove_cv_t<T>> requires(std::is_move_constructible_v<T> and std::convertible_to<U&&, T>)
		constexpr auto value_or(U&& default_value) && -> T {
			return has_value() ? *std::move(*this) : static_cast<T>(KANGARU5_FWD(default_value));
		}
		
		template<std::invocable<T&> F>
			requires(detail::is_specialisation_of_v<optional, std::invoke_result_t<F, T&>>)
		constexpr auto and_then(F&& f) & {
			if (engaged) {
				return std::invoke(KANGARU5_FWD(f), storage.object);
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T&>>{};
			}
		}
		
		template<std::invocable<T const&> F>
			requires(detail::is_specialisation_of_v<optional, std::invoke_result_t<F, T const&>>)
		constexpr auto and_then(F&& f) const& {
			if (engaged) {
				return std::invoke(KANGARU5_FWD(f), storage.object);
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T const&>>{};
			}
		}
		
		template<std::invocable<T&&> F>
			requires(detail::is_specialisation_of_v<optional, std::invoke_result_t<F, T&&>>)
		constexpr auto and_then(F&& f) && {
			if (engaged) {
				return std::invoke(KANGARU5_FWD(f), std::move(storage.object));
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T>>{};
			}
		}
		
		template<std::invocable<T const&&> F>
			requires(detail::is_specialisation_of_v<optional, std::invoke_result_t<F, T const&&>>)
		constexpr auto and_then(F&& f) const&& {
			if (engaged) {
				return std::invoke(KANGARU5_FWD(f), std::move(storage.object));
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T const>>{};
			}
		}
		
		template<std::invocable<T&> F>
		constexpr auto transform(F&& f) & {
			using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
			if (engaged) {
				return optional<U>(in_place, std::invoke(KANGARU5_FWD(f), storage.object));
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T const&> F>
		constexpr auto transform(F&& f) const& {
			using U = std::remove_cv_t<std::invoke_result_t<F, T const&>>;
			if (engaged) {
				return optional<U>(in_place, std::invoke(KANGARU5_FWD(f), storage.object));
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T&&> F>
		constexpr auto transform(F&& f) && {
			using U = std::remove_cv_t<std::invoke_result_t<F, T&&>>;
			if (engaged) {
				return optional<U>(std::invoke(KANGARU5_FWD(f), std::move(storage.object)));
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T const&&> F>
		constexpr auto transform(F&& f) const&& {
			using U = std::remove_cv_t<std::invoke_result_t<F, T const&&>>;
			if (engaged) {
				return optional<U>(std::invoke(KANGARU5_FWD(f), std::move(storage.object)));
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable F> requires std::copy_constructible<T>
		constexpr auto or_else(F&& f) const& -> optional {
			static_assert(std::same_as<optional<T>, std::remove_cvref_t<std::invoke_result_t<F>>>);
			return *this ? *this : KANGARU5_FWD(f)();
		}
		
		template<std::invocable F> requires std::move_constructible<T>
		constexpr auto or_else(F&& f) && -> optional {
			static_assert(std::same_as<optional<T>, std::remove_cvref_t<std::invoke_result_t<F>>>);
			return *this ? std::move(*this) : KANGARU5_FWD(f)();
		}
		
	private:
		struct storage_initialize_t {} static constexpr storage_initialize{};
		
		union storage_t {
			constexpr explicit storage_t() : empty{} {}
			constexpr explicit storage_t(detail::optional_private::empty) : empty{} {}
			
			template<typename... Args>
			explicit(sizeof...(Args) == 0) storage_t(storage_initialize_t, Args&&... args) : object(KANGARU5_FWD(args)...) {}
			
			constexpr ~storage_t() requires(std::is_trivially_destructible_v<T>) = default;
			constexpr ~storage_t() requires(not std::is_trivially_destructible_v<T>) {}
			
			detail::optional_private::empty empty;
			std::remove_cv_t<T> object;
		} storage;
		
		constexpr auto destroy() noexcept -> void {
			if constexpr (not std::is_trivially_destructible_v<T>) {
				std::destroy_at(std::addressof(storage.object));
			}
			storage.empty = {};
			engaged = false;
		}
		
		template<typename... Args> requires std::constructible_from<T, Args&&...>
		constexpr auto construct(Args&&... args) {
			std::construct_at(std::addressof(storage.object), KANGARU5_FWD(args)...);
			engaged = true;
		}
		
		bool engaged = false;
	};
	
	template<lvalue_reference T>
	struct optional<T> {
	private:
		using object_type = std::remove_reference_t<T>;
		
	public:
		using value_type = std::remove_reference_t<T>;
		
		constexpr optional() = default;
		constexpr optional(optional const&) = default;
		constexpr optional(optional&&) = default;
		constexpr auto operator=(optional const&) -> optional& = default;
		constexpr auto operator=(optional&&) -> optional& = default;
		constexpr ~optional() = default;
		
		template<object U>
			requires(
				    std::convertible_to<U*, object_type*>
				and not detail::is_specialisation_of_v<optional, std::remove_cv_t<U>>
			)
		explicit(false) constexpr optional(U& ref) noexcept : pointer{std::addressof(ref)} {}
		
		template<lvalue_reference U> requires(std::convertible_to<U*, object_type*>)
		constexpr optional(optional<U> const& opt) noexcept : pointer{opt ? std::addressof(*opt) : nullptr} {}
		
		explicit(false) constexpr optional(nullopt_t) noexcept : pointer{nullptr} {}
		
		constexpr auto operator=(T ref) noexcept -> optional& {
			pointer = std::addressof(ref);
			return *this;
		}
		
		constexpr auto operator=(nullopt_t) noexcept -> optional& {
			pointer = nullptr;
			return *this;
		}
		
		constexpr auto swap(optional& other) noexcept -> void {
			std::swap(pointer, other.pointer);
		}
		
		constexpr auto reset() noexcept -> void {
			pointer = nullptr;
		}
		
		template<std::convertible_to<object_type&> U>
		constexpr auto emplace(U&& value) noexcept -> object_type& {
			pointer = std::addressof(static_cast<object_type&>(value));
			return *pointer;
		}
		
		constexpr auto value() const& -> object_type& {
			return *pointer;
		}
		
		constexpr auto operator*() const& noexcept -> object_type& {
			return *pointer;
		}
		
		constexpr auto operator->() const& noexcept -> object_type* {
			return pointer;
		}
		
		constexpr auto has_value() const noexcept -> bool {
			return pointer != nullptr;
		}
		
		explicit constexpr operator bool() noexcept {
			return has_value();
		}
		
		template<typename U = object_type> requires std::convertible_to<U&&, object_type&>
		constexpr auto value_or(U&& default_value) const& noexcept -> object_type& {
			return has_value() ? **this : static_cast<object_type&>(KANGARU5_FWD(default_value));
		}
		
		template<std::invocable<T&> F>
		constexpr auto and_then(F&& f) & {
			if (pointer) {
				return std::invoke(KANGARU5_FWD(f), *pointer);
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T&>>{};
			}
		}
		
		template<std::invocable<T const&> F>
		constexpr auto and_then(F&& f) const& {
			if (pointer) {
				return std::invoke(KANGARU5_FWD(f), *pointer);
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T const&>>{};
			}
		}
		
		template<std::invocable<T&> F>
		constexpr auto and_then(F&& f) && {
			if (pointer) {
				return std::invoke(KANGARU5_FWD(f), *pointer);
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T&>>{};
			}
		}
		
		template<std::invocable<T const&> F>
		constexpr auto and_then(F&& f) const&& {
			if (pointer) {
				return std::invoke(KANGARU5_FWD(f), *pointer);
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T const&>>{};
			}
		}
		
		template<std::invocable<T&> F>
		constexpr auto transform(F&& f) & {
			using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
			if (pointer) {
				return optional<U>{std::invoke(KANGARU5_FWD(f), *pointer)};
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T const&> F>
		constexpr auto transform(F&& f) const& {
			using U = std::remove_cv_t<std::invoke_result_t<F, T const&>>;
			if (pointer) {
				return optional<U>{std::invoke(KANGARU5_FWD(f), *pointer)};
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T&> F>
		constexpr auto transform(F&& f) && {
			using U = std::remove_cv_t<std::invoke_result_t<F, T&>>;
			if (pointer) {
				return optional<U>{std::invoke(KANGARU5_FWD(f), *pointer)};
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T const&> F>
		constexpr auto transform(F&& f) const&& {
			using U = std::remove_cv_t<std::invoke_result_t<F, T const&>>;
			if (pointer) {
				return optional<U>{std::invoke(KANGARU5_FWD(f), *pointer)};
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable F> requires std::copy_constructible<T>
		constexpr auto or_else(F&& f) const& -> optional {
			static_assert(std::same_as<optional<T>, std::remove_cvref_t<std::invoke_result_t<F>>>);
			return *this ? *this : KANGARU5_FWD(f)();
		}
		
		template<std::invocable F> requires std::move_constructible<T>
		constexpr auto or_else(F&& f) && -> optional {
			static_assert(std::same_as<optional<T>, std::remove_cvref_t<std::invoke_result_t<F>>>);
			return *this ? std::move(*this) : KANGARU5_FWD(f)();
		}
		
	private:
		object_type* pointer = nullptr;
	};
}

#include "undef.hpp"

#endif

#endif // KANGARU5_DETAIL_OPTIONAL_HPP
