# Chapter 4: Method System

## 4.1 Method Lookup

The method system is the heart of the Objective-C runtime. Methods are stored as function pointers with associated metadata, allowing dynamic dispatch based on selector IDs.

### Selector to Method Resolution

Selectors are unique integer identifiers derived from method names. The runtime maps these selector IDs to actual method implementations through method table lookups.

**Important:** Functions with `_nofail` suffix raise exceptions if the requested item is not found, eliminating the need for NULL checks.

```c
struct _mulle_objc_method *method;
mulle_objc_methodid_t    method_id;

method_id = mulle_objc_uniqueid_from_string("description");
method = mulle_objc_class_search_method_nofail(class, method_id);
// No NULL check needed - _nofail guarantees success or exception
mulle_printf("Found method: %s\n", _mulle_objc_method_get_name(method));
```

### Method Structure

Methods contain the implementation pointer (IMP), signature for type checking, and metadata for introspection.

```c
struct _mulle_objc_method {
    mulle_objc_methodid_t  methodid;    // Unique method identifier
    mulle_objc_implementation_t imp;    // Function pointer to implementation
    char                   *signature;  // Type encoding string
    uint32_t               bits;        // Method attributes and flags
};
```

### Instance vs Class Methods

Instance methods operate on object instances, while class methods operate on the class itself. The runtime distinguishes between these through separate method tables.

```c
// Instance method lookup
instance_method = mulle_objc_class_search_method_nofail(class, method_id);

// Class method lookup (via metaclass)
metaclass = _mulle_objc_infraclass_get_metaclass(class);
class_method = mulle_objc_class_search_method_nofail(metaclass, method_id);
```

## 4.2 Method Cache

The method cache prevents repeated method lookups by storing recently accessed methods in a hash table. This dramatically improves performance for frequently called methods.

### Cache Structure

The cache uses a simple hash table with linear probing for collision resolution. The cache is transparent to users - the runtime manages it automatically.

### Method Lookup with Caching

For most use cases, use the cached lookup functions:

```c
// Simple cached lookup (preferred)
method = mulle_objc_class_search_method_nofail(class, method_id);

// Direct method search (uncached)
method = mulle_objc_class_search_method_nofail(class, method_id);
```

## 4.3 Inheritance Traversal

Method lookup follows the inheritance chain when methods aren't found in the current class.

### Superclass Method Search

The runtime searches superclasses recursively until it finds the method or reaches the root class. The search functions handle inheritance automatically.

```c
// Search with inheritance (default behavior)
method = mulle_objc_class_search_method_nofail(class, method_id);

// Search a specific superclass (via superid)
method = _mulle_objc_class_supersearch_method_nofail(class, superid);
```

## 4.4 Method Enumeration

Classes can enumerate their methods for introspection and debugging purposes.

```c
typedef struct {
    int count;
} method_context_t;

static mulle_objc_walkcommand_t print_method_name(
    struct _mulle_objc_method *method,
    struct _mulle_objc_methodlist *list,
    struct _mulle_objc_class *cls,
    void *userinfo)
{
    method_context_t *context = (method_context_t *)userinfo;
    
    mulle_printf("  %s%s\n", 
           _mulle_objc_method_get_name(method),
           _mulle_objc_method_get_signature(method));
    context->count++;
    return mulle_objc_walk_ok;
}

// Enumerate all instance methods
method_context_t context = {0};
_mulle_objc_class_walk_methods((struct _mulle_objc_class *)class, 0, print_method_name, &context);
```

## 4.5 Method Attributes

Methods have attributes that control their behavior and visibility.

### Method Type Encoding

Method signatures use type encoding to describe parameter and return types.

```c
// Getting method signature
const char *signature = _mulle_objc_method_get_signature(method);

// Common type encodings:
// "i" - int
// "f" - float  
// "d" - double
// "@" - object (id)
// ":" - selector (SEL)
// "v" - void
// "*" - char *
```

### Method Flags

Methods have flags indicating their characteristics:

```c
// Check method flags
uint32_t bits = _mulle_objc_method_get_bits(method);
if (bits & MULLE_OBJC_METHOD_IS_CLASS_METHOD)
    printf("Class method\n");
if (bits & MULLE_OBJC_METHOD_IS_OPTIONAL)
    printf("Optional protocol method\n");
```

## 4.6 Dynamic Method Resolution

The runtime supports adding methods dynamically at runtime.

### Runtime Method Addition

```c
// Create a new method descriptor
struct _mulle_objc_methoddescriptor descriptor;
descriptor.methodid = mulle_objc_uniqueid_from_string("dynamicMethod");
descriptor.signature = "v@:";  // void return, id self, SEL _cmd

// Create method with implementation
struct _mulle_objc_method *method = _mulle_objc_method_create(
    &descriptor, 
    (mulle_objc_implementation_t)my_dynamic_implementation,
    0);

// Add method to class
_mulle_objc_infraclass_add_method(class, method);
```

## 4.7 Method Swizzling

Method swizzling allows replacing method implementations at runtime.

```c
// Swap method implementations
struct _mulle_objc_method *original_method;
mulle_objc_methodid_t method_id = mulle_objc_uniqueid_from_string("originalMethod");

original_method = _mulle_objc_infraclass_search_method(class, method_id);

if (original_method) {
    mulle_objc_implementation_t original_imp = _mulle_objc_method_get_implementation(original_method);
    _mulle_objc_method_set_implementation(original_method, replacement_imp);
}
```


## 4.5 Practical Example: Find All Methods

Here's a complete example for listing all methods of any class:

```c
// Complete example: list all methods of a class (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

static int print_method(void *method, void *cls, void *unused)
{
    struct _mulle_objc_universe *universe = ((struct _mulle_objc_class *)cls)->universe;
    printf("  %s\n", 
           mulle_objc_universe_lookup_methodname(universe, 
                   ((struct _mulle_objc_method *)method)->descriptor.methodid));
    return 0;
}

void list_class_methods(const char *class_name)
{
    struct _mulle_objc_infraclass *infra;
    
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        mulle_objc_global_get_defaultuniverse(),
        mulle_objc_classid_from_string(class_name)
    );
    
    printf("Methods in %s:\n", class_name);
    _mulle_objc_class_walk_methods(&infra->base, -1, print_method, NULL);
}
```
```