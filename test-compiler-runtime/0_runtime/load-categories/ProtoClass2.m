#import "ProtoClass2.h"

#include <stdio.h>

@interface ProtoClass2 < ProtoClass2>
@end


@implementation ProtoClass2

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "ProtoClass2");
}
@end
