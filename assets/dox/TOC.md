# mulle-objc-runtime Library Documentation for AI

## 1. Introduction & Purpose

mulle-objc-runtime is the core runtime engine for mulle-objc, implementing dynamic dispatch, method lookup, object creation/destruction, and all fundamental Objective-C semantics. It is the foundation on which all mulle-objc code runs, providing the metaclass system, message dispatch, and object lifecycle management.

## 2. Key Concepts & Design Philosophy

- **Message Dispatch**: Fast lookup and invocation of methods via selectors
- **Dynamic Typing**: Runtime type system with metaclasses
- **Object Lifecycle**: Initialization, copying, destruction of objects
- **Selector Management**: Global selector registry and caching
- **Inline Cache**: Fast-path caching of method lookups for performance
- **Non-Thread-Safe by Default**: Requires external synchronization for concurrent access

## 3. Core API & Data Structures

### Primary Structures

- `struct mulle_objc_class`: Represents a class (contains methods, ivars, superclass)
- `struct mulle_objc_object`: Base object structure (isa pointer, retained count)
- `struct mulle_objc_method`: Method definition (selector, implementation, signature)
- `mulle_objc_methodid_t`: Selector/method identifier
- `mulle_objc_implementation_t`: Function pointer for method implementation

### Core Functions (Representative)

#### Object Manipulation

- `mulle_objc_retain(obj)` → `id`: Increments retain count
- `mulle_objc_release(obj)` → `id`: Decrements retain count
- `mulle_objc_autorelease(obj)` → `id`: Adds to autorelease pool
- `mulle_objc_dealloc(obj)` → `void`: Deallocates object

#### Method Dispatch

- `mulle_objc_call_class_method(...)`: Invoke class method
- `mulle_objc_call_instance_method(...)`: Invoke instance method
- `mulle_objc_send_message(obj, sel)`: Send message with selector

#### Class/Metadata

- `mulle_objc_class_for_classid(id)`: Lookup class by ID
- `mulle_objc_classid_for_name(name)`: Get class ID by name
- `mulle_objc_method_for_classid_and_methodid(...)`: Find method

## 4. Performance Characteristics

- **Message Dispatch**: O(1) average with inline cache; O(depth) without cache where depth is inheritance chain
- **Retain/Release**: O(1) atomic operation
- **Memory**: Fixed per-object overhead; efficient object layout
- **Inlining**: Many operations can be inlined for performance
- **Thread Safety**: Not thread-safe by default; external locking required

## 5. AI Usage Recommendations & Patterns

### Best Practices

- **Use High-Level APIs**: Prefer NSObject/Foundation APIs over runtime directly
- **Cache Selectors**: Store frequently-used selector IDs
- **Error Handling**: Check nil returns from method calls
- **Memory Management**: Always balance retain/release calls
- **Avoid Direct Struct Access**: Use accessor functions instead

### Common Pitfalls

- **Thread Safety**: No automatic synchronization; use locks for concurrent access
- **Selector ID Lookups**: Looking up selector IDs repeatedly has overhead
- **Exception Safety**: Manual memory management; ensure cleanup in error paths
- **Class Registration**: Classes must be properly registered before use
- **Method Signatures**: Mismatched signatures lead to undefined behavior

## 6. Integration Examples

### Example 1: Basic Message Sending

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

// Send "init" message to object
mulle_objc_method_id_t init_sel = mulle_objc_methodid_for_name("init");
id result = mulle_objc_send_message(obj, init_sel);
```

### Example 2: Memory Management

```c
id obj = ...;
mulle_objc_retain(obj);
// Use obj...
mulle_objc_release(obj);
```

### Example 3: Class Lookup

```c
mulle_objc_classid_t classid = mulle_objc_classid_for_name("MyClass");
struct mulle_objc_class *cls = mulle_objc_class_for_classid(classid);
```

## 7. Dependencies

- mulle-c11
- mulle-allocator
- mulle-concurrent (for thread-safe operations)
