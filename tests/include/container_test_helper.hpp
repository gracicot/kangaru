#pragma once

#include <kangaru/kangaru.hpp>

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

struct empty {};
struct empty_injectable {};

template<>
struct kangaru::allow_empty_injection<empty_injectable> : std::true_type {};

struct unmapped_abstract {
	unmapped_abstract(float value) : value{value} {}
	virtual ~unmapped_abstract() = 0;
	float value;
};

unmapped_abstract::~unmapped_abstract() = default;

struct unmapped_concrete : unmapped_abstract {
	unmapped_concrete(float value) : unmapped_abstract{value} {}
};

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
