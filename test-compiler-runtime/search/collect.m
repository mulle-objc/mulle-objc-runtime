#import <mulle-objc-runtime/mulle-objc-runtime.h>


@interface A
@end

// @protocolclass P
@class P;
@protocol P
@end
@interface P < P>
@end

// @protocolclass Q
@class Q;
@protocol Q
@end
@interface Q < Q>
@end

@interface B : A < P, Q>
@end


@interface A( X)
@end
@interface A( Y)
@end


@interface B( X)
@end
@interface B( Y)
@end


@implementation A
+ (id) new
{
   return( (id) _mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self));
}
+ (Class) class
{
   return( self);
}
- (void) dealloc
{
   _mulle_objc_instance_free( self);
}
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end


@implementation P
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end

@implementation Q
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end

@implementation B
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end
@implementation B( C)
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end


@implementation A( X)
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end
@implementation A( Y)
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end

@implementation B( X)
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end
@implementation B( Y)
- (void) foo
{
   mulle_printf( "%s\n", __FUNCTION__);
}
@end



static int   method_collector( struct _mulle_objc_class *cls,
                               struct _mulle_objc_searcharguments *search,
                               unsigned int inheritance,
                               struct _mulle_objc_searchresult *result)
{
   struct mulle_pointerarray *methods;

   methods = _mulle_objc_searcharguments_get_userinfo( search);
   mulle_pointerarray_add( methods, result->method);
   return( mulle_objc_walk_ok);
}



static void   test_collect( id obj,
                            SEL methodsel,
                            unsigned int inheritance,
                            mulle_objc_classid_t stop_classid)

{
   struct _mulle_objc_searcharguments   args;
   struct _mulle_objc_searchresult      result;
   struct _mulle_objc_method            *method;
   struct _mulle_objc_class             *cls;
   mulle_objc_implementation_t          imp;

   mulle_pointerarray_do_flexible( methods, 32)
   {
      // find the first method, if we find none we are done
      args = mulle_objc_searcharguments_make_default( methodsel);
      _mulle_objc_searcharguments_set_stop_classid( &args, stop_classid);
      _mulle_objc_searcharguments_set_callback( &args, method_collector);
      _mulle_objc_searcharguments_set_userinfo( &args, methods);

      cls = _mulle_objc_object_get_isa( obj);
      mulle_objc_class_search_method( cls,
                                      &args,
                                      inheritance,
                                      &result);

      // now we have all the methods collected
      mulle_pointerarray_for( methods, method)
      {
         imp = _mulle_objc_method_get_implementation( method);
         mulle_objc_implementation_invoke( imp, obj, methodsel, obj);
      }
   }
   mulle_printf( "\n");
}


int   main()
{
   B   *b;

   b = [B new];

   test_collect( b, @selector( foo), MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES, @selector( A));
   test_collect( b, @selector( foo), MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS, @selector( A));
   test_collect( b, @selector( foo), MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES, @selector( A));

   test_collect( b, @selector( foo), MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES, 0);
   test_collect( b, @selector( foo), MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS, 0);
   test_collect( b, @selector( foo), MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES, 0);

   [b dealloc];
}