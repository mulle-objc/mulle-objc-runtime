# Property

A property wraps instance variables and methods together and add some semantics.
The chief advantage of a property is, that it frees the user from writing
accessors, which in turn makes AAM and other simplifications possible.

##  Functions

### `mulle_objc_property_get_name`

```
char   *mulle_objc_property_get_name( struct _mulle_objc_property *property)
```

Get the name of the property. The returned string's lifetime is that of the property. Don't free it.


### `mulle_objc_property_get_signature`

```
char   *mulle_objc_property_get_signature( struct _mulle_objc_property *property)
```

Get the signature of the property. This is not the same as the signature of the instance variable. The returned string's lifetime is that of the property. Don't free it.

### `mulle_objc_property_get_propertyid`

```
mulle_objc_propertyid_t  mulle_objc_property_get_propertyid( struct _mulle_objc_property *property)
```

Get the **propertyid** of the property.


### `mulle_objc_property_get_getter`

```
mulle_objc_methodid_t    mulle_objc_property_get_getter( struct _mulle_objc_property *property)
```

Get the methodid of the "getter" for the property.


### `mulle_objc_property_get_setter`

```
mulle_objc_methodid_t    mulle_objc_property_get_setter( struct _mulle_objc_property *property)
```

Get the methodid of the "setter" for the property. This may be `MULLE_OBJC_NO_METHODID` if the property is read-only and therefore has no setter. Read-only properties are rarely a good idea.


# Propertylist

This is a list of properties (not a list of pointers to properties). It
is read only: once created and added to the runtime it must not be modified.

## Functions

### `mulle_objc_propertylist_walk`

```
int   mulle_objc_propertylist_walk( struct _mulle_objc_propertylist *list,
                                  int (*f)( struct _mulle_objc_property *, struct _mulle_objc_class *, void *),
                                  struct _mulle_objc_class *cls,
                                  void *userinfo)
```

Iterate over a `propertylist` calling a callback for each `property` in it.

Parameter  | Description
-----------|-----------------------------
`list`     | The propertylist to walk
`f`        | Your callback function. `mulle_objc_propertylist_walk` will stop, when `f` returns a value that is not zero. This value will be returned by `mulle_objc_propertylist_walk`.
`cls`      | This will be passed to the callback function
`userinfo` | This will also be passed to the callback function

This is basically the same as `mulle_objc_methodlist_walk` so check that documentation for an example, how to use it.


### `mulle_objc_propertylist_enumerate`


```
struct  _mulle_objc_propertylistenumerator   mulle_objc_propertylist_enumerate( struct _mulle_objc_propertylist *list)
```

Enumerate a propertylist. `list` may be NULL.



# Propertylist Enumerator

## Functions


### `mulle_objc_propertylistenumerator_next`

```
struct _mulle_objc_property   *mulle_objc_propertylistenumerator_next( struct _mulle_objc_propertylistenumerator *rover)
```

Get the next property from the enumerator. Returns NULL if done.


### `mulle_objc_propertylistenumerator_done`

```
void  mulle_objc_propertylistenumerator_done( struct _mulle_objc_propertylistenumerator *rover)
```

Mark the end of the lifetime of the enumerator. This is merely conventional.
