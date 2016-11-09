# Method

A method combines a unique ID, a C-function and the signature of the method.
It also has a name, which is nice for human readable output.


## Functions

### `mulle_objc_method_get_name`

```
char   *mulle_objc_method_get_name( struct _mulle_objc_method *method)
```

Returns the name of the method. This is an ASCII string, terminated by \0.
It's lifetime is the same as that of `method`. Don't free it.


### `mulle_objc_method_get_signature`

```
char   *mulle_objc_method_get_signature( struct _mulle_objc_method *method)
```

Returns the @encoded type of the method. This is an ASCII string, terminated
by \0. It's lifetime is the same as that of `method`. Don't free it.


### `mulle_objc_method_get_id`

```
mulle_objc_methodid_t  mulle_objc_method_get_id( struct _mulle_objc_method *method)
```

{
   return( method ? _mulle_objc_method_get_id( method) : MULLE_OBJC_NO_METHODID);
}


Returns the ID od the method. This is a unique hash across all methods.

### `mulle_objc_method_get_implementation`

```
mulle_objc_methodimplementation_t
mulle_objc_method_get_implementation( struct _mulle_objc_method *method)
```

Return the implementation (C-Function) of the method. The C-function has the
uniform signature `void *(*)( id self, mulle_objc_methodid_t _cmd, void *_param)`.

# Methodlist

## Functions

This is a list of methods (not a list of pointers to methods). It
is read only: once created and added to the runtime it must not be modified.


### `mulle_objc_methodlist_walk`

```
int   mulle_objc_methodlist_walk( struct _mulle_objc_methodlist *list,
                                  int (*f)( struct _mulle_objc_method *, struct _mulle_objc_class *, void *),
                                  struct _mulle_objc_class *cls,
                                  void *userinfo)
```

Iterate over a `methodlist` calling a callback for each `method` in it.

Parameter  | Description
-----------|-----------------------------
`list`     | The methodlist to walk
`f`        | Your callback function. `mulle_objc_methodlist_walk` will stop, when `f` returns a value that is not zero. This value will be returned by `mulle_objc_methodlist_walk`.
`cls`      | This will be passed to the callback function
`userinfo` | This will also be passed to the callback function

Here is an example implementation of a callback function

```
static int   print_method( struct _mulle_objc_method *method,
                           struct _mulle_objc_class *cls,
                           void *userinfo)
{
   printf( "%p: %s[%s %s] @encode:%s\n",
            (void *) mulle_objc_method_get_implementation( method),
            userinfo,
            mulle_objc_class_get_name( cls),
            mulle_objc_method_get_name( method),
            mulle_objc_method_get_signature( method));
   return( 0);
}

mulle_objc_methodlist_walk( list, print_method, cls, "-");
```


### `mulle_objc_methodlist_enumerate`


```
struct  _mulle_objc_methodlistenumerator   mulle_objc_methodlist_enumerate( struct _mulle_objc_methodlist *list)
```

Enumerate a methodlist. `list` may be NULL. Returns an enumertator.



# Methodlist Enumerator

## Functions

### `mulle_objc_methodlistenumerator_next`

```
struct _mulle_objc_method   *mulle_objc_methodlistenumerator_next( struct _mulle_objc_methodlistenumerator *rover)
```

Get the next method from the enumerator. Returns NULL if done.


### `mulle_objc_methodlistenumerator_done`

```
void  mulle_objc_methodlistenumerator_done( struct _mulle_objc_methodlistenumerator *rover)
```

Mark the end of the lifetime of the enumerator. This is merely conventional.
