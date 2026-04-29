#include <mulle-objc-runtime/mulle-objc-runtime.h>


// MEMO: not sure why this test is called protocol_cast ???

@protocol Foo
@end

// not a protocolclass anymore
@interface Foo < Foo>
@end


@implementation Foo

+ (void) initialize
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_infraclass_get_classpair( (struct _mulle_objc_infraclass *) self);
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


int   main( void)
{
   [Foo class];
   return( 0);
}
