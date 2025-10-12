#ifndef KANGARU5_DETAIL_OPTIONAL_HPP
#define KANGARU5_DETAIL_OPTIONAL_HPP

#include "concepts.hpp"
#include "source.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>
#include <compare>
#include <functional>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT struct in_place_t{} inline constexpr in_place;
	KANGARU5_EXPORT struct nullopt_t {} inline constexpr nullopt;
	
	KANGARU5_EXPORT template<injectable T>
	struct optional;
}

namespace kangaru::detail::optional {
	struct empty {};
	
	template<typename T, typename Arg>
	concept optional_forwarded_construction_object =
		    different_from<in_place_t, std::decay_t<Arg>>
		and injectable<T>
		and different_from<kangaru::optional<T>, std::decay_t<Arg>>
		and std::constructible_from<T, Arg&&>;
}

namespace kangaru {
	KANGARU5_EXPORT template<unqualified_object T>
	struct optional<T> {
		constexpr optional() = default;
		
		explicit(false) constexpr optional(nullopt_t) noexcept : engaged{false}, storage{detail::optional::empty{}} {}
		
		constexpr optional(optional const& other)
			requires(not std::is_trivially_copy_constructible_v<T>) : engaged{other.engaged}
		{
			if (engaged) {
				std::construct_at(std::addressof(storage.object), other.storage.object);
			}
		}
		
		constexpr optional(optional const& other)
			requires(std::is_trivially_copy_constructible_v<T>) = default;
		
		constexpr optional(optional&& other)
			noexcept(std::is_nothrow_move_constructible_v<T>)
			requires(not std::is_trivially_move_constructible_v<T>) :
				engaged{other.engaged}
		{
			if (engaged) {
				std::construct_at(std::addressof(storage.object), std::move(other.storage.object));
			}
		}
		
		constexpr optional(optional&& other)
			noexcept(std::is_nothrow_move_constructible_v<T>)
			requires(std::is_trivially_move_constructible_v<T>) = default;
		
		template<injectable U>
		explicit(!std::convertible_to<U&&, T>)
		constexpr optional(optional<U> const& other) : engaged{other.has_value()}, storage{static_cast<T>(*other)} {}
		
		template<injectable U>
		explicit(!std::convertible_to<U&&, T>)
		constexpr optional(optional<U>&& other) : engaged{other.has_value()}, storage{.object{static_cast<T>(*std::move(other))}} {}
		
		template<typename... Args> requires std::constructible_from<T, Args&&...>
		constexpr optional(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) :
			engaged{true}, storage{.object{KANGARU5_FWD(args)...}} {}
		
		template<typename U, typename... Args> requires std::constructible_from<T, std::initializer_list<U>&, Args&&...>
		constexpr explicit optional(in_place_t, std::initializer_list<U> ilist, Args&&... args) : engaged{true}, storage{.object{KANGARU5_FWD(args)...}} {}
		
		template<typename U>
			requires detail::optional::optional_forwarded_construction_object<T, U>
			explicit(!std::convertible_to<U&&, T>)
		constexpr optional(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>) :
			engaged{true}, storage{.object{KANGARU5_FWD(value)}} {}
		
		constexpr auto swap(optional const& other) -> void {
			if (other.engaged) {
				if (engaged) {
					using std::swap;
					swap(storage.object, other.storage.object);
				} else {
					construct(other.storage.object);
					other.reset();
				}
			} else {
				reset();
			}
		}
		
		constexpr auto operator=(optional const& rhs)
			noexcept(std::is_nothrow_copy_constructible_v<T> and std::is_nothrow_copy_assignable_v<T>) -> optional&
			requires(not std::is_trivially_copy_assignable_v<T>)
		{
			if (rhs.engaged) {
				if (engaged) {
					storage.object = rhs.storage.object;
				} else {
					emplace(std::move(rhs.storage.object));
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
					using std::swap;
					swap(storage.object, rhs.storage.object);
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
		
		constexpr ~optional() noexcept(std::is_nothrow_destructible_v<T>) {
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
		
		constexpr auto swap(optional&& other) -> void {
			if (other.engaged) {
				if (engaged) {
					using std::swap;
					swap(storage.object, std::move(other.storage.object));
				} else {
					construct(std::move(other.storage.object));
					other.reset();
				}
			} else {
				reset();
			}
		}
		
		constexpr auto reset() -> void {
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
			return storage.object;
		}
		
		constexpr auto value() const& -> T const& {
			return storage.object;
		}
		
		constexpr auto value() && -> T&& {
			return std::move(storage.object);
		}
		
		constexpr auto value() const&& -> T const&& {
			return std::move(storage.object);
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
		
		explicit constexpr operator bool() noexcept {
			return has_value();
		}
		
		template<std::three_way_comparable_with<T> U> requires unqualified_object<U>
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
		
		template<typename U = std::remove_cv_t<T>> requires (std::is_copy_constructible_v<T> and std::convertible_to<U&&, T>)
		constexpr auto value_or(U&& default_value) const& -> T {
			return has_value() ? **this : static_cast<T>(KANGARU5_FWD(default_value));
		}
		
		template<typename U = std::remove_cv_t<T>> requires (std::is_move_constructible_v<T> and std::convertible_to<U&&, T>)
		constexpr auto value_or(U&& default_value) && -> T {
			return has_value() ? *std::move(*this) : static_cast<T>(KANGARU5_FWD(default_value));
		}
		
		template<std::invocable<T&> F>
		constexpr auto and_then(F&& f) & {
			if (engaged) {
				return std::invoke(KANGARU5_FWD(f), storage.object);
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T&>>{};
			}
		}
		
		template<std::invocable<T const&> F>
		constexpr auto and_then(F&& f) const& {
			if (engaged) {
				return std::invoke(KANGARU5_FWD(f), storage.object);
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T const&>>{};
			}
		}
		
		template<std::invocable<T> F>
		constexpr auto and_then(F&& f) && {
			if (engaged) {
				return std::invoke(KANGARU5_FWD(f), std::move(storage.object));
			} else {
				return std::remove_cvref_t<std::invoke_result_t<F, T>>{};
			}
		}
		
		template<std::invocable<T const> F>
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
				return optional<U>{std::invoke(KANGARU5_FWD(f), storage.object)};
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T const&> F>
		constexpr auto transform(F&& f) const& {
			using U = std::remove_cv_t<std::invoke_result_t<F, T const&>>;
			if (engaged) {
				return optional<U>{std::invoke(KANGARU5_FWD(f), storage.object)};
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T> F>
		constexpr auto transform(F&& f) && {
			using U = std::remove_cv_t<std::invoke_result_t<F, T>>;
			if (engaged) {
				return optional<U>{std::invoke(KANGARU5_FWD(f), std::move(storage.object))};
			} else {
				return optional<U>{};
			}
		}
		
		template<std::invocable<T const> F>
		constexpr auto transform(F&& f) const&& {
			using U = std::remove_cv_t<std::invoke_result_t<F, T const>>;
			if (engaged) {
				return optional<U>{std::invoke(KANGARU5_FWD(f), std::move(storage.object))};
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
		union {
			detail::optional::empty empty{};
			T object;
		} storage{.empty{}};
		
		constexpr auto destroy() -> void {
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
	
	KANGARU5_EXPORT template<reference T>
	struct optional<T> {
	private:
		using object_type = std::remove_reference_t<T>;
		
	public:
		constexpr optional() = default;
		constexpr optional(optional const&) = default;
		constexpr optional(optional&&) = default;
		constexpr auto operator=(optional const&) -> optional& = default;
		constexpr auto operator=(optional&&) -> optional& = default;
		constexpr ~optional() = default;
		
		template<object U>
			requires(
				    std::is_lvalue_reference_v<T>
				and std::convertible_to<U*, T*>
				and not detail::utility::is_specialisation_of_v<optional, std::remove_cv_t<U>>
			)
		explicit(false) constexpr optional(U& ref) noexcept : pointer{std::addressof(ref)} {}
		
		template<object U>
			requires(
				    std::is_rvalue_reference_v<T>
				and std::convertible_to<U*, T*>
				and not detail::utility::is_specialisation_of_v<optional, std::remove_cv_t<U>>
			)
		explicit(false) constexpr optional(U&& ref) noexcept : pointer{std::addressof(ref)} {}
		
		template<reference U> requires std::convertible_to<U*, T*>
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
		
		template<std::convertible_to<T&> U>
		constexpr auto emplace(U&& value) noexcept -> T& {
			pointer = std::addressof(static_cast<T&>(value));
			return *pointer;
		}
		
		constexpr auto value() const& -> T& {
			return *pointer;
		}
		
		constexpr auto operator*() const& noexcept -> T& {
			return *pointer;
		}
		
		constexpr auto operator->() const& noexcept -> T* {
			return std::addressof(pointer);
		}
		
		constexpr auto has_value() const noexcept -> bool {
			return pointer != nullptr;
		}
		
		explicit constexpr operator bool() noexcept {
			return has_value();
		}
		
		template<typename U = std::remove_cvref_t<T>> requires std::convertible_to<U&&, T&>
		constexpr auto value_or(U&& default_value) const& noexcept -> T& {
			return has_value() ? **this : static_cast<T&>(KANGARU5_FWD(default_value));
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
		
	public:
		object_type* pointer = nullptr;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_OPTIONAL_HPP
