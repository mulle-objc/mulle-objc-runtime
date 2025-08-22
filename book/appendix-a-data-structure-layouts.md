# Appendix A: Data Structure Layouts

This appendix provides complete structure definitions for all runtime data structures. These definitions are exact copies from the source code headers and serve as the authoritative reference for understanding the runtime's internal data organization.

## A.1 Universe Structure

The universe is the global runtime state container. All runtime operations occur within the context of a universe.

### A.1.1 Universe Structure Definition

```c
struct _mulle_objc_universe
{
   //
   // A: these types are all designed to be concurrent, no locking needed
   //
   struct _mulle_objc_cachepivot            cachepivot;

   // try to keep this region stable for version checks >

   mulle_atomic_pointer_t                   version;
   mulle_atomic_pointer_t                   loadbits; // debugger can see if we have TAO or not
   char                                     *path;

   // try to keep this region stable for debugger and callbacks >

   struct mulle_concurrent_hashmap          classtable;  /// keep it here for debugger
   struct mulle_concurrent_hashmap          descriptortable;
   struct mulle_concurrent_hashmap          varyingtypedescriptortable;
   struct mulle_concurrent_hashmap          protocoltable;
   struct mulle_concurrent_hashmap          categorytable;
   struct mulle_concurrent_hashmap          supertable;
   struct mulle_concurrent_pointerarray     staticstrings;
   struct mulle_concurrent_pointerarray     hashnames;
   struct mulle_concurrent_pointerarray     gifts;  // external (!) allocations that we need to free

   struct _mulle_objc_taggedpointers        taggedpointers;
   struct _mulle_objc_fastclasstable        fastclasstable;

   struct _mulle_objc_universecallbacks     callbacks;

   // propertyid, indexed by methodid. If the there is clash (methodid
   // is used by two properties, its a problem -> we abort)
   struct mulle_concurrent_hashmap          propertyidtable;

   // unstable region, edit at will

   struct _mulle_objc_waitqueues            waitqueues;

   mulle_atomic_pointer_t                   retaincount_1;
   mulle_atomic_pointer_t                   cachecount_1; // #1#
   mulle_atomic_pointer_t                   classindex;
   mulle_atomic_pointer_t                   instancereuse; // (0:all, -1:none, other:future ideas...)
   mulle_thread_mutex_t                     lock;
   mulle_thread_tss_t                       threadkey;

   // these are 0 and NULL respectively for global and thread local
   mulle_objc_universeid_t                  universeid;
   char                                     *universename;
   uintptr_t                                compilebits;

   //
   // B: the rest is intended to be read only (setup at init time)
   //    if you think you need to change something, use the lock
   //
   char                                     compilation[ 128];   // debugging

   mulle_thread_t                           thread;  // init-done thread

   struct _mulle_objc_memorymanagement      memory;

   struct _mulle_objc_classdefaults         classdefaults;
   struct _mulle_objc_garbagecollection     garbage;
   // methodids_to_preload not methodid_stop_reload
   struct _mulle_objc_preloadmethodids      methodidstopreload;
   struct _mulle_objc_universefailures      failures;
   struct _mulle_objc_universeexceptionvectors   exceptionvectors;
   struct _mulle_objc_universeconfig        config;
   struct _mulle_objc_universedebug         debug;

   // these structs aren't all zeroes! they have some callbacks which are
   // initialized
   struct _mulle_objc_impcache              empty_impcache;
   struct _mulle_objc_impcache              initial_impcache;

   // It's all zeroes, so save some space with a union.
   // it would be "nicer" to have these in a const global
   // but due to windows, it's nicer to have as few globals
   // as possible
   //
   union
   {
      struct _mulle_objc_cache              empty_cache;
      struct _mulle_objc_ivarlist           empty_ivarlist;
      struct _mulle_objc_methodlist         empty_methodlist;
      struct _mulle_objc_propertylist       empty_propertylist;
      struct _mulle_objc_superlist          empty_superlist;
      struct _mulle_objc_uniqueidarray      empty_uniqueidarray;
   };

   //
   // this allows the foundation to come up during load without having to do
   // a malloc
   //
   struct _mulle_objc_universefriend     userinfo;    // for user programs
   intptr_t                              userinfospace[ S_MULLE_OBJC_UNIVERSE_USERINFO_SPACE / sizeof( intptr_t)];

   //
   // it must be assured that foundationspace always trails foundation
   //
   struct _mulle_objc_foundation         foundation;  // for foundation
   intptr_t                              foundationspace[ S_MULLE_OBJC_UNIVERSE_FOUNDATION_SPACE / sizeof( intptr_t)];
};
```

### A.1.2 Universe Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `cachepivot` | `struct _mulle_objc_cachepivot` | Cache pivot for method caching |
| `version` | `mulle_atomic_pointer_t` | Runtime version identifier |
| `loadbits` | `mulle_atomic_pointer_t` | Load compatibility flags |
| `path` | `char *` | Universe path for debugging |
| `classtable` | `struct mulle_concurrent_hashmap` | Class registry hash table |
| `descriptortable` | `struct mulle_concurrent_hashmap` | Method descriptor registry |
| `varyingtypedescriptortable` | `struct mulle_concurrent_hashmap` | Varying type descriptors |
| `protocoltable` | `struct mulle_concurrent_hashmap` | Protocol registry |
| `categorytable` | `struct mulle_concurrent_hashmap` | Category registry |
| `supertable` | `struct mulle_concurrent_hashmap` | Super class registry |
| `staticstrings` | `struct mulle_concurrent_pointerarray` | Static string storage |
| `hashnames` | `struct mulle_concurrent_pointerarray` | Hash names for debugging |
| `gifts` | `struct mulle_concurrent_pointerarray` | External allocations |
| `taggedpointers` | `struct _mulle_objc_taggedpointers` | Tagged pointer classes |
| `fastclasstable` | `struct _mulle_objc_fastclasstable` | Fast class lookup table |
| `callbacks` | `struct _mulle_objc_universecallbacks` | Universe callback functions |
| `propertyidtable` | `struct mulle_concurrent_hashmap` | Property ID mapping |
| `waitqueues` | `struct _mulle_objc_waitqueues` | Load wait queues |
| `retaincount_1` | `mulle_atomic_pointer_t` | Reference counter |
| `cachecount_1` | `mulle_atomic_pointer_t` | Cache change counter |
| `classindex` | `mulle_atomic_pointer_t` | Class index counter |
| `instancereuse` | `mulle_atomic_pointer_t` | Instance reuse policy |
| `lock` | `mulle_thread_mutex_t` | Global lock |
| `threadkey` | `mulle_thread_tss_t` | Thread-local storage key |
| `universeid` | `mulle_objc_universeid_t` | Unique universe identifier |
| `universename` | `char *` | Universe name for debugging |
| `compilebits` | `uintptr_t` | Compilation flags |
| `compilation` | `char[128]` | Compilation info string |
| `thread` | `mulle_thread_t` | Initialization thread |
| `memory` | `struct _mulle_objc_memorymanagement` | Memory management |
| `classdefaults` | `struct _mulle_objc_classdefaults` | Default class values |
| `garbage` | `struct _mulle_objc_garbagecollection` | Garbage collection |
| `methodidstopreload` | `struct _mulle_objc_preloadmethodids` | Preload method IDs |
| `failures` | `struct _mulle_objc_universefailures` | Failure handlers |
| `exceptionvectors` | `struct _mulle_objc_universeexceptionvectors` | Exception vectors |
| `config` | `struct _mulle_objc_universeconfig` | Configuration |
| `debug` | `struct _mulle_objc_universedebug` | Debug information |
| `empty_impcache` | `struct _mulle_objc_impcache` | Empty method cache template |
| `initial_impcache` | `struct _mulle_objc_impcache` | Initial method cache template |
| `empty_cache` | `struct _mulle_objc_cache` | Empty cache template (union) |
| `empty_ivarlist` | `struct _mulle_objc_ivarlist` | Empty ivar list (union) |
| `empty_methodlist` | `struct _mulle_objc_methodlist` | Empty method list (union) |
| `empty_propertylist` | `struct _mulle_objc_propertylist` | Empty property list (union) |
| `empty_superlist` | `struct _mulle_objc_superlist` | Empty super list (union) |
| `empty_uniqueidarray` | `struct _mulle_objc_uniqueidarray` | Empty unique ID array (union) |
| `userinfo` | `struct _mulle_objc_universefriend` | User extension data |
| `userinfospace` | `intptr_t[]` | User space storage |
| `foundation` | `struct _mulle_objc_foundation` | Foundation extension |
| `foundationspace` | `intptr_t[]` | Foundation space storage |

## A.2 Class Structure

Every class in the runtime uses the same structure for both infraclass and metaclass. The distinction is made by the `infraclass` field: NULL indicates an infraclass, non-NULL indicates a metaclass pointing to the instance class.

### A.2.1 Class Structure Definition

```c
struct _mulle_objc_class
{
   struct _mulle_objc_impcachepivot        cachepivot;  // DON'T MOVE

   /* ^^^ keep above like this, or change mulle_objc_fastmethodtable fault */

   // keep name, superclass, allocationsize in this order for gdb debugging
   struct _mulle_objc_class                *superclass;      // keep here for debugger (void **)[ 1]
   char                                    *name;            // offset (void **)[ 2]
   uintptr_t                               allocationsize;   // instancesize + header   (void **)[ 3]

   struct _mulle_objc_infraclass           *infraclass;
   struct _mulle_objc_universe             *universe;

   struct mulle_concurrent_pointerarray    methodlists;

   //
   // TODO: we could have a pointer to the load class and get the id
   //       information that way. (wouldn't save a lot though)
   //       or move to classpair
   //
   mulle_objc_classid_t                    classid;
   mulle_objc_classid_t                    superclassid;  // somewhat unused

   // TODO: general storage mechanism for KVC, needed in meta ? move to classpair ?
   uint16_t                                inheritance;
   uint16_t                                preloads;

   // vvv - from here on the debugger doesn't care

   uint16_t                                headerextrasize;  // memory before the header
   uint16_t                                extensionoffset;  // 4 later #1#

   struct _mulle_objc_method               *forwardmethod;
   void                                    (*invalidate_caches)( struct _mulle_objc_class *,
                                                                 struct _mulle_objc_methodlist *);

   struct _mulle_objc_kvccachepivot        kvc;
   mulle_atomic_pointer_t                  state;
   void                                    *userinfo;       // a void pointer for userinfo

#ifdef __MULLE_OBJC_FCS__
   struct _mulle_objc_fastmethodtable      vtab;            // dont' move it up, debugs nicer here
#endif
};
```

### A.2.2 Class Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `cachepivot` | `struct _mulle_objc_impcachepivot` | Method cache pivot for fast lookups |
| `superclass` | `struct _mulle_objc_class *` | Pointer to parent class |
| `name` | `char *` | Class name string |
| `allocationsize` | `uintptr_t` | Instance size including header |
| `infraclass` | `struct _mulle_objc_infraclass *` | NULL for infraclass, points to instance class for metaclass |
| `universe` | `struct _mulle_objc_universe *` | Runtime universe pointer |
| `methodlists` | `struct mulle_concurrent_pointerarray` | Method storage arrays |
| `classid` | `mulle_objc_classid_t` | Unique class identifier |
| `superclassid` | `mulle_objc_classid_t` | Parent class identifier |
| `inheritance` | `uint16_t` | Inheritance behavior flags |
| `preloads` | `uint16_t` | Method preload flags |
| `headerextrasize` | `uint16_t` | Memory before object header |
| `extensionoffset` | `uint16_t` | Extension data offset |
| `forwardmethod` | `struct _mulle_objc_method *` | Forwarding method for unrecognized selectors |
| `invalidate_caches` | `void (*)(struct _mulle_objc_class *, struct _mulle_objc_methodlist *)` | Cache invalidation callback |
| `kvc` | `struct _mulle_objc_kvccachepivot` | Key-Value Coding cache |
| `state` | `mulle_atomic_pointer_t` | Class state flags |
| `userinfo` | `void *` | User-defined data pointer |
| `vtab` | `struct _mulle_objc_fastmethodtable` | Fast method table (conditional compilation) |

## A.3 Method Structure

Methods combine metadata (descriptor) with implementation pointers.

### A.3.1 Method Structure Definition

```c
struct _mulle_objc_method
{
   struct _mulle_objc_descriptor   descriptor;
   union
   {
      mulle_objc_implementation_t      value;
      mulle_atomic_functionpointer_t   implementation;
   };
};
```

### A.3.2 Method Descriptor Structure

```c
struct _mulle_objc_descriptor
{
   mulle_objc_methodid_t   methodid;
   char                    *signature;
   char                    *name;
   uint32_t                bits;
};
```

### A.3.3 Method Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `descriptor` | `struct _mulle_objc_descriptor` | Method metadata |
| `value` | `mulle_objc_implementation_t` | Method implementation pointer |
| `implementation` | `mulle_atomic_functionpointer_t` | Atomic method implementation |

### A.3.4 Method Descriptor Fields

| Field | Type | Description |
|-------|------|-------------|
| `methodid` | `mulle_objc_methodid_t` | Unique method identifier |
| `signature` | `char *` | Method signature string |
| `name` | `char *` | Method name string |
| `bits` | `uint32_t` | Method flags and family information |

## A.4 Instance Variable Structure

Instance variables are described by a descriptor and offset.

### A.4.1 Ivar Structure Definition

```c
struct _mulle_objc_ivardescriptor
{
   mulle_objc_ivarid_t   ivarid;
   char                  *name;
   char                  *signature;
};

struct _mulle_objc_ivar
{
   struct _mulle_objc_ivardescriptor   descriptor;
   int                                 offset;  // if == -1 : hashed access (future)
};
```

### A.4.2 Ivar Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `descriptor` | `struct _mulle_objc_ivardescriptor` | Variable metadata |
| `offset` | `int` | Byte offset from object start, or -1 for hashed access |

### A.4.3 Ivar Descriptor Fields

| Field | Type | Description |
|-------|------|-------------|
| `ivarid` | `mulle_objc_ivarid_t` | Unique variable identifier |
| `name` | `char *` | Variable name string |
| `signature` | `char *` | Type signature string |

## A.5 Property Structure

Properties contain metadata and method identifiers for getter/setter methods.

### A.5.1 Property Structure Definition

```c
struct _mulle_objc_property
{
   mulle_objc_propertyid_t   propertyid;
   mulle_objc_propertyid_t   ivarid;
   char                      *name;
   char                      *signature;  // hmmm...
   mulle_objc_methodid_t     getter;
   mulle_objc_methodid_t     setter;
   mulle_objc_methodid_t     adder;
   mulle_objc_methodid_t     remover;
   uint32_t                  bits;        // for pointers/objects
};
```

### A.5.2 Property Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `propertyid` | `mulle_objc_propertyid_t` | Unique identifier for this property within the class. Used for runtime lookup and property introspection. Generated by the compiler from the property name. |
| `ivarid` | `mulle_objc_propertyid_t` | Identifier of the associated instance variable that stores the property's value. Can be 0 for @dynamic properties that don't use direct ivar storage. Links the property to its backing storage. |
| `name` | `char *` | The property's name as declared in the source code. Used for debugging, introspection, and KVC (Key-Value Coding) operations. |
| `signature` | `char *` | Type encoding string describing the property's type, including qualifiers like retain, copy, nonatomic. Follows Objective-C type encoding format (e.g., "@" for object, "i" for int). Used for runtime type checking and method signature generation. |
| `getter` | `mulle_objc_methodid_t` | Method identifier for the getter method that retrieves the property value. Generated from property name (e.g., "name" → "name" or "isEnabled" → "isEnabled"). |
| `setter` | `mulle_objc_methodid_t` | Method identifier for the setter method that sets the property value. Generated by capitalizing the first letter and prefixing with "set" (e.g., "name" → "setName:"). |
| `adder` | `mulle_objc_methodid_t` | Method identifier for adding objects to collection properties. Generated for properties with relationship attributes (e.g., "employees" → "addEmployeesObject:"). Only populated for container/relationship properties. |
| `remover` | `mulle_objc_methodid_t` | Method identifier for removing objects from collection properties. Generated for properties with relationship attributes (e.g., "employees" → "removeEmployeesObject:"). Only populated for container/relationship properties. |
| `bits` | `uint32_t` | Bitfield containing property attributes and flags. Encodes readonly, retain, copy, nonatomic, dynamic, and other property qualifiers. Used to determine memory management behavior, thread safety, and code generation requirements. |

## A.6 Protocol Structure

Protocols are minimal structures containing only an identifier and name.

### A.6.1 Protocol Structure Definition

```c
struct _mulle_objc_protocol
{
   mulle_objc_protocolid_t    protocolid;
   char                       *name;
};
```

### A.6.2 Protocol Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `protocolid` | `mulle_objc_protocolid_t` | Unique protocol identifier |
| `name` | `char *` | Protocol name string |

## A.7 Object Header Structure

Every object instance has a header that precedes the actual object data. This header contains the class pointer and runtime metadata.

### A.7.1 Object Header Structure Definition

```c
struct _mulle_objc_objectheader
{
#if MULLE_OBJC_TAO_OBJECT_HEADER
   void                       *_align;         // future: mulle-allocator ?
   mulle_atomic_pointer_t     _thread;
#endif
   mulle_atomic_pointer_t     _retaincount_1;  // negative means finalized
   struct _mulle_objc_class   *_isa;
};
```

### A.7.2 Object Header Fields

| Field | Type | Description |
|-------|------|-------------|
| `_align` | `void *` | Alignment padding for future allocator use (conditional) |
| `_thread` | `mulle_atomic_pointer_t` | Thread affinity for thread-affine objects (conditional) |
| `_retaincount_1` | `mulle_atomic_pointer_t` | Reference count, negative when object is finalized |
| `_isa` | `struct _mulle_objc_class *` | Class pointer for dynamic dispatch |

## A.8 Object Structure

The object structure represents the actual instance data visible to the programmer.

### A.8.1 Object Structure Definition

```c
struct _mulle_objc_object
{
   struct _mulle_objc_class *isa;
   // Instance variables follow immediately after
};
```

### A.8.2 Object Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `isa` | `struct _mulle_objc_class *` | Class pointer for dynamic dispatch |

## A.9 Tagged Pointer Layout

Tagged pointers store small objects directly in the pointer value. The exact bit layout varies by architecture and is handled by the runtime internally.

## A.10 Cache Structures

### A.10.1 Cache Structure Definition

```c
struct _mulle_objc_cache
{
   mulle_atomic_pointer_t          n;
   mulle_objc_cache_uint_t         size;  // don't optimize away (alignment!)
   mulle_objc_cache_uint_t         mask;
   struct _mulle_objc_cacheentry   entries[ 1];
};
```

### A.10.2 Cache Entry Structure Definition

```c
struct _mulle_objc_cacheentry
{
   union
   {
      mulle_objc_uniqueid_t            uniqueid;
      mulle_atomic_pointer_t           pointer;
   } key;
   union
   {
      mulle_atomic_functionpointer_t   functionpointer;
      mulle_atomic_pointer_t           pointer;
   } value;
};
```

### A.10.3 Cache Pivot Structure Definition

```c
struct _mulle_objc_cachepivot
{
   mulle_atomic_pointer_t   entries; // for atomic XCHG with pointer indirection
};
```

### A.10.4 Cache Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `n` | `mulle_atomic_pointer_t` | Number of entries in the cache |
| `size` | `mulle_objc_cache_uint_t` | Total capacity of the cache |
| `mask` | `mulle_objc_cache_uint_t` | Bitmask for cache indexing |
| `entries` | `struct _mulle_objc_cacheentry[1]` | Array of cache entries |

### A.10.5 Cache Entry Fields

| Field | Type | Description |
|-------|------|-------------|
| `key.uniqueid` | `mulle_objc_uniqueid_t` | Unique identifier (method ID or selector) |
| `key.pointer` | `mulle_atomic_pointer_t` | Generic pointer key |
| `value.functionpointer` | `mulle_atomic_functionpointer_t` | Method implementation pointer |
| `value.pointer` | `mulle_atomic_pointer_t` | Generic pointer value |

## A.11 List Structures

The runtime uses variable-length list structures to store collections of methods, ivars, properties, protocols, and super information.

### A.11.1 Ivar List Structure

```c
struct _mulle_objc_ivarlist
{
   unsigned int              n_ivars;  // must be #0 and same as struct _mulle_objc_methodlist
   struct _mulle_objc_ivar   ivars[ 1];
};
```

### A.11.2 Ivar List Fields

| Field | Type | Description |
|-------|------|-------------|
| `n_ivars` | `unsigned int` | Number of instance variables in this list. Must be the first field to maintain compatibility with other list types. |
| `ivars` | `struct _mulle_objc_ivar[1]` | Array of ivar structures. The actual size is determined by n_ivars at runtime. |

### A.11.3 Method List Structure

```c
struct _mulle_objc_methodlist
{
   unsigned int                     n_methods;  // must be #0 and same as struct _mulle_objc_ivarlist
   struct _mulle_objc_loadcategory  *loadcategory;
   struct _mulle_objc_method        methods[ 1];
};
```

### A.11.4 Method List Fields

| Field | Type | Description |
|-------|------|-------------|
| `n_methods` | `unsigned int` | Number of methods in this list. Must be the first field to maintain compatibility with other list types. |
| `loadcategory` | `struct _mulle_objc_loadcategory *` | Pointer to the category that loaded these methods. Used for category method injection. |
| `methods` | `struct _mulle_objc_method[1]` | Array of method structures. The actual size is determined by n_methods at runtime. |

### A.11.5 Property List Structure

```c
struct _mulle_objc_propertylist
{
   unsigned int                 n_properties;
   struct _mulle_objc_property  properties[ 1];
};
```

### A.11.6 Property List Fields

| Field | Type | Description |
|-------|------|-------------|
| `n_properties` | `unsigned int` | Number of properties in this list. |
| `properties` | `struct _mulle_objc_property[1]` | Array of property structures. The actual size is determined by n_properties at runtime. |

### A.11.7 Protocol List Structure

```c
struct _mulle_objc_protocollist
{
   unsigned int                   n_protocols;
   struct _mulle_objc_protocol    protocols[ 1];
};
```

### A.11.8 Protocol List Fields

| Field | Type | Description |
|-------|------|-------------|
| `n_protocols` | `unsigned int` | Number of protocols in this list. |
| `protocols` | `struct _mulle_objc_protocol[1]` | Array of protocol structures. The actual size is determined by n_protocols at runtime. |

### A.11.9 Super Structure

```c
struct _mulle_objc_super
{
   mulle_objc_superid_t    superid;
   char                    *name;
   mulle_objc_classid_t    classid;
   mulle_objc_methodid_t   methodid;
};
```

### A.11.10 Super Structure Fields

| Field | Type | Description |
|-------|------|-------------|
| `superid` | `mulle_objc_superid_t` | Unique identifier for this super declaration, generated from class and method names. |
| `name` | `char *` | The original selector string for debugging and introspection purposes. |
| `classid` | `mulle_objc_classid_t` | Identifier of the class that declared this super method override. |
| `methodid` | `mulle_objc_methodid_t` | Identifier of the method being overridden. |

### A.11.11 Super List Structure

```c
struct _mulle_objc_superlist
{
   unsigned int                n_supers;
   struct _mulle_objc_super    supers[ 1];
};
```

### A.11.12 Super List Fields

| Field | Type | Description |
|-------|------|-------------|
| `n_supers` | `unsigned int` | Number of super declarations in this list. |
| `supers` | `struct _mulle_objc_super[1]` | Array of super declarations. The actual size is determined by n_supers at runtime. |

## A.12 List Enumerator Structures

### A.12.1 Method List Enumerator

```c
struct _mulle_objc_methodlistenumerator
{
   struct _mulle_objc_method   *sentinel;
   struct _mulle_objc_method   *method;
};
```

### A.12.2 Property List Enumerator

```c
struct _mulle_objc_propertylistenumerator
{
   struct _mulle_objc_property   *sentinel;
   struct _mulle_objc_property   *property;
};
```

### A.12.3 Protocol List Enumerator

```c
struct _mulle_objc_protocollistenumerator
{
   struct _mulle_objc_protocol   *sentinel;
   struct _mulle_objc_protocol   *protocol;
};
```

## A.13 Structure Notes

- All structure definitions are taken directly from the source code headers
- Field types and sizes may vary by platform (32-bit vs 64-bit)
- Some structures contain conditional compilation blocks based on build configuration
- The runtime reserves the right to change structure layouts in future versions
- For authoritative definitions, always consult the actual header files