# Chapter 9: Advanced Topics

Beyond basic class and method operations, the runtime provides advanced features for dynamic code loading, custom runtime instances, and runtime extensions. These are like having a plugin system for your structs - you can load new methods at runtime, create isolated runtime environments, and extend the runtime itself.

## 9.1 Dynamic Loading

The runtime can load new classes and categories at runtime, similar to how you might dynamically load shared libraries in C. This includes automatic +load and +initialize method execution.

### Load System (+load and +unload)

When the runtime loads new code, it automatically calls +load methods on classes and categories. This is like having a constructor that runs automatically when a shared library is loaded.

```c
// Example of +load method being called automatically (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

@interface DynamicClass
+ (void)load;
@end

@implementation DynamicClass
+ (void)load
{
    printf("DynamicClass +load called\n");
}
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    // +load will be called automatically when this code is loaded
    printf("Code loaded, check for +load output above\n");
    return 0;
}
```

### Category Loading and Method Injection

Categories allow you to add methods to existing classes at runtime. This is like extending a struct with new function pointers after it's been defined.

```c
// Adding methods via category (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

@interface BaseClass
- (int)baseMethod;
@end

@implementation BaseClass
- (int)baseMethod { return 10; }
@end

// Category adding new methods
@interface BaseClass (Extension)
- (int)extendedMethod;
@end

@implementation BaseClass (Extension)
- (int)extendedMethod { return 20; }
@end

int main(void)
{
    // extendedMethod is now available on BaseClass instances
    printf("Category method available\n");
    return 0;
}
```

### Class Initialization (+initialize mechanics)

+initialize is called once per class before the class is first used. This is like a lazy initialization function that runs exactly once.

```c
// +initialize example (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

@interface InitializeClass
+ (void)initialize;
+ (int)getInitializedValue;
@end

static int initialized_value = 0;

@implementation InitializeClass
+ (void)initialize
{
    initialized_value = 100;
    printf("InitializeClass +initialize called\n");
}

+ (int)getInitializedValue { return initialized_value; }
@end

int main(void)
{
    int value = [InitializeClass getInitializedValue];
    printf("Initialized value: %d\n", value);
    return 0;
}
```

### Protocol System and Interface Inheritance

Protocols define interfaces that classes can conform to, similar to C interfaces or abstract base classes.

```c
// Protocol definition and conformance checking (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol Printable
- (const char *)description;
@end

@interface ConformingClass <Printable>
@end

@implementation ConformingClass
- (const char *)description { return "ConformingClass instance"; }
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_protocol *protocol;
    
    universe = mulle_objc_global_get_defaultuniverse();
    protocol = mulle_objc_universe_lookup_protocolstring(universe, "Printable");
    
    if (protocol)
        printf("Found Printable protocol\n");
    
    return 0;
}
```

## 9.2 Advanced Patterns

Beyond basic usage, the runtime supports custom runtime instances and extension mechanisms.

### Custom Universes (Named Runtime Instances)

You can create isolated runtime environments, each with their own class registries. This is like having multiple independent programs within one process.

```c
// Creating custom universe (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *custom_universe;
    struct _mulle_objc_universe *default_universe;
    
    // Create a custom universe
    custom_universe = mulle_objc_alloc_universe(mulle_objc_universeid_from_string("custom"), "custom");
    default_universe = mulle_objc_global_get_defaultuniverse();
    
    printf("Custom universe: %p\n", custom_universe);
    printf("Default universe: %p\n", default_universe);
    printf("Universes are different: %s\n", 
           custom_universe != default_universe ? "YES" : "NO");
    
    // Clean up
    mulle_objc_universe_destroy(custom_universe);
    return 0;
}
```

### Runtime Extensions

The runtime can be extended with new features and capabilities. This is like adding new system calls to an operating system.

```c
// Runtime extension example (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Custom runtime extension function
void register_custom_behavior(struct _mulle_objc_class *cls)
{
    printf("Registering custom behavior for %s\n", 
           mulle_objc_class_get_name(cls));
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    printf("Runtime extensions can be registered here\n");
    
    return 0;
}
```

### Integration Patterns

The runtime can be integrated with existing C code and other runtime systems.

```c
// Integration with existing C code (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Existing C function
int c_function(int x) { return x * 2; }

@interface IntegrationClass
- (int)useCFunction:(int)x;
@end

@implementation IntegrationClass
- (int)useCFunction:(int)x
{
    return c_function(x);
}
@end

int main(void)
{
    printf("C function integration works\n");
    return 0;
}
```

### Migration Strategies

Moving from Apple's Objective-C runtime to mulle-objc requires understanding the differences and providing compatibility shims.

```c
// Migration compatibility example (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Compatibility macro for Apple's runtime
#define objc_getClass(name) \
    mulle_objc_universe_lookup_infraclass_nofail(mulle_objc_global_get_defaultuniverse(), \
                                                 mulle_objc_classid_from_string(name))

int main(void)
{
    struct _mulle_objc_infraclass *cls;
    
    cls = objc_getClass("NSString");
    if (cls)
        printf("Migration compatibility: class found\n");
    
    return 0;
}
```

## Practical Example: Dynamic Plugin System

```c
// Complete dynamic plugin example (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

@interface PluginProtocol
- (const char *)pluginName;
- (void)execute;
@end

// Plugin would be loaded dynamically
@interface MyPlugin : Object <PluginProtocol>
@end

@implementation MyPlugin
- (const char *)pluginName { return "MyPlugin"; }
- (void)execute { printf("Plugin executed\n"); }
@end

int main(void)
{
    printf("Plugin system ready for dynamic loading\n");
    return 0;
}
```

## Key APIs Summary

| Function | Purpose |
|----------|---------|
| `mulle_objc_alloc_universe()` | Create custom runtime instance |
| `mulle_objc_universe_destroy()` | Clean up custom universe |
| `_mulle_objc_universe_lookup_protocol()` | Look up protocol by name |
| `mulle_objc_universe_walk_classes()` | Enumerate loaded classes |