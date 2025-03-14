#include <mulle-objc-runtime/mulle-objc-runtime.h>

@class A;
@protocol A
@optional
+ (void) print;
@end
@interface A < A>
@end
@implementation A
+ (void) print
{
   printf( "%s\n", __FUNCTION__);
}
@end

@class B;
@protocol B
@optional
+ (void) print;
@end
@interface B < B>
@end
@implementation B
+ (void) print
{
   printf( "%s\n", __FUNCTION__);
}
@end


@interface Foo <A, B>
@end

@implementation Foo
@end

@interface Bar <B, A>
@end
@implementation Bar
@end


int   main( void)
{
   [Foo print];   // B -> A -> Foo
   [Bar print];   // A -> B -> Bar
   return( 0);
}
