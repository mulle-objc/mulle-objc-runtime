#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


@interface Foo
@end


@implementation Foo

#pragma clang attribute push(__attribute__((annotate("objc_user_0"))), apply_to = objc_method)

- (void) a
{
}

#pragma clang attribute push(__attribute__((annotate("objc_user_1"))), apply_to = objc_method)

- (void) b
{
}

#pragma clang attribute pop

#pragma clang attribute push(__attribute__((annotate("objc_user_2"))), apply_to = objc_method)

- (void) c
{
}

#pragma clang attribute pop
#pragma clang attribute pop

@end


static void  print_desc( SEL sel)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_descriptor   *desc;

   universe = mulle_objc_global_get_universe( __MULLE_OBJC_UNIVERSEID__);
   desc     = _mulle_objc_universe_lookup_descriptor( universe, sel);

   printf( "%s = %d (0x%x)\n",
      _mulle_objc_descriptor_get_name( desc),
      _mulle_objc_descriptor_get_user_bits( desc),
      _mulle_objc_descriptor_get_bits( desc));
}



int  main( void)
{
   print_desc( @selector( a));
   print_desc( @selector( b));
   print_desc( @selector( c));

   return( 0);
}

