#ifndef KANGARU5_DETAIL_CONFIG_HPP
#define KANGARU5_DETAIL_CONFIG_HPP

#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru::detail::config {
	auto config(auto&&) -> void requires false;
	
	template<injectable T, typename Default, template<typename> typename Config>
	struct config_function {
		template<typename A>
		static constexpr auto lookup(int) -> decltype(config(std::declval<Config<A>&>()));
		
		template<typename A>
		static constexpr auto lookup(void*) -> Default;
	public:
		using type = decltype(lookup<T>(0));
	};
	
	template<injectable T, typename Default, template<typename> typename Config>
	using config_function_t = typename config_function<T, Default, Config>::type;
	
	template<template<typename> typename Trait, injectable T>
	struct evaluate_config {
		using type = Trait<T>;
	};
	
	template<template<typename> typename Trait, injectable T>
		requires requires{ typename Trait<T>::template ttype<T>; }
	struct evaluate_config<Trait, T> {
		using type = typename Trait<T>::template ttype<T>;
	};
	
	template<template<injectable> typename Trait, injectable T>
	using evaluate_config_t = typename evaluate_config<Trait, T>::type;
}

KANGARU5_EXPORT namespace kangaru {
	template<injectable T>
	struct allow_runtime_caching {
		template<injectable A>
		using ttype = typename detail::config::config_function_t<A, std::false_type, allow_runtime_caching>;
	};
	
	template<injectable T>
	inline constexpr auto allow_runtime_caching_v = detail::config::evaluate_config_t<allow_runtime_caching, T>::value;
	
	template<injectable T>
	struct allow_empty_injection {
		template<injectable A>
		using ttype = typename detail::config::config_function_t<A, std::false_type, allow_empty_injection>;
	};
	
	template<injectable T>
	inline constexpr auto allow_empty_injection_v = detail::config::evaluate_config_t<allow_empty_injection, T>::value;
	
	template<injectable T>
	struct overrides_types_in_cache {
		template<injectable A>
		struct ttype {
			using type = detail::config::config_function_t<A, std::tuple<>, overrides_types_in_cache>;
		};
	};
	
	template<typename T>
	using overrides_types_in_cache_t = typename detail::config::evaluate_config_t<overrides_types_in_cache, T>::type;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONFIG_HPP
