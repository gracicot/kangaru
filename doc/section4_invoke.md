Invoke
======

Invoke is one pretty neat feature. It calls a function that recieves services with the right set of parameter.

Let's say we have this function:

    int doThings(Notification n, FileManager& fm);
    
One would like to call the function this way:

    int result = container.invoke(doThings);
    
But this will not work.
To make it work, we must tell the container what parameter type is associated to which definition. This is what I call the `ServiceMap`. It is to you to declare the service map. It should look like this:

    template<typename>
    struct ServiceMap;
    
Then, for each service you want to work with `invoke`, specialize the struct like this:

    template<> struct ServiceMap<Notification> { using Service = NotificationService; };
    template<> struct ServiceMap<FileManager&> { using Service = FileManagerService;  };

Note the presence of the `&` after `FileManager`. This is because `FileManager`, `FileManager&` and `const FileManager*` could all be bound to different service definition.

Now that our service map is defined, we can use invoke this way:

    int doThings(Notification n, FileManager& fm);
    
    // ...
    
    int result = container.invoke<ServiceMap>(doThings);
    
With `ServiceMap`, the container can even call function from third party libraries!
 
[Next chapiter](section5_setters.md)
