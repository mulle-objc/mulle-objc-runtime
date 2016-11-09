# IVar : Instance Variable

`struct _mulle_objc_ivar` contains runtime information about the instance
variables that are part of a class. You retrieve them from the class with
`mulle_objc_class_search_ivar`.


## Functions

### `mulle_objc_ivar_get_name`

```
char   *mulle_objc_ivar_get_name( struct _mulle_objc_ivar *ivar)
```

Get the name of this instance variable. This pointer is alive as long as
`ivar` is alive and unmodified. Don't free it.


### `mulle_objc_ivar_get_id`

```
mulle_objc_ivarid_t   mulle_objc_ivar_get_id( struct _mulle_objc_ivar *ivar)
```

Get the **ivarid** of this instance variable.


### `mulle_objc_ivar_get_signature`

```
char   *mulle_objc_ivar_get_signature( struct _mulle_objc_ivar *ivar)
```

Get the [@encode](@ENCODE.md) signature of this instance variable. This pointer
is alive as long as `ivar` is alive and unmodified.  Don't free it.


### `mulle_objc_ivar_get_offset`

```
int    mulle_objc_ivar_get_offset( struct _mulle_objc_ivar *ivar)
```

Returns the offset into `self` for this instance variable.
