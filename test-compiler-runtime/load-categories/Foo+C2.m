#import "Foo.h"

#include <stdio.h>


@interface Foo( C2)
@end

@implementation Foo( C2)

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Foo( C2)");
}

+ (struct _mulle_objc_dependency *) dependencies
{
   static struct _mulle_objc_dependency  dependencies[] =
   {
      { @selector( Foo), @selector( C3) },
      0
   };

   return( dependencies);
}

@end
