Invoke
======

In the previous tutorial, we were doing injection wia class constructors. Kangaru offers another way to inject services, that is through function calls.

For example, let's say you have this kind of code:

```c++
bool result = process_inputs(
    container.service<KeyboardStateService>(),
    container.service<MessageBusService>()
);
```

We want to make that kind of code less painful. 
The idea behind this is you give the container a function to call, and the container will match
parameters to services as automatically as possible, giving a nice shortcut for this kind of code.
This is what `kgr::container::invoke` is all about. It recieves a function to call, and it call it with injected services.

## Specifying Definitions

Injection of parameters can be done manually by specifying every definition the function needs.

Let's say we have this little function and services:
```c++
struct KeyboardStateService : kgr::single_service<KeyboardState> {};
struct MessageBusService    : kgr::single_service<MessageBus> {};

bool process_inputs(KeyboardState& ks, MessageBus& mb);
```

Then you can use the container to call the function, specifying each needed services:

```c++
bool result = container.invoke<KeyboardStateService, MessageBusService>(process_inputs);
```

Of course, all additional parameters sent to `invoke` are forwarded to the invoked function. We can change our function to send a `bool` parameter.
Just like with constructors, additional parameters are forwarded only after injected parameters:

```c++
bool process_inputs_mod(KeyboardState& ks, MessageBus& mb, bool check_modifiers);

bool result = container.invoke<KeyboardStateService, MessageBusService>(process_inputs_mod, false);
```

## Mapped Service

While the utilities shown above is a great shortcut for calling `container.service` for each parameter, we can do even better.
If we can tell the service how to match each parameter to a corresponding service, we could omit listing every needed definitions.

We will associate a parameter to a specific service definition using the service map.
The form of a mapping look like this:

```
auto service_map(<parameter>) -> <definition>;
```
So for `KeyboardState` and `MessageBus` the mapping is done as follow:

```c++
auto service_map(KeyboardState const&) -> KeyboardStateService;
auto service_map(MessageBus const&)    -> MessageBusService;
```
    
The service map is a function that takes a parameter to be mapped, and has the associated service definition as return type.
Each entry must be in the same namespace as the argument it receives. The container will search the right `service_map` declaration through ADL.

Now calling our `process_inputs` will look like this:
```c++
bool result = container.invoke(process_inputs);
bool result = container.invoke(process_inputs_mod, true);
```

Neat! Now the container will match every injected parameters automatically.
The great thing about this is that if one day our function signature change to take another injected parameter,
the calling site won't need to get refactored. So for example, we change the function signature to this:

```c++
bool process_inputs(KeyboardState& ks, MessageBus& mb, Scene newScene);
```

Just like changing the dependencies of a service constructor, the calling site stays the same:

```c++
bool result = container.invoke(process_inputs);
```

## Callable Objects

Just like function pointer, callable objects like lambdas are supported too:

```c++
auto function = [](Window& window, MessageBus& bus, int data) {
    // Do stuff with window and bus
    return 10 + data;
};

int quantity = container.invoke(function, 32); // quantity == 42
```

C++14 generic lambda are also supported. The only restriction is that all `auto` must be at the end. Just like this:

```c++
auto function = [](Window& window, MessageBus& bus, int a, auto b) { // b to be deduced
    return 10 + data + b;
};

double quantity = container.invoke(function, 30, 2.1); // quantity == 42.1, b deduced as double
```

> WARNING: To inspect the parameter types, the container must instanciate the template function by using forwarded parameters, which maintain their reference type. If you're calling `container.invoke([](auto){}, std::move(some_integer))`, it will first instanciate the lambda with `int&&` as `auto` to inspect parameters, and then call the function as usual. This may make generic lambda function to instanciate two times. If you want to avoid this behavior, consider using `auto&&` for deduced parameters. That way, generic lambdas are only instanciated one time with the right types.

[Next chapter: Polymorphic Services](section03_polymorphic.md)
