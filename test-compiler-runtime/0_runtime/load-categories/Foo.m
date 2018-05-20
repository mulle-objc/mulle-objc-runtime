#import "Foo.h"

#include <stdio.h>


@implementation Foo

+ (void) load
{
    printf( "Foo\n");
}
@end
