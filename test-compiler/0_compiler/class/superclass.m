#include <mulle-objc-runtime/mulle-objc-runtime.h>


@implementation Foo

+ (id) new
{
   return( (Foo *) mulle_objc_infraclass_alloc_instance( self));
}

- (void) dealloc
{
   _mulle_objc_object_free( self);
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

@implementation Bar : Foo
@end



main()
{
   Bar  *bar;

   [Bar print];

   bar = [Bar new];
   [bar print];
   [bar dealloc];

   return( 0);
}
