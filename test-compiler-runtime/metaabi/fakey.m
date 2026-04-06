#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

@interface A @end

void   call( void **p_rval, void *obj, mulle_objc_methodid_t sel, float arg)
{
   struct { float v; }   tmp;
   union
   {
      __typeof__( *p_rval)  v;
      void   *ptr;
   } rval;

   tmp.v = arg;

   rval.ptr = mulle_objc_object_call( obj, sel, &tmp);

   *p_rval = (__typeof__( *p_rval)) (intptr_t) rval.ptr;
}





@implementation A

+ (Class) class
{
   return( self);
}

+ (void *) returnVoidptrWithFloat:(float) v
{
   return( (void *) 1849);
}

@end



int  main( void)
{
   void   *rval;
   Class   cls;

   cls = [A class];
   call( &rval, cls, @selector( returnVoidptrWithFloat:), 18.48f);
   mulle_printf( "%p\n", rval);
   return( 0);
}