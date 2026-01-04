#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

@interface TestClass
+ (void) method1;
+ (void) method2;
+ (void) method3;
@end

@implementation TestClass
+ (void) method1 { }
+ (void) method2 { }
+ (void) method3 { }
@end

int main(void)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_infraclass *cls;
   struct _mulle_objc_metaclass  *meta;
   struct _mulle_objc_cache      *cache;
   unsigned int                  count;
   
   universe = mulle_objc_global_get_defaultuniverse();
   
   // Enable preload_all_methods (should already be enabled in DEBUG)
   assert( universe->config.preload_all_methods);

   cls = mulle_objc_global_lookup_infraclass_nofail(
      MULLE_OBJC_DEFAULTUNIVERSEID,
      mulle_objc_classid_from_string("TestClass"));
   
   meta = _mulle_objc_infraclass_get_metaclass(cls);
   
   // Force +initialize to run by calling a method
   [TestClass method1];
   
   // Get the cache after +initialize has run
   cache = _mulle_objc_class_get_impcache_cache_atomic(_mulle_objc_metaclass_as_class(meta));
   count = _mulle_objc_cache_get_count(cache);
   
   printf("Cache entries: %u\n", count);
   printf("Cache size: %u\n", _mulle_objc_cache_get_size(cache));
   
   // Check that our methods are actually in the cache
   if (_mulle_objc_cache_probe_functionpointer(cache, mulle_objc_methodid_from_string("method1")))
      printf("method1 found in cache\n");
   else
      printf("ERROR: method1 NOT in cache!\n");
      
   if (_mulle_objc_cache_probe_functionpointer(cache, mulle_objc_methodid_from_string("method2")))
      printf("method2 found in cache\n");
   else
      printf("ERROR: method2 NOT in cache!\n");
      
   if (_mulle_objc_cache_probe_functionpointer(cache, mulle_objc_methodid_from_string("method3")))
      printf("method3 found in cache\n");
   else
      printf("ERROR: method3 NOT in cache!\n");
   
   return 0;
}
