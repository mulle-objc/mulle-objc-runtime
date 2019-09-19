#import "Base.h"

#include <stdio.h>


@interface Base( C2)
@end

@implementation Base( C2)

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Base( C2)");
}

@end
