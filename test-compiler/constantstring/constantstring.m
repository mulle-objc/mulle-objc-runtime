// Hackery for low level test don't do this otherwise
#ifdef __MULLE_OBJC_TPS__
# undef __MULLE_OBJC_TPS__
#endif
#define __MULLE_OBJC_NO_TPS__   1


#include <mulle-objc-runtime/mulle-objc-runtime.h>


// no support for the package keyword
@interface SomeString
{
   char  *_s;
   int   _len;
}
@end

SomeString  *foo = @"foo";
SomeString  *bar = @"bar";


@implementation SomeString

+ (Class) class
{
   return( self);
}


- (void) print
{
   if( self == foo)
      printf( "foo: ");
   if( self == bar)
      printf( "bar: ");

   printf( "%d: %s\n", self->_len, self->_s);
}

@end


int   main( void)
{
   struct _mulle_objc_universe    *universe;

   universe = mulle_objc_global_get_universe( __MULLE_OBJC_UNIVERSEID__);
   _mulle_objc_universe_set_staticstringclass( universe, [SomeString class], 1);

   [foo print];
   [bar print];
   [@"foo" print];

   return( 0);
}
