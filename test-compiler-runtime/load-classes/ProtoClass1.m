#import "ProtoClass1.h"

#include <stdio.h>

@interface ProtoClass1 < ProtoClass1>
@end


@implementation ProtoClass1

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "ProtoClass1");
}
@end
