<!-- Keywords: TPS, FCS, TAO, loadinfo, metaabi, ivarhash, mixin -->
# Quirks

## Compile-time defines must match the runtime build

Code including `<mulle-objc-runtime/mulle-objc-runtime.h>` must be compiled
with the same TPS/FCS/TAO flags that the runtime library was built with:
`__MULLE_OBJC_TPS__` or `__MULLE_OBJC_NO_TPS__`, `__MULLE_OBJC_FCS__` or
`__MULLE_OBJC_NO_FCS__`, `__MULLE_OBJC_TAO__` or `__MULLE_OBJC_NO_TAO__`.

The umbrella header enforces this with `#error` at `src/mulle-objc-runtime.h:46-63`.
In `.c` files (no ObjC compiler), define them manually before including the
header. Use `.m` files with mulle-clang instead.

Source: `src/mulle-objc-runtime.h:46-63`, `README.md:60-68`

## MetaABI call header is NOT in the umbrella

`mulle-metaabi-call.h` requires C23 (`__VA_OPT__`) and is guarded by
`#if MULLE_C_HAS_VA_OPT`. Include it explicitly on demand — it is not part of
`mulle-objc-runtime.h`.

Source: `src/mulle-metaabi-call.h:40-41`, `src/mulle-objc-runtime.h:87`

## Loadinfo structs must reside in permanent memory

Structures passed to `mulle_objc_loadinfo_enqueue_nofail()` must be statically
allocated or otherwise kept alive until the universe is destructed. The runtime
does not copy them.

Source: `asset/dox/TOC.md` §3.8

## Universe `__register_mulle_objc_universe` callback

This is a user-defined global function. Define it exactly once per executable
using `MULLE_OBJC_DEFINE_REGISTER_UNIVERSE` in a single `.c` file. Other files
get an extern declaration. Windows needs special handling for this callback.

Source: `src/mulle-objc-universe.h:57-73`

## Mixins cannot be instantiated directly

Classes with `MULLE_OBJC_CLASS_IS_MIXIN` (v21+, replaces `IS_PROTOCOLCLASS`)
cannot be allocated — `alloc` calloc asserts against them. Test with
`_mulle_objc_infraclass_is_mixin()` before allocating.

Source: `asset/dox/TOC.md` §3.3, §5

## Metaclass `classpropertylock` is dormant until `+initializeSelf`

Before `+initializeSelf` is called, the recursive mutex depth is -1 (sentinel).
Accessing class properties before initialization is unsafe. Use
`_mulle_objc_infraclass_call_initialize_self()` to activate it.

Source: `asset/dox/TOC.md` §3.11, §5

## Class property ivar access uses `MULLE_OBJC_CLASSPAIR_IVAR_BASE`

Access class property ivars at runtime as:
```c
(char *)self + MULLE_OBJC_CLASSPAIR_IVAR_BASE + field_offset
```
The macro accounts for the classpair structure layout including header extras.

Source: `src/mulle-objc-classpair.h`, `asset/dox/TOC.md` §3.9

## Tagged pointers produce NULL/null behavior for some helpers

TPS (tagged pointer support) is default-on when `__MULLE_OBJC_TPS__` is
defined. Some object helpers return NULL or behave differently for TPS objects.
Use `mulle_objc_object_get_taggedpointerindex()` to test.

Source: `asset/dox/TOC.md` §5, `src/mulle-objc-object.h`

## Retain count encoding is not a plain integer

The `_retaincount_1` field has sentinel meanings:
- `0` .. `INTPTR_MAX-2`: normal count (actual = retaincount_1 + 1)
- `-1`: released
- `INTPTR_MIN`: retainCount 0, during finalizing
- `MULLE_OBJC_NEVER_RELEASE` (`INTPTR_MAX-1`): static objects, never released
- `MULLE_OBJC_SLOW_RELEASE` (`INTPTR_MAX`): forces `-release` method call

Only the header's inline functions (`_mulle_objc_object_retain_inline`) should
manipulate this field. `MULLE_OBJC_INLINE_RELEASE` (`INTPTR_MAX-2`) caps inline
RC — past this, retain counting stops (no crash, but potential leak).

Source: `src/mulle-objc-retain-release.h`

## Root class MUST implement `-finalize` and `-dealloc`

The runtime requires your root class to implement both `-finalize` and
`-dealloc` methods. The default retain/release path calls into these methods.

Source: `src/mulle-objc-retain-release.h:80`

## `mulle_metaabi_union` — do NOT name the variable `_param`

The header comment warns: "DO NOT CALL IT `_param` THOUGH (triggers a compiler
bug)." Use a different variable name for the MetaABI union.

Source: `src/mulle-metaabi.h:132-134`

## Ivar lists must be sorted by ivarid

When constructing `_mulle_objc_ivarlist` manually, the ivars must be sorted by
`ivarid` ascending. The runtime uses binary search on ivar lists.

Source: `test/demo/demo1.c:309` (comment: "must be sorted by ivarid !!!")

## Signature comparison ignores leading offsets

`_mulle_objc_methodsignature_compare()` uses `strstr(a, "@0:")` to skip the
offset prefix before comparing. Use `_mulle_objc_signature_compare_strict()` for
exact string comparison when offsets matter.

Source: `src/mulle-objc-signature.h:241-250`

## Method cache `sizeof` must be a power of two

The cache entry struct size is assumed to be a power of two — the cache
addressing math depends on it. Asserted at init time.

Source: `src/mulle-objc-cache.h:78-79,115`
