#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


main()
{
   SEL  sel;

   // this just checks being able to compare with a constant
   sel = (SEL) 0x63743c39;
   if( sel == @selector( vfl))
      printf( "passed\n");
}
