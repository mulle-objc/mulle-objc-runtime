#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol Bar
@end

@interface Bar
@end

@interface Foo < Bar>
@end

@interface Foobar : Foo
@end

@interface Foobar( Whatevs)
@end


@implementation Foobar
@end

@implementation Foobar( Whatevs)
@end

@implementation Foo
@end

@implementation Bar
@end


main()
{
   return( 0);
}
