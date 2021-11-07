#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

typedef struct
{
   float  x;
   float  y;
   float  w;
   float  h;
} rect;

static inline rect  MakeRect( float x, float y, float w, float h)
{
   return( (rect){ .x = x, .y = y, .w = w, .h = h });
}


@interface Bar

@property( assign)             int   a;
@property( assign, observable) int   b;
@property( assign, observable) rect  c;

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
   Bar     *obj;
   rect    c;

   cls = [Bar class];

   obj = mulle_objc_infraclass_alloc_instance( cls);
   printf( "a:");
   [obj setA:18];
   printf( "\na = %d\nb:", [obj a]);
   [obj setB:48];
   printf( "b = %d\nc:", [obj b]);
   [obj setC:MakeRect( 1, 2, 3, 4)];
   c = [obj c];
   printf( "c = { %g, %g, %g, %g }\n", c.x, c.y, c.w, c.h);

   mulle_objc_instance_free( obj);

   return( 0);
}
