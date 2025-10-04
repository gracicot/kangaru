#ifndef KANGARU5_DETAIL_TAG_HPP
#define KANGARU5_DETAIL_TAG_HPP

#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<typename>
	struct tag_for {};
	
	KANGARU5_EXPORT struct tags_tag {};
	
	KANGARU5_EXPORT template<typename T>
	inline constexpr auto tag_for_v = tag_for<T>{};
	
	KANGARU5_EXPORT template<typename T>
	concept weak_tag = requires {
		requires std::same_as<tags_tag, typename T::meta>;
	};
	
	namespace detail::tag {
		auto tag(auto&&) -> void requires false;
		
		template<typename T>
		concept adl_nonmember_tag =
			requires(tag_for<T> t) {
				{ tag(t) } -> kangaru::weak_tag;
			};
		
		template<typename T>
		struct tag_function {};
		
		template<adl_nonmember_tag T>
		struct tag_function<T> {
			using type = decltype(tag(tag_for_v<T>));
		};
		
		template<typename T>
		using tag_function_t = typename tag_function<T>::type;
	}
	
	KANGARU5_EXPORT template<typename T>
	struct tags_of {};
	
	KANGARU5_EXPORT template<detail::tag::adl_nonmember_tag T>
	struct tags_of<T> {
		using type = detail::tag::tag_function_t<T>;
	};
	
	KANGARU5_EXPORT template<typename T>
	using tags_of_t = typename tags_of<T>::type;
	
	KANGARU5_EXPORT template<typename T>
	concept provides_tags = weak_tag<tags_of_t<T>>;
	
	KANGARU5_EXPORT template<typename T>
	concept tag_empty_injection_constructible =
		    provides_tags<T>
		and requires {
			requires std::same_as<typename tags_of_t<T>::allow_empty_injection, std::true_type>;
		};
	 
	KANGARU5_EXPORT template<typename T>
	concept tag_allow_caching =
		    provides_tags<T>
		and requires {
			requires std::same_as<typename tags_of_t<T>::allow_runtime_caching, std::true_type>;
		};
	
	KANGARU5_EXPORT template<typename T>
	inline constexpr auto is_empty_injection_constructible_v = tag_empty_injection_constructible<T>;
	
	KANGARU5_EXPORT template<typename T>
	concept tag_overrides_types_in_cache =
		    provides_tags<T>
		and requires {
			typename tags_of_t<T>::overrides_types_in_cache;
			{ detail::utility::decay_copy(std::tuple_size<typename tags_of_t<T>::overrides_types_in_cache>::value) } -> std::same_as<std::size_t>;
		};
	
	KANGARU5_EXPORT template<typename>
	struct overrides_types_in_cache {
		using type = std::tuple<>;
	};
	
	KANGARU5_EXPORT template<tag_overrides_types_in_cache T>
	struct overrides_types_in_cache<T> {
		using type = typename tags_of_t<T>::overrides_types_in_cache;
	};
	
	KANGARU5_EXPORT template<typename T>
	using overrides_types_in_cache_t = typename overrides_types_in_cache<T>::type;
	
	KANGARU5_EXPORT struct empty_injectable {
		using meta = tags_tag;
		using allow_empty_injection = std::true_type;
	};
	
	KANGARU5_EXPORT struct cached {
		using meta = tags_tag;
		using allow_runtime_caching = std::true_type;
	};
	
	KANGARU5_EXPORT template<typename T>
	inline constexpr auto is_cachable_v = tag_allow_caching<T>;
	
	KANGARU5_EXPORT template<typename... Ts> requires (sizeof...(Ts) > 0)
	struct overrides {
		using meta = tags_tag;
		
		// TODO: Use meta list
		using overrides_types_in_cache = std::tuple<Ts...>;
	};
	
	KANGARU5_EXPORT template<weak_tag... Tags>
	struct tags : Tags... {
		using meta = tags_tag;
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_TAG_HPP
