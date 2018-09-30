# Exceptions

The exception mechanism is a copy of the OS X "fragile" exception mechanism
using `setjmp` and `longjmp`. As such it is wise to only use it as an
exceptional flow control mechanism.

## Raising an exception

### mulle_objc_exception_throw

```
void   mulle_objc_exception_throw( void *exception);
```

Throw an an exception through the runtime mechanism. `exception` is a user
defined void pointer, that is just passed through. This is a convenient
breakpoint as all Objective-C exceptions are vectored through
`mulle_objc_exception_throw`.


## The try/catch exception mechanism

This may be of interest if you are writing a compiler for the runtime.


> Will write some more documentation on demand

Relevant links:

[clang](//github.com/llvm-mirror/clang/blob/master/lib/CodeGen/CGObjCMac.cpp#L4020)
[mulle_objc_exception_tryenter.h](../src/mulle_objc_exception_tryenter.h)
[mulle_objc_exception_tryenter.c](../src/mulle_objc_exception_tryenter.c)

