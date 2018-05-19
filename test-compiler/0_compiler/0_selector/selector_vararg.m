#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdarg.h>


static void  test_2( mulle_objc_uniqueid_t first, ...)
{
   va_list  args;
   char     *delim;

   delim="";
   va_start( args, first);
   while( first)
   {
      printf( "%s%x", delim, first);
      first = va_arg( args, mulle_objc_uniqueid_t);
      delim=", ";
   }
   va_end( args);
   printf( "\n", delim, first);
}


main()
{
   printf( "%x, %x, %x\n", @selector( a), @selector( b), @selector( c));

   test_2( @selector( a), @selector( b), @selector( c), 0);
   return( 0);
}
