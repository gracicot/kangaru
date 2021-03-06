Managing Containers
===================

The container can be seen as a repository of services.
It's possible to manage many container and operate between them with several operation.

We will use these operation to scope the lifetime of single services, and extend it.

## Fork

This operation creates a new container from a container. That new container will observe all instances contained in the source one.

```c++
kgr::container container1;

container1.emplace<SingleService1>();
container1.emplace<SingleService2>();

auto container2 = container1.fork();

container1.emplace<SingleService3>();
container2.emplace<SingleService3>();
```

In this example, `container1` first create and saves `Single1` and `Single2`. Then, we fork `container1` to create `container2`.
At that point both container 1 and 2 references the same services.

Then, in both container, we creates `Single3` in both containers. Since we forked the container, these two containers now have separated instances of `Single3`.

Here's a graph to represent that:

    container1  ---1---2---*---3
                            \
    container2               ---3'

Note that the lifetime of the `container2` must not extend the lifetime of `container1`.
This is because the first container is owner of `Single1` and `Single2`.
A container forked or rebased from another is considered invalidated if the source container dies.

In this example, while `container2` is still owner of `Single3`, that service and all it's references are also considered invalidated since `Service3` may depend on `Single1` and `Single2` from `container1` that died.

## Merge

The merge operation is the contrary of the fork. It will take all instances of one container and move them all into another container.
This is useful when you forked the container, created some services in the new container, and want to bring those services into the original before dropping the fork.
In this operation, the merged container will no longer be owner of any instances. You can drop that empty container without having any invalidated services.

In case of conflict, the original container will prefer it's own services over the merged container services.

```c++
kgr::container container1;

container1.emplace<SingleService1>();

{
    auto container2 = container1.fork();
    
    container1.emplace<SingleService2>();
    container2.emplace<SingleService2>();
    container2.emplace<SingleService3>();
    
    // At that point, both containers have their own SingleService2 instance.
    // Only container2 has SingleService3
    // container1 is owner of SingleService1, and container2 observes it.
    
    container1.merge(container2);
    
    // container2 is still valid, but is no longer owner of any services.
    // We can still use container2 and it still observe every service it owned before.
}

// At that point, no services are deleted yet.
// container1 is owner of all instances created so far.
// Since both containers had their own version of SingleService2, container1 kept his own instance.
// Now container1 has a SingleService3 that came from the merging of container2 into it.
```

A graph of this example look like this:

    container1  ---1---*---2-----------*---
                        \             /
    container2           ---2'---3---

## Rebase

Containers can also be rebased from another one. In fact, a fork is simply the creation of a new container rebased on the original.
This operation makes the forked container observe any new singles added in the original container since the fork.

```c++
kgr::container container1;

container1.emplace<SingleService1>();

auto container2 = container1.fork();

container2.contains<SingleService1>(); // true
container2.contains<SingleService2>(); // false

container1.emplace<SingleService2>();
container2.rebase(container1); // rebase from container1

container2.contains<SingleService1>(); // true
container2.contains<SingleService2>(); // true
```

Here's the graph for it:

    container1  ---1---*---2---*---
                        \       \
    container2           --------*---

Note that you can also rebase unrelated containers:

```c++
kgr::container container1;
kgr::container container2;

container1.emplace<SingleService1>();

container2.contains<SingleService1>(); // false
container2.rebase(container1);
container2.contains<SingleService1>(); // true
```

## Conclusion

As we can see, containers are not just a class that contains every instance for all your classes. Single services are not just plain singletons. You can manage multiple instances of those and operate on them, have local containers and more.

To see this in action, check out [example4](../examples/example4/example4.cpp), which shows more usage related to the snippets found on this page.

[Next chapter: Supplied Services](section05_supplied.md)
