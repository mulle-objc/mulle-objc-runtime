#include <mulle-objc-runtime/mulle-objc-runtime.h>

@interface Bar
{
   int  x;
}

@end


@implementation Bar

- (void) takeBarDefs:(struct { @defs( Bar); }*) defs
{
   self->x = defs->x;
}

@end


@interface Foo : Bar
{
   int y;
}

@property int   z;

@end


@implementation Foo

+ (id) new
{
   return( (Foo *) mulle_objc_infraclass_alloc_instance( self));
}


- (void) dealloc
{
   _mulle_objc_instance_free( self);
}


- (void) takeFooDefs:(struct { @defs( Foo); } *) defs
{
   [super takeBarDefs:(struct { @defs( Bar); } *) defs];

   self->y = defs->y;
   [self setZ:defs->_z];
}

@end


int   main( void)
{
   Foo  *foo;

   foo = [Foo new];
   [foo takeFooDefs:&( struct { @defs( Foo); }){ .x = 1, .y = 2, ._z = 3 }];
   [foo dealloc];
   return( 0);
}
