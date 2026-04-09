#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Foo
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( (struct _mulle_objc_infraclass *) self);
}

+ (void) test:(float) x
{
   if( x == FLT_MIN)
      puts( "FLT_MIN");
   else if( x == FLT_MAX)
      puts( "FLT_MAX");
   else
      printf( "%f\n", x);
}

- (void) test:(float) x
{
   if( x == FLT_MIN)
      puts( "FLT_MIN");
   else if( x == FLT_MAX)
      puts( "FLT_MAX");
   else
      printf( "%f\n", x);
}

@end


int main( int argc, const char * argv[])
{
   Foo  *foo;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);

   [Foo test:FLT_MIN];
   [Foo test:18.48f];
   [Foo test:0.0f];
   [Foo test:-18.48f];
   [Foo test:FLT_MIN];

   [foo test:FLT_MIN];
   [foo test:18.48f];
   [foo test:0.0f];
   [foo test:-18.48f];
   [foo test:FLT_MIN];

   mulle_objc_instance_free( foo);

   return( 0);
}
