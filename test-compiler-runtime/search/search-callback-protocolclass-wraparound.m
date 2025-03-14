#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif


_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wprotocol\"")
_Pragma("clang diagnostic ignored \"-Wobjc-root-class\"")
_Pragma("clang diagnostic ignored \"-Wobjc-missing-super-calls\"")

@class X;
@protocol X
@optional
+ (void) method;
- (void) method;
@end
@interface X < X>
@end

@interface C < X>     - (void) method; + (void) method; @end


@implementation C     - (void) method {}; + (void) method {}; @end
@implementation X     - (void) method {}; + (void) method {}; @end

static int   method_printer( struct _mulle_objc_class *cls,
                             struct _mulle_objc_searcharguments *search,
                             unsigned int inheritance,
                             struct _mulle_objc_searchresult *result)
{
   struct mulle__pointermap   *methods;
   intptr_t                  count;
   intptr_t                  index;

   methods = _mulle_objc_searcharguments_get_userinfo( search);
   count   = (intptr_t) mulle__pointermap_get_count( methods) + 1;
   index   = (intptr_t) mulle__pointermap_register( methods, result->method, (void *) count, NULL);
   mulle_printf( "\t%td: %c[%s",
                 index,
                 _mulle_objc_class_is_metaclass( result->class) ? '+' : '-',
                 _mulle_objc_class_get_name( result->class),
                 _mulle_objc_method_get_name( result->method));
   if( result->list->loadcategory)
      mulle_printf( "( %s)",
         _mulle_objc_methodlist_get_categoryname( result->list));
   mulle_printf( " %s]\n", _mulle_objc_method_get_name( result->method));

   return( mulle_objc_walk_ok);
}


static void   test( struct _mulle_objc_class *cls,
                    mulle_objc_methodid_t methodid,
                    struct mulle__pointermap *methods)
{
   struct _mulle_objc_searcharguments   args;
   struct _mulle_objc_searchresult      result;

   args = mulle_objc_searcharguments_make_default( methodid);
   _mulle_objc_searcharguments_set_callback( &args, method_printer);
   _mulle_objc_searcharguments_set_userinfo( &args, methods);

   mulle_objc_class_search_method( cls,
                                   &args,
                                   cls->inheritance,
                                   &result);
}


static void   test_class( struct _mulle_objc_universe *universe,
                          mulle_objc_classid_t classid)
{
   // translate addresses which are bad to diff, into indexes which are
   // stable
   mulle__pointermap_do( methods)
   {
      struct _mulle_objc_infraclass   *infra;
      struct _mulle_objc_metaclass    *meta;

      infra = _mulle_objc_universe_lookup_infraclass( universe, classid);
      meta  = _mulle_objc_infraclass_get_metaclass( infra);

      mulle_printf( "%s meta:\n", _mulle_objc_metaclass_get_name( meta));
      test( _mulle_objc_metaclass_as_class( meta), (mulle_objc_methodid_t) @selector( method), methods);

      mulle_printf( "%s infra:\n", _mulle_objc_infraclass_get_name( infra));

      test( _mulle_objc_infraclass_as_class( infra), (mulle_objc_methodid_t) @selector( method), methods);
      mulle_printf( "\n");
   }
}


int   main( void)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( MULLE_OBJC_DEFAULTUNIVERSEID);

   test_class( universe, (mulle_objc_classid_t) @selector( C));

   return( 0);
}
