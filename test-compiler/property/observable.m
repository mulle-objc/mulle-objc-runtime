#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Bar

@property( assign)             int  a;
@property( assign, observable) int  b;

@end


@implementation Bar

+ (Class) class
{
   return( self);
}

- (void) willChange
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end



main()
{
   Class   cls;
   id      obj;

   cls = [Bar class];

   obj = mulle_objc_infraclass_alloc_instance( cls);
   printf( "a:");
   [obj setA:18];
   printf( "\nb:");
   [obj setB:48];

   mulle_objc_instance_free( obj);

   return( 0);
}
