#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wreceiver-expr"

//
// Test class properties in a category with dynamic (user-provided) accessors
//
@interface Foo
@end

@interface Foo (Extra)
@property (class, dynamic, assign) int extra;
+ (int) getExtra;
@end

@implementation Foo
@end

static int _extra = 0;

@implementation Foo (Extra)
+ (int) extra       { return _extra; }
+ (void) setExtra:(int) v { _extra = v; }
+ (int) getExtra    { return [self extra]; }
+ (void) test
{
   [self setExtra:77];
   int v = [self getExtra];
   if( v != 77)
   {
      printf( "FAIL: [self getExtra] returned %d, expected 77\n", v);
      abort();
   }
}
@end

int   main( void)
{
   [Foo test];
   return( 0);
}
