# Chapter 7: Object System

Objects are instances of classes - they're the actual structs that contain your data. While classes define the layout (like a struct definition), objects are the actual allocated memory that holds the values. This chapter covers how to create, access, and work with objects in the runtime.

## 7.1 Object Creation

Objects are created through infraclass allocation, similar to how you'd allocate a struct in C. The runtime provides functions to allocate memory and initialize the object structure.

### Basic Object Allocation

```c
// Create a basic object (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    void *object;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("NSObject")
    );
    
    object = mulle_objc_infraclass_alloc_instance(cls);
    printf("Allocated object: %p\n", object);
    printf("Object class: %s\n", 
           mulle_objc_class_get_name(_mulle_objc_object_get_isa(object)));
    
    return 0;
}
```

### Object Initialization

After allocation, objects need initialization. The runtime automatically sets up the isa pointer (which points to the class), but you may need to initialize instance variables.

```c
// Initialize object with ivar access (18 lines)
void initialize_object_with_values(const char *class_name)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    void *object;
    struct _mulle_objc_ivar *ivar;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe, mulle_objc_classid_from_string(class_name));
    
    object = mulle_objc_infraclass_alloc_instance(cls);
    
    // Access ivars by offset
    cls = mulle_objc_infraclass_as_class(_mulle_objc_object_get_isa(object));
    ivar = mulle_objc_infraclass_search_ivar(mulle_objc_object_get_infraclass(object), 
                mulle_objc_ivarid_from_string("_count"));
    if (ivar)
        *(int *)((char *)object + ivar->offset) = 42;
    
    printf("Object %p has count = %d\n", 
           object, *(int *)((char *)object + ivar->offset));
}
```

### Tagged Pointers

Small objects can be encoded directly in the pointer value itself, avoiding allocation. This is transparent to the caller but affects pointer identity checks.

```c
// Check if object is tagged pointer (12 lines)
int is_object_tagged(void *object)
{
    return mulle_objc_object_get_taggedpointerindex(object) != 0;
}

// Create and check tagged vs normal objects
void demonstrate_tagged_pointers(void)
{
    // Small integers often become tagged pointers
    void *small_int = (void *)0x7;  // Example tagged pointer
    void *normal_obj = malloc(16);  // Normal allocated object
    
    printf("Small int tagged: %d\n", is_object_tagged(small_int));
    printf("Normal object tagged: %d\n", is_object_tagged(normal_obj));
    free(normal_obj);
}
```

## 7.2 Object Access

Once you have an object, you can access its class, instance variables, and call methods on it.

### Getting Class from Object

Every object knows its class through the isa pointer, similar to how a struct might have a type field.

```c
// Get class information from object (15 lines)
void inspect_object_class(void *object)
{
    struct _mulle_objc_infraclass *cls;
    
    cls = mulle_objc_object_get_infraclass(object);
    printf("Object class: %s\n", mulle_objc_class_get_name(&cls->base));
    printf("Instance size: %zu bytes\n", 
           _mulle_objc_class_get_instancesize(&cls->base));
    printf("Has superclass: %s\n", 
           mulle_objc_class_get_superclass(&cls->base) ? "yes" : "no");
}
```

### Direct Ivar Access

You can directly access instance variables by their offset, similar to accessing struct members.

```c
// Access ivars directly by offset (18 lines)
void access_object_ivars(void *object, const char *ivar_name)
{
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_ivar *ivar;
    void *ivar_ptr;
    
    cls = mulle_objc_object_get_infraclass(object);
    ivar = mulle_objc_infraclass_search_ivar(cls, 
                mulle_objc_ivarid_from_string(ivar_name));
    
    if (!ivar) {
        printf("Ivar %s not found\n", ivar_name);
        return;
    }
    
    ivar_ptr = (char *)object + ivar->offset;
    printf("Ivar %s at offset %zu: %p\n", 
           ivar_name, ivar->offset, ivar_ptr);
    
    // Example: if it's an int, read it
    if (strcmp(ivar->signature, "i") == 0)
        printf("Value: %d\n", *(int *)ivar_ptr);
}
```

### Method Calls from Object

To call a method on an object, you need to look up the method implementation and call it with both the object and selector as arguments.

```c
// Call method on object (20 lines)
int call_object_method(void *object, const char *method_name)
{
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_method *method;
    mulle_objc_methodid_t selector;
    
    cls = mulle_objc_object_get_infraclass(object);
    selector = mulle_objc_methodid_from_string(method_name);
    method = mulle_objc_infraclass_search_method_nofail(cls, selector);
    
    if (!method) {
        printf("Method %s not found\n", method_name);
        return -1;
    }
    
    return method->function_pointer(object, selector);  // Call the method
}
```

## 7.3 Memory Management

Objects need to be properly retained and released to avoid memory leaks.

### Reference Counting

The runtime uses reference counting for memory management, similar to malloc/free but with reference counts.

```c
// Basic retain/release operations (15 lines)
void demonstrate_retain_release(void *object)
{
    unsigned int retain_count;
    
    printf("Initial retain count: %u\n", 
           _mulle_objc_object_get_retaincount(object));
    
    mulle_objc_object_call_retain(object);
    printf("After retain: %u\n", 
           _mulle_objc_object_get_retaincount(object));
    
    mulle_objc_object_call_release(object);
    printf("After release: %u\n", 
           _mulle_objc_object_get_retaincount(object));
}
```

### Automatic Cleanup

When retain count reaches zero, the object is automatically deallocated.

```c
// Automatic deallocation example (18 lines)
void create_and_cleanup_object(void)
{
    struct _mulle_objc_infraclass *cls;
    void *object;
    
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        mulle_objc_global_get_defaultuniverse(),
        mulle_objc_classid_from_string("NSObject")
    );
    
    object = mulle_objc_infraclass_alloc_instance(cls);
    mulle_objc_object_call_retain(object);  // Retain count = 1
    
    printf("Object created: %p\n", object);
    
    // When we release, it will be deallocated
    mulle_objc_object_call_release(object);  // Retain count = 0, object freed
    printf("Object deallocated\n");
}
```

## 7.4 Cache Systems

The runtime uses various caches to speed up object operations.

### Method Cache

Every class has a method cache to avoid expensive lookups. Use `_mulle_objc_class_probe_implementation` to check the cache without triggering a full lookup.

```c
// Demonstrate cache behavior (15 lines)
void demonstrate_method_cache(void *object)
{
    struct _mulle_objc_class *cls;
    struct _mulle_objc_method *method;
    mulle_objc_methodid_t selector;
    
    cls = mulle_objc_object_get_infraclass(object);
    selector = mulle_objc_methodid_from_string("description");
    
    // Use standard method lookup - cache is handled transparently
    method = mulle_objc_class_search_method_nofail(cls, selector);
    printf("Method lookup: %p\n", method);
    
    // Subsequent lookups benefit from cache
    method = mulle_objc_class_search_method_nofail(cls, selector);
    printf("Cached lookup: %p\n", method);
}
```

### Class Cache

Class lookups are also cached for performance.

```c
// Demonstrate class caching (15 lines)
void demonstrate_class_cache(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls1, *cls2;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // First lookup
    cls1 = mulle_objc_universe_lookup_infraclass_nofail(
        universe, mulle_objc_classid_from_string("NSObject"));
    
    // Second lookup - should be cached
    cls2 = mulle_objc_universe_lookup_infraclass_nofail(
        universe, mulle_objc_classid_from_string("NSObject"));
    
    printf("Same class pointer: %s\n", cls1 == cls2 ? "yes" : "no");
}
```

## Object Copy and Advanced Operations

### Object Copy

The runtime provides a simple way to copy objects using the standard Objective-C copy mechanism:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void demonstrate_object_copy(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    void *original_object;
    void *copied_object;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe,
        mulle_objc_classid_from_string("NSString")
    );
    
    // Create original object
    original_object = mulle_objc_infraclass_alloc_instance(cls);
    
    // Copy object (calls -copy method)
    copied_object = mulle_objc_object_copy(original_object);
    
    printf("Original: %p\n", original_object);
    printf("Copy: %p\n", copied_object);
    printf("Same pointer: %s\n", original_object == copied_object ? "yes" : "no");
}
```

### Multiple Object Batch Calls

Efficiently call the same method on multiple objects using `mulle_objc_objects_call()`:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void demonstrate_batch_object_calls(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *cls;
    void *objects[3];
    mulle_objc_methodid_t selector;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_universe_lookup_infraclass_nofail(
        universe,
        mulle_objc_classid_from_string("NSObject")
    );
    
    // Create multiple objects
    objects[0] = mulle_objc_infraclass_alloc_instance(cls);
    objects[1] = mulle_objc_infraclass_alloc_instance(cls);
    objects[2] = mulle_objc_infraclass_alloc_instance(cls);
    
    // Batch call - call description on all objects
    selector = mulle_objc_methodid_from_string("description");
    
    printf("Calling description on %d objects:\n", 3);
    mulle_objc_objects_call(objects, 3, selector, NULL);
    
    // Note: This is a simplified example - in practice you'd handle return values
    // through the Meta-ABI parameter structure
}
```

### Thread Safety Requirements

When working with objects across multiple threads, proper thread registration is essential:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <pthread.h>

// Thread-safe object access
void *thread_safe_object_access(void *arg)
{
    struct _mulle_objc_universe *universe;
    void *object;
    
    // Register thread with runtime
    mulle_objc_thread_register(MULLE_OBJC_DEFAULTUNIVERSEID);
    
    // Setup thread context
    mulle_objc_thread_setup_threadinfo(
        mulle_objc_global_get_defaultuniverse()
    );
    
    // Now safe to use runtime objects
    universe = mulle_objc_global_get_defaultuniverse();
    object = mulle_objc_infraclass_alloc_instance(
        mulle_objc_universe_lookup_infraclass_nofail(
            universe,
            mulle_objc_classid_from_string("NSObject")
        )
    );
    
    // Perform thread-safe operations
    printf("Thread %p created object: %p\n", 
           (void *)pthread_self(), object);
    
    // Deregister before thread exit
    mulle_objc_thread_deregister(MULLE_OBJC_DEFAULTUNIVERSEID);
    
    return object;
}

int main(void)
{
    pthread_t thread1, thread2;
    
    // Main thread is automatically registered
    pthread_create(&thread1, NULL, thread_safe_object_access, NULL);
    pthread_create(&thread2, NULL, thread_safe_object_access, NULL);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    return 0;
}
```

## Key APIs Summary

| Function | Purpose |
|----------|---------|
| `mulle_objc_infraclass_alloc_instance()` | Allocate new object instance |
| `mulle_objc_object_get_infraclass()` | Get class from object |
| `mulle_objc_object_call_retain()` | Increment retain count |
| `mulle_objc_object_call_release()` | Decrement retain count |
| `_mulle_objc_object_get_retaincount()` | Get current retain count |
| `mulle_objc_object_get_taggedpointerindex()` | Check if pointer is tagged |
| `_mulle_objc_class_get_instancesize()` | Get instance size from class |
| `mulle_objc_object_copy()` | Create copy of object |
| `mulle_objc_objects_call()` | Batch call method on multiple objects |
| `mulle_objc_thread_register()` | Register thread for runtime access |
| `mulle_objc_thread_deregister()` | Unregister thread from runtime |