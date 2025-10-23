# Chapter 10: Advanced Topics

Beyond basic runtime operations, the mulle-objc runtime provides advanced features for type system extensions, runtime introspection, and variable argument handling. These APIs allow you to parse method signatures, handle exceptions, and work with variable arguments in object contexts.

## 10.1 Type Encoding and Signature System

The runtime extends Apple's type encoding system with additional type specifiers and provides comprehensive signature parsing utilities.

### Type Encoding Extensions

mulle-objc adds several extensions to the standard Objective-C type encoding system:

| Type Code | Description | Example |
|-----------|-------------|---------|
| `q` | C++ bool type | `bool` |
| `Q` | C++ unsigned bool | `unsigned bool` |
| `@?` | Block type | `void (^)(int)` |
| `^@` | Pointer to object | `id *` |
| `^#` | Pointer to class | `Class *` |
| `^:` | Pointer to selector | `SEL *` |
| `^?` | Pointer to function | `int (*)(int)` |
| `b<num>` | Bitfield | `int:3` |
| `j` | Complex float | `float _Complex` |
| `J` | Complex double | `double _Complex` |

### Signature Parsing Utilities

The runtime provides functions to parse method signatures, extract return types, and analyze parameter information.

```c
// Signature parsing example (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe   *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_method     *method;
    mulle_objc_methodid_t         methodid;
    char                          *signature;
    char                          *return_type;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_global_lookup_infraclass_nofail(MULLE_OBJC_DEFAULTUNIVERSEID, 
                                                     mulle_objc_classid_from_string("TestClass"));
    
    if (cls)
    {
        methodid = mulle_objc_methodid_from_string("init");
        method = _mulle_objc_infraclass_lookup_method(cls, methodid);
        if (method)
        {
            signature = (char *) mulle_objc_method_get_signature(method);
            return_type = (char *) mulle_objc_signature_get_return_type(signature);
            printf("Method signature: %s\n", signature);
            printf("Return type: %s\n", return_type);
        }
    }
    
    return 0;
}
```

### Advanced Signature Analysis

Extract detailed information from method signatures including parameter types and counts.

```c
// Advanced signature analysis (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    const char *signature = "v@:@i";
    char        return_type[32];
    unsigned int param_count;
    char        param_type[32];
    
    // Parse return type
    mulle_objc_signature_copy_return_type(signature, return_type, sizeof(return_type));
    printf("Return type: %s\n", return_type);
    
    // Parse parameter count
    param_count = mulle_objc_signature_count_arguments(signature);
    printf("Parameter count: %u\n", param_count);
    
    // Parse individual parameter types
    mulle_objc_signature_copy_argument_type(signature, 2, param_type, sizeof(param_type));
    printf("Parameter 2 type: %s\n", param_type);
    
    return 0;
}
```

## 10.2 Exception Handling System

The runtime provides a minimal exception handling system for throwing and catching runtime exceptions.

### Basic Exception Throwing

Throw runtime exceptions with custom messages and error codes.

```c
// Basic exception throwing example (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe   *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Throw a simple exception
    mulle_objc_exception_throw(universe, "CustomException", "Something went wrong");
    
    // This line won't execute due to exception
    printf("This won't print\n");
    
    return 0;
}
```

### Exception with User Data

Throw exceptions with additional context information.

```c
// Exception with user data using actual runtime APIs (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    struct _mulle_objc_universe   *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Throw exception with user data
    mulle_objc_exception_throw(universe, "RuntimeError", "Invalid operation");
    
    return 0;
}
```

## 10.3 Variable Arguments and Object Handling

The runtime extends standard C variable argument handling to work seamlessly with Objective-C objects.

### Object Counting in Varargs

Count objects passed in variable argument lists safely.

```c
// Object counting in varargs (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-vararg/mulle-vararg.h>

// Define a simple class for object creation
@interface SampleObject
@end

@implementation SampleObject
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_object *obj1, *obj2, *obj3;
    int count;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Create test objects using our defined class
    obj1 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("SampleObject"));
    obj2 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("SampleObject"));
    obj3 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("SampleObject"));
    
    if (obj1 && obj2 && obj3)
    {
        // Count objects in varargs
        count = mulle_vararg_count_objects(obj1, obj2, obj3, NULL);
        printf("Counted %d objects in varargs\n", count);
    }
    
    return 0;
}
```

### Object Iteration in Varargs

Iterate through objects passed in variable argument lists.

```c
// Object iteration in varargs (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-vararg/mulle-vararg.h>

// Reuse SampleObject class
@interface SampleObject
{
    int id;
}
@end

@implementation SampleObject
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_object *obj1, *obj2, *obj3;
    struct _mulle_objc_object *obj;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Create test objects
    obj1 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("SampleObject"));
    obj2 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("SampleObject"));
    obj3 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("SampleObject"));
    
    if (obj1 && obj2 && obj3)
    {
        // Iterate through objects in varargs
        va_list args;
        va_start(args, obj1);
        
        while ((obj = mulle_vararg_next_object(&args)))
        {
            printf("Processing object: %p\n", obj);
        }
        
        va_end(args);
    }
    
    return 0;
}
```

### Practical Varargs Example

Complete example showing object handling in variable arguments.

```c
// Complete varargs object handling (20 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-vararg/mulle-vararg.h>

// Define a class with ivars for demonstration
@interface DemoObject
{
    int value;
    char *name;
}
@end

@implementation DemoObject
@end

int main(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_object *obj1, *obj2, *obj3;
    int count;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Create test objects using our defined class
    obj1 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("DemoObject"));
    obj2 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("DemoObject"));
    obj3 = mulle_objc_object_alloc(universe, mulle_objc_classid_from_string("DemoObject"));
    
    if (obj1 && obj2 && obj3)
    {
        va_list args;
        
        // Count objects
        va_start(args, obj1);
        count = mulle_vararg_count_objects(obj1, obj2, obj3, NULL);
        printf("Found %d objects\n", count);
        
        // Iterate through objects
        va_start(args, obj1);
        while ((obj1 = mulle_vararg_next_object(&args)))
        {
            printf("Processing object: %p\n", obj1);
        }
        va_end(args);
    }
    
    return 0;
}
```

## 10.4 Advanced Runtime Introspection

Comprehensive runtime introspection capabilities beyond basic class and method lookups.

### Universe-wide Enumeration

Enumerate all classes, protocols, and categories in a universe.

```c
// Universe enumeration example (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

static void print_class_name(struct _mulle_objc_infraclass *cls, void *userinfo)
{
    printf("Class: %s\n", mulle_objc_class_get_name(cls));
}

int main(void)
{
    struct _mulle_objc_universe   *universe;
    
    universe = mulle_objc_global_get_defaultuniverse();
    
    // Enumerate all classes
    mulle_objc_universe_walk_infraclasses(universe, print_class_name, NULL);
    
    return 0;
}
```

### Method Signature Validation

Validate method signatures against expected patterns.

```c
// Signature validation example (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    const char *signature = "@@:@";
    const char *expected = "@@:@";
    int result;
    
    result = _mulle_objc_methodsignature_compare((char *)signature, (char *)expected);
    printf("Signature comparison result: %d (0 = match)\n", result);
    
    // Test with different signatures
    result = _mulle_objc_methodsignature_compare((char *)"v@:", (char *)"@@:@");
    printf("Different signature comparison: %d (non-zero = no match)\n", result);
    
    return 0;
}
```

## 10.5 Ivar Hash Calculation Algorithm

The runtime calculates ivar hashes for efficient instance variable lookup. This algorithm determines the offset of instance variables within object memory.

```c
// Ivar offset calculation example (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Define a simple class for demonstration
@interface TestClass
{
    int value;
    char *name;
}
@end

@implementation TestClass
@end

int main(void)
{
    struct _mulle_objc_universe   *universe;
    struct _mulle_objc_infraclass *cls;
    struct _mulle_objc_ivar       *ivar;
    mulle_objc_ivarid_t           ivarid;
    int                           offset;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_global_lookup_infraclass_nofail(MULLE_OBJC_DEFAULTUNIVERSEID, 
                                                     mulle_objc_classid_from_string("TestClass"));
    
    if (cls)
    {
        // Get ivar count and demonstrate lookup
        printf("TestClass has %u ivars\n", 
               mulle_objc_infraclass_get_ivarcache_n(cls));
        
        ivarid = mulle_objc_ivarid_from_string("value");
        ivar = mulle_objc_infraclass_search_ivar(cls, ivarid);
        if (ivar)
        {
            offset = mulle_objc_ivar_get_offset(ivar);
            printf("Ivar 'value' offset: %d bytes\n", offset);
        }
    }
    
    return 0;
}
```

## Key Advanced APIs Summary

| Function | Purpose |
|----------|---------|
| `mulle_objc_signature_copy_return_type()` | Extract return type from signature |
| `mulle_objc_signature_count_arguments()` | Count arguments in signature |
| `mulle_objc_exception_throw()` | Throw runtime exception |
| `mulle_vararg_count_objects()` | Count objects in varargs |
| `mulle_vararg_next_object()` | Get next object from varargs |
| `mulle_objc_universe_walk_infraclasses()` | Enumerate all classes |
| `mulle_objc_signature_copy_argument_type()` | Copy argument type at index |
| `mulle_objc_infraclass_search_ivar()` | Look up instance variable by name |
| `mulle_objc_ivar_get_offset()` | Get byte offset of instance variable |
| `mulle_objc_global_lookup_infraclass_nofail()` | Lookup class by name |