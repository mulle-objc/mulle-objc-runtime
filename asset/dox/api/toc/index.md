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

### 3.18. [mulle-objc-call.h] — Method Call Infrastructure

- Purpose: core dynamic dispatch — generates calls from `obj + methodid + parameter` into the impleentation cache then invokes the resolved IMP.

- Non-inline call:
  - `void  *mulle_objc_object_call(void *obj, mulle_objc_methodid_t methodid, void *parameter);`
  - `void  *_mulle_objc_object_call(void *obj, mulle_objc_methodid_t methodid, void *parameter);`

- Inline call variants (increasing aggressiveness; compiler selects based on `-fobjc-inline-method-calls=` level):
  - `void  *mulle_objc_object_call_inline_minimal(void *obj, mulle_objc_methodid_t methodid, void *parameter);` — nil_check + dispatch.
  - `void  *mulle_objc_object_call_inline_partial(void *obj, mulle_objc_methodid_t methodid, void *parameter);` — inline FCS check + cache probe with callback fallback.
  - `void  *mulle_objc_object_call_inline(void *obj, mulle_objc_methodid_t methodid, void *parameter);` — inline cache lookup with collision callback.
  - `void  *mulle_objc_object_call_inline_full(void *obj, mulle_objc_methodid_t methodid, void *parameter);` — full inline loop through cache until hit/miss.
  - `void  *mulle_objc_object_call_inline_variable(void *obj, mulle_objc_methodid_t methodid, void *parameter);` — inline but skips FCS (best for variable selectors).
  - `void  *mulle_objc_object_call_inline_full_variable(void *obj, mulle_objc_methodid_t methodid, void *parameter);`

- Super call variants:
  - `void  *mulle_objc_object_call_super(void *obj, mulle_objc_methodid_t methodid, void *parameter, mulle_objc_superid_t superid);`
  - `void  *mulle_objc_object_call_super_inline(void *obj, mulle_objc_methodid_t methodid, void *parameter, mulle_objc_superid_t superid);`
  - `void  *mulle_objc_object_call_super_inline_full(void *obj, mulle_objc_methodid_t methodid, void *parameter, mulle_objc_superid_t superid);`
  - `void  *mulle_objc_object_call_super_inline_partial(void *obj, mulle_objc_methodid_t methodid, void *parameter, mulle_objc_superid_t superid);`

- Batch call:
  - `void  mulle_objc_objects_call(void **objects, unsigned int n, mulle_objc_methodid_t sel, void *params);`

- Implementation invoke:
  - `void  *mulle_objc_implementation_invoke(mulle_objc_implementation_t imp, void *self, mulle_objc_methodid_t sel, void *parameter);` (inline)
  - `void  *mulle_objc_implementation_invoke(mulle_objc_implementation_t imp, void *self, mulle_objc_methodid_t sel, void *parameter);` (with debug trace)

- Debug/trace:
  - `void  mulle_objc_implementation_trace(mulle_objc_implementation_t imp, void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls);`
  - `void  mulle_objc_object_taocheck_call(void *obj, mulle_objc_methodid_t methodid);`

### 3.19. [mulle-objc-builtin.h] — Compiler Builtins & Property Accessors

- Purpose: compiler-facing shortcuts for common operations (copy/retain/release/KVO/container), property accessor strategy flags, and compiler-synthesized property getter/setter logic.

- Standard call shortcuts:
  - `void  *mulle_objc_object_call_copy(void *self);`
  - `void  *mulle_objc_object_call_mutablecopy(void *self);`
  - `void  *mulle_objc_object_call_mulle_allocator(void *self);`
  - `void  *mulle_objc_object_call_autorelease(void *self);`
  - `void   mulle_objc_object_call_willchange(void *self);`
  - `void  *mulle_objc_object_call_willreadrelationship(void *self, void *value);`
  - `void   mulle_objc_object_call_addobject(void *self, void *value);`
  - `void   mulle_objc_object_call_removeobject(void *self, void *value);`

- Property accessor strategy enum:
  - `mulle_objc_property_accessor_autorelease` (0x0), `mulle_objc_property_accessor_noautorelease` (0x1), `mulle_objc_property_accessor_atomic` (0x2), `mulle_objc_property_accessor_copy` (0x4), `mulle_objc_property_accessor_mutable_copy` (0x8).

- Compiler-synthesized property accessors:
  - `void  mulle_objc_object_set_property_value(void *self, mulle_objc_methodid_t _cmd, ptrdiff_t offset, struct _mulle_objc_object *value, int strategy);`
  - `void  *mulle_objc_object_get_property_value(void *self, mulle_objc_methodid_t _cmd, ptrdiff_t offset, int strategy);`

- Container (to-many) helpers:
  - `void  mulle_objc_object_add_to_container(void *self, ptrdiff_t offset, void *value);`
  - `void  mulle_objc_object_remove_from_container(void *self, ptrdiff_t offset, void *value);`

- KVO helpers:
  - `void  mulle_objc_object_will_change(void *self);`
  - `void  mulle_objc_object_will_read_relationship(void *self, ptrdiff_t offset);`

### 3.20. [mulle-objc-cache.h | mulle-objc-impcache.h] — Cache Subsystem

- `struct _mulle_objc_cacheentry`: `key` / `value` union pair for pointer or function-pointer lookups. Fixed-size entries so `mask = (size-1) * sizeof(entry)` (pre-shifted for direct byte-offset AND).
- `struct _mulle_objc_cache`: `n` (atomic count), `size`, `mask`, `entries[1]` (variable-length). Uses open-addressing / linear-probe hash.
- `struct _mulle_objc_cachepivot`: `entries` (atomic pointer into cache.entries). Allows atomic cache replacement via CAS.
  - `_mulle_objc_cachepivot_get_entries_atomic(p)` / `_mulle_objc_cachepivot_get_cache_atomic(p)`.
  - `_mulle_objc_cachepivot_cas_entries(p, new, old)` — atomic compare-and-swap of entries pointer.
  - `_mulle_objc_cachepivot_swap(pivot, cache, old_cache, allocator)` — swap pivot to new cache, ABA-safe-free old.
- Cache operations:
  - `mulle_objc_cache_new(size, allocator)` / `_mulle_objc_cache_free` / `_mulle_objc_cache_abafree`.
  - `_mulle_objc_cache_probe_pointer(cache, uniqueid)` / `_mulle_objc_cache_probe_functionpointer`.
  - `_mulle_objc_cache_add_pointer_entry` / `_mulle_objc_cache_add_functionpointer_entry` (active), and `*_inactive` variants.
  - `_mulle_objc_cache_get_count/size/mask`.
  - `mulle_objc_cache_calculate_fillpercentage` / `mulle_objc_cache_calculate_hits` — stats.
  - `_mulle_objc_cache_grow_with_strategy(cache, strat, allocator)` — grow/shrink cache.
- `struct _mulle_objc_impcache_callback`: callback vector with `call`, `call_cache_collision`, `call_cache_miss`, `refresh_method_nofail`, `supercall`, `supercall_cache_collision`, `supercall_cache_miss`, `refresh_supermethod_nofail`, `userinfo`.
- `struct _mulle_objc_impcache`: embeds `callback` + `_mulle_objc_cache cache`.
- `struct _mulle_objc_impcachepivot`: wraps `_mulle_objc_cachepivot pivot`.
  - `mulle_objc_impcache_new(size, callback, allocator)` / `_mulle_objc_impcache_free`.
  - `_mulle_objc_impcachepivot_fill(cachepivot, imp, uniqueid, strategy, universe)` — populate entry, grow if needed.
  - `_mulle_objc_impcachepivot_convenient_swap(cachepivot, newcache, universe)` — atomic cache swap (preferred).
  - `_mulle_objc_impcachepivot_probe_inline` / `_mulle_objc_impcachepivot_lookup_inline_full` — inline cache probes without callback fallback.
- Constants: `MULLE_OBJC_MIN_CACHE_SIZE` (4), `MULLE_OBJC_DEFAULT_CACHE_SIZE` (8), `MULLE_OBJC_MAX_CACHE_SIZE` (4M).
- `enum mulle_objc_cachesizing_t`: `SHRINK` (-1), `STAGNATE` (0), `GROW` (2).

### 3.21. [mulle-objc-class-lookup.h] — Class Method Lookup (Cache-Aware)

- Purpose: cache-probe-first then search-and-fill-on-miss resolution. Primary interface for getting implementations from a class.

- Cache probe:
  - `struct _mulle_objc_cacheentry  *_mulle_objc_class_probe_cacheentry_inline(struct _mulle_objc_class *cls, mulle_objc_superid_t methodid);`
  - `mulle_objc_implementation_t  _mulle_objc_class_probe_implementation_inline(struct _mulle_objc_class *cls, mulle_objc_superid_t methodid);`
  - `mulle_objc_implementation_t  _mulle_objc_class_probe_implementation(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);` (non-inline)

- Lookup variants:
  - `mulle_objc_implementation_t  _mulle_objc_class_lookup_implementation(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);` — returns forward: if nothing found.
  - `mulle_objc_implementation_t  _mulle_objc_class_lookup_implementation_nofail(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);` — no fail if no forward.
  - `mulle_objc_implementation_t  _mulle_objc_class_lookup_implementation_nofill(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);` — probe only, no cache fill.
  - `mulle_objc_implementation_t  _mulle_objc_class_lookup_implementation_noforward(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);` — fill cache but no forward:.

- Refresh (search + update cache, bypass probe):
  - `mulle_objc_implementation_t  _mulle_objc_class_refresh_implementation_nofail(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);`
  - `struct _mulle_objc_method  *_mulle_objc_class_refresh_method_nofail(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);`
  - `struct _mulle_objc_method  *_mulle_objc_class_refresh_supermethod_nofail(struct _mulle_objc_class *cls, mulle_objc_superid_t superid);`

- Super lookup:
  - `mulle_objc_implementation_t  _mulle_objc_class_lookup_superimplementation_nofail(struct _mulle_objc_class *cls, mulle_objc_superid_t superid);`
  - `mulle_objc_implementation_t  _mulle_objc_class_lookup_superimplementation_inline_nofail(struct _mulle_objc_class *cls, mulle_objc_superid_t superid);`
  - `mulle_objc_implementation_t  _mulle_objc_object_lookup_superimplementation_nofail(void *obj, mulle_objc_superid_t superid);`

### 3.22. [mulle-objc-class-search.h] — Class Method Search (Uncached)

- Purpose: authoritative uncached method search through the class/methodlist hierarchy. Used for super calls, overridden method lookup, and forwarding setup.

- Search modes:
  - `MULLE_OBJC_SEARCH_DEFAULT` (0), `MULLE_OBJC_SEARCH_IMP` (1), `MULLE_OBJC_SEARCH_SUPER_METHOD` (2), `MULLE_OBJC_SEARCH_OVERRIDDEN_METHOD` (3), `MULLE_OBJC_SEARCH_SPECIFIC_METHOD` (4), `MULLE_OBJC_SEARCH_PREVIOUS_METHOD` (5).

- `struct _mulle_objc_searchargumentscachable`: `mode`, `methodid`, `classid`, `categoryid` — composable search key.
- `struct _mulle_objc_searcharguments`: extends cachable with `imp`, `previous_method`, `stop_classid`, `callback`, `userinfo`, `initialize`.
- `struct _mulle_objc_searchresult`: `class`, `list`, `method`, `error`.

- Factory functions:
  - `mulle_objc_searcharguments_make_default(methodid)`, `mulle_objc_searcharguments_make_super(methodid, classid)`, `mulle_objc_searcharguments_make_overridden(methodid, classid, category)`, `mulle_objc_searcharguments_make_specific(methodid, classid, category)`, `mulle_objc_searcharguments_make_imp(imp)`.

- Core search:
  - `struct _mulle_objc_method  *mulle_objc_class_search_method(struct _mulle_objc_class *cls, struct _mulle_objc_searcharguments *search, unsigned int inheritance, struct _mulle_objc_searchresult *result);`
  - `struct _mulle_objc_method  *mulle_objc_class_defaultsearch_method(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);`
  - `struct _mulle_objc_method  *mulle_objc_class_search_method_nofail(struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);` — falls back to forward:.

- Super search:
  - `struct _mulle_objc_method  *_mulle_objc_class_supersearch_method(struct _mulle_objc_class *cls, mulle_objc_superid_t superid);`
  - `struct _mulle_objc_method  *_mulle_objc_class_supersearch_method_nofail(struct _mulle_objc_class *cls, mulle_objc_superid_t superid);`

- Forwarding:
  - `mulle_objc_implementation_t  mulle_objc_class_get_forwardimplementation(struct _mulle_objc_class *cls);`
  - `struct _mulle_objc_method  *_mulle_objc_class_get_forwardmethod_lazy_nofail(struct _mulle_objc_class *cls, mulle_objc_methodid_t missing_method);`

- Fatal errors:
  - `MULLE_C_NO_RETURN void  _mulle_objc_class_fail_methodnotfound(struct _mulle_objc_class *cls, mulle_objc_methodid_t missing_method);`
  - `MULLE_C_NO_RETURN void  _mulle_objc_class_fail_forwardmethodnotfound(struct _mulle_objc_class *cls, mulle_objc_methodid_t missing_method, int error);`

- Clobber-chain enumeration (iterates each override of a method in inheritance order):
  - `struct mulle_objc_clobberchainenumerator` with `mulle_objc_class_clobberchain_enumerate(cls, methodid)`, `_mulle_objc_clobberchainenumerator_next(rover, &imp)`, `mulle_objc_clobberchainenumerator_done(rover)`.
  - Macro: `mulle_objc_class_clobberchain_for(cls, sel, item)`.

### 3.23. [mulle-objc-class-convenience.h | mulle-objc-object-convenience.h] — Instance Allocation & Object Convenience

- Purpose: object instance allocation/deallocation helpers and object-level ISA convenience wrappers.

- Instance creation (zeroed / non-zeroed):
  - `void  *mulle_objc_infraclass_alloc_instance(struct _mulle_objc_infraclass *infra);` — nil-safe, zeroed alloc.
  - `void  *mulle_objc_infraclass_alloc_instance_extra(struct _mulle_objc_infraclass *infra, size_t extra);`
  - `void  *_mulle_objc_infraclass_alloc_instance_extra_nonzeroed(struct _mulle_objc_infraclass *infra, size_t extra);`
  - `void  *_mulle_objc_infraclass_allocator_alloc_instance_extra(struct _mulle_objc_infraclass *infra, size_t extra, struct mulle_allocator *allocator);`

- Instance destruction:
  - `void  mulle_objc_instance_free(void *obj);` — nil-safe.
  - `void  _mulle_objc_instance_free(void *obj);` — non-nil.
  - `void  _mulle_objc_infraclass_allocator_free_instance(struct _mulle_objc_infraclass *infra, void *obj, struct mulle_allocator *allocator);`

- Low-level allocator wrappers (with reuse support):
  - `void  *_mulle_objc_infraclass_alloc_calloc(struct _mulle_objc_infraclass *infra, size_t size, struct mulle_allocator *allocator);`
  - `void  *_mulle_objc_infraclass_alloc_malloc(struct _mulle_objc_infraclass *infra, size_t size, struct mulle_allocator *allocator);`

- Object-level conveniences:
  - `char  *_mulle_objc_object_get_isa_name(void *obj);`
  - `mulle_objc_implementation_t  _mulle_objc_object_probe_implementation(void *obj, mulle_objc_methodid_t methodid);`
  - `mulle_objc_implementation_t  _mulle_objc_object_lookup_implementation(void *obj, mulle_objc_methodid_t methodid);` — full lookup with fill.
  - `mulle_objc_implementation_t  _mulle_objc_object_lookup_implementation_no_forward(void *obj, mulle_objc_methodid_t methodid);`
  - `struct _mulle_objc_method  *_mulle_objc_object_search_method_default(void *obj, mulle_objc_methodid_t methodid);`
  - `struct mulle_allocator  *_mulle_objc_instance_get_allocator(void *obj);`
  - `int  mulle_objc_object_conformsto_protocolid(void *obj, mulle_objc_protocolid_t protocolid);`

### 3.24. [mulle-objc-taggedpointer.h] — Tagged Pointers (TPS)

- Purpose: encode small values (integers, floats, doubles) directly into pointer space. The bottom 2 bits (32-bit) or 3 bits (64-bit) carry the tag index; remaining bits hold the value.

- Configuration:
  - `unsigned int  mulle_objc_get_taggedpointer_mask(void);` — 0x3 (32-bit) / 0x7 (64-bit).
  - `unsigned int  mulle_objc_get_taggedpointer_shift(void);` — 2 or 3.
  - `unsigned int  mulle_objc_taggedpointer_get_index(void *pointer);`

- NSUInteger:
  - `int  mulle_objc_taggedpointer_is_valid_unsigned_value(uintptr_t value);`
  - `void  *mulle_objc_create_unsigned_taggedpointer(uintptr_t value, unsigned int index);`
  - `uintptr_t  mulle_objc_taggedpointer_get_unsigned_value(void *pointer);`

- NSInteger:
  - `int  mulle_objc_taggedpointer_is_valid_signed_value(intptr_t value);`
  - `void  *mulle_objc_create_signed_taggedpointer(intptr_t value, unsigned int index);`
  - `intptr_t  mulle_objc_taggedpointer_get_signed_value(void *pointer);`

- double (64-bit only):
  - `void  *mulle_objc_create_double_taggedpointer(double d, unsigned int index);`
  - `double  mulle_objc_taggedpointer_get_double_value(void *pointer);`

- float:
  - `void  *mulle_objc_create_float_taggedpointer(float f, unsigned int index);`
  - `float  mulle_objc_taggedpointer_get_float_value(void *pointer);`

### 3.25. [mulle-objc-super.h] — Super Call Structures

- `struct _mulle_objc_super`: `superid` (hash of class+method+category), `name`, `classid`, `methodid`.
- `struct _mulle_objc_superlist`: `n_supers` + `supers[1]` variable array.
- Accessors:
  - `mulle_objc_superid_t  _mulle_objc_super_get_superid(struct _mulle_objc_super *p);`
  - `mulle_objc_classid_t  _mulle_objc_super_get_classid(struct _mulle_objc_super *p);`
  - `mulle_objc_methodid_t  _mulle_objc_super_get_methodid(struct _mulle_objc_super *p);`
  - `char  *_mulle_objc_super_get_name(struct _mulle_objc_super *p);`
- Hashing: `mulle_objc_superid_t  mulle_objc_superid_from_classid_and_categoryname(mulle_objc_classid_t classid, char *s);`
- Validation: `int  mulle_objc_super_is_sane(struct _mulle_objc_super *p);`

### 3.26. [mulle-objc-try-catch-finally.h] — Exception Handling

- Purpose: compiler-facing runtime for `@try`/`@catch`/`@finally`. Known as builtins by mulle-clang.

- Functions:
  - `void  mulle_objc_exception_throw(void *exception, mulle_objc_universeid_t universe);`
  - `void  mulle_objc_exception_tryenter(void *localExceptionData, mulle_objc_universeid_t universe);`
  - `void  mulle_objc_exception_tryexit(void *localExceptionData, mulle_objc_universeid_t universe);`
  - `void  *mulle_objc_exception_extract(void *localExceptionData, mulle_objc_universeid_t universe);`
  - `int  mulle_objc_exception_match(void *exception, mulle_objc_universeid_t universeid, mulle_objc_classid_t classid);` — short-circuits for NSException classid.
  - `int  _mulle_objc_exception_match(void *exception, mulle_objc_universeid_t universe, mulle_objc_classid_t classid);`

### 3.27. [mulle-objc-version.h] — Runtime Version

- `MULLE_OBJC_RUNTIME_VERSION` = `((0UL << 20) | (29 << 8) | 0)`
- `MULLE_OBJC_RUNTIME_VERSION_MAJOR` (0), `MULLE_OBJC_RUNTIME_VERSION_MINOR` (29), `MULLE_OBJC_RUNTIME_VERSION_PATCH` (0) — read by the compiler at compile time.
- `mulle_objc_version_get_major/minor/patch(uint32_t version)` — extract fields at runtime.

### 3.28. [mulle-objc-universe-class.h] — Universe Class Lookup

- Purpose: 4-stage pipeline to resolve `classid` → `struct _mulle_objc_infraclass *`: FastClass → classcache → classtable → user callback.

- Naming convention: `_probe_` = cache only, no fill; `_refresh_` = fill cache from classtable; `_lookup_` = probe + refresh; `_fill_` = fill directly; `_nofast` = skip fastclass table.

- Core lookup:
  - `struct _mulle_objc_infraclass  *_mulle_objc_universe_lookup_infraclass(struct _mulle_objc_universe *universe, mulle_objc_classid_t classid);`
  - `struct _mulle_objc_infraclass  *_mulle_objc_universe_lookup_infraclass_inline(struct _mulle_objc_universe *universe, mulle_objc_classid_t classid);`
  - `struct _mulle_objc_infraclass  *mulle_objc_universe_lookup_infraclass_nofail(struct _mulle_objc_universe *universe, mulle_objc_classid_t classid);`

- Refresh:
  - `struct _mulle_objc_cacheentry  *_mulle_objc_universe_fill_classcache(struct _mulle_objc_universe *universe, struct _mulle_objc_infraclass *infra);`
  - `struct _mulle_objc_cacheentry  *_mulle_objc_universe_refresh_classcache(struct _mulle_objc_universe *universe, mulle_objc_classid_t classid);`
  - `struct _mulle_objc_cacheentry  *_mulle_objc_universe_refresh_classcache_nofail(...);`

- Global conveniences:
  - `struct _mulle_objc_infraclass  *mulle_objc_global_lookup_infraclass_inline_nofail(mulle_objc_universeid_t universeid, mulle_objc_classid_t classid);`
  - `struct _mulle_objc_infraclass  *mulle_objc_global_lookup_infraclass_nofail(mulle_objc_universeid_t universeid, mulle_objc_classid_t classid);`

- Object conveniences:
  - `struct _mulle_objc_infraclass  *mulle_objc_object_lookup_infraclass_inline_nofail(void *obj, mulle_objc_universeid_t universeid, mulle_objc_classid_t classid);`

### 3.29. [mulle-objc-universe-fail.h] — Fatal Error Handling

- Purpose: runtime crash-printing and vectored failure handlers (can be overridden per universe). All functions are `MULLE_C_NO_RETURN`.

- Generic fails:
  - `void  mulle_objc_universe_fail_code(struct _mulle_objc_universe *universe, int errnocode);`
  - `void  mulle_objc_universe_fail_generic(struct _mulle_objc_universe *universe, char *format, ...);`
  - `void  mulle_objc_universe_fail_inconsistency(struct _mulle_objc_universe *universe, char *format, ...);`

- Vectored fails (can be intercepted):
  - `void  mulle_objc_universe_fail_classnotfound(struct _mulle_objc_universe *universe, mulle_objc_classid_t classid);`
  - `void  mulle_objc_universe_fail_methodnotfound(struct _mulle_objc_universe *universe, struct _mulle_objc_class *class, mulle_objc_methodid_t methodid);`
  - `void  mulle_objc_universe_fail_supernotfound(struct _mulle_objc_universe *universe, mulle_objc_superid_t superid);`
  - `void  mulle_objc_universe_fail_wrongthread(struct _mulle_objc_universe *universe, struct _mulle_objc_object *obj, mulle_thread_id_t affinity_thread, struct _mulle_objc_descriptor *desc);`

- Low-level abort (non-vectored, cannot be intercepted):
  - `void  _mulle_objc_printf_abort(char *format, ...);`
  - `void  _mulle_objc_universe_abort_classnotfound(...);` / `_abort_methodnotfound(...);` / `_abort_supernotfound(...);`

### 3.30. [mulle-objc-fastmethodtable.h | mulle-objc-fastclasstable.h] — Fast Class/Method Tables (FCS)

- Conditionally compiled under `__MULLE_OBJC_FCS__`. Provides O(1) vtable-based dispatch bypassing the cache.

- `struct _mulle_objc_fastmethodtable`: `union _mulle_objc_atomicmethodpointer_t methods[24]` — embedded in every class struct. Slots 0-5: `alloc` (0), `init` (1), `finalize` (2), `dealloc` (3), `instance` (4), `autorelease` (5). Slots 6-23 configurable by Foundation via `MULLE_OBJC_FASTMETHODHASH_n` macros.
  - `void  _mulle_objc_fastmethodtable_init(struct _mulle_objc_fastmethodtable *table);`
  - `int  mulle_objc_get_fastmethodtable_index(mulle_objc_methodid_t methodid);` — returns -1 if not a fast method.

- `struct _mulle_objc_fastclasstable`: `union _mulle_objc_atomicclasspointer_t classes[64]` — one per universe. Configurable slots via `MULLE_OBJC_FASTCLASSHASH_n` macros.
  - `struct _mulle_objc_infraclass  *mulle_objc_fastclasstable_get_infraclass(struct _mulle_objc_fastclasstable *table, unsigned int i);`
  - `struct _mulle_objc_infraclass  *mulle_objc_fastclasstable_get_infraclass_nofail(struct _mulle_objc_fastclasstable *table, unsigned int i);`
  - `int  mulle_objc_get_fastclasstable_index(mulle_objc_classid_t classid);` — returns -1 if not a fast class.

### 3.31. [mulle-objc-kvccache.h] — KVC Cache

- Purpose: caches resolved KVC accessor lookups (get/take/storedGet/storedTake) for string keys, used by Foundation's KVO/KVC implementation.

- `struct _mulle_objc_kvcinfo`: `implementation[4]`, `methodid[4]`, `offset`, `valueType[4]`, `cKey[1]` (flexible array).
  - `struct _mulle_objc_kvcinfo  *_mulle_objc_kvcinfo_new(char *cKey, struct mulle_allocator *allocator);`
- `struct _mulle_objc_kvccache`: wraps `_mulle_objc_cache base`.
- `struct _mulle_objc_kvccachepivot`: `entries` (atomic pointer).
  - `struct _mulle_objc_kvcinfo  *_mulle_objc_kvccache_lookup_kvcinfo(struct _mulle_objc_kvccache *cache, char *key);`
  - `int  _mulle_objc_kvccachepivot_invalidate(struct _mulle_objc_kvccachepivot *pivot, struct _mulle_objc_kvccache *empty_cache, struct mulle_allocator *allocator);`
  - `int  _mulle_objc_kvccachepivot_set_kvcinfo(struct _mulle_objc_kvccachepivot *pivot, struct _mulle_objc_kvcinfo *info, struct _mulle_objc_kvccache *empty_cache, struct mulle_allocator *allocator);`

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

- Last committed at 9cdf1f4. Updated to cover all public headers in the umbrella `<mulle-objc-runtime/mulle-objc-runtime.h>`. Added sections 3.18–3.31 covering: call infrastructure (call.h), compiler builtins (builtin.h), cache/impcache subsystem (cache.h, impcache.h), class method lookup (class-lookup.h), class method search (class-search.h), instance allocation/object convenience (class-convenience.h, object-convenience.h), tagged pointers (taggedpointer.h), super call structures (super.h), exception handling (try-catch-finally.h), runtime version (version.h), universe class lookup (universe-class.h), fatal error handling (universe-fail.h), fast class/method tables FCS (fastmethodtable.h, fastclasstable.h), and KVC cache (kvccache.h).
