#ifndef KANGARU5_DETAIL_METADATA_HPP
#define KANGARU5_DETAIL_METADATA_HPP

#include "concepts.hpp"
#include <type_traits>

namespace kangaru {
	template<typename>
	struct metadata_for {};
	
	struct metadata_tag {};
	
	template<typename T>
	inline constexpr auto metadata_for_v = metadata_for<T>{};
	
	template<typename T>
	concept weak_metadata = requires {
		requires std::same_as<metadata_tag, typename T::meta>;
	};
	
	namespace detail::metadata {
		auto meta(metadata_for<struct metadata_poison>) = delete;
		
		template<typename T>
		concept adl_nonmember_metadata =
			requires(metadata_for<T> tag) {
				{ meta(tag) } -> kangaru::weak_metadata;
			};
		
		template<typename T>
		struct metadata_function {};
		
		template<adl_nonmember_metadata T>
		struct metadata_function<T> {
			using type = decltype(meta(metadata_for_v<T>));
		};
		
		template<typename T>
		using metadata_function_t = typename metadata_function<T>::type;
	}
	
	template<typename T>
	struct metadata {};
	
	template<detail::metadata::adl_nonmember_metadata T>
	struct metadata<T> {
		using type = detail::metadata::metadata_function_t<T>;
	};
	
	template<typename T>
	using metadata_t = typename metadata<T>::type;
	
	template<typename T>
	concept provides_metadata = weak_metadata<metadata_t<T>>;
	
	template<typename T>
	struct is_empty_injection_constructible : std::false_type {};
	
	template<typename T>
	concept metadata_empty_injection_constructible =
		    provides_metadata<T>
		and requires {
			requires std::same_as<typename metadata_t<T>::allow_empty_injection, std::true_type>;
		};
	
	template<metadata_empty_injection_constructible T>
	struct is_empty_injection_constructible<T> : std::true_type {};
	
	template<typename T>
	concept empty_injection_constructible = is_empty_injection_constructible<T>::value;
	
	struct empty_injectable {
		using meta = metadata_tag;
		using allow_empty_injection = std::true_type;
	};
	
	struct cached {
		using meta = metadata_tag;
		using allow_runtime_caching = std::true_type;
	};
	
	template<typename... Metadatas>
	struct meta : Metadatas... {};
}

#endif // KANGARU5_DETAIL_METADATA_HPP
