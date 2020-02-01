#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


@interface Foo
@end


@implementation Foo

- (void) noValue
{
}

- (id) value
{
   return( 0);
}

- (id) notAValue:(id) param
{
   return( param);
}


- (void) setValue:(id) param
{
}


- (id) setNotAValue:(id) param
{
   return( param);
}

- (void) setNotAValue:(id) param
            notAValue:(id) param2
{
}


- (void) setNoValue
{
}


@end


static void  print_desc( SEL sel)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_descriptor   *desc;

   universe = mulle_objc_global_get_universe( __MULLE_OBJC_UNIVERSEID__);
   desc     = _mulle_objc_universe_lookup_descriptor( universe, sel);

   printf( "%s = (%d) %s%s\n", _mulle_objc_descriptor_get_name( desc),
      _mulle_objc_descriptor_get_methodfamily( desc),
      _mulle_objc_descriptor_is_getter_method( desc) ? " getter" : "",
      _mulle_objc_descriptor_is_setter_method( desc) ? " setter" : "");
}



int  main( void)
{
   print_desc( @selector( noValue));
   print_desc( @selector( value));
   print_desc( @selector( notAValue:));

   print_desc( @selector( setValue:));
   print_desc( @selector( setNotAValue:));
   print_desc( @selector( setNotAValue:notAValue:));
   print_desc( @selector( setNoValue));

   return( 0);
}

