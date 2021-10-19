#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Bar

@property( relationship, assign) id  a;
@property( getter=b, assign) id  b;

@end


@implementation Bar

+ (Class) class
{
   return( self);
}

- (id) b
{
   mulle_objc_object_will_read_relationship( self, (char *) &self->_a - (char *) self);
   return( _a);
}


- (id) willReadRelationship:(id) value
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   return( (id ) ((intptr_t) value + 1));
}

@end


main()
{
   Class   cls;
   id      obj;

   cls = [Bar class];

   obj = mulle_objc_infraclass_alloc_instance( cls);

   printf( "a: %d\n", (int) (intptr_t) [obj a]);
   printf( "a: %d\n", (int) (intptr_t) [obj a]);
   printf( "a: %d\n", (int) (intptr_t) [obj a]);

   [obj setA:(id) 0];

   printf( "b: %d\n", (int) (intptr_t) [obj b]);
   printf( "b: %d\n", (int) (intptr_t) [obj b]);
   printf( "b: %d\n", (int) (intptr_t) [obj b]);

   mulle_objc_instance_free( obj);

   return( 0);
}
