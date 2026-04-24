#ifndef KANGARU5_DETAIL_TYPE_ID_HPP
#define KANGARU5_DETAIL_TYPE_ID_HPP

#include "concepts.hpp"
#include "murmur.hpp"

#ifndef KANGARU5_MODULES
#include <string_view>
#include <cstddef>
#include <compare>
#endif

#include "define.hpp"

namespace kangaru::detail::type_id_private {
	template<typename T>
	inline consteval auto raw_typed_signature() -> std::string_view {
		return KANGARU5_FUNCTION_SIGNATURE;
	}
	
	inline constexpr auto signature_prefix_length = std::size_t{raw_typed_signature<int>().find("int")};
	inline constexpr auto signature_postfix_length = std::size_t{raw_typed_signature<int>().size() - signature_prefix_length - std::string_view{"int"}.size()};
	
	static_assert(raw_typed_signature<int>().substr(signature_prefix_length, 3) == "int");
	
	static_assert(signature_prefix_length != std::string_view::npos, "Cannot find the type name in the function signature");
	
	template<typename T>
	inline consteval auto type_name_prefix_length() -> std::size_t {
		using namespace std::literals;
		auto const sig_prefix_trimmed = KANGARU5_NO_ADL(raw_typed_signature<T>)().substr(signature_prefix_length);
		
		if (sig_prefix_trimmed.starts_with("class ")) {
			return signature_prefix_length + "class "sv.size();
		}
		
		if (sig_prefix_trimmed.starts_with("struct ")) {
			return signature_prefix_length + "struct "sv.size();
		}
		
		return signature_prefix_length;
	}
	
	struct key {};
}

namespace kangaru {
	template<typename T>
	inline consteval auto type_name() -> std::string_view {
		auto const sig_prefix_trimmed = detail::type_id_private::raw_typed_signature<T>().substr(
			detail::type_id_private::type_name_prefix_length<T>()
		);
		
		return sig_prefix_trimmed.substr(
			0,
			sig_prefix_trimmed.size() - detail::type_id_private::signature_postfix_length
		);
	}
	
	template<typename T>
	struct static_type_id {
		template<typename U>
		friend constexpr auto operator==(static_type_id const&, static_type_id<U> const&) -> bool {
			return std::same_as<U, T>;
		}
		
		template<typename U>
		friend constexpr auto operator<=>(static_type_id const&, static_type_id<U> const&) {
			return KANGARU5_NO_ADL(type_name<T>)() <=> KANGARU5_NO_ADL(type_name<U>)();
		}
		
	};
	
	struct type_id {
		template<typename T>
		explicit(false) constexpr type_id(static_type_id<T> const&) :
			hash{detail::murmur::murmur64a(KANGARU5_NO_ADL(type_name<T>)())},
			name{KANGARU5_NO_ADL(type_name<T>)()} {}
		
	private:
		std::size_t hash;
		std::string_view name;
		
		friend constexpr auto operator==(type_id const& lhs, type_id const& rhs) -> bool {
			return lhs.hash == rhs.hash and lhs.name == rhs.name;
		}
		
		friend constexpr auto operator<=>(type_id const& lhs, type_id const& rhs) -> std::strong_ordering {
			// This is still strong ordering, since hash is always derivated from name
			return lhs.name <=> rhs.name;
		}
		
		friend std::hash<type_id>;
	};
	
	template<typename T>
	inline consteval auto type_id_for() -> static_type_id<T> {
		return static_type_id<T>{};
	}
}

namespace std {
	template<>
	struct hash<::kangaru::type_id> {
		inline constexpr auto operator()(::kangaru::type_id const& type) const noexcept -> std::size_t {
			return type.hash;
		}
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_TYPE_ID_HPP
