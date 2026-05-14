#include <mulle-objc-runtime/mulle-objc-runtime.h>

@mixin A;
@mixin A
@optional
+ (void) print;
@end

@implementation A
+ (void) print
{
   printf( "%s\n", __FUNCTION__);
}
@end

@mixin B;
@mixin B
@optional
+ (void) print;
@end

@implementation B
+ (void) print
{
   printf( "%s\n", __FUNCTION__);
}
@end


@interface Foo <A, B>
@end

@implementation Foo
@end

@interface Bar <B, A>
@end
@implementation Bar
@end


int   main( void)
{
   [Foo print];   // B -> A -> Foo
   [Bar print];   // A -> B -> Bar
   return( 0);
}
