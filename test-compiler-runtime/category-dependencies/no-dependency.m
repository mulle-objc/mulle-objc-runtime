//
// Two categories override the same method. Cat2 overrides Cat1 but
// Cat2 does NOT declare +dependencies. The check should detect this.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface A
@end

@implementation A
- (void) foo
{
}
@end


@interface A( Cat1)
@end

@implementation A( Cat1)
- (void) foo
{
}
@end


@interface A( Cat2)
@end

@implementation A( Cat2)
// no +dependencies declared!
- (void) foo
{
}
@end


int   main( void)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe( MULLE_OBJC_DEFAULTUNIVERSEID);
   if( _mulle_objc_universe_check_category_method_dependencies( universe)
          == mulle_objc_universe_is_ok)
   {
      mulle_printf( "FAIL: should have detected missing dependency\n");
      return( 1);
   }
   mulle_printf( "PASS\n");
   return( 0);
}
