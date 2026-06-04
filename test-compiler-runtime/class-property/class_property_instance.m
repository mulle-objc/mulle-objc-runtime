#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wreceiver-expr"

//
// Test multiple class properties and accessor access from instance methods
//
@interface Bar
@property (class, assign) int   count;
@property (class, assign) int   flag;
@end

@implementation Bar

+ (void) reset
{
   self->_count = 0;
   self->_flag  = 0;
}

- (void) bumpCount
{
   [Bar setCount:[Bar count] + 1];
}

- (void) setFlagOn
{
   [Bar setFlag:1];
}

@end

int   main( void)
{
   struct _mulle_objc_infraclass   *cls;
   void                            *obj;

   cls = mulle_objc_global_lookup_infraclass_nofail( MULLE_OBJC_DEFAULTUNIVERSEID,
                                                     mulle_objc_classid_from_string( "Bar"));
   obj = mulle_objc_infraclass_alloc_instance( cls);

   [Bar reset];

   if( [Bar count] != 0 || [Bar flag] != 0)
   {
      printf( "FAIL: initial values not zero\n");
      return( 1);
   }

   [(id) obj bumpCount];
   [(id) obj bumpCount];
   [(id) obj setFlagOn];

   if( [Bar count] != 2)
   {
      printf( "FAIL: count != 2\n");
      return( 1);
   }
   if( [Bar flag] != 1)
   {
      printf( "FAIL: flag != 1\n");
      return( 1);
   }

   mulle_objc_instance_free( obj);
   return( 0);
}
