#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


struct Medium
{
   char   a;
   short  b;
   int    c;
   float  d;
};


@interface Foo
@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( (struct _mulle_objc_infraclass *) self);
}

+ (void) test:(struct Medium) m
{
   printf( "%d,%d,%d,%f\n", (int) m.a, (int) m.b, m.c, m.d);
}

@end


int main( int argc, const char * argv[])
{
   struct Medium  m;

   m.a = 18;  m.b = 1848;  m.c = 1848;  m.d = 18.48f;
   [Foo test:m];

   m.a = 36;  m.b = 3696;  m.c = 3696;  m.d = 36.96f;
   [Foo test:m];

   m.a = 18;  m.b = 1848;  m.c = 1848;  m.d = 18.48f;
   [Foo test:m];

   return( 0);
}
