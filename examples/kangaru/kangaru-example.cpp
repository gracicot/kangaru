#include <iostream>
#include "kangaru.hpp"

using namespace std;
using namespace kgr;

struct MyContainer;

///////////////////////////////
//      Service Classes      //
///////////////////////////////
struct A {
	explicit A(int val = 0) : n{val} {}

	// A needs nothing
	int n;
};

struct B {
	// B needs A
	B(shared_ptr<A> a_ptr) : a{std::move(a_ptr)} {}

	shared_ptr<A> a;
};

struct AC {
	virtual ~AC();
	virtual int getN() const = 0;
};

AC::~AC() = default;

struct C : AC {
	// C needs A and B
	C(shared_ptr<A> a_ptr, shared_ptr<B> b_ptr) : a{std::move(a_ptr)}, b{std::move(b_ptr)} {}

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
	D(shared_ptr<B> b_ptr, shared_ptr<AC> c_ptr) : b{std::move(b_ptr)}, c{std::move(c_ptr)} {}

	shared_ptr<B> b;
	shared_ptr<AC> c;

	~D() { cout << "D is destroyed\n"; }
};

struct E : C {
	// E needs MyContainer, A and B
	// We needs A and B because C needs them
	E(shared_ptr<MyContainer> cont_ptr, shared_ptr<A> a_ptr, shared_ptr<B> b_ptr) :
		C {std::move(a_ptr), std::move(b_ptr)}, container{cont_ptr} {}

	int getN() const override;

	// weak_ptr because we don't want the MyContainer to hold a shared_ptr to himself
	weak_ptr<MyContainer> container;
};

int E::getN() const
{
	return 66;
}

struct MyContainer : Container {
protected:
	virtual void init() override;
};

void MyContainer::init()
{
	instance<A>(8);
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
int main()
{
	auto container = make_container<MyContainer>();

	container->callback([](std::shared_ptr<B> b, std::shared_ptr<AC> ac, int i) {
		cout << "a D is built with: " << i << '\n';
		return make_shared<D>(std::move(b), std::move(ac));
	});

	// let's get some services
	auto a = container->service<A>();
	auto b = container->service<B>();
	auto c = container->service<C>(); // I'm a E!
	auto ac = container->service<AC>(); // I'm a C!
	auto d1 = container->service<D>();
	auto d2 = container->service<D>(4);
	auto e = container->service<E>();

	cout << "a default value: " << a->n;

	a->n = 9;
	b->a->n = 8;
	// 5 will be the last value.
	// Since A is single, we will see the value 5 across all classes
	e->a->n = 5;

	cout << boolalpha;
	cout << "\nis same container: " << (e->container.lock() == container);
	cout << "\nis same D: " << (d1 == d2);
	cout << "\nis same A: " << ((a == b->a) && (a == e->a));
	cout << "\na: " << a->n;
	cout << "\nb: " << b->a->n;
	cout << "\nc (it's a E): " << c->getN();
	cout << "\nac (it's a C): " << ac->getN();
	cout << "\nd1 n: " << d1->b->a->n;
	cout << "\nd2 c getN(): " << d2->c->getN() << '\n';
}
