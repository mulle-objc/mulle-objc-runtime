#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stddef.h>
#include <stdint.h>


@interface Bar
{
   int  _a;
}
@property char b;
@property int c;    // default sorted up by size
@end

@interface Foo : Bar
{
   char  _d;
}
@property int e;
@property char d;   // ivar specified
@end

@implementation Bar
@end


@implementation Foo

@synthesize d = _d;

@end


main()
{
   printf( "%s\n", offsetof( struct{ @defs( Foo); }, _a) == 0 ? "PASS" : "FAIL");
   printf( "%s\n", offsetof( struct{ @defs( Foo); }, _a) < offsetof( struct{ @defs( Foo); }, _b) ? "PASS" : "FAIL");
   printf( "%s\n", offsetof( struct{ @defs( Foo); }, _b) < offsetof( struct{ @defs( Foo); }, _c) ? "PASS" : "FAIL");
   printf( "%s\n", offsetof( struct{ @defs( Foo); }, _c) < offsetof( struct{ @defs( Foo); }, _d) ? "PASS" : "FAIL");
   printf( "%s\n", offsetof( struct{ @defs( Foo); }, _d) < offsetof( struct{ @defs( Foo); }, _e) ? "PASS" : "FAIL");
}
