#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_tostring.hpp>

#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

#include <compare>
#include <type_traits>


#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

template<typename T>
struct agg_unmapped_dependent_on {
	T value;
};

template<typename T>
struct agg_mapped_value_dependent_on {
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<agg_mapped_value_dependent_on<U>>) -> std::true_type;
};

template<typename T>
struct agg_mapped_ref_dependent_on {
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<agg_mapped_ref_dependent_on<U>&>) -> std::true_type;
};

template<typename T>
struct agg_mapped_sptr_dependent_on {
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<std::shared_ptr<agg_mapped_sptr_dependent_on<U>>>) -> std::true_type;
};

template<typename T>
struct agg_mapped_rref_dependent_on {
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<agg_mapped_rref_dependent_on<U>&&>) -> std::true_type;
};

template<typename T>
struct unmapped_dependent_on {
	explicit unmapped_dependent_on(T value) : value(FWD(value)) {}
	T value;
};

template<typename T>
struct mapped_value_dependent_on {
	explicit mapped_value_dependent_on(T value) : value(FWD(value)) {}
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<mapped_value_dependent_on<U>>) -> std::true_type;
};

template<typename T>
struct mapped_ref_dependent_on {
	explicit mapped_ref_dependent_on(T value) : value(FWD(value)) {}
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<mapped_ref_dependent_on<U>&>) -> std::true_type;
};

template<typename T>
struct mapped_sptr_dependent_on {
	explicit mapped_sptr_dependent_on(T value) : value(FWD(value)) {}
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<std::shared_ptr<mapped_sptr_dependent_on<U>>>) -> std::true_type;
};

template<typename T>
struct mapped_rref_dependent_on {
	explicit mapped_rref_dependent_on(T value) : value(FWD(value)) {}
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<mapped_rref_dependent_on<U>&&>) -> std::true_type;
};

auto access(auto& object) -> auto& {
	return object.value;
}

template<typename T>
auto access(std::unique_ptr<T>& object) -> auto& {
	return object->value;
}

template<typename T>
auto access(std::shared_ptr<T>& object) -> auto& {
	return object->value;
}

template<std::size_t level>
auto accessor(auto& object) -> auto& {
	if constexpr (level == 0) {
		return access(object);
	} else {
		return accessor<level - 1>(access(object));
	}
}

template<std::size_t depth, std::size_t level, typename T, typename Final = T>
auto test_provide(auto&& container, auto check) {
	if constexpr (depth > 0) {
		SECTION("Provide through a aggregate unmapped type") {
			decltype(auto) result = kangaru::provide<agg_unmapped_dependent_on<T>>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, agg_unmapped_dependent_on<T>, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a unique_ptr aggregate unmapped type") {
			decltype(auto) result = kangaru::provide<std::unique_ptr<agg_unmapped_dependent_on<T>>>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, std::unique_ptr<agg_unmapped_dependent_on<T>>, Final>(FWD(container), check);
		}
		
		if constexpr (std::copy_constructible<T>)
		SECTION("Provide through a aggregate mapped value type") {
			decltype(auto) result = kangaru::provide<agg_mapped_value_dependent_on<T>>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, agg_mapped_value_dependent_on<T>, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a aggregate mapped ref type") {
			decltype(auto) result = kangaru::provide<agg_mapped_ref_dependent_on<T>&>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, agg_mapped_ref_dependent_on<T>&, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a aggregate mapped sptr type") {
			decltype(auto) result = kangaru::provide<std::shared_ptr<agg_mapped_sptr_dependent_on<T>>>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, std::shared_ptr<agg_mapped_sptr_dependent_on<T>>, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a aggregate mapped rvalue ref type") {
			decltype(auto) result = kangaru::provide<agg_mapped_rref_dependent_on<T>&&>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, agg_mapped_rref_dependent_on<T>&&, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a unmapped type") {
			decltype(auto) result = kangaru::provide<unmapped_dependent_on<T>>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, unmapped_dependent_on<T>, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a unique_ptr unmapped type") {
			decltype(auto) result = kangaru::provide<std::unique_ptr<unmapped_dependent_on<T>>>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, std::unique_ptr<unmapped_dependent_on<T>>, Final>(FWD(container), check);
		}
		
		if constexpr (std::copy_constructible<T>)
		SECTION("Provide through a mapped value type") {
			decltype(auto) result = kangaru::provide<mapped_value_dependent_on<T>>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, mapped_value_dependent_on<T>, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a mapped ref type") {
			decltype(auto) result = kangaru::provide<mapped_ref_dependent_on<T>&>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, mapped_ref_dependent_on<T>&, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a aggregate mapped sptr type") {
			decltype(auto) result = kangaru::provide<std::shared_ptr<mapped_sptr_dependent_on<T>>>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, std::shared_ptr<mapped_sptr_dependent_on<T>>, Final>(FWD(container), check);
		}
		
		SECTION("Provide through a mapped rref type") {
			decltype(auto) result = kangaru::provide<mapped_rref_dependent_on<T>&&>(FWD(container));
			check(static_cast<Final>(accessor<level>(result)));
			test_provide<depth - 1, level + 1, mapped_rref_dependent_on<T>&&, Final>(FWD(container), check);
		}
	}
}

struct initial_value {
	std::uint32_t value = 0xc0ffee;
	friend auto operator<=>(initial_value const&, initial_value const&) = default;
};

template<>
struct Catch::StringMaker<initial_value> {
	static auto convert(initial_value const& v) -> std::string {
		return fmt::format("initial_value{{.value = {}}}", std::to_string(v.value));
	}
};

struct mapped_value {
	initial_value initial;
	friend auto operator<=>(mapped_value const&, mapped_value const&) = default;
	friend auto attribute(kangaru::allow_runtime_caching<mapped_value>) -> std::true_type;
	
	static auto validate_initial(mapped_value const& object) -> void {
		CHECK(object.initial.value != 0xc0ffee);
	}
};

template<>
struct Catch::StringMaker<mapped_value> {
	static auto convert(mapped_value const& v) -> std::string {
		return fmt::format("mapped_value{{.initial = {{.value = {}}}}}", std::to_string(v.initial.value));
	}
};

struct mapped_value_ref_initial_value {
	initial_value& initial;
	friend auto operator==(mapped_value_ref_initial_value const& lhs, mapped_value_ref_initial_value const& rhs) {
		return std::addressof(lhs.initial) == std::addressof(rhs.initial);
	}
	friend auto attribute(kangaru::allow_runtime_caching<mapped_value_ref_initial_value>) -> std::true_type;
	
	static auto validate_initial(mapped_value_ref_initial_value const& object) -> void {
		CHECK(object.initial.value != 0xc0ffee);
	}
};

template<>
struct Catch::StringMaker<mapped_value_ref_initial_value> {
	static auto convert(mapped_value_ref_initial_value const& v) -> std::string {
		return fmt::format("mapped_value_ref_initial_value{{.initial = {{.value = {}}}}}", std::to_string(v.initial.value));
	}
};

struct mapped_ref {
	friend auto attribute(kangaru::allow_runtime_caching<mapped_ref&>) -> std::true_type;
};

struct empty_inject {
	friend auto operator==(empty_inject const& lhs, empty_inject const& rhs) -> bool {
		return true;
	}
};

template<>
struct kangaru::allow_empty_injection<empty_inject> : std::true_type {};

struct mapped_has_empty_injectable {
	explicit mapped_has_empty_injectable(empty_inject o) : o{o} {}
	empty_inject o;
	int instance_id = get_id();
	
	static auto get_id() -> int {
		static int i = 0;
		return ++i;
	}
	
	friend auto operator==(mapped_has_empty_injectable const& lhs, mapped_has_empty_injectable const& rhs) -> bool {
		return lhs.instance_id == rhs.instance_id;
	}
	
	friend auto attribute(kangaru::allow_runtime_caching<mapped_has_empty_injectable>) -> std::true_type;
};

template<>
struct Catch::StringMaker<mapped_has_empty_injectable> {
	static auto convert(mapped_has_empty_injectable const& v) -> std::string {
		return fmt::format("mapped_has_empty_injectable{{.instance_id = {}}}", std::to_string(v.instance_id));
	}
};

struct unmapped_has_empty_injectable {
	empty_inject o;
	
	friend auto operator==(unmapped_has_empty_injectable const& lhs, unmapped_has_empty_injectable const& rhs) -> bool {
		return true;
	}
};

template<>
struct Catch::StringMaker<unmapped_has_empty_injectable> {
	static auto convert(unmapped_has_empty_injectable const& v) -> std::string {
		return "unmapped_has_empty_injectable{}";
	}
};

template<>
struct Catch::StringMaker<mapped_ref> {
	static auto convert(mapped_ref const& v) -> std::string {
		return "mapped_ref{}";
	}
};

struct base_container_default {
	static auto make_base() {
		return kangaru::none_source{};
	}
};

struct base_container_object_source {
	static auto make_base() {
		return kangaru::object_source{initial_value{42}};
	}
};

struct base_container_reference_source {
	static auto make_base() {
		return kangaru::reference_source{initial_value{42}};
	}
};

struct base_container_reference_source_source {
	static auto make_base() {
		return kangaru::object_source{kangaru::reference_source{initial_value{42}}};
	}
};

struct base_container_object_source_source {
	static auto make_base() {
		return kangaru::object_source{kangaru::object_source{initial_value{42}}};
	}
};

template<typename T, typename MakeBase>
struct provide_test {
	using provided_type = T;
	
	static auto make_container() {
		return kangaru::container{MakeBase::make_base()};
	}
};

template<typename T, typename MakeBase>
struct provide_test_polymorphic {
	using provided_type = T;
	
	static auto make_container() {
		return kangaru::polymorphic_container{MakeBase::make_base()};
	}
};

using provide_test_self = provide_test<kangaru::container<>&, base_container_default>;
using provide_test_polymorphic_self = provide_test_polymorphic<kangaru::polymorphic_container<>&, base_container_default>;
using provide_test_mapped_ref = provide_test<mapped_ref&, base_container_default>;
using provide_test_polymorphic_mapped_ref = provide_test_polymorphic<mapped_ref&, base_container_default>;
using provide_test_mapped_value_base_object_source = provide_test<mapped_value, base_container_object_source>;
using provide_test_polymorphic_mapped_value_base_object_source = provide_test_polymorphic<mapped_value, base_container_object_source>;
using provide_test_mapped_value_base_object_source_source = provide_test<mapped_value, base_container_object_source_source>;
using provide_test_polymorphic_mapped_value_base_object_source_source = provide_test_polymorphic<mapped_value, base_container_object_source_source>;
using provide_test_mapped_value_base_reference_source = provide_test<mapped_value_ref_initial_value, base_container_reference_source>;
using provide_test_polymorphic_mapped_value_base_reference_source = provide_test_polymorphic<mapped_value_ref_initial_value, base_container_reference_source>;
using provide_test_mapped_value_base_reference_source_source = provide_test<mapped_value_ref_initial_value, base_container_reference_source_source>;
using provide_test_polymorphic_mapped_value_base_reference_source_source = provide_test_polymorphic<mapped_value_ref_initial_value, base_container_reference_source_source>;
using provide_test_mapped_has_empty = provide_test<mapped_has_empty_injectable, base_container_default>;
using provide_test_polymorphic_mapped_has_empty = provide_test_polymorphic<mapped_has_empty_injectable, base_container_default>;
using provide_test_unmapped_has_empty = provide_test<unmapped_has_empty_injectable, base_container_default>;
using provide_test_polymorphic_unmapped_has_empty = provide_test_polymorphic<unmapped_has_empty_injectable, base_container_default>;
using provide_test_empty_injectable = provide_test<empty_inject, base_container_default>;
using provide_test_polymorphic_empty_injectable = provide_test_polymorphic<empty_inject, base_container_default>;

using TestType = KANGARU5_TEST_TYPE;

template<typename T>
auto validate_initial(T& object) {
	if constexpr (requires(T& p) { T::validate_initial(p); }) {
		T::validate_initial(object);
	}
}

TEST_CASE("Exhaustive provide expansion", "[container]") {
	auto container = TestType::make_container();
	decltype(auto) provided = kangaru::provide<TestType::provided_type>(container);
	validate_initial(provided);
	test_provide<2, 0, TestType::provided_type>(container, [&](auto&& value) {
		if constexpr (std::is_reference_v<TestType::provided_type>) {
			CHECK(std::addressof(provided) == std::addressof(value));
		} else {
			CHECK(provided == value);
		}
	});
}

TEST_CASE("Exhaustive provide expansion rvalue", "[container]") {
	auto container = TestType::make_container();
	decltype(auto) provided = kangaru::provide<TestType::provided_type>(container);
	validate_initial(provided);
	test_provide<2, 0, TestType::provided_type>(std::move(container), [&](auto&& value) {
		if constexpr (std::is_reference_v<TestType::provided_type>) {
			CHECK(std::addressof(provided) == std::addressof(value));
		} else {
			CHECK(provided == value);
		}
	});
}
