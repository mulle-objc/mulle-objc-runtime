#include <stddef.h>
#include <stdio.h>

int  main( void)
{
   max_align_t   foo;  // if unknown, mulle-clang is not properly c11 configured
   
   printf( "max_align_t is %td bytes\n", sizeof( foo));
   return( 0);
}

