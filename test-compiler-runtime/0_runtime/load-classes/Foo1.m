#import "Foo1.h"

#include <stdio.h>


@implementation Foo1

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Foo1");
}
@end
