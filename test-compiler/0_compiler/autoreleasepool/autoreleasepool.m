#include <mulle-objc-runtime/mulle-objc-runtime.h>



@interface NSAutoreleasePool
@end


@implementation NSAutoreleasePool

+ (instancetype) new
{
   return( mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self));
}


- (void) finalize
{
}


- (void) dealloc
{
   printf( "dealloc\n");
   mulle_objc_instance_free( self);
}

@end


NSAutoreleasePool *
   _MulleAutoreleasePoolPush( mulle_objc_universeid_t universe_id)
{
   printf( "alloc (%u)\n", universe_id);
   return( [NSAutoreleasePool new]);
}


MULLE_C_NEVER_INLINE
void   foo( void)
{
   @autoreleasepool
   {
      printf( "pool\n");
   }
}


int   main( void)
{
   foo();
   return( 0);
}


