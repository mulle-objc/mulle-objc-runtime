#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


struct CGPoint 
{
   float   x, y;
};

struct CGSize 
{
   float   w, h;
};


struct CGRect
{
   struct CGPoint   origin;
   struct CGSize    size;
};


struct Float2S
{
   float  values[ 2];
};

union Float2U
{
   float  values[ 2];
};


struct Float4S
{
   float  values[ 4];
};

union Float4U
{
   float  values[ 4];
};




static void  test( char *a, char *b)
{
   char   *s;

   printf( "%s and %s are", a, b);
   fflush( stdout);

   s = _mulle_objc_ivarsignature_is_binary_compatible( a, b)
       ? "compatible"
       : "incompatible";

   printf( " %s\n", s);
}


int   main( void)
{
   test( @encode( int), @encode( int));
   test( @encode( int), @encode( unsigned int));
   test( @encode( int), @encode( long));
   test( @encode( struct CGPoint), @encode( struct CGPoint));
   test( @encode( struct CGPoint), @encode( struct CGSize));
   test( @encode( struct CGPoint), @encode( struct CGRect));
   test( @encode( struct CGPoint), @encode( struct Float2S));
   test( @encode( struct CGPoint), @encode( union Float2U));
   test( @encode( struct CGRect), @encode( struct Float4S));
   test( @encode( struct CGRect), @encode( union Float4U));
   test( @encode( struct CGRect), @encode( struct Float2S));
   test( @encode( struct CGPoint[ 1]), @encode( struct Float2S));
   test( @encode( struct CGPoint[ 2]), @encode( struct Float2S));

   return( 0);
}
