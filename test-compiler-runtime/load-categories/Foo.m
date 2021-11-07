#import "Foo.h"

#include <stdio.h>


@implementation Foo

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Foo");
}
@end
