#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

@interface A @end

struct tiny
{
   char   a[ 3];
};


@implementation A

+ (Class) class
{
   return( self);
}

+ (void) checkTiny:(struct tiny) v
{
   // v is the compiler shadow — unpacked from _param via *(struct tiny*)&_param
   // verify values arrived correctly
   mulle_printf( "v = '%c' '%c' '%c'\n", v.a[ 0], v.a[ 1], v.a[ 2]);
   // verify v is a copy, not aliasing _param
   mulle_printf( "v is copy: %s\n", (void *) &v != (void *) &_param ? "yes" : "no");
}

@end


int   main( void)
{
   struct tiny   s = { { 'X', 'Y', 'Z' } };

   [[A class] checkTiny: s];
   return( 0);
}
