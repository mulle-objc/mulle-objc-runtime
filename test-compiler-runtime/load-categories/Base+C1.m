#import "Base.h"

#include <stdio.h>


@class ProtoClass1;
@protocol ProtoClass1;

@interface Base( C1) < ProtoClass1>
@end

@implementation Base( C1)

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Base( C1)");
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
