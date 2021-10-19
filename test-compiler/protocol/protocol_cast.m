#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol  Baz;


@protocol Foo
@end

@interface Foo < Foo>
@end


@implementation Foo

+ (void) initialize
{
   struct _mulle_objc_classpair   *pair;

   // has is shallow, conforms is deep
   pair = _mulle_objc_infraclass_get_classpair( self);
   if( _mulle_objc_classpair_has_protocolid( pair, @protocol( Foo)))
      printf( "OUI\n");
   else
      printf( "NON\n");
}


+ (Class) class
{
   return( self);
}

@end


main()
{
   [Foo class];
   return( 0);
}
