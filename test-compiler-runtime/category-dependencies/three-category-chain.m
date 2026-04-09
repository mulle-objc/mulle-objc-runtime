//
// Three categories form a proper chain: Cat3 -> Cat2 -> Cat1.
// Each overrider depends on the previous. Should pass.
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
+ (struct _mulle_objc_dependency *) dependencies
{
   static struct _mulle_objc_dependency  dependencies[] =
   {
      { @selector( A), @selector( Cat1) },
      0
   };
   return( dependencies);
}

- (void) foo
{
}
@end


@interface A( Cat3)
@end

@implementation A( Cat3)
+ (struct _mulle_objc_dependency *) dependencies
{
   static struct _mulle_objc_dependency  dependencies[] =
   {
      { @selector( A), @selector( Cat2) },
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
          != mulle_objc_universe_is_ok)
   {
      mulle_printf( "FAIL: chain should be valid\n");
      return( 1);
   }
   mulle_printf( "PASS\n");
   return( 0);
}
