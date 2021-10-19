#include <mulle-objc-runtime/mulle-objc-runtime.h>

// use mulle_objc_method_call
#define __MULLE_OBJC_INLINE_METHOD_CALLS__ 0


@interface Foo

+ (id) a:(id) a
       b:(id) b;
- (void) c:(id) a
         d:(id) b;
@end


@implementation Foo

- (id) a:(id) a
       b:(id) b
{
   return( self);
}

- (void) c:(id) a
         d:(id) b
{
}


@end

//
// compile with several inlining CFLAGS
// size x86_64 with mulle-clang 12.0.0.1 (no optimize reuse): 92 bytes
//
int   main( void)
{
   id   foo;

   foo = [Foo a:(id) 0 b:(id) 1];
   [foo a:(id) 2 b:(id) 3];

   return( 0);
}

