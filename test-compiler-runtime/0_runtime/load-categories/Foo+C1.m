#import "Foo.h"

#include <stdio.h>


@interface Foo( C1)
@end

@implementation Foo( C1)

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Foo( C1)");
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
