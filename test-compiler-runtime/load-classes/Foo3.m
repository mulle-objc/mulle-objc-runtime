#import "Foo3.h"

#include <stdio.h>


@implementation Foo3

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Foo3");
}
@end
