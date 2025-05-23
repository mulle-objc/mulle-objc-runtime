#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol  A      // 0x7fc56270e7a70fa8
@end

@protocol  B < A>   // 0x9d5ed678fe57bcca
@end


// the compiler should emit <A, B> here
@interface Foo <B>
@end


@implementation Foo

+ (Class) class
{
  return( self);
}

@end


int   main( void)
{
   Class   cls;

   cls = [Foo class];
   printf( "A: %s\n",
       _mulle_objc_infraclass_conformsto_protocolid( cls,
                                              @protocol( A)) ? "YES" : "NO");
   printf( "B: %s\n",
       _mulle_objc_infraclass_conformsto_protocolid( cls,
                                              @protocol( B)) ? "YES" : "NO");
   return( 0);
}
