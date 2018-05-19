#include <mulle-objc-runtime/mulle-objc-runtime.h>



int   main( void)
{
   static struct _mulle_objc_dependency   array[] =
   {
      { @selector( Foo), @selector( Bar) } // happy if compiles
   };
   return( 0);
}


