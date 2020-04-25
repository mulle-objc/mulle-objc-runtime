#include <mulle-objc-runtime/mulle-objc-runtime.h>

struct bar
{
   int   a;
   int   b;
};

@interface Foo
@end

@implementation Foo

+ (void) main
{
   struct bar   bar = { 18, 48 };

   // break here and
   // p bar
   printf( "%d,%d\n", bar.a, bar.b);
}

@end


int   main( void)
{
   [Foo main];
   return( 0);
}

