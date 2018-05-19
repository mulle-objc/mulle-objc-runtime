#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo

+ (char *)  :(id) minimal;
+ (char *) _:(id) minimal;
+ (char *) $:(id) minimal;

@end


@implementation Foo

+ (char *) :(id) minimal
{
   return( mulle_objc_lookup_methodname( _cmd));
}

+ (char *) _:(id) minimal;
{
   return( mulle_objc_lookup_methodname( _cmd));
}

+ (char *) $:(id) minimal;
{
   return( mulle_objc_lookup_methodname( _cmd));
}

@end


main()
{
   printf( "%s = %s\n",
      mulle_objc_lookup_methodname( @selector( :)),
      [Foo  :0]);
   printf( "%s = %s\n",
      mulle_objc_lookup_methodname( @selector( _:)),
      [Foo _:0]);
   printf( "%s = %s\n",
      mulle_objc_lookup_methodname( @selector( $:)),
      [Foo $:0]);
}
