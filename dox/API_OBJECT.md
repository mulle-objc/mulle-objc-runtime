# Object

In this documentation **object** and **instance** are not synonym. An object can be an instance or a class, but a class is not considered to be an instance.

* objects  : everything that can be messaged, that has a class
* class    : a runtime object
* instance : objects that are not classes


## Functions

### `mulle_objc_object_call`

```
void   *mulle_objc_object_call( void *obj,
                                mulle_objc_methodid_t methodid,
                                void *parameter);
```

> The mulle-clang compiler emits this for `-O0`.

The non-inlining variant of sending a message to an object (calling an objects method). Parameters and return values are passed via `parameter` according to the [MetaABI](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_meta_call_convention.html).

`obj` must be a pointer to a valid object or NULL.


### `mulle_objc_object_partialinlinecall`

```
void  *mulle_objc_object_partialinlinecall( void *obj,
                                                             mulle_objc_methodid_t methodid,
                                                             void *parameter)
```

This is an optimized variant of `mulle_objc_object_call` in cases, where
`methodid` is a known constant. The compiler uses this method when optimizing
with -O1.

Right: `mulle_objc_object_partialinlinecall( self, 0x1848, NULL)`,
Wrong: `mulle_objc_object_partialinlinecall( self, _cmd, NULL)


### `mulle_objc_object_inlinecall`

```
void  *mulle_objc_object_inlinecall( void *obj,
                                                      mulle_objc_methodid_t methodid,
                                                      void *parameter)
```

This is an even further optimized variant of `mulle_objc_object_call` in cases,
where `methodid` is a known constant. The compiler uses this method when
optimized with -O2 and up. Frequent use of this function can increase the size
of your executable by quite a bit.

Right: `mulle_objc_object_inlinecall( self, 0x1848, NULL)`,
Wrong: `mulle_objc_object_inlinecall( self, _cmd, NULL)


### `mulle_objc_object_call_classid`

```
void   *mulle_objc_object_call_classid( void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter,
                                        mulle_objc_classid_t classid);
```

Instead of using the `isa` as the class to lookup the method, use the class
given by `classid`. This is used by the compiler for super calls. It is a much
slower operatin than `mulle_objc_object_call`.



### `mulle_objc_objects_call`

```
void   mulle_objc_objects_call( void **objects, unsigned int n, mulle_objc_methodid_t methodid, void *params)
```

Call all `n``objects` with message `methoid` and arguments `params` in sequence.
This method is singlethreaded and executes from the `objects[ 0]` up to but not
including `objects[ n]`. It is OK, if objects contains NULL pointers.

Using `mulle_objc_objects_call` can be more efficient then using a boiler plate
for loop yourself.


### `mulle_objc_object_retain`

```
void   *mulle_objc_object_retain( void *obj)
```

Increment the reference count of `obj`. `obj` may be NULL. Returns `obj`
back, this is a convenience and somewhat of an optimisation, since the caller
does not not need to save `obj` for the call.

So `obj = mulle_objc_object_retain( obj)` is safe and may be faster than
just `mulle_objc_object_retain( obj)`.


### `mulle_objc_object_release`

```
void   mulle_objc_object_release( void *obj)
```

Decrement the reference count of `obj`. `obj` may be NULL. If the reference
count goes into negative, an internal version of `mulle_objc_object_finalize`
will be called, unless `mulle_objc_object_finalize` has been called before on
this object. If the object did not get retained during finalization, then an
internal version of `mulle_objc_object_dealloc` is called. The object is then
to be considered "dead" and must not be messaged any more.

See: [mulle_objc: object layout, retain counting, finalize](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_finalize_makes_a_comeback.html]


### `mulle_objc_object_get_class`

```
struct _mulle_objc_class   *mulle_objc_object_get_class( void *obj)
```

Returns the class of `obj`, basically what `-class` does. This is different than asking an `obj` for its `isa` class, becauses classes and instances both return the same class here (`assert( [foo class] == [Foo class]`)


### `mulle_objc_object_get_runtime`

```
struct _mulle_objc_runtime   *mulle_objc_object_get_runtime( void *obj)
```

Return the runtime, the class of `obj` belongs to. In mulle-objc multiple runtimes can coexist, so this is in general more correct, than just calling `mulle_objc_get_runtime`.


### `mulle_objc_object_get_isa`

```
struct _mulle_objc_class *cls  mulle_objc_object_get_isa( void *obj)
```

Returns the `isa` class of `obj`. This is not necessarily the same as the `isa` pointer in the object header, because the function resolves tagged pointers.


### `mulle_objc_object_set_isa`

```
void  mulle_objc_object_set_isa( void *obj, struct _mulle_objc_class *cls)
```

Change the `isa` of `obj` to `cls`. This will fail and raise an exception, if `obj` is of a tagged pointer class. If either value is NULL, the function does nothing.


### `mulle_objc_object_dealloc`

```
void   mulle_objc_object_dealloc( void *obj)
```

A C way of calling `-dealloc` on an object. If you call this function yourself,
you are probably "doing it wrong".


### `mulle_objc_object_copy`

```
void   *mulle_objc_object_copy( void *obj)
```

A C way of calling `-copy` on an object, returns `obj`.


### `mulle_objc_object_release`

```
void   mulle_objc_object_release( void *obj)
```

A C way of calling `mulle_objc_object_release` on an object. It's not any faster
then calling `-release`.

### `mulle_objc_object_autorelease`

```
void   *mulle_objc_object_autorelease( void *obj)
```

A C way of calling `-autorelease` on an object, returns `obj`.
