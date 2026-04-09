#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@protocol X
@end

// class A
@interface A
@end

@implementation A
@end


// class B
@interface B : A
@end

@implementation B
@end

// category B ( X) implements X
@interface B( X) < X>
@end

@implementation B( X)
@end

// class C has a property for A( X), so assigning from B should be fine
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

   // amusingly, if you change 'c' to 'C' you get a hard to figure out bug
   // because setOther: wraps around to the root metaclass and then clobbers
   // the imp cache. Its hard to figure out how to assert against this or how
   // the compiler should warn.
   
   [c setOther:b];
   return( 0);
}
