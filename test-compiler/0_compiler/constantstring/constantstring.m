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


main()
{
   struct _mulle_objc_universe   *universe;


   universe = mulle_objc_get_universe();

   _mulle_objc_universe_set_staticstringclass( universe, [SomeString class]);

   mulle_objc_dotdump_universe_to_tmp();

   [foo print];
   [bar print];
   [@"foo" print];

   return( 0);
}
