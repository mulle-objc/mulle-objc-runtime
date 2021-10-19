#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo

+ (char *)  :(id) minimal;
+ (char *) _:(id) minimal;
+ (char *) $:(id) minimal;

@end


@implementation Foo

+ (char *) :(id) minimal
{
   return( mulle_objc_global_lookup_methodname( 0, _cmd));
}

+ (char *) _:(id) minimal;
{
   return( mulle_objc_global_lookup_methodname( 0, _cmd));
}

+ (char *) $:(id) minimal;
{
   return( mulle_objc_global_lookup_methodname( 0, _cmd));
}

@end


main()
{
   printf( "%s = %s\n",
      mulle_objc_global_lookup_methodname( 0, @selector( :)),
      [Foo  :0]);
   printf( "%s = %s\n",
      mulle_objc_global_lookup_methodname( 0, @selector( _:)),
      [Foo _:0]);
   printf( "%s = %s\n",
      mulle_objc_global_lookup_methodname( 0, @selector( $:)),
      [Foo $:0]);
}
