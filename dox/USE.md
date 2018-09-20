## Using the runtime

The **mulle-objc** runtime can be used without a mulle-objc compatible compiler,
but setting up a complex class system by hand is extremely tedious and
error-prone.
(See the few examples in `tests/`).


## The missing function `__get_or_create_mulle_objc_runtime`

The runtime needs an external function to work. This initializer function
is called `__get_or_create_mulle_objc_runtime`.

Here is a minimal implementation:

```
MULLE_C_CONST_RETURN  // always returns same value (in same thread)
struct _mulle_objc_runtime  *__get_or_create_mulle_objc_runtime( void)
{
   struct _mulle_objc_runtime  *runtime;

   runtime = __mulle_objc_get_runtime();
   if( ! _mulle_objc_runtime_is_initalized( runtime))
      __mulle_objc_runtime_setup( runtime, NULL);
   return( runtime);
}
```

You may want to tweak the runtime when running the `__mulle_objc_runtime_setup`
branch of the `if`.

> You should not change this runtime after returning it to the caller for the first time.

If you want to write a Foundation yourself, check out the "foundation" spaces
available to you in `struct _mulle_objc_runtime` and `struct _mulle_objc_threadconfig`.


# Headers

MulleObjC wants to serve several masters

* C code compiled as C code (.c)
* Any C or Objective-C code compiled as Objective-C code (.m)


Objective-C code gets everything, you #import <MulleObjC/MulleObjC.h>.
If you want to compile .c, you usually don't want the inclusion of
mulle-objc-runtime. Then you include <MulleObjC/ns-c.h>. This gives you
access to a runtime-independent subset. (maybe should be elsewhere ?)

If you have for some strange reason a C-compiler but no mulle-clang compiler,
and the compiled binaries, yet want to use some more MulleObjC + the runtime
yo include <MulleObjC/C.h

