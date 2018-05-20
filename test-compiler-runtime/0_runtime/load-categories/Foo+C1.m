#import "Foo.h"

#include <stdio.h>


@interface Foo( C1)
@end

@implementation Foo( C1)

+ (void) load
{
    printf( "Foo( C1)\n");
}


+ (struct _mulle_objc_dependency *) dependencies
{
   static struct _mulle_objc_dependency  dependencies[] =
   {
      { @selector( Foo), @selector( C2) },
      0
   };

   return( dependencies);
}

@end
