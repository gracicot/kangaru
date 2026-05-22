#include <kangaru/kangaru.hpp>

struct type1 { int id; };
struct type2 { int id; };
struct aggregate { type1 t1; type2& t2; };
struct type3 { aggregate agg; };
struct type4 { type3 t3; };
struct type5 { type4 t4; };

int main() {
	auto source = kangaru::with_recursion{
		kangaru::make_source_with_non_empty_construction(
			kangaru::composed_source{
				kangaru::object_source{type1{}},
				kangaru::reference_source{type2{}},
			}
		)
	};
	
	auto t5 = kangaru::provide<type5>(source);

	(void) t5.t4.t3.agg.t1.id;
	(void) t5.t4.t3.agg.t2.id;
}
