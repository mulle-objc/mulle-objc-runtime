//
// Cat2 declares +dependencies but lists Cat3 (a different category)
// instead of Cat1 which it actually overrides. The check should detect this.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>


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


@interface A( Cat3)
@end

@implementation A( Cat3)
- (void) bar
{
}
@end


@interface A( Cat2)
@end

@implementation A( Cat2)
+ (struct _mulle_objc_dependency *) dependencies
{
   static struct _mulle_objc_dependency  dependencies[] =
   {
      // wrong: depends on Cat3 instead of Cat1
      { @selector( A), @selector( Cat3) },
      0
   };
   return( dependencies);
}

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
      mulle_printf( "FAIL: should have detected wrong dependency\n");
      return( 1);
   }
   mulle_printf( "PASS\n");
   return( 0);
}
