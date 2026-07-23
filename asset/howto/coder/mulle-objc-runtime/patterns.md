<!-- Keywords: object_call, loadinfo, metaabi_call, signature, universe -->
# Patterns

## 1. Messaging: calling a method on an object

Parameters are passed as a single `void *` to a struct. Use inline calls for
performance hot paths.

```c
void *obj;
// single-parameter call via compound literal
mulle_objc_object_call(obj, MULLE_OBJC_METHODID(0xf6bb528e),
   &(struct { int a; int b; }){ .a = 18, .b = 48 });

// zero-parameter call
mulle_objc_object_call(obj, ___init__methodid, NULL);
```

Source: `test/demo/demo1.c:566-574`

### Call-level variants

| Variant | When to use |
|---|---|
| `mulle_objc_object_call(obj, sel, param)` | Debug/standard, null-safe |
| `mulle_objc_object_call_inline(obj, sel, param)` | Release, single cache probe |
| `mulle_objc_object_call_inline_full(obj, sel, param)` | Hot path, full cache loop |
| `mulle_objc_object_call_inline_variable(obj, sel, param)` | Non-constant selector, no FCS |
| `mulle_objc_object_call_super(obj, sel, param, superid)` | `[super ...]` equivalent |

Source: `src/mulle-objc-call.h`

## 2. MetaABI: dynamic dispatch with type safety

Use `mulle_metaabi_call()` when the compiler cannot infer the method signature.
Include `mulle-metaabi-call.h` on demand (requires C23).

```c
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-objc-runtime/mulle-metaabi-call.h>

// void return, one int parameter
mulle_metaabi_call(mulle_metaabi_void, obj,
                   MULLE_OBJC_METHODID(0xfeedface),
                   42);

// int return, no parameters
int count;
mulle_metaabi_call(&count, obj, MULLE_OBJC_METHODID(0xdeadbeef));
```

Source: `src/mulle-metaabi-call.h`, `asset/dox/TOC.md` §6 example 5

## 3. Load: registering classes at startup

Define load classes and categories as static structs, then enqueue via
`mulle_objc_loadinfo_enqueue_nofail()` in a constructor.

```c
static struct _mulle_objc_loadclass  Foo_loadclass =
{
   .base.classid         = MULLE_OBJC_CLASSID(0xc7e16770),
   .base.classname       = "Foo",
   .superclassid         = MULLE_OBJC_CLASSID(0x58e64dae),
   .superclassname       = "Object",
   .instancesize         = sizeof(struct Foo),
   .instancevariables    = (void *) &Foo_ivarlist,
   .base.instancemethods = (void *) &Foo_instance_methodlist,
};

MULLE_C_CONSTRUCTOR(__load)
static void __load()
{
   mulle_objc_loadinfo_enqueue_nofail(&load_info);
}
```

Source: `test/demo/demo1.c:361-522`

### Loadinfo version

Always set `version.load = MULLE_OBJC_RUNTIME_LOAD_VERSION` (21) and include
correct TPS/FCS/TAO bits:

```c
.version = {
   .load    = MULLE_OBJC_RUNTIME_LOAD_VERSION,
   .runtime = MULLE_OBJC_RUNTIME_VERSION,
   .bits    = TPS_BIT | FCS_BIT | TAO_BIT
}
```

Source: `test/demo/demo1.c:495-506`

## 4. Universe: registration callback

Define exactly one callback per executable using
`MULLE_OBJC_DEFINE_REGISTER_UNIVERSE`:

```c
#define MULLE_OBJC_DEFINE_REGISTER_UNIVERSE

MULLE_C_CONST_RETURN
struct _mulle_objc_universe *
   __register_mulle_objc_universe(mulle_objc_universeid_t universeid,
                                  char *universename)
{
   struct _mulle_objc_universe *universe;

   universe = __mulle_objc_global_get_universe(universeid, universename);
   if (!_mulle_objc_universe_is_initialized(universe))
   {
      _mulle_objc_universe_bang(universe, 0, NULL, NULL);
      universe->config.ignore_ivarhash_mismatch = 1;
   }
   return(universe);
}
```

Source: `test/demo/demo1.c:525-539`, `src/mulle-objc-universe.h:57-80`

## 5. Signature parsing: iterating method types

```c
char *types = "@:i";  // self, _cmd, int
struct mulle_objc_typeinfo info;

while (mulle_objc_signature_supply_typeinfo(types, NULL, &info))
{
   printf("type=%s size=%zu\n", info.type, info.natural_size);
   types = mulle_objc_signature_next_type(types);
   if (!types || !*types)
      break;
}
```

Source: `asset/dox/TOC.md` §6 example 2

## 6. Object creation and teardown

```c
struct _mulle_objc_infraclass  *cls;
struct _mulle_objc_object      *obj;

cls  = mulle_objc_global_lookup_infraclass_nofail(MULLE_OBJC_DEFAULTUNIVERSEID,
                                                   ___Foo_classid);
_mulle_objc_infraclass_setup_if_needed(cls);
obj  = mulle_objc_infraclass_alloc_instance(cls);
// use obj...
mulle_objc_instance_free(obj);
```

Source: `test/demo/demo1.c:556-578`

## 7. Protocol conformance (no compiler)

Build a protocol list sorted by protocolid and attach to the loadclass:

```c
static struct _mulle_objc_protocollist Foo_protocollist =
{
   .n_protocols = 3,
   .protocols   = {
      { .protocolid = MULLE_OBJC_PROTOCOLID(0x...) },
      ...
   }
};

// in loadclass:
.base.protocols = &Foo_protocollist,
```

Source: `test/demo/demo2.c` (protocol conformance without compiler)
