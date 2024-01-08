#include <mulle-objc-runtime/mulle-objc-runtime.h>



int   main( void)
{
   static SEL   array[] =
   {
      @selector( Foo) // happy if compiles
   };
   return( 0);
}


