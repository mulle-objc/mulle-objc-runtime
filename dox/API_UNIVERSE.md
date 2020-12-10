# Universe

The **mulle-objc** universe is not a class, but a C struct. It is the root of
everything: classes and instances. In a well constructed mulle-objc runtime
system, every object is reachable from the universe.

By default there is one global universe per process. This is the simplest and
most efficient setup.

![global setup](global_universe.png)

But the universe library can also be compiled with `-fobjc-universename=<universename>`
so that multiple universes with different class hierarchies can coexist
(but not message each other).

This could be beneficial for plugins that get dynamically loaded and unloaded
(e.g. Apache Web Server)

There is a limitation. Only the global universe can be compiled with TPS (tagged pointer support).



## Functions


### `mulle_objc_global_get_universe`

```
struct _mulle_objc_universe  *mulle_objc_global_get_universe( void);
```

Returns the universe for the current thread. If this thread has no universe
associated with it, this function may crash.


### `mulle_objc_global_get_universe_inline`

```
struct _mulle_objc_universe  * mulle_objc_global_get_universe_inline( void);
```

A slightly faster version of above `mulle_objc_global_get_universe`. If you use it you
should be aware that your compiled code is committed to using either the global
or the thread local universe scheme.


### `mulle_objc_universe_new_classpair`

```
struct _mulle_objc_classpair   *mulle_objc_universe_new_classpair(
				struct _mulle_objc_universe *universe,
				mulle_objc_classid_t  classid,
				char *name,
				size_t instance_size,
				struct _mulle_objc_infraclass *superclass);
```

Create a new `_mulle_objc_classpair`, the class pair is not yet added to the
universe. The classpair must be added to the universe, that created it and no
other.

Parametername       |  Description
--------------------|----------------------
`universe`          | Pointer to the universe, must not be NULL. Use `__register_mulle_objc_universe` (not `mulle_objc_global_get_universe`) to get the proper one for your thread
`classid`           | Compute the `classid` from `name` with `mulle_objc_classid_from_string`
`name`              | The name of the class, this must be a non-empty ASCII string. This string is not copied. It must remain alive for the duration of the class's existence.
`instance_size`     | The space needed for instance variables (don't add the header size)
`superclass`        | Class to inherit from, can be NULL for root classes.

Returns NULL on error. Check `errno` for error codes.


### `mulle_objc_universe_add_infraclass`

```
int   mulle_objc_universe_add_infraclass( struct _mulle_objc_universe *universe,
                                          struct _mulle_objc_infraclass *cls);
```

Add a class `cls` to `universe`. Returns 0 on success, otherwise check `errno`
for error codes.

> It is generally recommended that you use
> [`_mulle_objc_loadinfo`](API_LOADINFO.md) to add
> classes to the universe.

Example:

```
struct _mulle_objc_universe    *universe;
struct _mulle_objc_classpair   *pair;
struct _mulle_objc_class       *cls;
char                           *name;
mulle_objc_classid_t           classid;

universe =  __register_mulle_objc_universe( MULLE_OBJC_UNIVERSEID, NULL);
name    = "Foo";
classid = mulle_objc_classid_from_string( name);
pair    = mulle_objc_universe_new_classpair( universe, classid, name, 0, 0, NULL);
cls     = mulle_objc_classpair_get_infraclass( pair);
mulle_objc_universe_add_class( universe, cls);
```

### `_mulle_objc_universe_lookup_infraclass`

```
struct _mulle_objc_class  *_mulle_objc_universe_lookup_infraclass(
                    struct _mulle_objc_universe *universe,
                    mulle_objc_classid_t classid)
```

Retrieve class with `classid` from `universe`. Returns NULL if not found.


### `mulle_objc_universe_calloc`

```
void   *mulle_objc_universe_calloc( struct _mulle_objc_universe *universe, size_t n, size_t size)
```

`calloc` some memory using the memory allocator of the universe. This automatically gifts the memory to the universe!

### `mulle_objc_universe_realloc`

```
void   *mulle_objc_universe_realloc( struct _mulle_objc_universe *universe, void *block, size_t size)
```

`realloc` some memory using the memory allocator of the universe.This automatically gifts the memory to the universe!

