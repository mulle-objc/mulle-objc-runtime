#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

// Test protocols and classes for preload map generation
@protocol TestProtocol
+ (void) protocolClassMethod;
- (void) protocolInstanceMethod;
@end

@protocol SecondProtocol  
+ (void) secondProtocolClassMethod;
- (void) secondProtocolInstanceMethod;
@end

// Forward declarations for protocolclasses
@class SecondProtocol;

// Root class
@interface RootClass
+ (void) rootClassMethod;
- (void) rootInstanceMethod;
@end

// Base class
@interface BaseClass : RootClass <TestProtocol>
+ (void) baseClassMethod;
- (void) baseInstanceMethod;
@end

// Protocolclass
@interface SecondProtocol <SecondProtocol>
@end

// Derived class using protocolclass
@interface DerivedClass : BaseClass <SecondProtocol>
+ (void) derivedClassMethod;
- (void) derivedInstanceMethod;
@end

// Implementations
@implementation RootClass
+ (id) new {
   return (RootClass *) mulle_objc_infraclass_alloc_instance(self);
}
- (void) dealloc {
   _mulle_objc_instance_free(self);
}
+ (void) rootClassMethod { printf( "%s\n", __FUNCTION__); }
- (void) rootInstanceMethod { printf( "%s\n", __FUNCTION__); }
@end

@implementation BaseClass
+ (void) baseClassMethod
{
   printf( "%s\n", __FUNCTION__);
   [super rootClassMethod];  // Super call to RootClass
}
- (void) baseInstanceMethod
{
   printf( "%s\n", __FUNCTION__);
   [super rootInstanceMethod];  // Super call to RootClass
}
+ (void) protocolClassMethod { printf( "%s\n", __FUNCTION__); }
- (void) protocolInstanceMethod { printf( "%s\n", __FUNCTION__); }
@end

@implementation SecondProtocol
+ (void) secondProtocolClassMethod { printf( "%s\n", __FUNCTION__); }
- (void) secondProtocolInstanceMethod { printf( "%s\n", __FUNCTION__); }
@end

@implementation DerivedClass
+ (void) derivedClassMethod
{
   printf( "%s\n", __FUNCTION__);
   [super baseClassMethod];  // Super call to BaseClass
}
- (void) derivedInstanceMethod
{
   printf( "%s\n", __FUNCTION__);
   [super baseInstanceMethod];  // Super call to BaseClass
}
@end

// Separate class hierarchy that should NOT be included in preload filtering
@interface UnrelatedRoot
+ (void) unrelatedRootClassMethod;
- (void) unrelatedRootInstanceMethod;
@end

@interface UnrelatedChild : UnrelatedRoot
+ (void) unrelatedChildClassMethod;
- (void) unrelatedChildInstanceMethod;
@end

@implementation UnrelatedRoot
+ (id) new
{
   return( (UnrelatedRoot *) mulle_objc_infraclass_alloc_instance( self));
}
- (void) dealloc
{
   _mulle_objc_instance_free( self);
}
+ (void) unrelatedRootClassMethod { }
- (void) unrelatedRootInstanceMethod { }
@end

@implementation UnrelatedChild
+ (void) unrelatedChildClassMethod
{
   [super unrelatedRootClassMethod];  // Super call that should NOT be collected
}
- (void) unrelatedChildInstanceMethod
{
   [super unrelatedRootInstanceMethod];  // Super call that should NOT be collected
}
@end

// Simple comparison function for sorting pointers as integers
static int pointer_compare( void *a, void *b, void *userinfo)
{
   uintptr_t pa = *(uintptr_t *) a;
   uintptr_t pb = *(uintptr_t *) b;
   
   if( pa < pb)
      return( -1);
   if( pa > pb)
      return( 1);
   return( 0);
}

// Test function to generate preload map
static int   _mulle_objc_class_should_build_preload_map( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;
   
   universe = _mulle_objc_class_get_universe( cls);
   
   if( universe->config.preload_all_methods)
      return( 1);
      
   if( _mulle_objc_class_count_preloadmethods( cls) == 0)
      return( 0);
      
   return( 1);
}

// Callback context for collecting preload methods
struct preload_collect_context
{
   struct mulle__pointermap        *preload_map;
   struct _mulle_objc_class        *original_cls;
   int                             count;
};

static mulle_objc_walkcommand_t   collect_preload_methods( struct _mulle_objc_class *cls,
                                                           struct _mulle_objc_methodlist *list,
                                                           void *userinfo)
{
   struct preload_collect_context           *context;
   struct _mulle_objc_methodlistenumerator  rover;
   struct _mulle_objc_method                *method;
   mulle_objc_methodid_t                    methodid;
   mulle_objc_implementation_t              imp;
   
   context = (struct preload_collect_context *) userinfo;
   
   printf( "   Checking %s methodlist from %s\n", 
           _mulle_objc_methodlist_get_categoryname( list) ?: "class",
           _mulle_objc_class_get_name( cls));
   
   rover = _mulle_objc_methodlist_enumerate( list);
   while( (method = _mulle_objc_methodlistenumerator_next( &rover)))
   {
      methodid = _mulle_objc_method_get_methodid( method);
      
      // Check if method should be preloaded
      int should_preload = 0;
      if( method->descriptor.bits & _mulle_objc_method_preload)
         should_preload = 1;
      else if( context->original_cls->universe->config.preload_all_methods)
         should_preload = 1;
      
      printf( "     Method: %s (id: %08x) - %s\n", 
              _mulle_objc_method_get_name( method), methodid,
              should_preload ? "PRELOAD" : "skip");
      
      if( should_preload)
      {
         // Store method pointer instead of IMP for easier name extraction
         mulle__pointermap_set( context->preload_map, (void *)(uintptr_t) methodid, method, NULL);
         context->count++;
      }
   }
   _mulle_objc_methodlistenumerator_done( &rover);
   
   return( mulle_objc_walk_ok);
}

// Context for super collection
struct super_collect_context
{
   struct mulle__pointermap        *preload_map;
   struct _mulle_objc_class        *original_cls;
   int                             count;
};

static mulle_objc_walkcommand_t   collect_super_calls( struct _mulle_objc_universe *universe,
                                                       struct _mulle_objc_super *super,
                                                       void *userinfo)
{
   struct super_collect_context    *context;
   mulle_objc_superid_t            superid;
   struct _mulle_objc_method       *method;
   
   context = (struct super_collect_context *) userinfo;
   superid = _mulle_objc_super_get_superid( super);
   
   fprintf( stderr, "SUPER CHECK: superid=%08x classid=%08x methodid=%08x\n",
           superid,
           _mulle_objc_super_get_classid( super),
           _mulle_objc_super_get_methodid( super));
   
   printf( "     Checking super: %08x (classid: %08x, methodid: %08x)\n",
           superid,
           _mulle_objc_super_get_classid( super),
           _mulle_objc_super_get_methodid( super));
   
   // Try to resolve super call - use regular supersearch, let it return NULL if not found
   method = _mulle_objc_class_supersearch_method( context->original_cls, superid);
   if( method)
   {
      fprintf( stderr, "SUPER RESOLVED: superid=%08x -> method=%s\n", superid, _mulle_objc_method_get_name( method));
      printf( "       -> RESOLVED: Super: %08x\n", superid);
      
      mulle__pointermap_set( context->preload_map, (void *)(uintptr_t) superid, method, NULL);
      context->count++;
   }
   else
   {
      fprintf( stderr, "SUPER SKIPPED: superid=%08x (could not resolve)\n", superid);
      printf( "       -> SKIPPED: Could not resolve super %08x\n", superid);
   }
   
   return( mulle_objc_walk_ok);
}


static void   _mulle_objc_class_collect_super_calls( struct _mulle_objc_class *cls,
                                                     struct mulle__pointermap *preload_map)
{
   struct _mulle_objc_universe     *universe;
   struct super_collect_context    context;
   
   universe = _mulle_objc_class_get_universe( cls);
   
   printf( "   Collecting super calls:\n");
   
   // Debug: Check if universe has any supers at all
   fprintf( stderr, "UNIVERSE DEBUG: Checking universe %p for supers\n", universe);
   
   context.preload_map  = preload_map;
   context.original_cls = cls;
   context.count        = 0;
   
   _mulle_objc_universe_walk_supers( universe, collect_super_calls, &context);
   
   printf( "   Total super calls collected: %d\n", context.count);
   
   // Additional debug: Let's see if we can find any supers manually
   fprintf( stderr, "UNIVERSE DEBUG: Finished walking supers, found %d\n", context.count);
}



static void   _mulle_objc_class_build_preload_map( struct _mulle_objc_class *cls,
                                                   struct mulle__pointermap *preload_map)
{
   struct preload_collect_context   context;
   
   printf( "Building preload map for class: %s\n", _mulle_objc_class_get_name( cls));
   printf( "Should build: %s\n", _mulle_objc_class_should_build_preload_map( cls) ? "YES" : "NO");
   
   if( ! _mulle_objc_class_should_build_preload_map( cls))
      return;
   
   // Initialize context
   context.preload_map  = preload_map;
   context.original_cls = cls;
   context.count        = 0;
   
   // Walk methodlists and collect preload methods
   mulle_objc_class_methodlist_walk( cls, collect_preload_methods, &context);
   
   printf( "   Total methods collected: %d\n", context.count);
   
   // Collect super calls
   _mulle_objc_class_collect_super_calls( cls, preload_map);
}


static void  dump_uniqueid_method_map( struct mulle__pointermap *map, char *text)
{
   struct mulle_pointerarray    keys;
   struct _mulle_objc_method    *method;
   void                         *key;
   void                         *value;
   void                         *keyptr;

   mulle_pointerarray_do( keys)
   {
      // Extract all keys using the macro
      mulle__pointermap_for( map, key, value)
      {
         mulle_pointerarray_add( keys, key);
      }

      // Sort keys
      mulle_pointerarray_qsort_r( keys, (mulle_pointerarray_cmp_t *) pointer_compare, NULL);

      // Display sorted methods
      mulle_pointerarray_for( keys, key)
      {
         method = (struct _mulle_objc_method *) mulle__pointermap_get( map, key);
         printf( "      %08tx: %s\n", (uintptr_t) key, _mulle_objc_method_get_name( method));
      }

      printf( "   Total %s methods: %zu\n", text, mulle__pointermap_get_count( map));
   }
}

int main( void)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_universe     *universe;

   // Enable preload_all_methods for testing
   universe = mulle_objc_global_get_defaultuniverse();
   assert( universe->config.preload_all_methods);

   printf( "=== Preload Map Generation Test ===\n\n");
   
   // Initialize preload map
   mulle__pointermap_do( preload_map)
   {
      // Get DerivedClass
      infra = mulle_objc_global_lookup_infraclass_nofail( MULLE_OBJC_DEFAULTUNIVERSEID,
                                                          mulle_objc_classid_from_string( "DerivedClass"));

      // Test metaclass
      meta = _mulle_objc_infraclass_get_metaclass( infra);
      cls  = _mulle_objc_metaclass_as_class( meta);

      printf( "1. Testing DerivedClass METACLASS:\n");
      _mulle_objc_class_build_preload_map( cls, preload_map);

      // Show metaclass methods
      printf( "\n   Metaclass preloaded methods:\n");
      dump_uniqueid_method_map( preload_map, "metaclass");
   }

   mulle__pointermap_do( preload_map)
   {
      // Test infraclass
      cls = _mulle_objc_infraclass_as_class( infra);

      printf( "\n2. Testing DerivedClass INFRACLASS:\n");
      _mulle_objc_class_build_preload_map( cls, preload_map);

      // Show infraclass methods
      printf( "\n   Infraclass preloaded methods:\n");
      dump_uniqueid_method_map( preload_map, "infraclass");
   }
   return( 0);
}
