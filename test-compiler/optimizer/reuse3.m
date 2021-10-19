#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Foo
@end


@implementation Foo

+ (id)  test2:(id) a
             :(id) b
{
   return( (id) b);
}


+ (id)  test:(id) a
            :(id) b
{
   return( [Foo test2:b :a], [Foo test2:a :b]);
}

@end


int   main( void)
{
   id   v;

   v = [Foo test:(id) 1 :(id) 2];
   printf( "%td\n", (intptr_t) v);
   return( 0);
}
