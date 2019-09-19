#import "Foo2.h"

#include <stdio.h>


@implementation Foo2

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Foo2");
}
@end
