#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@implementation Foo

+ (id)  test2:(id) a
             :(id) b
{
   return( b);
}


+ (id)  test:(id) a
            :(id) b
{
   id   c;

   c = [self test2:b
                  :a];
   return( c);
}

@end


main()
{
   id   foo;

   foo = [Foo test:(id) 0x18 : (id) 0x48];
   if( foo != (id) 0x18)
   {
      fprintf( stderr, "foo is %p\n", foo);
      return( 1);
   }
   return( 0);
}
