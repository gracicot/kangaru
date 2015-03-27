sdicl
=====
sdicl, pronounced S-dicle, is a simple dependency injection container library for C++11. It manages multiple level of dependency, and can even inject himself into a service!
sdicl stands for "Simple Dependency Injection Container Library"

Getting Started
---------------
Getting started with sdicl is easy. First of all, you need to include the library:

    #include <sdicl.hpp>

Take note that you will need either to add the header to your include paths or to add it to your project.
Every declarations are made in the namespace sdicl.
The namespace sdicl is containing the namespace detail which contains implementation detail.

The Container
-------------
The container contains three methods:
 * `single<T, Bases...>()`
 * `service<T>()`
 * `init()`

### single
single will instanciate an object of type T and will register it as a type that is meant to have a single instance within the container. It only accept class that are a service. If you want an abstract class to be represented by this single, you can replace `Bases...` by all of it's abstract base you need to.

### service
service will instanciate a service of type T (if needed) and will return it. If the service as been registered as a "single", it will reuse the same instance everytime. However, the default behaviour is to construct new object each time.

### init
init is empty and virtual, it is meant to register every "single" services. The function is empty by default.

make_container
--------------
make container is a helper function to construct and initialize a container of any type. T must extends from Container.
It recieve every argument you need to send to your container's constructor (this should be very rare) and forward them just like make_shared.
It returns a shared_ptr of your container's type.

Declare a service
-----------------
In order to have the container to construct all of your objects, you must declare a service. Declaring a service is pretty is done in a couple of lines:
    
    struct MyClass {
        // MyClass needs Foo and Bar
        MyClass(shared_ptr<Foo> foo, shared_ptr<Bar> bar) : foo{foo}, bar{bar} {}
    
        shared_ptr<Foo> foo;
        shared_ptr<Bar> bar;
    };

    template<>
    struct Service<MyClass> {
        // MyClass depends on Foo and Bar
        using Dependencies = Dependency<Foo, Bar>;
    };
What all of this mean? Okay. Let's take a look at the first and second line after our class. We are declaring a template struct.

It is the specialization of the struct "Service" with your class. The only thing it contains is the dependencies of the service, which we can see at the fourth line after our class. The order of each dependency is the order the dependency is in our constructor.

Take note that Foo and Bar need to be services too to make this example valid.
It's possible to have an abstract class as a dependency. It will work as long as you register a concrete class to be the default service for this abstract service.

When you have no dependency, it is still required to declare this struct. the dependencies will be equal to `Dependency<>`

Container aware services
------------------------
Sometimes, you need to manage services in a service. That's why it's possible to send the container as a service. The container's type can be polymorphic. However, if the container fail to convert itself to it's base class, it will create a new container of this type as a "single"
    
    struct MyClass {
        // MyClass needs MyContainer
        MyClass(weak_ptr<MyContainer> container) : container{container} {}
    
        weak_ptr<MyContainer> container;
    };

    namespace sdicl {

    template<>
    struct Service<MyClass> {
        // MyClass depends on MyContainer
        using Dependencies = Dependency<MyContainer>;
    };
    
    }
Recieving the provided Container class works too:
    
    struct MyClass {
        // MyClass needs Foo and Bar
        MyClass(weak_ptr<Container> container) : container{container} {}
    
        weak_ptr<Container> container;
    };

    namespace sdicl {
    
    template<>
    struct Service<MyClass> {
        // MyClass depends on MyContainer
        using Dependencies = Dependency<Container>;
    };
    
    }
Notice how we changed the type.
As I said before, if the container cannot convert himself to a MyContainer as in this example or whatever you container type is, it will create a new one as a "single" service.
We can notice that we used weak_ptr instead of a shared_ptr. This practice prevent memory leak in recursive structures (Container -> MyClass -> Container ...)
  
Using your services
-------------------
Using your services is probably the easiest part. You just have to use the `service<T>()` method and you're done!
    
    using namespace sdicl;
    auto container = make_container<MyContainer>();
	
    // let's get some services
    auto myObject = container->service<MyClass>();
    myObject->foo->baz(); // myObject got a Foo injected!
The process of resolving dependencies recursively is completely abstracted.

If you missed something, here's a complete example of a small program using sdicl:

    #include <iostream>

    #include "sdicl.hpp"

    using namespace std;
    using namespace sdicl;

    class MyContainer;

    ///////////////////////////////
    //      Service Classes      //
    ///////////////////////////////
    struct A {
        // A needs nothing
        int n = 0;
    };

    struct B {
        // B needs A
        B(shared_ptr<A> a) : a {a} {}

        shared_ptr<A> a;
    };

    struct AC {
        virtual int getN() const = 0;
    };

    struct C : AC {
        // C needs A and B
        C(shared_ptr<A> a, shared_ptr<B> b) : a {a}, b {b} {}

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
        D(shared_ptr<B> b, shared_ptr<AC> c) : b {b}, c {c} {}

        shared_ptr<B> b;
        shared_ptr<AC> c;
    };

    struct E : C {
        // E needs MyContainer, A and B
        // We needs A and B because C needs them
        // weak_ptr because we don't want the MyContainer to hold a shared_ptr to himself
        E(weak_ptr<MyContainer> container, shared_ptr<A> a, shared_ptr<B> b) :
            C {a, b}, container {container} {}

        weak_ptr<MyContainer> container;
    };

    struct MyContainer : Container {
        void init() override;
    };

    void MyContainer::init()
    {
        single<A>();
        single<B>();
		// the service C will be registered as AC too
        single<C, AC>();
        // let's say that D is not single
        single<E>();
    }


    ///////////////////////////////
    //     Service Meta Data     //
    ///////////////////////////////
    namespace sdicl {

    template<>
    struct Service<A> {
        // A depends on nothing
        using Dependencies = Dependency<>;
    };

    template<>
    struct Service<B> {
        // B depends on A
        using Dependencies = Dependency<A>;
    };

    template<>
    struct Service<C> {
        // C depends on A and B
        using Dependencies = Dependency<A, B>;
    };

    template<>
    struct Service<D> {
        // D depends on B and AC
		// The AC class will be a C (see init for details)
        using Dependencies = Dependency<B, AC>;
    };

    template<>
    struct Service<E> {
        // E depends on MyContainer, A and B
        using Dependencies = Dependency<MyContainer, A, B>;
    };

    }

    ///////////////////////////////
    //        Usage Example      //
    ///////////////////////////////
    int main(int argc, char** argv)
    {
        auto container = make_container<MyContainer>();

        // let's get some services
        auto a = container->service<A>();
        auto b = container->service<B>();
        auto c = container->service<C>();
        auto d1 = container->service<D>();
        auto d2 = container->service<D>();
        auto e = container->service<E>();

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
        cout << "d1: " << d1->b->a->n << endl;
        cout << "d2: " << d2->c->getN() << endl;

        return 0;
    }


This program should output:

    is same container: true
    is same D: false
    is same A: true
    a: 5
    b: 5
    d1: 5
    d2: 21

What's next?
------------
There is some feature I would like to see become real. Here's a list of those, feel free to contribute!
 * Testing with mutliple / virtual inheritance
 * Have callback to initialize services
 * Cleanup the code
