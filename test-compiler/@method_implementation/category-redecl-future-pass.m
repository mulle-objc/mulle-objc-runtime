#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic error "-Wmulle-method-implementation"

//
// Test that a category adopting MulleObjCFuture does not produce a
// "method is already declared in category" warning when another
// category implements the method.
//

@protocol MulleObjCFuture
@end

@interface Foo
@end

@interface Foo( Future) <MulleObjCFuture>
- (int) value;
@end

@interface Foo( Actual)
- (int) value;
@end


@implementation Foo
@end

@implementation Foo( Actual)

- (int) value
{
   return( 48);
}

@end


int   main( void)
{
   printf( "%d\n", 48);
   return( 0);
}
