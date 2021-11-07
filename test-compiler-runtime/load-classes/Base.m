#import "Base.h"

#include <stdio.h>


@implementation Base

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Base");
}

@end
