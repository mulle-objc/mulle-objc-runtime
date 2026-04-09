#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <limits.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


struct Tiny
{
   char  x;
};


@interface Foo
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( (struct _mulle_objc_infraclass *) self);
}

+ (void) test:(struct Tiny) t
{
   if( t.x == CHAR_MIN)
      puts( "CHAR_MIN");
   else if( t.x == CHAR_MAX)
      puts( "CHAR_MAX");
   else
      printf( "%d\n", (int) t.x);
}

- (void) test:(struct Tiny) t
{
   if( t.x == CHAR_MIN)
      puts( "CHAR_MIN");
   else if( t.x == CHAR_MAX)
      puts( "CHAR_MAX");
   else
      printf( "%d\n", (int) t.x);
}

@end


int main( int argc, const char * argv[])
{
   Foo         *foo;
   struct Tiny  t;

   foo = mulle_objc_infraclass_alloc_instance( [Foo class]);

   t.x = CHAR_MIN; [Foo test:t];
   t.x = 18;       [Foo test:t];
   t.x = 0;        [Foo test:t];
   t.x = -18;      [Foo test:t];
   t.x = CHAR_MAX; [Foo test:t];

   t.x = CHAR_MIN; [foo test:t];
   t.x = 18;       [foo test:t];
   t.x = 0;        [foo test:t];
   t.x = -18;      [foo test:t];
   t.x = CHAR_MAX; [foo test:t];

   mulle_objc_instance_free( foo);

   return( 0);
}
