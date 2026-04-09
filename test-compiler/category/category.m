#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Foo
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

@end


@implementation Foo( Category1)

- (void) printA
{
	printf( "A\n");
}

+ (void) printA
{
	printf( "+A\n");
}

@end


@implementation Foo( Category2)

- (void) printB
{
	printf( "B\n");
}


+ (void) printB
{
	printf( "+B\n");
}

@end


int   main( void)
{
   Foo  *foo;

   [Foo printA];
   [Foo printB];

   foo = [Foo new];

   [foo printA];
   [foo printB];

   [foo dealloc];
   return( 0);
}
