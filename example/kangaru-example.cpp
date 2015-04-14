#include <iostream>
#include "kangaru.hpp"

using namespace std;
using namespace kgr;

class MyContainer;

///////////////////////////////
//      Service Classes      //
///////////////////////////////
struct A {
	A(int n = 0) : n{n} {};

	// A needs nothing
	int n = 0;
};

struct B {
	// B needs A
	B(shared_ptr<A> a) : a{a} {}

	shared_ptr<A> a;
};

struct AC {
	virtual int getN() const = 0;
};

struct C : AC {
	// C needs A and B
	C(shared_ptr<A> a, shared_ptr<B> b) : a{a}, b{b} {}

	int getN() const override;

	shared_ptr<A> a;
	shared_ptr<B> b;
};

int C::getN() const
{
	return 21;
}


struct D {
	// D needs B and AC
	D(shared_ptr<B> b, shared_ptr<AC> c) : b{b}, c{c} {}

	shared_ptr<B> b;
	shared_ptr<AC> c;
};

struct E : C {
	// E needs MyContainer, A and B
	// We needs A and B because C needs them
	E(shared_ptr<MyContainer> container, shared_ptr<A> a, shared_ptr<B> b) :
		C {a, b}, container{container} {}

	int getN() const override;

	// weak_ptr because we don't want the MyContainer to hold a shared_ptr to himself
	weak_ptr<MyContainer> container;
};

int E::getN() const
{
	return 66;
}

class MyContainer : public Container {
public:
	void init() override;
};

void MyContainer::init()
{
	instance(make_shared<A>(8));
	instance<C>();
	instance<E>();
}

///////////////////////////////
//     Service Meta Data     //
///////////////////////////////
namespace kgr {

template<> struct Service<A> : NoDependencies, Single {};

// B depends on A
template<> struct Service<B> : Dependency<A>, Single {};

// C depends on A and B
template<> struct Service<AC> : NoDependencies, Single {};

// C depends on A and B
template<> struct Service<C> : Dependency<A, B>, Overrides<AC> {};

// D depends on B and AC
// The AC class will be a C (see init for details)
template<> struct Service<D> : Dependency<B, AC> {};

// E depends on MyContainer, A and B
template<> struct Service<E> : Dependency<MyContainer, A, B>, Overrides<C> {};

}

///////////////////////////////
//        Usage Example      //
///////////////////////////////
int main(int argc, char** argv)
{
	auto container = make_container<MyContainer>();

	container->callback<D>([](std::shared_ptr<B> b, std::shared_ptr<AC> ac) {
		cout << "a D is built" << endl;
		return make_shared<D>(b, ac);
	});

	// let's get some services
	auto a = container->service<A>();
	auto b = container->service<B>();
	auto c = container->service<C>(); // I'm a E!
	auto ac = container->service<AC>(); // I'm a C!
	auto d1 = container->service<D>();
	auto d2 = container->service<D>();
	auto e = container->service<E>();

	cout << "a default value: " << a->n << endl;

	a->n = 9;
	b->a->n = 8;
	// 5 will be the last value.
	// Since A is single, we will see the value 5 across all classes
	e->a->n = 5;

	cout << "is same container: " << (e->container.lock() == container ? "true" : "false") << endl;
	cout << "is same D: " << (d1 == d2 ? "true" : "false") << endl;
	cout << "is same A: " << ((a == b->a) && (a == e->a) ? "true" : "false") << endl;
	cout << "a: " << a->n << endl;
	cout << "b: " << b->a->n << endl;
	cout << "c (it's a E): " << c->getN() << endl;
	cout << "ac (it's a C): " << ac->getN() << endl;
	cout << "d1 n: " << d1->b->a->n << endl;
	cout << "d2 c getN(): " << d2->c->getN() << endl;

	return 0;
}
