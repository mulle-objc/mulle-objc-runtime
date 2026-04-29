#import "Foo.h"

#include <stdio.h>


@protocol_class ProtoClass2;

@interface Foo( C3) < ProtoClass2>
@end

@implementation Foo( C3)

+ (void) load
{
   void  add_to_loaded( char *s);

   add_to_loaded( "Foo( C3)");
}

@end
