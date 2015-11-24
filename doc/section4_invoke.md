Invoke
======

Invoke is one pretty cool feature. It calls a function that receives services with the right set of parameters.

Let's say we have this function:

    int doThings(Notification n, FileManager& fm);
    
One would like to call the function this way:

    int result = container.invoke(doThings);
    
But this would not work.
In order to make it work, we must tell the container what parameter type is associated with which definition. This is what we call the `ServiceMap`. It is up to you to declare the `ServiceMap`. It should look like this:

    template<typename>
    struct ServiceMap;
    
Then, for each service in which you want to be able work with `invoke`, specialize the struct like this:

    template<> struct ServiceMap<Notification> { using Service = NotificationService; };
    template<> struct ServiceMap<FileManager&> { using Service = FileManagerService;  };

Note the presence of the `&` after `FileManager`. This is because `FileManager`, `FileManager&` and `const FileManager*` could all be bound to different service definitions.

Now that our service map is defined, we can use invoke like this:

    int doThings(Notification n, FileManager& fm);
    
    // ...
    
    int result = container.invoke<ServiceMap>(doThings);
    
With `ServiceMap`, the container can even call function from third party libraries!
