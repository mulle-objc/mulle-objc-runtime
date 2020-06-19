# Global Functions and Macros


## Macros

### `mulle_metaabi_struct`

```
mulle_metaabi_struct( param_type, rval_type)
```

To construct a properly sized meta-ABI parameter block, you should use this
macro.

Example:

Construct a struct from  your parameters. Use the meta-ABI rules as explained
in [mulle-objc: a meta calling convention](//www.mulle-kybernetik.com/weblog/2015/mulle_objc_meta_call_convention.html)
and do not forget the
[C argument promotions](//stackoverflow.com/questions/1255775/default-argument-promotions-in-c-function-calls).

Here is an example how to construct a call to `- (long long) fooWithA:(double) a b:(double) b`:


```
   mulle_metaabi_struct( struct { double a; double b;}, long long)  _param;

   _param.p.a = 18.0;
   _param.p.b = 0.48;

   mulle_objc_object_call( self, 0xc2d87842, &_param);

   return( _param.r)
```


### `mulle_metaabi_struct_void_return`

```
mulle_metaabi_struct_void_return( param_type)
```

This is a simplified version of `mulle_metaabi_struct`, if the return
value is **void**.


## ID Functions


### `mulle_objc_classid_from_string`

```
mulle_objc_classid_t   mulle_objc_classid_from_string( char *s)
```

Compute the **classid** from ASCII string `s`. Will return
`MULLE_OBJC_NO_CLASSID` if `s` is NULL.


### `mulle_objc_propertyid_from_string`

```
mulle_objc_propertyid_t   mulle_objc_propertyid_from_string( char *s)
```

Compute the **propertyid** from ASCII string `s`. Will return
`MULLE_OBJC_NO_PROPERTYID` if `s` is NULL.


### `mulle_objc_ivarid_from_string`

```
mulle_objc_ivarid_t   mulle_objc_ivarid_t( char *s)
```

Compute the **ivarid** from ASCII string `s`. Will return
`MULLE_OBJC_NO_IVARID` if `s` is NULL.


### `mulle_objc_methodid_from_string`

```
mulle_objc_methodid_t   mulle_objc_methodid_from_string( char *s)
```

Compute the **methodid** from ASCII string `s`. Will return
`MULLE_OBJC_NO_METHODID` if `s` is NULL.


### `mulle_objc_protocolid_from_string`

```
mulle_objc_protocolid_t   mulle_objc_protocolid_from_string( char *s)
```

Compute the **protocolid** from ASCII string `s`. Will return
`MULLE_OBJC_NO_PROTOCOLID` if `s` is NULL.


## Thread Functions

If your thread wants to acces the mulle-runtime (except the thread you
initialized the runtime with), you must call `mulle_objc_thread_register`
first. You should periodically call `mulle_objc_thread_checkin` to
progress the internal  garbage collection of the runtime. At the end of the
thread you must call `mulle_objc_thread_deregister`.


### `mulle_objc_thread_register`

```
void   mulle_objc_thread_register( mulle_objc_universeid_t universeid)
```

Register the current thread in the global runtime. If you are using multiple
runtimes, check the source code for more details.


### `mulle_objc_thread_deregister`

```
void   mulle_objc_thread_deregister( mulle_objc_universeid_t universeid)
```

Unregister the current thread from the global runtime. Do not access the
runtime or any of its classes or objects afterwards.




### `mulle_objc_thread_checkin`

```
void   mulle_objc_thread_checkin( mulle_objc_universeid_t universeid)
```

Check in the current thread. Failing to do this often enough can result in
bloat of the app.
