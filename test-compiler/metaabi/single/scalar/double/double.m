#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <float.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Foo
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( (struct _mulle_objc_infraclass *) self);
}

+ (void) test:(double) x
{
   if( x == DBL_MIN)
      puts( "DBL_MIN");
   else if( x == DBL_MAX)
      puts( "DBL_MAX");
   else
      printf( "%f\n", x);
}

- (void) test:(double) x
{
   if( x == DBL_MIN)
      puts( "DBL_MIN");
   else if( x == DBL_MAX)
      puts( "DBL_MAX");
   else
      printf( "%f\n", x);
}

@end


int main( int argc, const char * argv[])
{
   Foo  *foo;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);

   [Foo test:DBL_MIN];
   [Foo test:18.48];
   [Foo test:0.0];
   [Foo test:-18.48];
   [Foo test:DBL_MAX];

   [foo test:DBL_MIN];
   [foo test:18.48];
   [foo test:0.0];
   [foo test:-18.48];
   [foo test:DBL_MAX];

   mulle_objc_instance_free( foo);

   return( 0);
}
