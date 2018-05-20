#import "Base.h"

#include <stdio.h>


@class ProtoClass1;
@protocol ProtoClass1;

@interface Base( C1) < ProtoClass1>
@end

@implementation Base( C1)

+ (void) load
{
    printf( "Base( C1)\n");
}


+ (struct _mulle_objc_dependency *) dependencies
{
   static struct _mulle_objc_dependency  dependencies[] =
   {
      { @selector( Base), @selector( C2) },
      0
   };

   return( dependencies);
}

@end
