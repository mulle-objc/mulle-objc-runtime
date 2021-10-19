#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface NSException
@end


@interface NSOtherException : NSException
@end


@implementation NSException

+ (id) new
{
   return( [mulle_objc_infraclass_alloc_instance( self) init]);
}

- (void) dealloc
{
   _mulle_objc_instance_free( self);
}


- (id) init
{
   return( self);
}

@end


@implementation NSOtherException
@end


main()
{
   NSException   *exception;
   @try
   {
      printf( "@try\n");
      @throw( [NSOtherException new]);
   }
   @catch( NSOtherException *exception)
   {
      printf( "@catch NSOtherException\n");
      [exception dealloc];
   }
   @catch( NSException *exception)
   {
      printf( "@catch NSException\n");
      [exception dealloc];
   }
   @finally
   {
      printf( "@finally\n");
   }
   return( 0);
}


