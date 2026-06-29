#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic error "-Wobjc-property-implementation"

//
// Test that @property(forward) in a category does not produce a
// "property requires method to be defined" warning.
//

@protocol MulleObjCForwarding
@end

@interface Foo
@end

@interface Foo( Forwarding) <MulleObjCForwarding>

@property( forward, assign) int   value;
@property( forward, assign) int   anotherValue;

@end


@implementation Foo
@end

@implementation Foo( Forwarding)

- (int) value
{
   return( 18);
}

// anotherValue getter and setter intentionally not implemented
// forward property should suppress the warning

@end


int   main( void)
{
   printf( "%d\n", 18);
   return( 0);
}
