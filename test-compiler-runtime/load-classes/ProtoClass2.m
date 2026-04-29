#import "ProtoClass2.h"

#include <stdio.h>


@protocol_implementation ProtoClass2

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "ProtoClass2");
}
@end
