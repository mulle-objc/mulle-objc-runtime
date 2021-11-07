#import "Root.h"

#include <stdio.h>


@implementation Root

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Root");
}
@end
