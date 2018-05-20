#import "ProtoClass2.h"

#include <stdio.h>

@interface ProtoClass2 < ProtoClass2>
@end


@implementation ProtoClass2

+ (void) load
{
    printf( "ProtoClass2\n");
}
@end
