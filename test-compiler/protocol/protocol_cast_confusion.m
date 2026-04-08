#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

@protocol  Baz;


@protocol X
@end

@interface A
@end

@implementation A
@end

@interface B : A
@end

@implementation B
@end

@interface B( X) < X>
@end

@implementation B( X)
@end

@interface C
@property( assign) A <X>  *other;
@end

@implementation C
@end


// should just compile cleanly

int   main( void)
{
   C   *c = NULL;
   B   *b = NULL;

   [c setOther:b];
   return( 0);
}
