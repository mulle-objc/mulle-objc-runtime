#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wreceiver-expr"

//
// Regression: protocol instance properties must NOT appear in classproperties.
// Bug was: PushProtocolProperties added all protocol properties to the class
// property list regardless of IsClassProperty, causing +setTarget: to be
// looked up as a class method at teardown.
//
@protocol HasTarget
@property (assign) id target;
@end

@interface Widget <HasTarget>
{
   id   _target;
}
@property (class, assign) int refCount;
@end

@implementation Widget

- (id) target       { return _target; }
- (void) setTarget:(id) t { _target = t; }

+ (void) bump
{
   self->_refCount = self->_refCount + 1;
}
@end

int   main( void)
{
   struct _mulle_objc_infraclass   *cls;
   void                            *obj;

   cls = mulle_objc_global_lookup_infraclass_nofail( MULLE_OBJC_DEFAULTUNIVERSEID,
                                                     mulle_objc_classid_from_string( "Widget"));
   obj = mulle_objc_infraclass_alloc_instance( cls);

   // instance property works
   [(id) obj setTarget:(id) 0x42];
   if( [(id) obj target] != (id) 0x42)
   {
      printf( "FAIL: instance target\n");
      return( 1);
   }

   // class property works
   [Widget bump];
   if( [Widget refCount] != 1)
   {
      printf( "FAIL: class refCount\n");
      return( 1);
   }

   mulle_objc_instance_free( obj);
   return( 0);
}
