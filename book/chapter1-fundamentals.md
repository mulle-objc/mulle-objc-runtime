# Chapter 1: Runtime Fundamentals

The **universe** is your connection to the runtime - a global struct variable that contains everything: classes, methods, ivars, and the mapping between string names to unique integer IDs.

## Working with Universes

### Getting the Default Universe

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    // Get the default universe for this thread
    universe = mulle_objc_global_get_defaultuniverse();
    
    printf("Default universe: %p\n", universe);
    return 0;
}
```

### Named Universe Access

You can also access universes by name, which is useful for managing multiple isolated runtime contexts:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    mulle_objc_universeid_t universe_id;
    
    // Get universe by unique ID
    universe_id = mulle_objc_universeid_from_string("MyAppContext");
    universe = mulle_objc_global_lookup_universe(universe_id);
    
    if (!universe)
    {
        // Create if it doesn't exist
        universe = mulle_objc_alloc_universe(mulle_objc_universeid_from_string("MyAppContext"), "MyAppContext");
    }
    
    printf("Named universe: %p\n", universe);
    return 0;
}
```

### Preparing a thread for Universe Access

Each thread, that wants to access the universe needs to set up thread-local storage that will
contain runtime context. You can setup the the universe's thread-local storage
like thois:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <pthread.h>

void *thread_function(void *arg)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_threadinfo *threadinfo;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Setup thread context (call only once per thread lifetime)
    mulle_objc_thread_setup_threadinfo(universe);
    
    threadinfo = _mulle_objc_thread_get_threadinfo(universe);
    
    printf("Thread %p: universe=%p, threadinfo=%p, thread_nr=%lu\n", 
           (void*)pthread_self(), universe, threadinfo, 
           _mulle_objc_threadinfo_get_nr(threadinfo));
    
    return NULL;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_threadinfo *threadinfo;
    pthread_t thread1, thread2;
    
    // Main thread - setup is automatic
    universe = mulle_objc_global_get_defaultuniverse();
    threadinfo = _mulle_objc_thread_get_threadinfo(universe);
    
    printf("Main thread: threadinfo=%p, thread_nr=%lu\n", 
           threadinfo, _mulle_objc_threadinfo_get_nr(threadinfo));
    
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}
```

## Unique Identifier System

Interacting with the runtime you use 32-bit integer IDs for fast lookup. These
unique ids are derived from C strings:

### Converting Names to IDs

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    mulle_objc_classid_t   class_id;
    mulle_objc_methodid_t  method_id;
    
    // Convert class name to ID
    class_id = mulle_objc_classid_from_string("NSObject");
    printf("NSObject class ID: 0x%08x\n", class_id);
    
    // Convert method name to ID
    method_id = mulle_objc_methodid_from_string("init");
    printf("init method ID: 0x%08x\n", method_id);
    
    return 0;
}
```

### Working with IDs

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    mulle_objc_classid_t id;
    const char *name;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Get ID for a class
    id = mulle_objc_classid_from_string("NSString");
    
    // Convert ID back to name
    cls = mulle_objc_universe_lookup_infraclass(universe, id);
    name = cls ? _mulle_objc_infraclass_get_name(cls) : NULL;
    printf("Class name for 0x%08x: %s\n", id, name);
    
    return 0;
}
```

## Basic Class Discovery

### Finding Classes by Name

The runtime provides both safe and exception-raising variants of lookup functions. Functions with the `_nofail` suffix will raise an exception if the requested item is not found, eliminating the need for NULL checks.

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // _nofail suffix means this will raise exception if not found
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe,
        mulle_objc_classid_from_string("NSObject")
    );
    
    // No NULL check needed - _nofail guarantees success or exception
    printf("NSObject found at %p\n", cls);
    
    return 0;
}
```


## Multiple Universes and TPS Limitations

The runtime supports multiple isolated universes that can coexist but cannot message each other. However, there's an important limitation:

### Tagged Pointer Support (TPS) Limitation

Only the **global universe** can be compiled with TPS (tagged pointer support). Named universes cannot use tagged pointers.

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *global_universe;
    struct _mulle_objc_universe *named_universe;
    
    // Global universe supports TPS
    global_universe = mulle_objc_global_get_defaultuniverse();
    
    // Named universe - no TPS support
    named_universe = mulle_objc_alloc_universe(
        mulle_objc_universeid_from_string("PluginContext"), 
        "PluginContext"
    );
    
    printf("Global universe supports TPS: %d\n", 
           mulle_objc_universe_supports_tagged_pointers(global_universe));
    printf("Named universe supports TPS: %d\n", 
           mulle_objc_universe_supports_tagged_pointers(named_universe));
    
    return 0;
}
```

## Thread Registration and Lifecycle

Each thread must properly register with the runtime before accessing objects and deregister when done.

### Complete Thread Lifecycle Example

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <pthread.h>

void *worker_thread(void *arg)
{
    struct _mulle_objc_universe *universe;
    
    // Register this thread with the runtime
    mulle_objc_thread_register(MULLE_OBJC_DEFAULTUNIVERSEID);
    
    // Setup thread context
    mulle_objc_thread_setup_threadinfo(
        mulle_objc_global_get_defaultuniverse()
    );
    
    // Now safe to use runtime objects
    universe = mulle_objc_global_get_defaultuniverse();
    printf("Thread registered, universe: %p\n", universe);
    
    // Periodic check-in for garbage collection
    mulle_objc_thread_checkin(MULLE_OBJC_DEFAULTUNIVERSEID);
    
    // Do runtime work here...
    
    // Deregister before thread exit
    mulle_objc_thread_deregister(MULLE_OBJC_DEFAULTUNIVERSEID);
    
    return NULL;
}

int main(void)
{
    pthread_t thread;
    
    // Main thread is automatically registered
    pthread_create(&thread, NULL, worker_thread, NULL);
    pthread_join(thread, NULL);
    
    return 0;
}
```

## ID Calculation with mulle-objc-uniqueid Tool

The runtime provides a standalone tool for calculating IDs:

```bash
$ mulle-objc-uniqueid NSString
40413ff3

$ mulle-objc-uniqueid init
c2d87842

$ mulle-objc-uniqueid "setFoo:"
7a3e5f1b
```

### Programmatic ID Calculation

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    mulle_objc_classid_t class_id;
    mulle_objc_methodid_t method_id;
    mulle_objc_protocolid_t protocol_id;
    mulle_objc_ivarid_t ivar_id;
    mulle_objc_propertyid_t property_id;
    
    // Calculate all types of IDs
    class_id = mulle_objc_classid_from_string("MyClass");
    method_id = mulle_objc_methodid_from_string("doSomething");
    protocol_id = mulle_objc_protocolid_from_string("MyProtocol");
    ivar_id = mulle_objc_ivarid_from_string("_instanceVar");
    property_id = mulle_objc_propertyid_from_string("name");
    
    printf("MyClass: 0x%08x\n", class_id);
    printf("doSomething: 0x%08x\n", method_id);
    printf("MyProtocol: 0x%08x\n", protocol_id);
    printf("_instanceVar: 0x%08x\n", ivar_id);
    printf("name: 0x%08x\n", property_id);
    
    return 0;
}
```

## Key APIs Summary

| Function | Purpose |
|----------|---------|
| `mulle_objc_global_get_defaultuniverse()` | Get default universe |
| `mulle_objc_alloc_universe()` | Create custom universe |
| `mulle_objc_classid_from_string()` | Convert class name to ID |
| `mulle_objc_methodid_from_string()` | Convert method name to ID |
| `mulle_objc_universe_lookup_infraclass()` | Find class by ID |
| `mulle_objc_infraclass_alloc_instance()` | Create object instance |
| `mulle_objc_class_get_instancesize()` | Get object size |
| `mulle_objc_class_get_name()` | Get class name |
| `mulle_objc_thread_register()` | Register thread with runtime |
| `mulle_objc_thread_deregister()` | Unregister thread |
| `mulle_objc_thread_checkin()` | Periodic GC check-in |
| `mulle_objc_universe_supports_tagged_pointers()` | Check TPS support |

**Next**: Chapter 2 dives deeper into class system operations with more advanced examples and real-world usage patterns.