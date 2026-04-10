# mulle-objc-runtime Library Documentation for AI
<!-- Keywords: objc, runtime, classes, methods, universe, messaging -->

## 1. Introduction & Purpose

- A lightweight, portable Objective-C runtime implemented in C11. Implements class/object metadata, method dispatch, property/ivar layout, signature parsing, and load-time installation of compiled ObjC class data.
- Solves: provide an Objective-C runtime for environments without Apple's runtime, with focus on speed (inline calls), multi-threading, and multiple coexisting "universes".
- Key features: fast inlineable messaging, per-class caches, tagged pointers (TPS), retain/release built-ins, binary load format (loadinfo).
- Relationship: foundational runtime used by MulleObjC and depends on mulle-core and mulle-core-all-load.

## 2. Key Concepts & Design Philosophy

- Universe: multiple independent runtimes can coexist; a universe encapsulates class tables, tagged pointers, and allocators.
- Inline messaging: the runtime favors inlineable call paths and caches (fastmethod tables, imp caches) for speed.
- IDs not names: classes/selectors/protocols use unique IDs (hashes) rather than string lookups.
- Load-time install model: compiler emits binary loadinfo structures which are enqueued into a universe to install classes, categories, methods and strings.
- Low-level C-first API: headers expose structs and functions for AIs to reason about runtime behavior without Objective-C sugar.

## 3. Core API & Data Structures

### 3.1. [mulle-objc-runtime.h]
- Purpose: umbrella header; includes core public headers and compile-time checks (TPS/FCS/TAO flags).
- Note: compile Objective-C code with the same compile-time flags used to build the runtime.

### 3.2. [mulle-objc-universe.h | mulle-objc-universe-global.h]
- struct _mulle_objc_universe
  - Purpose: runtime instance container (class tables, tagged pointer config, allocators, debug/config).
  - Key fields: class cache tables, tagged pointer tables, allocator pointers, config flags.
  - Lifecycle functions:
    - mulle_objc_global_get_universe_inline / mulle_objc_global_get_defaultuniverse: get universe pointer.
    - __mulle_objc_global_register_universe / __mulle_objc_global_unregister_universe: register named universes.
    - mulle_objc_global_reset_universetable: clear registered universes (cleanup).
  - Core operations: get allocator, lookup/register classes and universes, iterate universes.

### 3.3. [mulle-objc-class.h | mulle-objc-class-struct.h]
- struct _mulle_objc_class
  - Purpose: represents a class (infraclass/metaclass) and its runtime metadata.
  - Key fields: name, superclass, allocationsize, methodlists pointerarray, classid, inheritance flags, kvc pivot, cache pivot.
  - Lifecycle functions:
    - _mulle_objc_class_init / _mulle_objc_class_done: initialize and cleanup class structures.
    - _mulle_objc_class_setup_pointerarrays: finalize pointer arrays after creation.
  - Core operations:
    - mulle_objc_class_add_methodlist, _mulle_objc_class_add_methodlist_nocache: add methods/categories.
    - _mulle_objc_class_lookup_method / mulle_objc_class_lookup_method: method lookup (uses imp caches).
    - _mulle_objc_class_invalidate_impcache / invalidate_kvccache: cache invalidation.
    - Accessors: mulle_objc_class_get_name, get_instancesize, get_universe, get_metaclass, is_infraclass/is_metaclass.
  - Inspection: count depth, get methodlists count, is_sane checks.

### 3.4. [mulle-objc-object.h | mulle-objc-objectheader.h]
- struct _mulle_objc_objectheader
  - Purpose: header prefixed to every instance, contains retain count and isa.
  - Key fields: _retaincount_1 (atomic), _isa pointer.
  - Helpers: _mulle_objc_objectheader_init, get/set isa, get_retaincount.
- Object helpers:
  - _mulle_objc_object_get_isa, mulle_objc_object_get_universe, object extra pointer accessors, ivar read/write helpers.
  - Tagged pointers: tagged pointer path exists (TPS); functions handle TPS index and fallback.

### 3.5. [mulle-objc-method.h | mulle-objc-methodlist.h]
- struct _mulle_objc_descriptor / _mulle_objc_method
  - Purpose: describes method id, name, signature and implementation function pointer.
  - Key fields: methodid, signature, name, bits (attributes), implementation (atomic function pointer).
  - Operations: get/set implementation, cas_implementation (atomic replace), bsearch/sort utilities, method family helpers.
- struct _mulle_objc_methodlist
  - Purpose: contiguous method array (n_methods + methods[]). Category id + origin.
  - Operations: sort, binary search vs linear search threshold (heuristic), enumeration, adding +load to callqueue.

### 3.6. [mulle-objc-property.h]
- struct _mulle_objc_property
  - Purpose: property descriptor with getter/setter/adder/remover methodids, ivarid and bits.
  - Operations: accessors for name, signature, getter/setter ids, bit tests (readonly/dynamic/observable/etc.).

### 3.7. [mulle-objc-ivar.h]
- struct _mulle_objc_ivar
  - Purpose: ivar descriptor for name/signature and offset.
  - API: get_name, get_signature, get_offset, bsearch and sort helpers.

### 3.8. [mulle-objc-load.h]
- load data structures: _mulle_objc_loadinfo, _mulle_objc_loadclass, _mulle_objc_loadcategory
  - Purpose: describe binary emitted class/category/method/property data to be installed into a universe.
  - Core operations: mulle_objc_loadinfo_enqueue_nofail(info) — enqueue and install into runtime; mulle_objc_universe_assert_loadinfo for compatibility checks.
  - Versioning: MULLE_OBJC_RUNTIME_LOAD_VERSION and load version bits in structures.

### 3.9. [mulle-objc-signature.h]
- Signature/type parsing helpers
  - API: mulle_objc_signature_supply_typeinfo, mulle_objc_signature_next_type, supply_size_and_alignment, signature enumerator helpers.
  - Purpose: parse ObjC encoded signatures into size/alignment/typeinfo for marshalling and call ABI classification.

### 3.10. [mulle-objc-retain-release.h]
- Built-in retain/release mechanisms
  - Inline retain/release helpers (_mulle_objc_object_retain_inline, _mulle_objc_object_release_inline) for performance and special constants (MULLE_OBJC_NEVER_RELEASE etc.).
  - API for bulk retain/release of object arrays, finalize/dealloc helpers.

### 3.11. [mulle-objc-protocol.h]
- struct _mulle_objc_protocol: protocolid + name; sort/bsearch helpers; protocols live in universe tables.

## 4. Performance Characteristics

- Method dispatch: optimized via per-class imp caches and optional fastmethod tables. Cache hit is effectively O(1). Cold lookup may walk methodlists: cost depends on methodlist size (binary search O(log n) for large lists, linear for small).
- Methodlist search: binary search used for n_methods >= 14 (heuristic), otherwise linear scan.
- Retain/release: inline atomic increment/decrement for the common case (O(1)). Special states (SLOW_RELEASE, NEVER_RELEASE) trigger method calls.
- Loading: code loading uses global synchronization but normal operation avoids global locks; per-class +initialize uses per-class synchronization.
- Memory vs speed: class structures are sizable (~1 KB/class on 64-bit) to favor runtime speed and cache locality.
- Thread-safety: designed for multi-threading; many structures use atomic pointers and concurrent maps. Loading and certain initialization paths still require locking.

## 5. AI Usage Recommendations & Patterns

- Best practices:
  - Use umbrella header <mulle-objc-runtime/mulle-objc-runtime.h> for high-level operations.
  - Always use supplied lifecycle functions (_init/_done, enqueue loadinfo) instead of manually mutating structs.
  - Respect compile-time options (TPS/FCS/TAO) — mismatch leads to incompatible behavior.
  - Prefer inline API helpers (mulle_objc_object_get_isa, mulle_objc_method_get_implementation) for performance.
- Common pitfalls:
  - Do not directly modify struct internals in multi-threaded contexts; use provided functions.
  - Be aware of tagged pointers: some helpers return NULL or different behavior for TPS objects.
  - loadinfo structures passed to enqueue must reside in permanent memory until universe destructed.
- Idioms:
  - Use mulle_objc_loadinfo_enqueue_nofail to install compiled class data.
  - Use signature parsing helpers to marshal parameters for manual call dispatch.

## 6. Integration Examples

### Example 1: Getting a class name from an instance

```c
// 3-space indent, C89 rules
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void
print_class_name( void *obj)
{
   struct _mulle_objc_class  *cls;

   cls = mulle_objc_object_get_isa( obj);
   if( cls)
      printf( "class: %s\n", mulle_objc_class_get_name( cls));
}
```

### Example 2: Parsing a method signature

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void
dump_signature( char *sig)
{
   struct mulle_objc_typeinfo  info;

   while( mulle_objc_signature_supply_typeinfo( sig, NULL, &info))
   {
      printf( "type=%s size=%zu\n", info.type, info.natural_size);
      sig = mulle_objc_signature_next_type( sig);
      if( ! sig || ! *sig)
         break;
   }
}
```

## 7. Dependencies

- mulle-core (mulle-core amalgamation)
- mulle-core-all-load
- mulle-allocator (via mulle-core)
- mulle-sde for build/install integration

## 8. Shortcut

- If an existing TOC.md is present, prefer to inspect its commit history. This file was generated from README.md and the public headers under src/ to produce an AI-friendly concise API map.
