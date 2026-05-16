// invocation-test.h — shared stubs for @invocation compiler tests.
// Provides nil and stub implementations of the NSInvocation runtime functions
// so tests link against mulle-objc-runtime without needing MulleObjC.

#ifndef INVOCATION_TEST_H
#define INVOCATION_TEST_H

#ifndef nil
# define nil ((id) 0)
#endif

// Stub: NSInvocationCreateWithMetaABIFrame(sig, target, sel, frame, imp) -> id
// Used by Form 1 and Form 2 @invocation.
static __attribute__((unused))
id NSInvocationCreateWithMetaABIFrame( id sig, id target, SEL sel, void *frame, void *imp)
{
   (void) sig; (void) target; (void) sel; (void) frame; (void) imp;
   return( (id) 0);
}

// Stub: NSInvocationCreateDynamic(target, sel, sig, ...) -> id
// Used by Mode 3 (runtime variable SEL/sig) @invocation.
static __attribute__((unused))
id NSInvocationCreateDynamic( id target, SEL sel, id sig, ...)
{
   (void) target; (void) sel; (void) sig;
   return( (id) 0);
}

#endif /* INVOCATION_TEST_H */
