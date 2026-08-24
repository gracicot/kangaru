#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

TEST_CASE("Deducer can deduce reference types", "[deducer]") {
	enum how_t {
		by_value,
		by_lvalue_reference,
		by_rvalue_reference,
		by_lvalue_const_reference,
		by_rvalue_const_reference
	};
	
	struct sneezy {
		how_t how;
	};
	
	auto by_plain_value = sneezy{.how = by_value};
	auto by_reference = sneezy{.how = by_lvalue_reference};
	auto const by_reference_const = sneezy{.how = by_lvalue_const_reference};
	auto by_rvalue = sneezy{.how = by_rvalue_reference};
	auto const by_rvalue_const = sneezy{.how = by_rvalue_const_reference};
	
	auto source_by_value = kangaru::object_source{by_plain_value};
	auto source_by_lvalue_ref = kangaru::external_reference_source{by_reference};
	auto source_by_lvalue_ref_const = kangaru::external_reference_source{by_reference_const};
	auto source_by_rvalue_ref = kangaru::external_rvalue_source{std::move(by_rvalue)};
	auto source_by_rvalue_ref_const = kangaru::external_rvalue_source{std::move(by_rvalue_const)};
	
	SECTION("Will deduce the right type of reference") {
		auto source = kangaru::tie(
			source_by_value,
			source_by_lvalue_ref,
			source_by_lvalue_ref_const,
			source_by_rvalue_ref_const,
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::make_simple_injector(source);
		auto deducer = kangaru::basic_deducer<decltype(source)&>{source};
		
		auto inject_by_value = [](sneezy s) {
			CHECK(s.how == by_value);
		};
		
		auto inject_by_lvalue = [](sneezy& s) {
			CHECK(s.how == by_lvalue_reference);
		};
		
		auto inject_by_lvalue_const = [](sneezy const& s) {
			CHECK(s.how == by_lvalue_const_reference);
		};
		
		auto inject_by_rvalue = [](sneezy&& s){
			CHECK(s.how == by_rvalue_reference);
		};
		
		auto inject_by_rvalue_const = [](sneezy const&& s){
			CHECK(s.how == by_rvalue_const_reference);
		};
		
		injector(inject_by_value);
		injector(inject_by_lvalue);
		injector(inject_by_rvalue);
		injector(inject_by_lvalue_const);
		injector(inject_by_rvalue_const);
		
		kangaru::call_with_deducers(inject_by_value, deducer);
		kangaru::call_with_deducers(inject_by_lvalue, deducer);
		kangaru::call_with_deducers(inject_by_rvalue, deducer);
		kangaru::call_with_deducers(inject_by_lvalue_const, deducer);
		kangaru::call_with_deducers(inject_by_rvalue_const, deducer);
		
		static_assert(kangaru::callable_with_deducers<decltype(inject_by_value), decltype(deducer)>);
		static_assert(kangaru::callable_with_deducers<decltype(inject_by_lvalue), decltype(deducer)>);
		static_assert(kangaru::callable_with_deducers<decltype(inject_by_lvalue_const), decltype(deducer)>);
		static_assert(kangaru::callable_with_deducers<decltype(inject_by_rvalue), decltype(deducer)>);
		static_assert(kangaru::callable_with_deducers<decltype(inject_by_rvalue_const), decltype(deducer)>);
		
		SECTION("Exclude deducer excludes a particular deduction") {
			auto deducer = kangaru::exclude_deduction<sneezy&>(kangaru::basic_deducer<decltype(source)&>{source});
			
			kangaru::call_with_deducers(inject_by_value, deducer);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_lvalue), decltype(deducer)>);
			kangaru::call_with_deducers(inject_by_rvalue, deducer);
			kangaru::call_with_deducers(inject_by_lvalue_const, deducer);
			kangaru::call_with_deducers(inject_by_rvalue_const, deducer);
		}
		
		SECTION("Exclude special constructor deducer excludes a type completely") {
			auto deducer = kangaru::exclude_special_constructors_for<sneezy>(kangaru::basic_deducer<decltype(source)&>{source});
			
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_value), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_lvalue), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_lvalue_const), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_rvalue), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_rvalue_const), decltype(deducer)>);
		}
		
		SECTION("Allow value category") {
			auto deducer = kangaru::allow_value_category_deduction<kangaru::reference_kind::lvalue_const_reference>(kangaru::basic_deducer<decltype(source)&>{source});
			
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_value), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_lvalue), decltype(deducer)>);
			kangaru::call_with_deducers(inject_by_lvalue_const, deducer);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_rvalue), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_rvalue_const), decltype(deducer)>);
		}
		
		SECTION("Allow value category mixed") {
			auto deducer = kangaru::allow_value_category_deduction<kangaru::reference_kind::lvalue_const_reference_and_rvalue_const_reference>(kangaru::basic_deducer<decltype(source)&>{source});
			
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_value), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_lvalue), decltype(deducer)>);
			kangaru::call_with_deducers(inject_by_lvalue_const, deducer);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_rvalue), decltype(deducer)>);
			kangaru::call_with_deducers(inject_by_rvalue_const, deducer);
		}
		
		SECTION("Allow prvalue") {
			auto deducer = kangaru::allow_value_category_deduction<kangaru::reference_kind::none>(kangaru::basic_deducer<decltype(source)&>{source});
			
			kangaru::call_with_deducers(inject_by_value, deducer);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_lvalue), decltype(deducer)>);
			
			// prvalue can call into these with non-strict deducer
			static_assert(kangaru::callable_with_deducers<decltype(inject_by_lvalue_const), decltype(deducer)>);
			static_assert(kangaru::callable_with_deducers<decltype(inject_by_rvalue), decltype(deducer)>);
			static_assert(kangaru::callable_with_deducers<decltype(inject_by_rvalue_const), decltype(deducer)>);
		}
		
		SECTION("Allow prvalue strict") {
			auto deducer = kangaru::allow_value_category_deduction<kangaru::reference_kind::none>(kangaru::strict_deducer<decltype(source)&>{source});
			
			kangaru::call_with_deducers(inject_by_value, deducer);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_lvalue), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_lvalue_const), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_rvalue), decltype(deducer)>);
			static_assert(not kangaru::callable_with_deducers<decltype(inject_by_rvalue_const), decltype(deducer)>);
		}
	}
	
	SECTION("Will fallback to mutable ref if no const is available") {
		auto source = kangaru::tie(
			source_by_lvalue_ref,
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::make_simple_injector(source);
		
		injector([](sneezy& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy const& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy s) {
			CHECK(s.how == by_rvalue_reference);
		});
	}
	
	SECTION("By value fallback") {
		SECTION("All reference types") {
			auto source = kangaru::tie(
				source_by_lvalue_ref,
				source_by_lvalue_ref_const,
				source_by_rvalue_ref_const,
				source_by_rvalue_ref
			);
			
			auto injector = kangaru::make_simple_injector(source);
			
			injector([](sneezy s) {
				CHECK(s.how == by_rvalue_reference);
			});
		}
		
		SECTION("No rvalue") {
			auto source = kangaru::tie(
				source_by_lvalue_ref,
				source_by_lvalue_ref_const,
				source_by_rvalue_ref_const
			);
			
			auto injector = kangaru::make_simple_injector(source);
			
			injector([](sneezy s) {
				CHECK(s.how == by_rvalue_const_reference);
			});
		}
		
		SECTION("No rvalue const") {
			auto source = kangaru::tie(
				source_by_lvalue_ref,
				source_by_lvalue_ref_const
			);
			
			auto injector = kangaru::make_simple_injector(source);
			
			injector([](sneezy s) {
				CHECK(s.how == by_lvalue_const_reference);
			});
		}
	}
	
	SECTION("By lvalue const fallback") {
		SECTION("All reference types") {
			auto source = kangaru::tie(
				source_by_lvalue_ref,
				source_by_rvalue_ref_const,
				source_by_rvalue_ref
			);
			
			auto injector = kangaru::make_simple_injector(source);
			
			injector([](sneezy const& s) {
				CHECK(s.how == by_lvalue_reference);
			});
		}
		
		SECTION("No lvalue") {
			auto source = kangaru::tie(
				source_by_rvalue_ref_const,
				source_by_rvalue_ref
			);
			
			auto injector = kangaru::make_simple_injector(source);
			
			injector([](sneezy const& s) {
				CHECK(s.how == by_rvalue_const_reference);
			});
		}
	}
	
	SECTION("Will fallback to rvalue ref if no const lvalue is available") {
		auto source = kangaru::tie(
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::make_simple_injector(source);
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy const& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		SECTION("But prefers const") {
			auto source = kangaru::tie(
				source_by_rvalue_ref,
				source_by_rvalue_ref_const
			);
			
			auto injector = kangaru::make_simple_injector(source);
			
			injector([](sneezy&& s) {
				CHECK(s.how == by_rvalue_reference);
			});
			
			injector([](sneezy const& s) {
				CHECK(s.how == by_rvalue_const_reference);
			});
			
			injector([](sneezy const&& s) {
				CHECK(s.how == by_rvalue_const_reference);
			});
		}
	}
	
	SECTION("Can copy simple values") {
		auto source = source_by_value;
		
		auto injector = kangaru::make_simple_injector(source);
		
		injector([](sneezy s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy const& s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_value);
		});
	}
	
	SECTION("Can convert convertible references") {
		auto injector_rvalue = kangaru::make_simple_injector(source_by_rvalue_ref);
		auto injector_const_rvalue = kangaru::make_simple_injector(source_by_rvalue_ref_const);
		auto injector_lvalue = kangaru::make_simple_injector(source_by_lvalue_ref);
		
		injector_rvalue([](sneezy const& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector_const_rvalue([](sneezy const& s) {
			CHECK(s.how == by_rvalue_const_reference);
		});
		
		injector_lvalue([](sneezy const& s) {
			CHECK(s.how == by_lvalue_reference);
		});
	}
	
	SECTION("Strict deducers does not convert convertible references") {
		auto injector_rvalue = kangaru::make_strict_simple_injector(source_by_rvalue_ref);
		auto injector_const_rvalue = kangaru::make_strict_simple_injector(source_by_rvalue_ref_const);
		auto injector_lvalue = kangaru::make_strict_simple_injector(source_by_lvalue_ref);
		
		auto function_lvalue_const_ref = [](sneezy const& s) {};
		
		static_assert(not kangaru::callable<decltype(injector_rvalue), decltype(function_lvalue_const_ref)>);
		static_assert(not kangaru::callable<decltype(injector_const_rvalue), decltype(function_lvalue_const_ref)>);
		static_assert(not kangaru::callable<decltype(injector_lvalue), decltype(function_lvalue_const_ref)>);
	}
	
	SECTION("Strict deducer deduces the right type of reference") {
		auto source = kangaru::tie(
			source_by_value,
			source_by_lvalue_ref,
			source_by_lvalue_ref_const,
			source_by_rvalue_ref_const,
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::make_strict_simple_injector(source);
		
		injector([](sneezy s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		injector([](sneezy const& s) {
			CHECK(s.how == by_lvalue_const_reference);
		});
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_rvalue_const_reference);
		});
	}
	
	SECTION("Strict deducer has no fallback") {
		auto source = kangaru::tie(
			source_by_value,
			source_by_lvalue_ref,
			source_by_rvalue_ref_const,
			source_by_rvalue_ref
		);
		
		auto injector = kangaru::make_strict_simple_injector(source);
		
		injector([](sneezy s) {
			CHECK(s.how == by_value);
		});
		
		injector([](sneezy& s) {
			CHECK(s.how == by_lvalue_reference);
		});
		
		injector([](sneezy&& s) {
			CHECK(s.how == by_rvalue_reference);
		});
		
		auto function_lvalue_const_ref = [](sneezy const&){};
		static_assert(not kangaru::callable<decltype(injector), decltype(function_lvalue_const_ref)>);
		
		injector([](sneezy const&& s) {
			CHECK(s.how == by_rvalue_const_reference);
		});
	}

	SECTION("Can deduce values prvalues") {
		struct grumpy { int token; };
		struct grumpy_source {
			constexpr auto provide() & -> grumpy {
				return grumpy{.token = token++};
			}
			
			int token = 0;
		};

		auto source = grumpy_source{};
		auto deducer = kangaru::basic_deducer<grumpy_source&>{source};
		
		CHECK([](grumpy g) { return g; }(deducer).token == 0);
		CHECK([](grumpy g) { return g; }(deducer).token == 1);
	}
	
	SECTION("By const rvalue non strict") {
		auto source = kangaru::tie(
			source_by_rvalue_ref_const
		);
		
		auto deducer = kangaru::basic_deducer<decltype(source)&>{source};
		
		auto lvalue = [](sneezy&) {};
		auto rvalue = [](sneezy&&) {};
		auto lvalue_const = [](sneezy const&) {};
		
		static_assert(not kangaru::callable<decltype(lvalue), decltype(deducer)>);
		
		// Call to function receiving rvalue can materialize temporary
		static_assert(kangaru::callable<decltype(rvalue), decltype(deducer)>);
		static_assert(kangaru::callable<decltype(lvalue_const), decltype(deducer)>);
	}
	
	SECTION("Correctly deduce const& when has a source of any other kind of references") {
		auto source = kangaru::compose(
			source_by_lvalue_ref,
			source_by_rvalue_ref
		);
		
		auto deducer = kangaru::basic_deducer<decltype(source)&>{source};
		
		CHECK([](sneezy const& g) { return g; }(deducer).how == by_lvalue_reference);
	}
	
	SECTION("Chooses between const& and &&") {
		auto source = kangaru::compose(
			source_by_lvalue_ref,
			source_by_rvalue_ref
		);
		
		auto deducer = kangaru::basic_deducer<decltype(source)&>{source};
		
		CHECK([](sneezy const& g) { return g; }(deducer).how == by_lvalue_reference);
		CHECK([](sneezy&& g) { return g; }(deducer).how == by_rvalue_reference);
		
		SECTION("Prioritize rvalue when there's a choice") {
			CHECK([](sneezy g) { return g; }(deducer).how == by_rvalue_reference);
		}
		
		SECTION("Prioritize prvalue when possible") {
			auto source2 = kangaru::compose(source_by_value, source);
			auto deducer = kangaru::basic_deducer<decltype(source2)&>{source2};
			
			CHECK([](sneezy g) { return g; }(deducer).how == by_value);
		}
	}
}

struct injected {};

struct type_0000 {
	explicit type_0000(injected);
};

struct type_0001 {
	explicit type_0001(injected&);
};

struct type_0010 {
	explicit type_0010(injected const&);
};

struct type_0100 {
	explicit type_0100(injected&&);
};

struct type_1000 {
	explicit type_1000(injected const&&);
};

struct agg_0000 {
	injected i;
};

struct agg_0001 {
	injected& i;
};

struct agg_0010 {
	injected const& i;
};

struct agg_0100 {
	injected&& i;
};

struct agg_1000 {
	injected const&& i;
};

struct level2_type_0000 { type_0000 member; };
struct level2_type_0001 { type_0001 member; };
struct level2_type_0010 { type_0010 member; };
struct level2_type_0100 { type_0100 member; };
struct level2_type_1000 { type_1000 member; };

struct level2_agg_0000 { agg_0000 member; };
struct level2_agg_0001 { agg_0001 member; };
struct level2_agg_0010 { agg_0010 member; };
struct level2_agg_0100 { agg_0100 member; };
struct level2_agg_1000 { agg_1000 member; };

auto multi_param = [](injected const&, injected&&, injected, injected&){};
using multi_param_t = decltype(multi_param);

TEST_CASE("Deducer internals can detect value categories", "[deducer]") {
	SECTION("Deducer internals") {
		SECTION("Value category detection on functions") {
			static_assert(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<multi_param_t, 0, 4>));
			static_assert(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<multi_param_t, 1, 4>));
			static_assert((kangaru::detail::deducer_private::function_nth_parameter_prvalue<multi_param_t, 2, 4>));
			static_assert(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<multi_param_t, 3, 4>));
			
			// Out of bound is false
			static_assert(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<multi_param_t, 4, 4>));
			
			static_assert(
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, multi_param_t, 0, 4>()
				== kangaru::reference_kind::lvalue_const_reference
			);
			
			static_assert(
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, multi_param_t, 1, 4>()
				== kangaru::reference_kind::rvalue_reference
			);
			
			static_assert(
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, multi_param_t, 2, 4>()
				== kangaru::reference_kind::none
			);
			
			static_assert(
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, multi_param_t, 3, 4>()
				== kangaru::reference_kind::lvalue_reference
			);
		}
		
		SECTION("Class type with constructors") {
			using constructor_0000_t = kangaru::constructor_function<type_0000>;
			using constructor_0001_t = kangaru::constructor_function<type_0001>;
			using constructor_0010_t = kangaru::constructor_function<type_0010>;
			using constructor_0100_t = kangaru::constructor_function<type_0100>;
			using constructor_1000_t = kangaru::constructor_function<type_1000>;
			
			auto constructor_0000 = constructor_0000_t{};
			auto constructor_0001 = constructor_0001_t{};
			auto constructor_0010 = constructor_0010_t{};
			auto constructor_0100 = constructor_0100_t{};
			auto constructor_1000 = constructor_1000_t{};
			
			CHECK((kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0000_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0001_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0010_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_0100_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_1000_t, 0, 1>));
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0000_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0001_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0010_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_0100_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_1000_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_const_reference);
			
			CHECK(kangaru::construction_tree_needs<type_0000, injected>);
			CHECK(not kangaru::construction_tree_needs<type_0000, injected&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, injected const&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, injected&&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0000, int>);
			CHECK(not kangaru::construction_tree_needs<type_0000, int&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_0000, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0001, injected>);
			CHECK(kangaru::construction_tree_needs<type_0001, injected&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, injected const&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, injected&&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0001, int>);
			CHECK(not kangaru::construction_tree_needs<type_0001, int&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_0001, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0010, injected>);
			CHECK(not kangaru::construction_tree_needs<type_0010, injected&>);
			CHECK(kangaru::construction_tree_needs<type_0010, injected const&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, injected&&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0010, int>);
			CHECK(not kangaru::construction_tree_needs<type_0010, int&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_0010, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0100, injected>);
			CHECK(not kangaru::construction_tree_needs<type_0100, injected&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, injected const&>);
			CHECK(kangaru::construction_tree_needs<type_0100, injected&&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_0100, int>);
			CHECK(not kangaru::construction_tree_needs<type_0100, int&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_0100, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_1000, injected>);
			CHECK(not kangaru::construction_tree_needs<type_1000, injected&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, injected const&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, injected&&>);
			CHECK(kangaru::construction_tree_needs<type_1000, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<type_1000, int>);
			CHECK(not kangaru::construction_tree_needs<type_1000, int&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, int const&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, int&&>);
			CHECK(not kangaru::construction_tree_needs<type_1000, int const&&>);
			
			CHECK(kangaru::construction_tree_needs<agg_0000, injected>);
			CHECK(not kangaru::construction_tree_needs<agg_0000, injected&>);
			CHECK(not kangaru::construction_tree_needs<agg_0000, injected const&>);
			CHECK(not kangaru::construction_tree_needs<agg_0000, injected&&>);
			CHECK(not kangaru::construction_tree_needs<agg_0000, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_0000, int>);
			CHECK(not kangaru::construction_tree_needs<agg_0000, int&>);
			CHECK(not kangaru::construction_tree_needs<agg_0000, int const&>);
			CHECK(not kangaru::construction_tree_needs<agg_0000, int&&>);
			CHECK(not kangaru::construction_tree_needs<agg_0000, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_0001, injected>);
			CHECK(kangaru::construction_tree_needs<agg_0001, injected&>);
			CHECK(not kangaru::construction_tree_needs<agg_0001, injected const&>);
			CHECK(not kangaru::construction_tree_needs<agg_0001, injected&&>);
			CHECK(not kangaru::construction_tree_needs<agg_0001, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_0001, int>);
			CHECK(not kangaru::construction_tree_needs<agg_0001, int&>);
			CHECK(not kangaru::construction_tree_needs<agg_0001, int const&>);
			CHECK(not kangaru::construction_tree_needs<agg_0001, int&&>);
			CHECK(not kangaru::construction_tree_needs<agg_0001, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_0010, injected>);
			CHECK(not kangaru::construction_tree_needs<agg_0010, injected&>);
			CHECK(kangaru::construction_tree_needs<agg_0010, injected const&>);
			CHECK(not kangaru::construction_tree_needs<agg_0010, injected&&>);
			CHECK(not kangaru::construction_tree_needs<agg_0010, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_0010, int>);
			CHECK(not kangaru::construction_tree_needs<agg_0010, int&>);
			CHECK(not kangaru::construction_tree_needs<agg_0010, int const&>);
			CHECK(not kangaru::construction_tree_needs<agg_0010, int&&>);
			CHECK(not kangaru::construction_tree_needs<agg_0010, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_0100, injected>);
			CHECK(not kangaru::construction_tree_needs<agg_0100, injected&>);
			CHECK(not kangaru::construction_tree_needs<agg_0100, injected const&>);
			CHECK(kangaru::construction_tree_needs<agg_0100, injected&&>);
			CHECK(not kangaru::construction_tree_needs<agg_0100, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_0100, int>);
			CHECK(not kangaru::construction_tree_needs<agg_0100, int&>);
			CHECK(not kangaru::construction_tree_needs<agg_0100, int const&>);
			CHECK(not kangaru::construction_tree_needs<agg_0100, int&&>);
			CHECK(not kangaru::construction_tree_needs<agg_0100, int const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_1000, injected>);
			CHECK(not kangaru::construction_tree_needs<agg_1000, injected&>);
			CHECK(not kangaru::construction_tree_needs<agg_1000, injected const&>);
			CHECK(not kangaru::construction_tree_needs<agg_1000, injected&&>);
			CHECK(kangaru::construction_tree_needs<agg_1000, injected const&&>);
			
			CHECK(not kangaru::construction_tree_needs<agg_1000, int>);
			CHECK(not kangaru::construction_tree_needs<agg_1000, int&>);
			CHECK(not kangaru::construction_tree_needs<agg_1000, int const&>);
			CHECK(not kangaru::construction_tree_needs<agg_1000, int&&>);
			CHECK(not kangaru::construction_tree_needs<agg_1000, int const&&>);
		}
		
		SECTION("Class type aggregate") {
			using constructor_agg_0000_t = kangaru::constructor_function<agg_0000>;
			using constructor_agg_0001_t = kangaru::constructor_function<agg_0001>;
			using constructor_agg_0010_t = kangaru::constructor_function<agg_0010>;
			using constructor_agg_0100_t = kangaru::constructor_function<agg_0100>;
			using constructor_agg_1000_t = kangaru::constructor_function<agg_1000>;
			
			auto constructor_agg_0000 = constructor_agg_0000_t{};
			auto constructor_agg_0001 = constructor_agg_0001_t{};
			auto constructor_agg_0010 = constructor_agg_0010_t{};
			auto constructor_agg_0100 = constructor_agg_0100_t{};
			auto constructor_agg_1000 = constructor_agg_1000_t{};
			
			CHECK((kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_0000_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_0001_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_0010_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_0100_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_agg_1000_t, 0, 1>));
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_0000_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_0001_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_0100_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_0010_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_agg_1000_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_const_reference);
		}
		
		SECTION("Wrapped class type aggregate") {
			using constructor_source_agg_0000_t = kangaru::constructor_function<kangaru::reference_source<agg_0000>>;
			using constructor_source_agg_0001_t = kangaru::constructor_function<kangaru::reference_source<agg_0001>>;
			using constructor_source_agg_0010_t = kangaru::constructor_function<kangaru::reference_source<agg_0010>>;
			using constructor_source_agg_0100_t = kangaru::constructor_function<kangaru::reference_source<agg_0100>>;
			using constructor_source_agg_1000_t = kangaru::constructor_function<kangaru::reference_source<agg_1000>>;
			
			auto constructor_source_agg_0000 = constructor_source_agg_0000_t{};
			auto constructor_source_agg_0001 = constructor_source_agg_0001_t{};
			auto constructor_source_agg_0010 = constructor_source_agg_0010_t{};
			auto constructor_source_agg_0100 = constructor_source_agg_0100_t{};
			auto constructor_source_agg_1000 = constructor_source_agg_1000_t{};
			
			CHECK((kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_0000_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_0001_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_0010_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_0100_t, 0, 1>));
			CHECK(not (kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_source_agg_1000_t, 0, 1>));
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_0000_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_0001_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_0100_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_0010_t, 0, 1>()
			) == kangaru::reference_kind::lvalue_const_reference);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_source_agg_1000_t, 0, 1>()
			) == kangaru::reference_kind::rvalue_const_reference);
		}
		
		SECTION("Level 2 aggregate with aggregate member") {
			using constructor_level2_agg_0000_t = kangaru::constructor_function<level2_agg_0000>;
			using constructor_level2_agg_0001_t = kangaru::constructor_function<level2_agg_0001>;
			using constructor_level2_agg_0010_t = kangaru::constructor_function<level2_agg_0010>;
			using constructor_level2_agg_0100_t = kangaru::constructor_function<level2_agg_0100>;
			using constructor_level2_agg_1000_t = kangaru::constructor_function<level2_agg_1000>;
			
			auto constructor_level2_agg_0000 = constructor_level2_agg_0000_t{};
			auto constructor_level2_agg_0001 = constructor_level2_agg_0001_t{};
			auto constructor_level2_agg_0010 = constructor_level2_agg_0010_t{};
			auto constructor_level2_agg_0100 = constructor_level2_agg_0100_t{};
			auto constructor_level2_agg_1000 = constructor_level2_agg_1000_t{};
			
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_0000_t, 0, 1>);
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_0001_t, 0, 1>);
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_0010_t, 0, 1>);
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_0100_t, 0, 1>);
			CHECK(kangaru::detail::deducer_private::function_nth_parameter_prvalue<constructor_level2_agg_1000_t, 0, 1>);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_0000_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_0001_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_0100_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_0010_t, 0, 1>()
			) == kangaru::reference_kind::none);
			
			CHECK((
				kangaru::detail::deducer_private::reference_kind_for_nth_parameter<kangaru::placeholder_deducer, constructor_level2_agg_1000_t, 0, 1>()
			) == kangaru::reference_kind::none);
		}
	}
}
