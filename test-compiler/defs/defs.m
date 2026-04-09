#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

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
   return( (Foo *) mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self));
}


- (void) dealloc
{
   _mulle_objc_instance_free( self);
}


- (void) takeFooDefs:(struct { @defs( Foo); } *) defs
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
   [super takeBarDefs:defs];
#pragma clang diagnostic pop

   self->y = defs->y;
   [self setZ:defs->_z];
}

@end


int   main( void)
{
   Foo  *foo;

   foo = [Foo new];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
   [foo takeFooDefs:(struct { @defs( Foo); } *)(void *) &( struct { @defs( Foo); }){ .x = 1, .y = 2, ._z = 3 }];
#pragma clang diagnostic pop
   [foo dealloc];
   return( 0);
}
