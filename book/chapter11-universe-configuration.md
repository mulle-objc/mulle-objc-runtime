# Chapter 11: Universe Configuration

The universe is the runtime's global state container. While you can use the default universe without configuration, advanced applications often need to customize the runtime behavior. This chapter explains how to configure universes using `_mulle_objc_universe_bang()` and related APIs.

## Understanding Universe Initialization

When a universe is first accessed, it needs to be initialized. This is done through the `_mulle_objc_universe_bang()` function, which is the runtime's equivalent of a constructor for the universe.

### The Bang Function

`_mulle_objc_universe_bang()` performs the heavy lifting of universe initialization:

```c
void _mulle_objc_universe_bang(
    struct _mulle_objc_universe *universe,
    void (*bang)(struct _mulle_objc_universe *universe,
                 struct mulle_allocator *allocator,
                 void *userinfo),
    struct mulle_allocator *allocator,
    void *userinfo
);
```

The function takes four parameters:
- **universe**: The universe to initialize
- **bang**: Optional callback for custom initialization
- **allocator**: Custom allocator (NULL for default)
- **userinfo**: Context data for the bang callback

### Runtime Startup: When and How Universes Are Initialized

The `__register_mulle_objc_universe` function is automatically called by the Objective-C runtime startup mechanism. You don't call it directly - it's part of the runtime's initialization process.

#### Who Calls It
- **The Foundation**: Your Objective-C framework (Foundation/Foundation) or test program defines this function
- **Called Once**: Exactly once per universe ID, automatically when the runtime starts
- **Implicit Trigger**: Triggered by the first Objective-C message send or class access

#### When It's Called
The initialization sequence works like this:

1. **Program startup**: Your app starts
2. **Objective-C usage**: First time you use Objective-C (create object, send message, etc.)
3. **Runtime startup**: Runtime calls `__register_mulle_objc_universe` with default universe ID
4. **Universe bang**: `_mulle_objc_universe_bang` initializes the universe
5. **Ready to use**: Universe is now configured and ready

#### Example from Runtime Startup

```c
// This is defined by your Foundation/Foundation equivalent
// It's automatically invoked by the runtime startup process
MULLE_C_GLOBAL
MULLE_C_CONST_RETURN struct _mulle_objc_universe  *
   __register_mulle_objc_universe( mulle_objc_universeid_t universeid,
                                   char *universename)
{
   struct _mulle_objc_universe   *universe;

   // Get or create the universe
   universe = __mulle_objc_global_get_universe( universeid, universename);
   
   // Initialize if not already done
   if( _mulle_objc_universe_is_uninitialized( universe))
      _mulle_objc_universe_bang( universe, 0, NULL, NULL);

   return( universe);
}
```

#### What This Means for You
- **Most users**: Never need to worry about this - it's automatic
- **Custom frameworks**: If you're writing a Foundation, you define this function
- **Advanced users**: You can override this with your own registration function
- **Testing**: You can call it explicitly to ensure universe is initialized

## Custom Universe Configuration

### Using the Bang Callback

You can customize universe behavior by providing a bang callback:

```c
// Custom universe configuration
void my_universe_bang(struct _mulle_objc_universe *universe,
                      struct mulle_allocator *allocator,
                      void *userinfo)
{
    // Custom configuration here
    printf("Configuring universe %p\n", universe);
    
    // The runtime handles the rest
    _mulle_objc_universe_defaultbang(universe, allocator, userinfo);
}

// Register custom universe
struct _mulle_objc_universe *
configure_custom_universe(const char *name)
{
    mulle_objc_universeid_t universeid;
    struct _mulle_objc_universe *universe;
    
    universeid = mulle_objc_universeid_from_string(name);
    universe = mulle_objc_global_register_universe(universeid, (char *)name);
    
    if (universe)
        _mulle_objc_universe_bang(universe, my_universe_bang, NULL, NULL);
    
    return universe;
}
```

### Default Bang Behavior

When no custom bang is provided, `_mulle_objc_universe_defaultbang()` is used, which:
- Initializes internal data structures
- Sets up method caches
- Configures garbage collection
- Establishes foundation defaults

## Practical Configuration Examples

### Custom Allocator Configuration

```c
// Configure universe with custom allocator
void configure_with_custom_allocator(struct _mulle_objc_universe *universe)
{
    struct mulle_allocator *custom_allocator;
    
    // Create or obtain custom allocator
    custom_allocator = &my_custom_allocator;
    
    // Apply configuration using default bang with custom allocator
    _mulle_objc_universe_bang(universe, NULL, custom_allocator, NULL);
}
```

### Configuration with User Data

```c
// Configuration context structure
struct universe_config {
    int cache_size;
    int preload_methods;
};

// Custom configuration callback
void custom_bang(struct _mulle_objc_universe *universe,
                 struct mulle_allocator *allocator,
                 void *userinfo)
{
    struct universe_config *config = userinfo;
    
    // Use default setup first
    _mulle_objc_universe_defaultbang(universe, allocator, userinfo);
    
    // Apply custom configuration
    if (config) {
        // Custom cache sizing logic would go here
        printf("Configured universe with cache_size=%d\n", config->cache_size);
    }
}

// Register configured universe
struct _mulle_objc_universe *
register_configured_universe(const char *name, int cache_size)
{
    static struct universe_config config = { 1024, 1 };
    mulle_objc_universeid_t universeid;
    struct _mulle_objc_universe *universe;
    
    universeid = mulle_objc_universeid_from_string(name);
    universe = mulle_objc_global_register_universe(universeid, (char *)name);
    
    if (universe)
        _mulle_objc_universe_bang(universe, custom_bang, NULL, &config);
    
    return universe;
}
```

## Configuration Timing

### Early Initialization

You can configure the universe before any runtime usage:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    // Get and configure universe before any runtime usage
    universe = mulle_objc_global_register_universe(
        mulle_objc_universeid_from_string("MyApp"), "MyApp");
    
    if (universe) {
        _mulle_objc_universe_bang(universe, my_universe_bang, NULL, NULL);
    }
    
    // Now safe to use runtime
    return 0;
}
```

### Lazy Initialization

For most applications, you can let the runtime handle initialization lazily:

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    // Runtime will initialize on first access
    universe = mulle_objc_global_get_defaultuniverse();
    printf("Using default universe: %p\n", universe);
    
    return 0;
}
```

## Key APIs Summary

| Function | Purpose |
|----------|---------|
| `_mulle_objc_universe_bang()` | Initialize universe with custom configuration |
| `_mulle_objc_universe_defaultbang()` | Default universe initialization |
| `mulle_objc_global_register_universe()` | Create/register named universe |
| `__register_mulle_objc_universe()` | Runtime startup registration (from startup project) |