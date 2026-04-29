#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol_class A;
@protocol_interface A
@optional
+ (void) print;
@end

@protocol_implementation A
+ (void) print
{
   printf( "%s\n", __FUNCTION__);
}
@end

@protocol_class B;
@protocol_interface B
@optional
+ (void) print;
@end

@protocol_implementation B
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
