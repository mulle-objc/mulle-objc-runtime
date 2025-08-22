# Chapter 8: Call Mechanism

Method calls are function pointer calls with extra steps. Instead of directly calling `my_struct->function(struct_ptr, arg1, arg2)`, the runtime looks up the function pointer by selector ID, finds it in a hash table, then calls it with the right calling convention. This chapter shows how this lookup and dispatch actually works.

## 8.1 Method Dispatch

When you write `[obj method:arg]`, the compiler turns this into a series of steps: convert the method name to a unique ID (selector), look up the function pointer (IMP) in the class's method table, then call it with the object as the first parameter.

### Selector Resolution

Selectors are unique integer IDs that map to method names. The runtime maintains a global string-to-ID mapping so that `@selector(methodName)` always returns the same ID for the same string.

```c
// Convert string to selector (ID) and back (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

int main(void)
{
    mulle_objc_methodid_t selector_id;
    const char *method_name;
    
    selector_id = mulle_objc_methodid_from_string("description");
    method_name = mulle_objc_universe_lookup_methodname(
        mulle_objc_global_get_defaultuniverse(), 
        selector_id);
    
    printf("Selector ID for 'description': %llu\n", (unsigned long long) selector_id);
    printf("Method name for ID: %s\n", method_name);
    return 0;
}
```

### Method Lookup

The runtime uses a hash table to map selectors to method implementations (IMPs). This is like having a dictionary where the key is the selector ID and the value is the function pointer.

```c
// Direct method lookup from class (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void *lookup_method_implementation(const char *class_name, const char *method_name)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_method *method;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, 
        mulle_objc_classid_from_string(class_name));
    
    method = mulle_objc_class_search_method_nofail(&infra->base,
        mulle_objc_methodid_from_string(method_name));
    
    return method ? method->function_pointer : NULL;
}
```

### IMP Extraction and Implementation Signatures

An **IMP (Implementation Pointer)** is just a regular C function pointer. Every method implementation is a C function with exactly these parameters: `id self`, `SEL _cmd`, plus any method parameters. See [Appendix A](appendix-a-data-structure-layouts.md) for complete struct layouts.

```c
// Valid method implementations are just C functions
static int add_int(id self, SEL _cmd, int value) { return 42; }
static void set_name(id self, SEL _cmd, char *name) { /* ... */ }
```

The calling convention is: `ReturnType (*)(id self, SEL _cmd, ...params...)` for instance methods and `ReturnType (*)(Class cls, SEL _cmd, ...params...)` for class methods.

### Meta-ABI: Struct-Based Calling Convention

The mulle-objc runtime uses a **[Meta-ABI](https://www.mulle-kybernetik.com/weblog/2015/mulle_objc_meta_call_convention.html)** - a flexible calling convention that handles parameter passing uniformly across platforms while optimizing for common cases.

**How it works:**
- **A single parameter** is passed as an individual argument when it fits in a register (pointers, integers)
- **Multiple parameters** are wrapped into a parameter struct passed as `void *`
- **Float/double** parameters always use the struct mechanism due to ABI compatibility
- **Return values** follow the same rules - simple types return directly, complex structs use struct return
- **Uniform dispatch** handles both direct and struct parameter cases transparently

### Basic Parameter Struct Example

```c
// Meta-ABI parameter struct examples (25 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Simple integer parameters
struct add_params {
    int a;
    int b;
};

// Complex struct with mixed types
struct complex_params {
    char *name;
    double value;
    int count;
    struct _mulle_objc_object *other;
};

// Return value struct
struct return_value {
    int status;
    char *message;
};

void demonstrate_meta_abi(void)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_infraclass *infra;
    id obj;
    
    universe = mulle_objc_global_get_defaultuniverse();
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        universe, mulle_objc_classid_from_string("TestClass"));
    obj = mulle_objc_class_new(&infra->base);
    
    // Simple parameter struct
    struct add_params add = {5, 7};
    int sum = (int)(intptr_t) mulle_objc_object_call(obj, 
        mulle_objc_methodid_from_string("add:to:"), &add);
    printf("Simple: %d + %d = %d\n", add.a, add.b, sum);
    
    // Complex parameter struct
    struct complex_params params = {"hello", 3.14, 42, obj};
    mulle_objc_object_call(obj, 
        mulle_objc_methodid_from_string("process:double:count:other:"), &params);
}
```

## 8.2 Call Variants

Different types of method calls use different dispatch mechanisms. Instance methods go through the instance's class, class methods go through the metaclass, and super calls skip the current class.

### Instance Method Dispatch

Regular instance method calls use the instance's class method table. This is the most common type of dispatch.

```c
// Basic instance method dispatch (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

id dispatch_instance_method(id obj, const char *method_name)
{
    struct _mulle_objc_class *cls;
    struct _mulle_objc_method *method;
    
    cls = mulle_objc_object_get_class(obj);
    method = mulle_objc_class_search_method_nofail(cls,
        mulle_objc_methodid_from_string(method_name));
    
    if (method)
        return ((id (*)(id, SEL))method->function_pointer)(obj, 
            mulle_objc_methodid_from_string(method_name));
    return nil;
}
```

### Class Method Dispatch

Class methods (static methods) are dispatched through the metaclass. The metaclass is just another class that contains class-level methods.

```c
// Class method dispatch through metaclass (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

id dispatch_class_method(const char *class_name, const char *method_name)
{
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_metaclass *meta;
    struct _mulle_objc_method *method;
    
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        mulle_objc_global_get_defaultuniverse(),
        mulle_objc_classid_from_string(class_name));
    
    meta = mulle_objc_class_get_metaclass(&infra->base);
    method = mulle_objc_class_search_method_nofail(&meta->base,
        mulle_objc_methodid_from_string(method_name));
    
    return method ? ((id (*)(id, SEL))method->function_pointer)(
        (id)meta, mulle_objc_methodid_from_string(method_name)) : nil;
}
```

### Super Calls

Super calls bypass the current class and start method lookup from the superclass using `mulle_objc_searcharguments_make_super`. This is how Objective-C implements inheritance.

```c
// Super method dispatch (18 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

id dispatch_super_method(id obj, const char *method_name)
{
    struct _mulle_objc_universe *universe;
    struct _mulle_objc_class *cls;
    struct _mulle_objc_method *method;
    struct _mulle_objc_searcharguments args;
    mulle_objc_implementation_t imp;
    
    universe = mulle_objc_global_get_defaultuniverse();
    cls = mulle_objc_object_get_class(obj);
    
    args = mulle_objc_searcharguments_make_super(
        mulle_objc_methodid_from_string(method_name),
        mulle_objc_class_get_classid(cls));
    
    method = mulle_objc_class_search_method(cls, &args, cls->inheritance, NULL);
    if (!method) return nil;
    
    imp = _mulle_objc_method_get_implementation(method);
    return mulle_objc_implementation_invoke(imp, obj, 
        mulle_objc_methodid_from_string(method_name), obj);
}
```

### Variable Arguments Handling

Variable arguments are handled through the same parameter struct mechanism. The compiler packs varargs into a struct that contains the fixed parameters plus a va_list or array for the variable portion.

```c
// Varargs method signature example
// Methods like: - (void)log:(NSString *)format, ...
// Are handled with a parameter struct containing the fixed args
```

## 8.3 Signature System

Method signatures describe the parameter types and return types so the runtime can properly call the function. This is essential for ABI compliance and type safety.

### Type Encoding

The runtime uses a compact string encoding for parameter types. This encoding allows the runtime to determine the correct calling convention and parameter layout.

```c
// Parse method signature (15 lines)
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void print_method_signature(const char *class_name, const char *method_name)
{
    struct _mulle_objc_infraclass *infra;
    struct _mulle_objc_method *method;
    struct mulle_objc_signatureenumerator enumerator;
    struct mulle_objc_typeinfo info;
    
    infra = mulle_objc_universe_lookup_infraclass_nofail(
        mulle_objc_global_get_defaultuniverse(),
        mulle_objc_classid_from_string(class_name));
    
    method = mulle_objc_class_search_method_nofail(&infra->base,
        mulle_objc_methodid_from_string(method_name));
    
    if (method)
    {
        printf("Signature: %s\n", method->descriptor.signature);
        
        enumerator = mulle_objc_signature_enumerate(method->descriptor.signature);
        while (_mulle_objc_signatureenumerator_next(&enumerator, &info))
        {
            printf("  Parameter: type='%s'\n", info.type);
        }
        mulle_objc_signatureenumerator_done(&enumerator);
    }
}
```

## 8.4 Method Forwarding

When method lookup fails (no method found for a selector), the runtime enters method forwarding. This is the "last resort" mechanism that allows objects to handle messages they don't explicitly implement.

### Forwarding Process

Method forwarding occurs in three stages:
1. **Resolution**: Attempt to dynamically add the method
2. **Fast forwarding**: Check if another object can handle it
3. **Normal forwarding**: Create NSInvocation and forward


### Forwarding Target

Objects can implement `-forwardingTargetForSelector:` to redirect the message to another object that can handle it.

```c
// Forwarding target mechanism (conceptual)
// When method lookup fails, check if object has forwarding target
// This happens automatically through method forwarding infrastructure
```

### Forwarding Invocation

If no forwarding target is found, the runtime creates a method invocation object (NSInvocation equivalent) and calls `-forwardInvocation:`.

```c
// Method forwarding structure
// 1. Runtime searches method table → not found
// 2. Runtime calls resolveInstanceMethod: → returns NO
// 3. Runtime checks forwardingTargetForSelector: → returns nil
// 4. Runtime creates NSInvocation and calls forwardInvocation:
// 5. If still unhandled, raises "unrecognized selector sent to instance"
```

### Forwarding vs. Inheritance

Forwarding is different from inheritance - inheritance searches superclasses, while forwarding lets objects handle messages they don't implement by delegating to other objects or providing dynamic implementations.

## Key APIs Summary

| Function | Purpose |
|----------|---------|
| `mulle_objc_methodid_from_string()` | Convert string to selector ID |
| `mulle_objc_class_search_method_nofail()` | Look up method by selector |
| `_mulle_objc_method_get_implementation()` | Get the actual IMP |
| `mulle_objc_object_get_class()` | Get class from object |
| `mulle_objc_class_get_metaclass()` | Get metaclass for class methods |
| `mulle_objc_class_get_superclass()` | Get superclass for super calls |
| `mulle_objc_object_call()` | Call method with parameter struct |
| `mulle_objc_object_call_super()` | Call superclass method |
| `mulle_objc_signature_enumerate()` | Parse method signatures |
| `_mulle_objc_signature_supply_typeinfo()` | Get type information |
| `_mulle_objc_infraclass_add_method()` | Add method dynamically |
| `_mulle_objc_method_create()` | Create new method for forwarding |