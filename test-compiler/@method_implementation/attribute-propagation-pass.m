//
// Test that @method_implementation inherits method attributes from
// protocol declarations. The user_bit for objc_user_4 (which is used
// as MULLE_OBJC_THREADSAFE_METHOD) must be set in the method descriptor
// when @method_implementation is used for a method declared with that
// attribute in a protocol.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

typedef struct objc_selector *SEL;
typedef struct objc_object   *id;


@protocol Attributed

- (void *) threadsafe __attribute__((annotate("objc_user_4")));
- (void *) plain;

@end


void  *my_imp( id self, SEL _cmd, void *_param)
{
   return( NULL);
}


@interface Foo <Attributed>
@end


@implementation Foo

// threadsafe is declared with objc_user_4 in the protocol
// @method_implementation should propagate that attribute
@method_implementation -threadsafe = my_imp;
@method_implementation -plain      = my_imp;

@end


static void  print_desc( SEL sel)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_descriptor   *desc;

   universe = mulle_objc_global_get_universe( __MULLE_OBJC_UNIVERSEID__);
   desc     = _mulle_objc_universe_lookup_descriptor( universe, sel);

   printf( "%s user_bits=%d\n",
      _mulle_objc_descriptor_get_name( desc),
      _mulle_objc_descriptor_get_user_bits( desc));
}


int  main( void)
{
   print_desc( @selector( threadsafe));
   print_desc( @selector( plain));

   return( 0);
}
