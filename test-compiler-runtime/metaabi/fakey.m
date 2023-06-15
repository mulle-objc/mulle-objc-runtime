#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void   call( void **p_rval, void *obj, mulle_objc_methodid_t sel, float arg)
{
   void   *param;
   union
   {
      __typeof__( arg)  v;
      void   *param;
   } tmp;
   union
   {
      __typeof__( *p_rval)  v;
      void   *ptr;
   } rval;

   MULLE_C_ASSERT( sizeof( *p_rval) <= sizeof( void *));
   MULLE_C_ASSERT( sizeof( tmp) <= sizeof( void *));

   tmp.v = arg;

   rval.ptr = mulle_objc_object_call( obj, sel, tmp.param);

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
   printf( "%p\n", rval);
   return( 0);
}