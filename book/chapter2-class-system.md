# Chapter 2: Class System

Classes are structs with metadata. A plain `struct Point { int x; int y; }`
becomes a class by adding runtime metadata: the struct gains knowledge of its
own name, its parent struct, and contains function pointers (methods) that
operate on instances. The runtime keeps a registry of all these structs so
you can look them up by unique class ID at runtime.

## 2.1 Class Access from Universe

The runtime maintains a global registry of all classes, similar to how you
might keep an array of `struct typeinfo` in a C program. This registry is
called the "universe". Here's how to access classes from it.

### Getting Classes by Name

```c
// Get any class by name (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *string_class;
    
    universe = mulle_objc_global_get_defaultuniverse();
    string_class = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string("NSString")
    );
    
    printf("NSString class: %p\n", string_class);
    return 0;
}
```

### Inspecting Class Size

Use runtime functions to determine instance size, similar to `sizeof(struct mystruct)`. This includes both the fixed overhead and the actual instance variables.

```c
// Get class size and layout info (15 lines)
void inspect_class_layout(const char *class_name)
{
    struct _mulle_objc_infraclass *infra;
    
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        mulle_objc_global_get_defaultuniverse(),
        mulle_objc_classid_from_string(class_name)
    );
    
    printf("Class: %s\n", mulle_objc_class_get_name(&infra->base));
    printf("Instance size: %zu bytes\n", mulle_objc_class_get_instancesize(&infra->base));
    printf("Ivar size: %zu bytes\n", _mulle_objc_class_get_instancesize(&infra->base));
}
```

### Walking the Class Registry

The runtime keeps all loaded classes in a global registry. This is like having
an array of all defined structs in your program. You can iterate through them
with a callback function, similar to how you might use `qsort()` or `bsearch()`.
This is useful for debugging or dynamic analysis.

```c
// List all loaded classes (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

static int print_class_info(void *cls, void *unused)
{
    printf("%s\n", mulle_objc_class_get_name(cls));
    return 0;
}

int main(void)
{
    struct _mulle_objc_universe *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    mulle_objc_universe_walk_classes(universe, print_class_info, NULL);
    return 0;
}
```

### Accessing Class Members

Classes provide access to their contents through several types of metadata:

- **Instance Variables**: Storage locations within object instances
- **Methods**: Function implementations callable on instances
- **Properties**: Synthesized getter/setter combinations
- **Protocols**: Interface contracts that classes conform to

These member types are covered in detail in their respective chapters. Chapter 2 focuses on class discovery and structure - for detailed member access examples and runtime introspection, refer to Chapters 3, 4, 5, and 6.

### Metaclass and Infraclass: The Classpair Structure

Every Objective-C class is actually a **classpair** - two structs created together that share the same name but serve different purposes:

```c
// The classpair structure (conceptual)
struct _mulle_objc_classpair {
    struct _mulle_objc_infraclass *infraclass;  // Instance methods + ivars
    struct _mulle_objc_metaclass *metaclass;    // Class methods
};
```

**Infraclass** (`struct _mulle_objc_infraclass`):
- Contains instance methods and instance variables
- Used when you create objects: `[[MyClass alloc] init]`
- Has the actual class name: "NSString", "NSArray", etc.
- Inherits from parent infraclass for method chaining

**Metaclass** (`struct _mulle_objc_metaclass`):  
- Contains class methods (static methods in C terms)
- Used when you call methods on the class itself: `[MyClass classMethod]`
- Has the same name as infraclass but different method table
- Inherits from parent metaclass for class method chaining

```c
// Understanding the classpair relationship (20 lines)
void inspect_classpair(const char *class_name)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_metaclass *meta;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, mulle_objc_classid_from_string(class_name)
    );
    
    meta = mulle_objc_class_get_metaclass(&infra->base);
    
    printf("Class: %s\n", mulle_objc_class_get_name(&infra->base));
    printf("  Infraclass: %p (instance methods)\n", infra);
    printf("  Metaclass: %p (class methods)\n", meta);
    printf("  Same name: %s\n", mulle_objc_class_get_name(&meta->base));
    
    // Both inherit separately
    printf("  Infraclass inherits from: %s\n", 
           mulle_objc_class_get_superclass(&infra->base) ? 
           mulle_objc_class_get_name(mulle_objc_class_get_superclass(&infra->base)) : "none");
    printf("  Metaclass inherits from: %s\n", 
           mulle_objc_class_get_superclass(&meta->base) ? 
           mulle_objc_class_get_name(mulle_objc_class_get_superclass(&meta->base)) : "none");
}
```

**Note**: For actual classpair creation, see `test/c/class/classpair.c` in the source tree for working examples using `mulle_objc_universe_new_classpair()`.

### Accessing the Superclass

Every class has a superclass (except root classes like `NSObject`). The superclass relationship forms the inheritance chain that enables method overriding and property inheritance.

```c
// Walking the inheritance chain (18 lines)
void inspect_inheritance(const char *class_name)
{
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_class *super;
    
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        mulle_objc_global_get_defaultuniverse(),
        mulle_objc_classid_from_string(class_name)
    );
    
    printf("Inheritance chain for %s:\n", mulle_objc_class_get_name(&infra->base));
    
    super = mulle_objc_class_get_superclass(&infra->base);
    while (super) {
        printf("  %s\n", mulle_objc_class_get_name(super));
        super = mulle_objc_class_get_superclass(super);
    }
    printf("  (root)\n");
}
```

The superclass relationship exists separately for both the infraclass and metaclass, creating parallel inheritance hierarchies. When you subclass, both your instance methods (infraclass) and class methods (metaclass) inherit appropriately.

## Key APIs Summary

The runtime provides both safe and exception-raising variants for lookups. Functions ending with `_nofail` will raise an exception if the item is not found, eliminating NULL checks.

| Function | Purpose |
|----------|---------|
| `mulle_objc_universe_lookup_infraclass()` | Get class by name (safe) |
| `mulle_objc_universe_lookup_infraclass_nofail()` | Get class by name (raises exception if not found) |
| `mulle_objc_class_get_name()` | Get class name |
| `_mulle_objc_class_get_instancesize()` | Get instance size |
| `_mulle_objc_infraclass_get_metaclass()` | Get metaclass |
| `mulle_objc_universe_new_classpair()` | Create new classpair |
| `mulle_objc_universe_walk_classes()` | Enumerate all classes |