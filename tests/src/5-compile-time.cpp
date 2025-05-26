#include <kangaru/kangaru.hpp>

struct type1 { int id; };
struct type2 { int id; };
struct aggregate { type1 t1; type2& t2; };
struct type3 { aggregate agg; };

int main() {
	auto obj = type2{.id = 8};
	auto source = kangaru::with_recursion{
		kangaru::make_source_with_non_empty_construction(
			kangaru::composed_source{
				kangaru::object_source{type1{}},
				kangaru::reference_source{type2{}},
			}
		)
	};
	
	auto t3 = kangaru::provide<type3>(source);
}
