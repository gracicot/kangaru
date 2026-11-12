#pragma once

#include <kangaru/kangaru.hpp>
#include <kangaru/compatibility_v4.hpp>

namespace kangaru::compatibility_v4::detail {
	template<typename Service, typename... Args>
	struct is_service_valid {
		static constexpr auto value = requires(kgr::container& container, Args&&... args) {
			{ container.service<Service>(std::forward<Args>(args)...) } -> std::same_as<service_type<Service>>;
		};
	};
	
	template<typename Service, typename... Args>
	struct is_construction_valid {
		static constexpr auto value = requires(kgr::container& container, Args&&... args) {
			{ container.service<Service>(std::forward<Args>(args)...) } -> std::same_as<service_type<Service>>;
		};
	};
	
	template<typename Service, typename... Args>
	struct is_emplace_valid {
		static constexpr auto value = requires(kgr::container& container, Args&&... args) {
			{ container.emplace<Service>(std::forward<Args>(args)...) } -> std::same_as<bool>;
		};
	};
	
	template<typename Service>
	struct is_single {
		static constexpr auto value = std::derived_from<Service, single>;
	};
	
	template<typename Service>
	struct is_service {
		static constexpr auto value = v4_source<Service>;
	};
	
	template<typename Service>
	struct is_supplied_service {
		static constexpr auto value = std::derived_from<Service, supplied>;
	};
	
	template<typename Service, typename>
	struct is_valid_autocall_function {
		// autocall is not checked yet
		static constexpr auto value = true;
	};
}
