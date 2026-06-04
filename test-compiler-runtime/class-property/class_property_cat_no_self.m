#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wreceiver-expr"

//
// Test: [self msg] works in a category that does NOT declare class properties.
// self in + methods dispatches class methods normally.
//
@interface Foo
@property (class, assign) int sharedCount;
@end

@interface Foo (Helper)
// no class properties here — [self msg] dispatches class methods
@end

@implementation Foo
@end

@implementation Foo (Helper)
+ (void) doubleIt
{
   int v = [self sharedCount];
   [self setSharedCount: v * 2];
}
@end

int   main( void)
{
   [Foo setSharedCount:5];
   [Foo doubleIt];
   int v = [Foo sharedCount];
   if( v != 10)
   {
      printf( "FAIL: expected 10, got %d\n", v);
      return( 1);
   }
   return( 0);
}
