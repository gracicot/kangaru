#ifndef KANGARU5_DETAIL_CTTI_HPP
#define KANGARU5_DETAIL_CTTI_HPP

#include "concepts.hpp"
#include "murmur.hpp"

#include <string_view>
#include <cstddef>

#include "define.hpp"

namespace kangaru::detail::ctti {
	template<typename T>
	inline KANGARU5_CONSTEVAL_IF_POSSIBLE auto raw_typed_signature() -> std::string_view {
		return KANGARU5_FUNCTION_SIGNATURE;
	}
	
	inline constexpr auto signature_prefix_length = std::size_t{raw_typed_signature<int>().find("int")};
	inline constexpr auto signature_postfix_length = std::size_t{raw_typed_signature<int>().size() - signature_prefix_length - std::string_view{"int"}.size()};
	
	static_assert(signature_prefix_length != std::string_view::npos, "Cannot find the type name in the function signature");
	
	// TODO: Get stable type name on all compilers
	template<typename T>
	inline KANGARU5_CONSTEVAL_IF_POSSIBLE auto type_name_prefix_length() -> std::size_t {
		using namespace std::literals;
		auto const sig_prefix_trimmed = raw_typed_signature<T>().substr(signature_prefix_length);
		
		if (sig_prefix_trimmed.starts_with("class")) {
			return signature_prefix_length + "class"sv.size();
		}
		
		if (sig_prefix_trimmed.starts_with("struct")) {
			return signature_prefix_length + "struct"sv.size();
		}
		
		return signature_prefix_length;
	}
	
	template<typename T>
	inline KANGARU5_CONSTEVAL_IF_POSSIBLE auto type_name() -> std::string_view {
		auto const sig_prefix_trimmed = raw_typed_signature<T>().substr(type_name_prefix_length<T>());
		return raw_typed_signature<T>().substr(
			0,
			sig_prefix_trimmed.size() - signature_postfix_length
		);
	}
	
	template<typename T>
	inline KANGARU5_CONSTEVAL_IF_POSSIBLE auto type_id_for() {
		return detail::murmur::murmur64a(type_name<T>());
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CTTI_HPP
