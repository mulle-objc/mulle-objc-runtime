#include <mulle-objc-runtime/mulle-objc-runtime.h>



main()
{
   static SEL   array[] =
   {
      @selector( Foo) // happy if compiles
   };
   return( 0);
}


