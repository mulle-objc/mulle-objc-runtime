#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

//
// Test: @optional in @interface suppresses missing-method warnings.
// @optional properties must be dynamic or forward.
// @required methods still need implementation.
//
@interface Foo

- (void) mustImplement;

@optional
- (void) forwarded;
- (int) alsoForwarded;
@property (dynamic) int optionalProp;

@required
- (int) alsoMustImplement;

@end

@implementation Foo
- (void) mustImplement {}
- (int) alsoMustImplement { return 42; }
@end

int   main( void)
{
   struct _mulle_objc_infraclass  *cls;
   void                           *obj;

   cls = mulle_objc_global_lookup_infraclass_nofail( MULLE_OBJC_DEFAULTUNIVERSEID,
                                                     mulle_objc_classid_from_string( "Foo"));
   obj = mulle_objc_infraclass_alloc_instance( cls);

   if( [(id) obj alsoMustImplement] != 42)
   {
      printf( "FAIL\n");
      return( 1);
   }

   mulle_objc_instance_free( obj);
   printf( "passed\n");
   return( 0);
}
