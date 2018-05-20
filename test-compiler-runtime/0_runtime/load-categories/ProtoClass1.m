#import "ProtoClass1.h"

#include <stdio.h>

@interface ProtoClass1 < ProtoClass1>
@end


@implementation ProtoClass1

+ (void) load
{
    printf( "ProtoClass1\n");
}
@end
