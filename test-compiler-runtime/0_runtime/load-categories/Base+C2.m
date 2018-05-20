#import "Base.h"

#include <stdio.h>


@interface Base( C2)
@end

@implementation Base( C2)

+ (void) load
{
    printf( "Base( C2)\n");
}

@end
