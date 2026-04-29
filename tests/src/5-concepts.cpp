#include <kangaru/kangaru.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>

using namespace kangaru;

static_assert(different_from<int, float>);
static_assert(not different_from<int, int>);
static_assert(different_from<int const, int>);
static_assert(different_from<int&, int>);

static_assert(forwarded<int&&, int>);
static_assert(forwarded<int&, int>);
static_assert(forwarded<int, int>);
static_assert(forwarded<int const&, int>);
static_assert(forwarded<int const&&, int>);
static_assert(not forwarded<int*, int>);

static_assert(object<int>);
static_assert(object<int const>);
static_assert(object<int[5]>);
static_assert(object<int[]>);
static_assert(not object<int&>);
static_assert(not object<int()>);

static_assert(forwarded_object<int>);
static_assert(forwarded_object<int const>);
static_assert(forwarded_object<int&>);
static_assert(forwarded_object<int&&>);
static_assert(forwarded_object<int const&>);
static_assert(forwarded_object<int const&&>);
static_assert(forwarded_object<int(&)[5]>);
static_assert(not forwarded_object<int(&)()>);
static_assert(forwarded_object<int(*)()>);
static_assert(not forwarded<int, int&>);
static_assert(not forwarded<int, int const>);

static_assert(unqualified_object<int>);
static_assert(not unqualified_object<int&>);
static_assert(not unqualified_object<int const>);
static_assert(not unqualified_object<int volatile>);
static_assert(not unqualified_object<int const volatile>);
static_assert(unqualified_object<int[6]>);
static_assert(unqualified_object<int[]>);
static_assert(unqualified_object<int*>);
static_assert(not unqualified_object<int* const>);
static_assert(unqualified_object<int const*>);

static_assert(reference<int&>);
static_assert(reference<int&&>);
static_assert(reference<int(&)[5]>);
static_assert(not reference<int>);
static_assert(not reference<int const>);
static_assert(not reference<int()>);
static_assert(not reference<void>);

static_assert(lvalue_reference<int&>);
static_assert(not lvalue_reference<int&&>);
static_assert(not lvalue_reference<int>);
static_assert(not lvalue_reference<int const>);

static_assert(rvalue_reference<int&&>);
static_assert(not rvalue_reference<int&>);
static_assert(not rvalue_reference<int>);
static_assert(not rvalue_reference<int const>);

struct immovable {
	immovable(immovable&&) = delete;
	immovable(immovable const&) = delete;
	auto operator=(immovable&&) -> immovable& = delete;
	auto operator=(immovable const&) -> immovable& = delete;
};

struct abstract {
	virtual ~abstract() = 0;
};

static_assert(movable_object<int>);
static_assert(movable_object<int*>);
static_assert(not movable_object<int()>);
static_assert(not movable_object<abstract>);
static_assert(not movable_object<void>);
static_assert(not movable_object<int[]>);
static_assert(not movable_object<int[7]>);
static_assert(not movable_object<int const>);
static_assert(not movable_object<immovable>);
static_assert(not movable_object<int&>);

static_assert(forwarded_movable_object<int>);
static_assert(not forwarded_movable_object<immovable>);
static_assert(forwarded_movable_object<int&>);
static_assert(forwarded_movable_object<int&&>);
static_assert(not forwarded_movable_object<immovable&>);

static_assert(copiable_object<int>);
static_assert(not copiable_object<int()>);
static_assert(not copiable_object<abstract>);
static_assert(not copiable_object<void>);
static_assert(not copiable_object<int[]>);
static_assert(not copiable_object<int[7]>);
static_assert(not copiable_object<int const>);
static_assert(not copiable_object<immovable>);
static_assert(not copiable_object<int&>);

static_assert(forwarded_copiable_object<int>);
static_assert(not forwarded_copiable_object<std::unique_ptr<int>>);
static_assert(not forwarded_copiable_object<immovable>);
static_assert(forwarded_copiable_object<int&>);
static_assert(not forwarded_copiable_object<immovable&>);

static_assert(function_object<int>);
static_assert(function_object<int*>);
static_assert(not function_object<int()>);
static_assert(not function_object<abstract>);
static_assert(not function_object<void>);
static_assert(not function_object<int[]>);
static_assert(not function_object<int[7]>);
static_assert(not function_object<int const>);
static_assert(not function_object<immovable>);
static_assert(not function_object<int&>);

static_assert(forwarded_function_object<int>);
static_assert(not forwarded_function_object<immovable>);
static_assert(forwarded_function_object<int&>);
static_assert(forwarded_function_object<int&&>);
static_assert(not forwarded_function_object<immovable&>);

static_assert(not not_self<int, int>);
static_assert(not not_self<int&, int>);
static_assert(not not_self<int&&, int>);
static_assert(not not_self<int const&, int>);
static_assert(not not_self<int, int&>);
static_assert(not_self<float, int>);
static_assert(not not_self<float, int&>);
static_assert(not_self<int*, int[4]>);
static_assert(not not_self<int[4], int*>);

static_assert(not pointer<int>);
static_assert(not pointer<int&>);
static_assert(not pointer<int*&>);
static_assert(pointer<int*>);
static_assert(not pointer<int immovable::*>);
static_assert(pointer<int* const>);
static_assert(pointer<int(* const)[4]>);
static_assert(pointer<int(*)(int)>);

static_assert(not pointer_to_member<int>);
static_assert(not pointer_to_member<int*>);
static_assert(not pointer_to_member<int(* const)[4]>);
static_assert(pointer_to_member<int immovable::*>);
static_assert(pointer_to_member<int (immovable::*)(int)>);

static_assert(not pointer_to_member_function<int>);
static_assert(not pointer_to_member_function<int*>);
static_assert(not pointer_to_member_function<int(* const)[4]>);
static_assert(not pointer_to_member_function<int immovable::*>);
static_assert(pointer_to_member_function<int (immovable::*)(int)>);

static_assert(weak_injectable<int>);
static_assert(not weak_injectable<abstract>);
static_assert(not weak_injectable<abstract const>);
static_assert(weak_injectable<abstract*>);
static_assert(not weak_injectable<abstract* const>);
static_assert(weak_injectable<abstract&>);
static_assert(weak_injectable<abstract const&>);
static_assert(weak_injectable<int&>);
static_assert(not weak_injectable<int[]>);
static_assert(not weak_injectable<void>);
static_assert(not weak_injectable<int volatile>);
static_assert(weak_injectable<int volatile&>);
static_assert(weak_injectable<int&&>);
static_assert(weak_injectable<int const&>);
static_assert(weak_injectable<int const&&>);
static_assert(not weak_injectable<int const>);
static_assert(not weak_injectable<int()>);
static_assert(weak_injectable<int(&)()>);
static_assert(weak_injectable<int(*)()>);
static_assert(weak_injectable<int(&)[5]>);
static_assert(not weak_injectable<int[5]>);

struct converts_to_int {
	operator int() const;
};

struct converts_to_int_ref {
	operator int&() const;
};

struct converts_to_int_const_ref {
	operator int const&() const;
};

struct converts_to_int_rref {
	operator int&&() const;
};

struct rref_converts_to_int {
	operator int() &&;
};

static_assert(user_defined_convertible_to<converts_to_int, int>);
static_assert(user_defined_convertible_to<converts_to_int const, int>);
static_assert(not user_defined_convertible_to<converts_to_int, long>);
static_assert(not user_defined_convertible_to<converts_to_int, int&>);
static_assert(not user_defined_convertible_to<converts_to_int, int&&>);
static_assert(not user_defined_convertible_to<converts_to_int, int const&>);
static_assert(not user_defined_convertible_to<converts_to_int, int const&&>);
static_assert(not user_defined_convertible_to<converts_to_int_ref, int>);
static_assert(user_defined_convertible_to<converts_to_int_ref, int&>);
static_assert(not user_defined_convertible_to<converts_to_int_ref, int&&>);
static_assert(not user_defined_convertible_to<converts_to_int_ref, int const&>);
static_assert(not user_defined_convertible_to<converts_to_int_ref, int const&&>);
static_assert(not user_defined_convertible_to<converts_to_int_rref, int>);
static_assert(not user_defined_convertible_to<converts_to_int_rref, int&>);
static_assert(user_defined_convertible_to<converts_to_int_rref, int&&>);
static_assert(not user_defined_convertible_to<converts_to_int_rref, int const&>);
static_assert(not user_defined_convertible_to<converts_to_int_rref, int const&&>);
static_assert(not user_defined_convertible_to<converts_to_int_const_ref, int>);
static_assert(not user_defined_convertible_to<converts_to_int_const_ref, int&>);
static_assert(not user_defined_convertible_to<converts_to_int_const_ref, int&&>);
static_assert(user_defined_convertible_to<converts_to_int_const_ref, int const&>);
static_assert(not user_defined_convertible_to<converts_to_int_const_ref, int const&&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&, int>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&, int&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&, int&&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&, int const&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&, int const&&>);
static_assert(user_defined_convertible_to<rref_converts_to_int&&, int>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&&, int&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&&, int&&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&&, int const&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int&&, int const&&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int const, int>);
static_assert(not user_defined_convertible_to<rref_converts_to_int const, int&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int const, int&&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int const, int const&>);
static_assert(not user_defined_convertible_to<rref_converts_to_int const, int const&&>);
static_assert(not user_defined_convertible_to<int, int>);
static_assert(not user_defined_convertible_to<int, float>);

auto lambda_0 = []{};
auto lambda_1 = [](int){};
auto lambda_2 = [](int, int){};
auto lambda_1_2 = [](int, int = 0){};

struct function_only_rvalue {
	auto operator()() && -> void {}
};

auto lambda_returns = []{ return 0; };
auto lambda_returns_ref = [i = 0]() -> int const& { return i; };
auto lambda_takes_ref = [](int&){};
auto lambda_takes_rref = [](int&&){};
auto lambda_takes_const_ref = [](int const&){};

static_assert(callable<decltype(lambda_0)>);
static_assert(callable<decltype(lambda_0) const>);
static_assert(callable<decltype(lambda_0) const&>);
static_assert(not callable<decltype(lambda_1)>);
static_assert(not callable<decltype(lambda_2)>);
static_assert(not callable<decltype(lambda_1_2)>);
static_assert(not callable<decltype(lambda_0), int>);
static_assert(not callable<int>);
static_assert(not callable<int, int>);
static_assert(not callable<immovable&>);
static_assert(callable<decltype(lambda_1), int>);
static_assert(not callable<decltype(lambda_1), int*>);
static_assert(not callable<decltype(lambda_1), int, int>);
static_assert(callable<decltype(lambda_2), int, int>);
static_assert(not callable<decltype(lambda_2), int>);
static_assert(callable<decltype(lambda_1_2), int>);
static_assert(callable<decltype(lambda_1_2), int, int>);
static_assert(not callable<function_only_rvalue const&>);
static_assert(not callable<function_only_rvalue&>);
static_assert(callable<function_only_rvalue&&>);
static_assert(callable<function_only_rvalue>);
static_assert(not callable<decltype(lambda_takes_ref)>);
static_assert(callable<decltype(lambda_takes_ref), int&>);
static_assert(not callable<decltype(lambda_takes_ref), int&&>);
static_assert(not callable<decltype(lambda_takes_ref), int const&>);
static_assert(not callable<decltype(lambda_takes_ref), int const&&>);
static_assert(not callable<decltype(lambda_takes_ref), int>);
static_assert(callable<decltype(lambda_takes_rref), int&&>);
static_assert(not callable<decltype(lambda_takes_rref), int&>);
static_assert(not callable<decltype(lambda_takes_rref), int const&>);
static_assert(not callable<decltype(lambda_takes_rref), int const&&>);
static_assert(callable<decltype(lambda_takes_rref), int>);
static_assert(not callable<decltype(lambda_takes_const_ref)>);
static_assert(callable<decltype(lambda_takes_const_ref), int&>);
static_assert(callable<decltype(lambda_takes_const_ref), int&&>);
static_assert(callable<decltype(lambda_takes_const_ref), int const&>);
static_assert(callable<decltype(lambda_takes_const_ref), int const&&>);
static_assert(callable<decltype(lambda_takes_const_ref), int>);
static_assert(not callable<decltype(lambda_takes_const_ref)>);

static_assert(callable_returns<void, decltype(lambda_0)>);
static_assert(callable_returns<void, decltype(lambda_0) const>);
static_assert(callable_returns<void, decltype(lambda_0) const&>);
static_assert(not callable_returns<void, decltype(lambda_1)>);
static_assert(not callable_returns<void, decltype(lambda_2)>);
static_assert(not callable_returns<void, decltype(lambda_1_2)>);
static_assert(not callable_returns<void, decltype(lambda_0), int>);
static_assert(not callable_returns<void, int>);
static_assert(callable_returns<void, decltype(lambda_1), int>);
static_assert(not callable_returns<void, decltype(lambda_1), int*>);
static_assert(not callable_returns<void, decltype(lambda_1), int, int>);
static_assert(not callable_returns<void, decltype(lambda_1), int, int, int>);
static_assert(callable_returns<void, decltype(lambda_2), int, int>);
static_assert(not callable_returns<void, decltype(lambda_2), int>);
static_assert(callable_returns<void, decltype(lambda_1_2), int>);
static_assert(callable_returns<void, decltype(lambda_1_2), int, int>);
static_assert(not callable_returns<void, function_only_rvalue const&>);
static_assert(not callable_returns<void, function_only_rvalue&>);
static_assert(callable_returns<void, function_only_rvalue&&>);
static_assert(callable_returns<void, function_only_rvalue>);
static_assert(callable_returns<int, decltype(lambda_returns)>);
static_assert(callable_returns<int const&, decltype(lambda_returns_ref)>);
static_assert(not callable_returns<float, decltype(lambda_returns)>);
static_assert(not callable_returns<void, decltype(lambda_returns)>);
static_assert(not callable_returns<int&, decltype(lambda_returns_ref)>);
static_assert(not callable_returns<int, decltype(lambda_returns_ref)>);
static_assert(not callable_returns<void, decltype(lambda_takes_ref)>);
static_assert(callable_returns<void, decltype(lambda_takes_ref), int&>);
static_assert(not callable_returns<void, decltype(lambda_takes_ref), int&&>);
static_assert(not callable_returns<void, decltype(lambda_takes_ref), int const&>);
static_assert(not callable_returns<void, decltype(lambda_takes_ref), int const&&>);
static_assert(not callable_returns<void, decltype(lambda_takes_ref), int>);
static_assert(not callable_returns<void, decltype(lambda_takes_ref)>);
static_assert(callable_returns<void, decltype(lambda_takes_rref), int&&>);
static_assert(not callable_returns<void, decltype(lambda_takes_rref), int&>);
static_assert(not callable_returns<void, decltype(lambda_takes_rref), int const&>);
static_assert(not callable_returns<void, decltype(lambda_takes_rref), int const&&>);
static_assert(callable_returns<void, decltype(lambda_takes_rref), int>);
static_assert(not callable_returns<void, decltype(lambda_takes_const_ref)>);
static_assert(callable_returns<void, decltype(lambda_takes_const_ref), int&>);
static_assert(callable_returns<void, decltype(lambda_takes_const_ref), int&&>);
static_assert(callable_returns<void, decltype(lambda_takes_const_ref), int const&>);
static_assert(callable_returns<void, decltype(lambda_takes_const_ref), int const&&>);
static_assert(callable_returns<void, decltype(lambda_takes_const_ref), int>);
static_assert(not callable_returns<void, decltype(lambda_takes_const_ref)>);

struct agg_1 { int a; int b; };
struct agg_2 { int a; int b; int c; };
struct agg { agg_1 agg1; agg_2 agg2; };

static_assert(brace_constructible<int>);
static_assert(brace_constructible<int, int>);
static_assert(brace_constructible<int, int&>);
static_assert(not brace_constructible<int&, int>);
static_assert(not brace_constructible<int&, int&&>);
static_assert(brace_constructible<agg_1>);
static_assert(brace_constructible<agg_1, int>);
static_assert(brace_constructible<agg_1, int, int>);
static_assert(not brace_constructible<agg_1, int, int, int>);
static_assert(brace_constructible<agg_1, short>);
static_assert(not brace_constructible<agg_1, float>);
static_assert(brace_constructible<agg, agg_1, agg_2>);
static_assert(brace_constructible<agg, int, int, int, int, int>);
static_assert(brace_constructible<agg, int, int, agg_2>);
static_assert(not brace_constructible<agg, agg_2, agg_2>);

auto tlambda_0 = []<typename>{};
auto tlambda_1 = []<typename>(int){};
auto tlambda_2 = []<typename>(int, int){};
auto tlambda_1_2 = []<typename>(int, int = 0){};

struct tfunction_only_rvalue {
	template<typename>
	auto operator()() && -> void {}
};

auto tlambda_returns = []<typename>{ return 0; };
auto tlambda_returns_ref = [i = 0]<typename>() -> int const& { return i; };
auto tlambda_takes_ref = []<typename>(int&){};
auto tlambda_takes_rref = []<typename>(int&&){};
auto tlambda_takes_const_ref = []<typename>(int const&){};

static_assert(callable_template_1t<decltype(tlambda_0), int>);
static_assert(callable_template_1t<decltype(tlambda_0) const, int>);
static_assert(callable_template_1t<decltype(tlambda_0) const&, int>);
static_assert(not callable_template_1t<decltype(tlambda_1), int>);
static_assert(not callable_template_1t<decltype(tlambda_2), int>);
static_assert(not callable_template_1t<decltype(tlambda_1_2), int>);
static_assert(not callable_template_1t<decltype(tlambda_0), int, int>);
static_assert(not callable_template_1t<int, int>);
static_assert(not callable_template_1t<immovable&, int>);
static_assert(callable_template_1t<decltype(tlambda_1), int, int>);
static_assert(not callable_template_1t<decltype(tlambda_1), int, int*>);
static_assert(not callable_template_1t<decltype(tlambda_1), int, int, int>);
static_assert(callable_template_1t<decltype(tlambda_2), int, int, int>);
static_assert(not callable_template_1t<decltype(tlambda_2), int, int>);
static_assert(callable_template_1t<decltype(tlambda_1_2), int, int>);
static_assert(callable_template_1t<decltype(tlambda_1_2), int, int, int>);
static_assert(not callable_template_1t<tfunction_only_rvalue const&, int>);
static_assert(not callable_template_1t<tfunction_only_rvalue&, int>);
static_assert(callable_template_1t<tfunction_only_rvalue&&, int>);
static_assert(callable_template_1t<tfunction_only_rvalue, int>);
static_assert(not callable_template_1t<decltype(tlambda_takes_ref), int, int>);
static_assert(callable_template_1t<decltype(tlambda_takes_ref), int, int&>);
static_assert(not callable_template_1t<decltype(tlambda_takes_ref), int, int&&>);
static_assert(not callable_template_1t<decltype(tlambda_takes_ref), int, int const&>);
static_assert(not callable_template_1t<decltype(tlambda_takes_ref), int, int const&&>);
static_assert(not callable_template_1t<decltype(tlambda_takes_ref), int, int>);
static_assert(not callable_template_1t<decltype(tlambda_takes_ref), int>);
static_assert(callable_template_1t<decltype(tlambda_takes_rref), int, int&&>);
static_assert(not callable_template_1t<decltype(tlambda_takes_rref), int, int&>);
static_assert(not callable_template_1t<decltype(tlambda_takes_rref), int, int const&>);
static_assert(not callable_template_1t<decltype(tlambda_takes_rref), int, int const&&>);
static_assert(callable_template_1t<decltype(tlambda_takes_rref), int, int>);
static_assert(not callable_template_1t<decltype(tlambda_takes_const_ref), int>);
static_assert(callable_template_1t<decltype(tlambda_takes_const_ref), int, int&>);
static_assert(callable_template_1t<decltype(tlambda_takes_const_ref), int, int&&>);
static_assert(callable_template_1t<decltype(tlambda_takes_const_ref), int, int const&>);
static_assert(callable_template_1t<decltype(tlambda_takes_const_ref), int, int const&&>);
static_assert(callable_template_1t<decltype(tlambda_takes_const_ref), int, int>);
static_assert(not callable_template_1t<decltype(tlambda_takes_const_ref), int>);

static_assert(callable_template_1t_returns<void, decltype(tlambda_0), int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_0) const, int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_0) const&, int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_1), int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_2), int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_1_2), int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_0), int, int>);
static_assert(not callable_template_1t_returns<void, int, int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_1), int, int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_1), int, int*>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_1), int, int, int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_2), int, int, int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_2), int, int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_1_2), int, int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_1_2), int, int, int>);
static_assert(not callable_template_1t_returns<void, tfunction_only_rvalue const&, int>);
static_assert(not callable_template_1t_returns<void, tfunction_only_rvalue&, int>);
static_assert(callable_template_1t_returns<void, tfunction_only_rvalue&&, int>);
static_assert(callable_template_1t_returns<void, tfunction_only_rvalue, int>);
static_assert(callable_template_1t_returns<int, decltype(tlambda_returns), int>);
static_assert(callable_template_1t_returns<int const&, decltype(tlambda_returns_ref), int>);
static_assert(not callable_template_1t_returns<float, decltype(tlambda_returns), int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_returns), int>);
static_assert(not callable_template_1t_returns<int&, decltype(tlambda_returns_ref), int>);
static_assert(not callable_template_1t_returns<int, decltype(tlambda_returns_ref), int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_ref), int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_takes_ref), int, int&>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_ref), int, int&&>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_ref), int, int const&>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_ref), int, int const&&>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_ref), int, int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_ref), int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_takes_rref), int, int&&>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_rref), int, int&>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_rref), int, int const&>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_rref), int, int const&&>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_takes_rref), int, int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_const_ref), int>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_takes_const_ref), int, int&>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_takes_const_ref), int, int&&>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_takes_const_ref), int, int const&>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_takes_const_ref), int, int const&&>);
static_assert(callable_template_1t_returns<void, decltype(tlambda_takes_const_ref), int, int>);
static_assert(not callable_template_1t_returns<void, decltype(tlambda_takes_const_ref), int>);

struct a {};
struct needs_a{ explicit needs_a(a) {} };

struct base {};
struct derived : base {};

static_assert(allows_construction_of<int, int>);
static_assert(allows_construction_of<int&, int>);
static_assert(allows_construction_of<int&&, int>);
static_assert(allows_construction_of<int, int&&>);
static_assert(allows_construction_of<int, float>);
static_assert(allows_construction_of<float, int>);
static_assert(not allows_construction_of<a, int>);
static_assert(allows_construction_of<a, needs_a>);
static_assert(not allows_construction_of<needs_a, a>);
static_assert(not allows_construction_of<base*, derived*>);
static_assert(not allows_construction_of<void*, int*>);
static_assert(not allows_construction_of<int&, int&&>);
static_assert(allows_construction_of<int, int const&>);
static_assert(not allows_construction_of<int, int&>);
static_assert(not allows_construction_of<int const&, int&>);
static_assert(not allows_construction_of<int const*, int*>);

static_assert(explicitly_castable_to<int, int>);
static_assert(explicitly_castable_to<int&, int>);
static_assert(explicitly_castable_to<int&&, int>);
static_assert(explicitly_castable_to<int, int&&>);
static_assert(explicitly_castable_to<int, float>);
static_assert(explicitly_castable_to<float, int>);
static_assert(not explicitly_castable_to<a, int>);
static_assert(explicitly_castable_to<a, needs_a>);
static_assert(not explicitly_castable_to<needs_a, a>);
static_assert(explicitly_castable_to<base*, derived*>);
static_assert(explicitly_castable_to<void*, int*>);
static_assert(explicitly_castable_to<int&, int&&>);
static_assert(explicitly_castable_to<int, int const&>);
static_assert(not explicitly_castable_to<int, int&>);
static_assert(not explicitly_castable_to<int const&, int&>);
static_assert(not explicitly_castable_to<int const*, int*>);

static_assert(not conversion_materializes_temporary<int&, int&>);
static_assert(not conversion_materializes_temporary<int, int&>);
static_assert(conversion_materializes_temporary<int, int&&>);
static_assert(conversion_materializes_temporary<int, const int&>);
static_assert(not conversion_materializes_temporary<int&&, int&&>);
static_assert(not conversion_materializes_temporary<int&&, const int&>);
static_assert(conversion_materializes_temporary<long&&, int&&>);
static_assert(conversion_materializes_temporary<long, int&&>);
static_assert(not conversion_materializes_temporary<converts_to_int, int&>);
static_assert(conversion_materializes_temporary<converts_to_int, int&&>);
static_assert(conversion_materializes_temporary<converts_to_int, int const&>);
static_assert(conversion_materializes_temporary<converts_to_int, int const&&>);
static_assert(not conversion_materializes_temporary<converts_to_int, int>);
static_assert(not conversion_materializes_temporary<converts_to_int_ref, int&>);
static_assert(not conversion_materializes_temporary<converts_to_int_ref&, int&>);
static_assert(not conversion_materializes_temporary<converts_to_int_ref&&, int&>);
static_assert(not conversion_materializes_temporary<converts_to_int_ref const&, int&>);
static_assert(not conversion_materializes_temporary<converts_to_int_const_ref&, int const&>);
static_assert(not conversion_materializes_temporary<converts_to_int_const_ref&&, int const&>);
static_assert(not conversion_materializes_temporary<converts_to_int_const_ref const&, int const&>);
static_assert(not conversion_materializes_temporary<converts_to_int_rref&, int&&>);
static_assert(not conversion_materializes_temporary<converts_to_int_rref&&, int&&>);
static_assert(not conversion_materializes_temporary<converts_to_int_rref const&, int&&>);
static_assert(not conversion_materializes_temporary<derived*, base*>);
static_assert(not conversion_materializes_temporary<derived&, base&>);

static_assert(safe_convertible_to<int&, int&>);
static_assert(safe_convertible_to<int const&, int const&>);
static_assert(safe_convertible_to<int&, int>);
static_assert(not safe_convertible_to<int, int&>);
static_assert(not safe_convertible_to<int, int&&>);
static_assert(not safe_convertible_to<int, const int&>);
static_assert(safe_convertible_to<int&, int const&>);
static_assert(safe_convertible_to<int&&, int&&>);
static_assert(safe_convertible_to<int&&, const int&>);
static_assert(not safe_convertible_to<long&&, int&&>);
static_assert(not safe_convertible_to<long, int&&>);
static_assert(safe_convertible_to<derived&, base&>);
static_assert(safe_convertible_to<derived&, base const&>);
static_assert(safe_convertible_to<derived&&, base const&>);
static_assert(not safe_convertible_to<base&, derived&>);

static_assert(pack_distinct<int, float>);
static_assert(pack_distinct<int, float, short>);
static_assert(pack_distinct<int, float, int&>);
static_assert(pack_distinct<int>);
static_assert(pack_distinct<const int, int>);
static_assert(not pack_distinct<int, int>);
static_assert(not pack_distinct<int, float, int>);
static_assert(not pack_distinct<int, float, int, float>);
static_assert(not pack_distinct<int&, int&>);
static_assert(pack_distinct<int, int&>);
static_assert(pack_distinct<int[2], int[3]>);
static_assert(pack_distinct<int[2], int[]>);
static_assert(pack_distinct<int*, int[]>);
static_assert(pack_distinct<int(), int(*)()>);

TEST_CASE("concepts", "[concepts]") {
	// This test is successful and has at least one runtime assertion.
	REQUIRE(true);
}

