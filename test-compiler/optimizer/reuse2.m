#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@implementation Foo

+ (id)  test2:(id) a
             :(id) b
{
   return( (id) 0x33);
}


+ (void)  test:(id) a
              :(id *) b
{
   *b = [self test2:(id) 0x48
                   :(id) 0x1848];
}

@end


int   main( void)
{
   id   foo;

   [Foo test:(id) 0x18 :&foo];
   if( foo != (id) 0x33)
   {
      fprintf( stderr, "foo is %p\n", foo);
      return( 1);
   }
   return( 0);
}
