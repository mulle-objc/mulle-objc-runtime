## Using the runtime

The **mulle-objc** runtime can be used without a mulle-objc compatible compiler,
but setting up a complex class system by hand is extremely tedious and
error-prone.
(See the few examples in `tests/`).


## The missing function `__register_mulle_objc_universe`

The runtime needs an external function to work. This initializer function
is called `__register_mulle_objc_universe`.

Here is a minimal implementation:

```
MULLE_C_CONST_RETURN  // always returns same value (in same thread)
struct _mulle_objc_universe  *
   __register_mulle_objc_universe( mulle_objc_universeid_t universeid,
                                   char *universename)
{
   struct _mulle_objc_universe   *universe;

   universe = __mulle_objc_global_get_universe( universeid, universename);
   if( ! _mulle_objc_universe_is_initialized( universe))
      _mulle_objc_universe_bang( universe, bang, NULL, NULL);

   return( universe);
}
```

You may want to tweak the runtime when running the `_mulle_objc_universe_bang`
branch of the `if`.

> You should not change this runtime after returning it to the caller for the first time.

If you want to write a Foundation yourself, check out the "foundation" spaces
available to you in `struct _mulle_objc_universe` and `struct _mulle_objc_threadinfo`.



