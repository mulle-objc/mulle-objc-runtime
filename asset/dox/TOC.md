# mulle-objc-runtime Library Documentation for AI
<!-- Keywords: objc, runtime, classes, methods, mixins, universe, messaging -->

## 1. Introduction & Purpose

- A lightweight, portable Objective-C runtime implemented in C11. Implements class/object metadata, method dispatch, property/ivar layout, signature parsing, and load-time installation of compiled ObjC class data.
- Solves: provide an Objective-C runtime for environments without Apple's runtime, with focus on speed (inline calls), multi-threading, and multiple coexisting "universes".
- Key features: fast inlineable messaging, per-class caches, tagged pointers (TPS), retain/release built-ins, binary load format (loadinfo), mixin support (v21+), class properties/variables on metaclass (v21+), MetaABI call macros.
- Relationship: foundational runtime used by MulleObjC; depends on mulle-core and mulle-core-all-load.

## 2. Key Concepts & Design Philosophy

- Universe: multiple independent runtimes can coexist; a universe encapsulates class tables, tagged pointers, allocators, static instances, and foundation config.
- Inline messaging: the runtime favors inlineable call paths and caches (fastmethod tables, imp caches) for speed.
- IDs not names: classes/selectors/protocols use unique IDs (hashes) rather than string lookups at runtime.
- Load-time install model: compiler emits binary loadinfo structures (classes, categories, mixins, strings, hashed strings) which are enqueued into a universe to install all metadata.
- Mixins: v21+ runtime replaces the old "protocol class" concept with mixins — classes that provide method/property implementations adopted by other classes.
- Metaclass ownership: class properties and class variables live on the metaclass (`_mulle_objc_metaclass`), protected by a per-metaclass recursive lock.
- Low-level C-first API: headers expose structs and functions for AIs to reason about runtime behavior without Objective-C sugar.

## 3. Core API & Data Structures

### 3.1. [mulle-objc-runtime.h]
- Purpose: umbrella header; includes core public headers and compile-time checks (TPS/FCS/TAO flags).
- Note: compile Objective-C code with the same compile-time flags used to build the runtime.
- `mulle-metaabi-call.h` is NOT included by default — it requires C23 (`__VA_OPT__`). Include on demand.

### 3.2. [mulle-objc-universe.h | mulle-objc-universe-global.h | mulle-objc-universe-struct.h]
- `struct _mulle_objc_universe`
  - Purpose: runtime instance container (class tables, tagged pointer config, allocators, debug/config, foundation config, static instances, gifts).
  - Key fields: class cache tables (`classtable`, `fastclasstable`), tagged pointer tables, allocators, `foundation` (static instance classes, rootclassid, allocator, headerextrasize, utf8staticstrings), `gifts` for external allocations, `waitqueues`.
  - Lifecycle functions:
    - `mulle_objc_global_get_universe_inline` / `mulle_objc_global_get_defaultuniverse`: get universe pointer.
    - `__mulle_objc_global_register_universe` / `__mulle_objc_global_unregister_universe`: register named universes.
    - `mulle_objc_global_reset_universetable`: clear registered universes (cleanup).
    - `mulle_objc_global_release_defaultuniverse`: tear down all objects and release default universe.
  - Core operations: get allocator, lookup/register classes/categories/protocols, iterate universes, add gifts (`_mulle_objc_universe_add_gift` for tracking externally-allocated objects).
  - Static instances (v21+):
    - `struct _mulle_objc_foundation` has `staticinstanceclass[ MULLE_OBJC_STATICINSTANCE_CLASS_SLOTS]` (8 slots) replacing the old single `staticstringclass`. Slot 0 is the primary string class; slots 1-2 may be UTF8 classes when `utf8staticstrings` is set.
    - `_mulle_objc_universe_add_staticinstance( universe, instance)` — add a static instance object.
    - `_mulle_objc_universe_set_staticinstanceclasses( universe, infra[], constantify)` — merge non-NULL entries into `staticinstanceclass[]` and re-patch queued instances.
    - `_mulle_objc_universe_didchange_staticinstanceclass( universe, constantify)` — re-runnable scan to patch queued instances.
    - Backward-compat wrappers: `_mulle_objc_universe_add_staticstring`, `_mulle_objc_universe_set_staticstringclass`, `_mulle_objc_universe_get_staticstringclass` delegate to the multi-slot API (slot 0).
  - Foundation:
    - `_mulle_objc_universe_get_foundationallocator()` — get the allocator for objects.
    - `_mulle_objc_universe_get_rootclassid()` — get root class id.
    - `_mulle_objc_universe_get_staticinstances()` — get the static instances pointerarray.

### 3.3. [mulle-objc-class.h | mulle-objc-class-struct.h]
- `struct _mulle_objc_class`
  - Purpose: represents a class (infraclass/metaclass) and its runtime metadata.
  - Key fields: `name`, `superclass`, `allocationsize`, methodlists pointerarray, `classid`, `inheritance` flags, kvc/cache pivots, `universe`, `infraclass`.
  - `MULLE_OBJC_CLASS_IS_MIXIN` (0x0100): marks a class as a mixin (replaces old `MULLE_OBJC_CLASS_IS_PROTOCOLCLASS`). Mixins cannot be instantiated directly.
  - State bits: `IS_BORING_ALLOCATION`, `IS_MIXIN`, `IS_NOT_THREAD_AFFINE`, `INITIALIZING`, `INITIALIZE_DONE`, `FINALIZE_DONE`.
  - `_mulle_objc_class_is_mixin( cls)` — test if class is a mixin.
  - `_mulle_objc_class_get_classtypename( cls)` — returns "infraclass", "metaclass", "inframixin", or "metamixin".
  - Lifecycle: `_mulle_objc_class_init`, `_mulle_objc_class_done`, `_mulle_objc_class_setup_pointerarrays`, `_mulle_objc_class_setup_initial_cache_if_needed(cls, callback)` (see 3.12).
  - Core operations:
    - `mulle_objc_class_add_methodlist`, `_mulle_objc_class_add_methodlist_nocache`: add methods/categories.
    - `_mulle_objc_class_lookup_method` / `mulle_objc_class_lookup_method`: method lookup (uses imp caches).
    - `_mulle_objc_class_lookup_superimplementation_nofail`: super method lookup.
    - `_mulle_objc_class_invalidate_impcache` / `invalidate_kvccache`: cache invalidation.
    - Accessors: `mulle_objc_class_get_name`, `get_instancesize`, `get_universe`, `is_infraclass`/`is_metaclass`, `get_metaextrasize`.

### 3.4. [mulle-objc-object.h | mulle-objc-objectheader.h]
- `struct _mulle_objc_objectheader`
  - Header prefixed to every instance, contains `_retaincount_1` (atomic) and `_isa` pointer.
  - Helpers: `_mulle_objc_objectheader_init`, get/set isa, get_retaincount.
- Object helpers:
  - `_mulle_objc_object_get_isa(obj)` — get isa, asserts for TAO on missing class.
  - `__mulle_objc_object_get_isa(obj)` — get isa without TAO assert; does not need a "real" class yet.
  - `mulle_objc_object_get_universe`, object extra pointer accessors, ivar read/write helpers.
  - Tagged pointers: TPS path exists; functions handle TPS index and fallback.

### 3.5. [mulle-objc-method.h | mulle-objc-methodlist.h]
- `struct _mulle_objc_descriptor` / `_mulle_objc_method`
  - Describes method id, name, signature, bits, and implementation.
  - Key fields: `methodid`, `signature`, `name`, `bits` (attributes + family + MetaABI types), `implementation` (atomic function pointer).
  - Implementation/alias union: `mulle_objc_implementation_t value` / `mulle_objc_methodid_t alias` / `mulle_atomic_functionpointer_t implementation`.
  - Method family: extracted from bits via `_mulle_objc_methodfamily_shift` (16). Families: init, dealloc, copy, etc.
   - MetaABI type bits (v21+): `_mulle_objc_method_metaabi_rtype_mask` (bits 22-23) for return type, `_mulle_objc_method_metaabi_ptype_mask` (bits 24-25) for parameter type. Values: 0=VoidPointer, 1=Void, 2=ParameterBlock. Extract with `_mulle_objc_method_bits_get_metaabi_rtype(bits)` / `_mulle_objc_method_bits_get_metaabi_ptype(bits)`, combined with `_mulle_objc_method_bits_get_metaabi_calltype(bits)`, set with `_mulle_objc_method_bits_set_metaabi_types(bits, rType, pType)`. `_mulle_objc_descriptor_get_metaabiparamtype(desc)` / `_mulle_objc_descriptor_get_metaabirvaltype(desc)` extract from descriptor.
  - Alias bits (v21+): `_mulle_objc_method_infra_alias_on_load` (0x100) and `_mulle_objc_method_meta_alias_on_load` (0x200) — the method uses its `alias` field to find the target at load time.
   - Operations: `_mulle_objc_method_get_implementation` / `set_implementation` / `_cas_implementation` (atomic replace), method family helpers, descriptor accessors for methodid/signature/name/bits.
  - Thread affinity: `_mulle_objc_descriptor_is_threadaffine(desc)`, `_mulle_objc_method_is_threadaffine(method)` — check if method requires thread-local processing.
- `struct _mulle_objc_methodlist`
  - Contiguous method array (`n_methods` + `methods[]`). Category id + origin.
  - Operations: sort, binary search vs linear search threshold (heuristic, n >= 14), enumeration, adding +load to callqueue.

### 3.6. [mulle-objc-property.h]
- `struct _mulle_objc_property`
  - Property descriptor with `propertyid`, `ivarid`, `name`, `signature`, getter/setter/adder/remover `methodid`s, and `bits`.
  - Property attribute enum `enum mulle_objc_property_attribute` defines signature characters: `R`=readonly, `C`=copy, `&`=retain, `G`=getter, `S`=setter, `V`=ivar, `D`=dynamic, `N`=nonatomic, `O`=observable, `K`=container/class, etc.
  - Key property bits: `_mulle_objc_property_readonly`, `_mulle_objc_property_retain`, `_mulle_objc_property_copy`, `_mulle_objc_property_dynamic`, `_mulle_objc_property_container`, `_mulle_objc_property_observable`, `_mulle_objc_property_setterclear`, `_mulle_objc_property_autoreleaseclear`, `_mulle_objc_property_fake`.
  - New v21+ bits: `_mulle_objc_property_forward` (0x100000) — forwarded property not synthesized by runtime.
  - Operations: `_mulle_objc_property_get_name`, `_get_signature`, `_get_propertyid`, `_get_ivarid`, `_get_getter/setter/adder/remover`, `_get_bits`, `_is_dynamic`, `_is_readonly`, `_is_observable`, `_is_container`, `_is_forward`, etc.
  - `_mulle_objc_property_is_clearable(property)` — returns true if writable and has setter-clear or autorelease-clear semantics.

### 3.7. [mulle-objc-ivar.h]
- `struct _mulle_objc_ivar`
  - Ivar descriptor for name/signature and offset.
  - API: `get_name`, `get_signature`, `get_offset`, bsearch and sort helpers.

### 3.8. [mulle-objc-load.h] — Binary Load Format (v21)

- `MULLE_OBJC_RUNTIME_LOAD_VERSION` = 21

- `struct _mulle_objc_loadclassbase` (v21+):
  - Common fields shared by `_mulle_objc_loadclass` and `_mulle_objc_loadmixin`.
  - Fields: `classid`, `classname`, `classmethods`, `instancemethods`, `classproperties` (→ metaclass propertylist), `properties`, `protocols`, `origin`.

- `struct _mulle_objc_loadclass`:
  - Contains `struct _mulle_objc_loadclassbase base` plus: `classivarhash`, `superclassid`, `superclassname`, `superclassivarhash`, `fastclassindex`, `instancesize`, `classinstancesize` (v21: size of class property ivars incl. inherited), `instancevariables`, `classvariables` (v21: → metaclass ivarlist), `mixinids` (v21: replaces `protocolclassids`).

- `struct _mulle_objc_loadmixin` (v21+):
  - Contains only `struct _mulle_objc_loadclassbase base`. Describes a mixin — provides methods/properties but no instance variables or superclass.

- `struct _mulle_objc_loadcategory`:
  - Fields: `categoryid`, `categoryname`, `classid`, `classname`, `classivarhash`, `classmethods`, `instancemethods`, `classproperties` (v21: → metaclass propertylist), `properties`, `protocols`, `mixinids` (v21: replaces `protocolclassids`), `origin`.

- `struct _mulle_objc_loadclasslist` / `_mulle_objc_loadmixinlist` (v21+) / `_mulle_objc_loadcategorylist`:
  - Variable-length arrays of pointers; use `mulle_objc_sizeof_loadclasslist(n)`, `mulle_objc_sizeof_loadmixinlist(n)`, `mulle_objc_sizeof_loadcategorylist(n)` for allocation.

- `struct _mulle_objc_loadstringlist` / `_mulle_objc_loadhashedstringlist`:
  - For static strings and hashed string maps.

- `struct _mulle_objc_loadinfo`:
  - Top-level load container: `version`, `loaduniverse`, `loadclasslist`, `loadmixinlist` (v21+), `loadcategorylist`, `loadsuperlist`, `loadstringlist`, `loadhashedstringlist`, `origin`.
  - `version.bits` flags include `_mulle_objc_loadinfo_utf8_strings` (0x20, v21+) and runtime feature flags.
  - `mulle_objc_loadinfo_enqueue_nofail(info)` — enqueue and install into runtime; structures must reside in permanent memory until universe destructed.
  - `mulle_objc_universe_assert_loadinfo(universe, info)` — compatibility check.
  - `mulle_objc_loadinfo_get_origin(info)` — get source file origin.

### 3.9. [mulle-objc-classpair.h]
- `struct _mulle_objc_classpair`:
  - Packs `infraclassheader` + `infraclass` + `metaclassheader` + `metaclass` with aligned padding, followed by shared fields.
  - Shared fields: `mixins` pointerarray (v21: renamed from `protocolclasses`), `p_protocolids`, `p_categoryids`, `lock` (for +initialize), `thread_id`, `loadclass` (type `struct _mulle_objc_loadclassbase *` in v21), `classindex`.
  - `MULLE_OBJC_CLASSPAIR_IVAR_BASE` macro (v21+): offset from `&pair->infraclass` to the start of class property ivar area appended after the classpair. Used as `(char *)self + MULLE_OBJC_CLASSPAIR_IVAR_BASE + field_offset` for class property access.
  - Lifecycle: `_mulle_objc_classpair_plusinit` / `_mulle_objc_classpair_plusdone`, `_mulle_objc_classpair_call_class_finalize`, `mulle_objc_classpair_free`.
  - Accessors: `_mulle_objc_classpair_get_infraclass`, `_mulle_objc_classpair_get_metaclass`, `_mulle_objc_classpair_get_universe`, `_mulle_objc_classpair_get_name`, `_mulle_objc_classpair_get_classid`, `_mulle_objc_classpair_get_loadclass`, `_mulle_objc_classpair_set_loadclass`, `_mulle_objc_classpair_get_origin`.
  - Reverse: `_mulle_objc_infraclass_get_classpair(infra)`, `_mulle_objc_metaclass_get_classpair(meta)`, `_mulle_objc_class_get_classpair(cls)`.
- **Mixins** (v21: replaces protocolclasses):
  - `_mulle_objc_classpair_has_mixin(pair, proto_cls)` — check presence.
  - `_mulle_objc_classpair_get_mixincount(pair)`.
  - `_mulle_objc_classpair_walk_mixins(pair, inheritance, callback, userinfo)`.
  - `_mulle_objc_classpair_add_mixin(pair, proto_cls)`.
  - `mulle_objc_classpair_add_mixinids_nofail(pair, protocolids)`.
- **Mixinenumerator** (v21: replaces protocolclassenumerator):
  - `struct _mulle_objc_mixinenumerator` with `list_rover` and `infra`.
  - `_mulle_objc_classpair_enumerate_mixins(pair)` / `mulle_objc_classpair_enumerate_mixins(pair)` — create enumerator.
  - `_mulle_objc_mixinenumerator_next(rover)` — get next mixin infraclass.
  - `_mulle_objc_mixinenumerator_done(rover)`.
  - Also: `_mulle_objc_mixinreverseenumerator` for reverse enumeration.
- **Protocols**: `_mulle_objc_classpair_walk_protocolids`, `__mulle_objc_classpair_conformsto_protocolid`, `mulle_objc_classpair_add_protocollist_nofail`.
- **Categories**: `_mulle_objc_classpair_has_categoryid`, `_mulle_objc_classpair_walk_categoryids`, `mulle_objc_classpair_add_categoryid_nofail`.
- **Debug**: `mulle_objc_classpair_walk()`.

### 3.10. [mulle-objc-infraclass.h]
- `struct _mulle_objc_infraclass` — inherits from `_mulle_objc_class`.
  - State bits: `MULLE_OBJC_INFRACLASS_IS_MIXIN` (v21: replaces `IS_PROTOCOLCLASS`).
- Key functions:
  - `mulle_objc_infraclass_is_mixin(infra)` / `mulle_objc_infraclass_check_mixin(infra)` (v21: renamed from protocolclass versions).
  - `mulle_objc_infraclass_lock_classproperty(infra)` / `unlock` / `trylock` (v21+) — lock the metaclass's classpropertylock for atomic class property access.
  - Accessors: `_mulle_objc_infraclass_get_universe`, `get_classid`, `get_name`, `get_superclass`, `get_metaclass`, `get_classindex`.
  - Initialize/deinitialize lifecycle functions are declared in `mulle-objc-class-initialize.h` (see 3.12).

### 3.11. [mulle-objc-metaclass.h]
- `struct _mulle_objc_metaclass` — inherits from `_mulle_objc_class`.
  - v21 additions: `classpropertylock` (`mulle_thread_recursive_mutex_t`, dormant until `initializeSelf` is called), `propertylists` (`mulle_concurrent_pointerarray`), `ivarlists` (`mulle_concurrent_pointerarray`).
- Class property operations (v21+):
  - `mulle_objc_metaclass_add_propertylist(meta, list)` / `_nofail` — add a class propertylist.
  - `mulle_objc_metaclass_search_property(meta, propertyid)` — search class properties.
- Class variable operations (v21+):
  - `mulle_objc_metaclass_add_ivarlist(meta, list)` / `_nofail` — add a class ivarlist.
  - `mulle_objc_metaclass_search_ivar(meta, ivarid)` — search class ivars.
- Class property lock (v21+):
  - `_mulle_objc_metaclass_lock_classproperty(meta)` / `_unlock` / `_trylock`.
- Standard metaclass operations:
  - `mulle_objc_metaclass_add_methodlist_nofail(meta, list)`.
  - `mulle_objc_metaclass_defaultsearch_method(meta, methodid)`.
  - `_mulle_objc_metaclass_lookup_superimplementation(meta, superid)`.
  - `mulle_objc_metaclass_walk(meta, type, callback, parent, userinfo)`.
  - `mulle_objc_metaclass_is_sane(meta)`.
  - Init/done: `_mulle_objc_metaclass_plusinit` / `_mulle_objc_metaclass_plusdone`.

### 3.12. [mulle-objc-class-initialize.h]
- `_mulle_objc_class_setup(cls)` — force initialize/setup a class (unlaze).
- `_mulle_objc_class_warn_recursive_initialize(cls)` — warn on recursive +initialize.
- `_mulle_objc_infraclass_call_deinitialize(infra)` — call +deinitialize.
- `_mulle_objc_infraclass_call_initialize_self(infra)` (v21+) — call +initializeSelf.
- `_mulle_objc_infraclass_call_deinitialize_self(infra)` (v21+) — call +deinitializeSelf.
- `_mulle_objc_class_setup_initial_cache_if_needed(cls, struct _mulle_objc_impcache_callback *callback)` — set up impcache if not already done; returns 0 if cache was already set up by someone else.

### 3.13. [mulle-objc-methodidconstants.h]
- Built-in method IDs and class IDs (hashed from selector/class name strings):
  - Alloc/init: `MULLE_OBJC_ALLOC_METHODID`, `MULLE_OBJC_INIT_METHODID`, `MULLE_OBJC_INSTANTIATE_METHODID`.
  - Retain/release: `MULLE_OBJC_RETAIN_METHODID`, `MULLE_OBJC_RELEASE_METHODID`, `MULLE_OBJC_AUTORELEASE_METHODID`, `MULLE_OBJC_RETAINCOUNT_METHODID`.
  - Lifecycle: `MULLE_OBJC_INITIALIZE_METHODID`, `MULLE_OBJC_LOAD_METHODID`, `MULLE_OBJC_UNLOAD_METHODID`, `MULLE_OBJC_FINALIZE_METHODID`, `MULLE_OBJC_WILLFINALIZE_METHODID`, `MULLE_OBJC_DEALLOC_METHODID`, `MULLE_OBJC_DEINITIALIZE_METHODID`.
  - v21 additions: `MULLE_OBJC_INITIALIZESELF_METHODID` ("initializeSelf"), `MULLE_OBJC_DEINITIALIZESELF_METHODID` ("deinitializeSelf").
  - Copy: `MULLE_OBJC_COPY_METHODID`, `MULLE_OBJC_MUTABLECOPY_METHODID`.
  - Other: `MULLE_OBJC_CLASS_METHODID`, `MULLE_OBJC_INSTANCE_METHODID`, `MULLE_OBJC_FORWARD_METHODID`, `MULLE_OBJC_DEPENDENCIES_METHODID`, `MULLE_OBJC_GENERIC_GETTER_METHODID`.
  - KVO: `MULLE_OBJC_WILLCHANGE_METHODID`, `MULLE_OBJC_WILLREADRELATIONSHIP_METHODID`.
  - Container: `MULLE_OBJC_ADDOBJECT_METHODID`, `MULLE_OBJC_REMOVEOBJECT_METHODID`.
  - Internal: `MULLE_OBJC_MULLE_ALLOCATOR_METHODID`, `MULLE_OBJC_MULLE_COUNT_OBJECT_METHODID`, `MULLE_OBJC_MULLEOBJCDEPS_CLASSID` (class ID for MulleObjCDeps dependency check).

### 3.14. [mulle-objc-signature.h]
- Signature/type parsing helpers:
  - `struct mulle_objc_typeinfo`: contains `type` (pointer into type string), `pure_type_end` (pointer past closed struct/union), `member_type_start` (pointer to first member type), `name`, `n_members` (0 for scalar, n for struct members/array/bitfield), `natural_size`, `bits_size`, `invocation_offset` (NSInvocation byte offset), `bits_struct_alignment`, `natural_alignment`, `has_object`, `has_retainable_type`.
  - `struct mulle_methodsignature_arginfo` (v21+): captures `invocation_offset`, `natural_size`, `type_offset` (offset into types string), `natural_alignment`, `has_retainable_type` per argument for frame layout.
- Core parsing:
  - `mulle_objc_signature_supply_typeinfo(type, supplier, info)` — parse one type.
  - `mulle_objc_signature_next_type(type)` — skip to next encoded type.
  - `mulle_objc_signature_supply_size_and_alignment(type, supplier, size, alignment)` — compute size/alignment for a struct type.
  - `mulle_objc_signature_count_typeinfos(types)` — count encoded type descriptors.
  - `mulle_objc_signature_fill_arginfos(types, infos, count)` (v21+) — fill array of `mulle_methodsignature_arginfo` structs.
- MetaABI helpers:
  - `mulle_objc_signature_get_metaabiparamtype(types)` — analyze full method signature to determine if _param is void, void*, or struct.
  - `mulle_objc_signature_get_metaabireturntype(type)` — inspect return type only (not full signature); returns `mulle_metaabi_param_error` (-1), `mulle_metaabi_param_void_pointer` (0), `mulle_metaabi_param_void` (1), or `mulle_metaabi_param_struct` (2).
  - `_mulle_objc_signature_sizeof_metaabistruct(type)` (v21: renamed from `_mulle_objc_signature_sizeof_metabistruct`).
- Value conversion helpers (v21+):
  - `_mulle_objc_typeinfo_demote_value_to_natural(p, dst, src)` / `_mulle_objc_typeinfo_promote_value_from_natural(p, dst, src)` — convert between C ABI and natural size using typeinfo (e.g. float↔double, char↔int, sel↔methodid).
  - `_mulle_methodsignature_arginfo_demote_value_to_natural(p, types, dst, src)` / `_mulle_methodsignature_arginfo_promote_value_from_natural(p, types, dst, src)` — same conversions using arginfo structs.
- Signature enumerator: `struct mulle_objc_signatureenumerator` (no leading underscore) with helpers `mulle_objc_signature_enumerate`, `_mulle_objc_signatureenumerator_next`, `_mulle_objc_signatureenumerator_rval`, `_mulle_objc_signatureenumerator_done` for iterating self, _cmd, args, and rval.

### 3.15. [mulle-objc-protocol.h]
- `struct _mulle_objc_protocol`: protocolid + name; sort/bsearch helpers; protocols live in universe tables.

### 3.16. [mulle-objc-retain-release.h]
- Built-in retain/release mechanisms:
  - Inline retain/release helpers: `_mulle_objc_object_retain_inline`, `_mulle_objc_object_release_inline`.
  - Special constants: `MULLE_OBJC_NEVER_RELEASE` etc.
  - Bulk retain/release of object arrays, finalize/dealloc helpers.

### 3.17. [mulle-metaabi.h | mulle-metaabi-call.h]
- `enum mulle_metaabi_param`: `mulle_metaabi_param_error` = -1, `mulle_metaabi_param_void_pointer` = 0, `mulle_metaabi_param_void` = 1, `mulle_metaabi_param_struct` = 2. (v21: values match MulleObjCMetaABIType.)
- `mulle_metaabi_call(p_rval, obj, sel, ...)` — macro dispatcher (v21: renamed from `mulle_metaabi_object_call`). Automatically selects the optimal calling convention based on return type and parameter count/types. Use `mulle_metaabi_void` as `p_rval` for void returns. Handles void-return, voidptr-return, and struct-return paths transparently via compile-time type analysis — no separate `_call_return_struct` macro needed.
- `mulle_metaabi_return(rval, _param)` — companion macro for implementing methods that return void*. Handles struct, voidptr, and integer return values in method bodies.
- `mulle_metaabi_param_struct(...)` — define a MetaABI parameter struct type for multi-arg calls.
- `mulle_metaabi_get_parameter_n(p_value, _param, ...)` — extract a named parameter from a _param struct.
- `mulle_metaabi_get_voidptr_parameter(p_value, _param)` — extract a single voidptr-compatible parameter.
- `MULLE_METAABI_STRUCT_FIELD(expr, s)` / `MULLE_METAABI_STRUCT_VALUE(expr, s)` — helper macros for defining struct fields / initializers within MetaABI parameter structs.
- `mulle-metaabi-call.h` requires C23 (`__VA_OPT__`), guarded by `#if MULLE_C_HAS_VA_OPT`. Include it explicitly when needed.

From `mulle-metaabi.h` (always available, no C23 required):
- `mulle_metaabi_is_int(expr)`, `mulle_metaabi_zero_non_int(expr, value)`, `mulle_metaabi_zero_fp(expr, value)` (from call.h, C23 required) — type-generic helpers for MetaABI argument/return packing.
- `mulle_metaabi_is_fp_expression(expr)` — `_Generic`-based float/double detection.
- `mulle_metaabi_is_voidptr_storage_compatible(type_or_expr)` — true if type fits in void* and alignment is compatible.
- `mulle_metaabi_is_voidptr_compatible_expression(expr)` — voidptr-storage-compatible AND not floating-point.
- `mulle_metaabi_is_struct_expression(expr)` (v21+) — uses `__builtin_classify_type` to detect structs for return value handling.
- `mulle_metaabi_is_voidptr_compatible_return_expression(expr)` (v21+) — voidptr-compatible AND not a struct (structs always use struct-return convention).

## 4. Performance Characteristics

- Method dispatch: optimized via per-class imp caches and optional fastmethod tables. Cache hit is effectively O(1). Cold lookup may walk methodlists: binary search O(log n) for large lists (n >= 14), linear scan O(n) for small.
- Retain/release: inline atomic increment/decrement for the common case (O(1)). Special states (SLOW_RELEASE, NEVER_RELEASE) trigger method calls.
- Loading: code loading uses global synchronization but normal operation avoids global locks; per-class +initialize uses per-class synchronization.
- Class property lock: per-metaclass recursive mutex, dormant (depth = -1 sentinel) until +initializeSelf activates it; O(1) once initialized.
- Memory vs speed: class structures are sizable (~1 KB/class on 64-bit) to favor runtime speed and cache locality.
- Thread-safety: designed for multi-threading; many structures use atomic pointers and concurrent maps. Loading and certain initialization paths still require locking.

## 5. AI Usage Recommendations & Patterns

- Best practices:
  - Use umbrella header `<mulle-objc-runtime/mulle-objc-runtime.h>` for high-level operations.
  - Always use supplied lifecycle functions (`_init`/`_done`, `enqueue_nofail`) instead of manually mutating structs.
  - Respect compile-time options (TPS/FCS/TAO) — mismatch leads to incompatible behavior.
  - Prefer inline API helpers (`mulle_objc_object_get_isa`, `mulle_objc_method_get_implementation`) for performance.
  - For class property ivar access at offset, use `(char *)self + MULLE_OBJC_CLASSPAIR_IVAR_BASE + field_offset`.
  - Use `mulle_metaabi_call()` macro for dynamic dispatch where the compiler doesn't know the signature at compile time.
  - Use `_mulle_objc_infraclass_call_initialize_self` / `_call_deinitialize_self` to invoke class-property lifecycle methods.
- Common pitfalls:
  - Do not directly modify struct internals in multi-threaded contexts; use provided functions.
  - Be aware of tagged pointers: some helpers return NULL or different behavior for TPS objects.
  - loadinfo structures passed to `enqueue_nofail` must reside in permanent memory until universe destructed.
  - Mixins (formerly "protocol classes") cannot be instantiated directly — `_mulle_objc_class_is_mixin()` returns true, and `alloc` calloc asserts against mixins.
  - The metaclass `classpropertylock` is dormant (depth = -1) until `+initializeSelf` is called — accessing class properties before that is unsafe.
  - `mulle-metaabi-call.h` is NOT in the umbrella; include it explicitly and ensure C23 (`__VA_OPT__`).
- Idioms:
  - Use `mulle_objc_loadinfo_enqueue_nofail` to install compiled class data.
  - Use `mulle_objc_universe_new_classpair(...)` to create classes dynamically.
  - Use signature parsing helpers to marshal parameters for manual call dispatch.
  - Enumerate mixins with `mulle_objc_classpair_enumerate_mixins` / `mulle_objc_mixinenumerator_next`.
  - For forward declarations, extract loadinfo origin with `mulle_objc_classpair_get_origin()`.

## 6. Integration Examples

### Example 1: Getting a class name from an instance

```c
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

### Example 3: Filling method signature arginfos (v21+)

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void
dump_arginfo( char *types)
{
   unsigned int                           n;
   struct mulle_methodsignature_arginfo   *infos;

   n     = mulle_objc_signature_count_typeinfos( types);
   infos = calloc( n, sizeof( struct mulle_methodsignature_arginfo));
   mulle_objc_signature_fill_arginfos( types, infos, n);

   for( unsigned int i = 0; i < n; i++)
      printf( "arg[%u]: offset=%u size=%u retainable=%u\n",
              i, infos[ i].invocation_offset,
              infos[ i].natural_size, infos[ i].has_retainable_type);

   free( infos);
}
```

### Example 4: Enumerating mixins (v21+)

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>

void
walk_mixins( struct _mulle_objc_classpair *pair)
{
   struct _mulle_objc_mixinenumerator   rover;
   struct _mulle_objc_infraclass        *infra;

   rover = mulle_objc_classpair_enumerate_mixins( pair);
   while( (infra = mulle_objc_mixinenumerator_next( &rover)))
      printf( "mixin: %s\n", mulle_objc_infraclass_get_name( infra));
   mulle_objc_mixinenumerator_done( &rover);
}
```

### Example 5: Using MetaABI call macros (v21+, requires C23)

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-objc-runtime/mulle-metaabi-call.h>

// void return, one int parameter
void   call_setFrame( void *obj, int x)
{
   mulle_metaabi_call( mulle_metaabi_void, obj,
                       MULLE_OBJC_METHODID( 0xfeedface),
                       x);
}

// int return, no parameters
int   call_getCount( void *obj)
{
   int   count;

   mulle_metaabi_call( &count, obj,
                       MULLE_OBJC_METHODID( 0xdeadbeef));
   return( count);
}
```

## 7. Dependencies

- mulle-core (mulle-core amalgamation: mulle-c11, mulle-allocator, mulle-concurrent, mulle-thread, mulle-vararg)
- mulle-core-all-load

## 8. Shortcut

- If an existing TOC.md is present, prefer to inspect its commit history. This file was last updated at commit e51c9a5 (v21 runtime changes). Corrections applied for accuracy: fixed `struct mulle_objc_typeinfo` fields (had invented `is_float`/`is_integer`/`is_void_ptr_compatible`), fixed signature enumerator name (drop leading `_`), removed non-existent `mulle_metaabi_call_return_struct` macro, moved initializeSelf/deinitializeSelf functions to correct header, fixed `_mulle_objc_class_setup_initial_cache_if_needed` signature, expanded methodID constants list, added missing MetaABI introspection macros, added classpair loadclass/origin accessors and method threadaffine checks.
