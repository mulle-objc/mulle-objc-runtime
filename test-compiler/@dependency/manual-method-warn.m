#include <mulle-objc-runtime/mulle-objc-runtime.h>

// manual +dependencies method alongside @dependency: warn, manual wins
@interface Foo
+ (void *)dependencies;
@end

@implementation Foo
@dependency Local;

+ (void *)dependencies
{
   return( 0);
}
@end

int main(void) { return 0; }
