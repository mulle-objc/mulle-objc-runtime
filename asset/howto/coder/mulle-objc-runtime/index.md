<!-- Keywords: runtime, messaging, universe, loadinfo, MetaABI, signature, retain -->
# mulle-objc-runtime

Use this topic when writing C code that directly uses the mulle-objc runtime
APIs — messaging, class/universe lifecycle, method signature parsing, or the
MetaABI calling convention — without the MulleObjC Foundation layer.

## Understand first

```bash
mulle-sde api apropos mulle-objc-runtime
mulle-sde api cat mulle-objc-runtime
mulle-sde howto show --keyword styleguide --keyword c
```

## Local references

| Source | Path |
|---|---|
| Umbrella header | `src/mulle-objc-runtime.h` |
| MetaABI call (C23) | `src/mulle-metaabi-call.h` |
| Signature parsing | `src/mulle-objc-signature.h` |
| Cache internals | `src/mulle-objc-cache.h`, `src/mulle-objc-call.h` |
| Class lifecycle | `src/mulle-objc-class-initialize.h` |
| Retain/release | `src/mulle-objc-retain-release.h` |
| Type encodings | `src/mulle-metaabi.h` |
| Demo: manual class+call | `test/demo/demo1.c` |
| Demo: protocols | `test/demo/demo2.c` |
| Demo: impcache low-level | `test/demo/impcache.c` |
| Demo: cache pivot | `demo/src/main-demo1.c` |
| Demo: struct ABI | `test/c/struct/call.c` |
| Test: classpair | `test/c/class/classpair.c` |

## Dominant API families

| Family | Entry point / header | Role |
|---|---|---|
| **Messaging & dispatch** | `src/mulle-objc-call.h` | `mulle_objc_object_call()`, inline variants, impcache |
| **Class/universe lifecycle** | `src/mulle-objc-load.h`, `src/mulle-objc-universe.h` | Loadinfo registration, universe creation, +initialize |
| **MetaABI calling convention** | `src/mulle-metaabi.h`, `src/mulle-metaabi-call.h` | `mulle_metaabi_call()`, parameter structs |
| **Signature parsing** | `src/mulle-objc-signature.h` | Type encoding, MetaABI param classification |
| **Object retain/release** | `src/mulle-objc-retain-release.h` | Inline RC, finalize, dealloc |

## Primary workflow

1. Include `<mulle-objc-runtime/mulle-objc-runtime.h>` (ensures TPS/FCS/TAO defines are set).
2. Register classes via `_mulle_objc_loadinfo` + `mulle_objc_loadinfo_enqueue_nofail()`
   in a `MULLE_C_CONSTRUCTOR` (see `test/demo/demo1.c:495-522`).
3. Define a `__register_mulle_objc_universe()` callback (use
   `MULLE_OBJC_DEFINE_REGISTER_UNIVERSE` in exactly one `.c` file).
4. Call objects with `mulle_objc_object_call()` or the MetaABI convenience
   macro `mulle_metaabi_call()`.
5. Use `mulle_objc_instance_free()` to release instances.

For method signature introspection use the `mulle_objc_signature_*` family from
`src/mulle-objc-signature.h`. For MetaABI parameter blocks use
`mulle_metaabi_union()` and the push/next macros from `src/mulle-metaabi.h`.

## Verify

```bash
mulle-sde craft --debug && mulle-sde test
```

Tests live under `test/` and use simple `assert()`-based `main()` returning 0
on pass (see `test/c/class/classpair.c` pattern).
