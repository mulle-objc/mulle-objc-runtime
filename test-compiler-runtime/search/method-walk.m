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


static int  method_printer( struct _mulle_objc_method *method,
                            struct _mulle_objc_methodlist *list,
                            struct _mulle_objc_class *cls,
                            void *info)
{
   struct mulle__pointermap   *methods = info;
   intptr_t                  count;
   intptr_t                  index;

   count = (intptr_t) mulle__pointermap_get_count( methods) + 1;
   index = (intptr_t) mulle__pointermap_register( methods, method, (void *) count, NULL);
   mulle_printf( "\t%td: %c[%s",
                 index,
                 _mulle_objc_class_is_metaclass( cls) ? '+' : '-',
                 _mulle_objc_class_get_name( cls),
                 _mulle_objc_method_get_name( method));
   if( list->loadcategory)
      mulle_printf( "( %s)",
         _mulle_objc_methodlist_get_categoryname( list));
   mulle_printf( " %s]\n", _mulle_objc_method_get_name( method));

   return( mulle_objc_walk_ok);
}


static void   test( struct _mulle_objc_class *cls,
                    struct mulle__pointermap *methods)
{
   _mulle_objc_class_walk_methods( cls,
                                   cls->inheritance,
                                   method_printer,
                                   methods);
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

      test( _mulle_objc_infraclass_as_class( infra), methods);

      mulle_printf( "%s meta:\n", _mulle_objc_metaclass_get_name( meta));
      test( _mulle_objc_metaclass_as_class( meta), methods);
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
