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
- (void) x;
+ (void) x;
@end
@interface X < X>
@end

@class Y;
@protocol Y
@optional
- (void) y;
+ (void) y;
@end
@interface Y < Y> @end


@interface A          - (void) a; + (void) a; @end
@interface B : A      - (void) b; + (void) b; @end
@interface C < X, Y>  - (void) c; + (void) c; @end
@interface D : C      - (void) d; + (void) d; @end

@implementation A     - (void) a {}; + (void) a {}; @end
@implementation B     - (void) b {}; + (void) b {}; @end
@implementation C     - (void) c {}; + (void) c {}; @end
@implementation D     - (void) d {}; + (void) d {}; @end
@implementation X     - (void) x {}; + (void) x {}; @end
@implementation Y     - (void) y {}; + (void) y {}; @end
@implementation D( D) - (void) d {}; + (void) d {}; @end
@implementation D( Z) - (void) z {}; + (void) z {}; @end

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
   long                                 before;

   args = mulle_objc_searcharguments_make_default( methodid);
   _mulle_objc_searcharguments_set_callback( &args, method_printer);
   _mulle_objc_searcharguments_set_userinfo( &args, methods);

   before = ftell( stdout);
   mulle_objc_class_search_method( cls,
                                   &args,
                                   cls->inheritance,
                                   &result);
   if( ftell( stdout) != before)
      mulle_printf( "\n");
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

      mulle_printf( "%s infra:\n", _mulle_objc_infraclass_get_name( infra));

      test( _mulle_objc_infraclass_as_class( infra), (mulle_objc_methodid_t) @selector( a), methods);
      test( _mulle_objc_infraclass_as_class( infra), (mulle_objc_methodid_t) @selector( b), methods);
      test( _mulle_objc_infraclass_as_class( infra), (mulle_objc_methodid_t) @selector( c), methods);
      test( _mulle_objc_infraclass_as_class( infra), (mulle_objc_methodid_t) @selector( d), methods);
      test( _mulle_objc_infraclass_as_class( infra), (mulle_objc_methodid_t) @selector( x), methods);
      test( _mulle_objc_infraclass_as_class( infra), (mulle_objc_methodid_t) @selector( y), methods);
      test( _mulle_objc_infraclass_as_class( infra), (mulle_objc_methodid_t) @selector( z), methods);

      mulle_printf( "%s meta:\n", _mulle_objc_metaclass_get_name( meta));
      test( _mulle_objc_metaclass_as_class( meta), (mulle_objc_methodid_t) @selector( a), methods);
      test( _mulle_objc_metaclass_as_class( meta), (mulle_objc_methodid_t) @selector( b), methods);
      test( _mulle_objc_metaclass_as_class( meta), (mulle_objc_methodid_t) @selector( c), methods);
      test( _mulle_objc_metaclass_as_class( meta), (mulle_objc_methodid_t) @selector( d), methods);
      test( _mulle_objc_metaclass_as_class( meta), (mulle_objc_methodid_t) @selector( x), methods);
      test( _mulle_objc_metaclass_as_class( meta), (mulle_objc_methodid_t) @selector( y), methods);
      test( _mulle_objc_metaclass_as_class( meta), (mulle_objc_methodid_t) @selector( z), methods);
      mulle_printf( "\n");
   }
}


int   main( void)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( MULLE_OBJC_DEFAULTUNIVERSEID);

   test_class( universe, (mulle_objc_classid_t) @selector( A));
   test_class( universe, (mulle_objc_classid_t) @selector( B));
   test_class( universe, (mulle_objc_classid_t) @selector( C));
   test_class( universe, (mulle_objc_classid_t) @selector( D));

   return( 0);
}
