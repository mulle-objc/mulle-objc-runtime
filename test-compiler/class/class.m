#ifdef __MULLE_OBJC__
#include <mulle-objc-runtime/mulle-objc-runtime.h>
#endif

#include <stdio.h>

@implementation Foo

+ (id) new
{
   return( (Foo *) mulle_objc_infraclass_alloc_instance( self));
}

- (void) dealloc
{
   _mulle_objc_instance_free( self);
}


+ (void) print
{
   printf( "+Foo\n");
}


- (void) print
{
   printf( "-Foo\n");
}

@end


main()
{
   Foo  *foo;

   [Foo print];

   foo = [Foo new];
   [foo print];
   [foo dealloc];

   return( 0);
}
