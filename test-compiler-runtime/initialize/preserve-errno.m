#ifdef __MULLE_OBJC__
# include <mulle-objc-runtime/mulle-objc-runtime.h>
#else
# import <Foundation/Foundation.h>
#endif

#include <errno.h>



@interface A
@end


@implementation A


+ (void) initialize
{
   if( errno != 1848)
      printf( "Fail 1\n");
   errno = 2;
}


+ (int) action
{
   if( errno != 1848)
      printf( "Fail 2\n");
   errno = 3;
   return( -1);
}

@end


@interface B
@end


@implementation B


+ (int) action
{
   if( errno != 1848)
      printf( "Fail 4\n");
   errno = 3;
   return( -1);
}

@end



int  main( void)
{
   errno = 1848;

   // should lookup class, run +initialize and then the action
   // initialize should not interfere with errno,

   if( [A action])
   {
      if( errno != 3)
      {
         printf( "Fail 3\n");
         return( 1);
      }
   }

   // nor the likely setup of
   // a cache + malloc

   errno = 1848;

   if( [B action])
   {
      if( errno != 3)
      {
         printf( "Fail 5\n");
         return( 1);
      }
   }
   printf( "OK\n");

   return( 0);
}
