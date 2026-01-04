#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

struct super_walker_info
{
   int                        count;
   struct _mulle_objc_cache   *cache;
};

static mulle_objc_walkcommand_t
   check_super( struct _mulle_objc_universe *universe,
                struct _mulle_objc_super *super,
                void *userinfo)
{
   struct super_walker_info   *info = userinfo;
   mulle_objc_superid_t       superid;
   
   info->count++;
   superid = _mulle_objc_super_get_superid( super);
   
   printf("  Super %d: superid=%08x classid=%08x methodid=%08x\n",
          info->count,
          superid,
          _mulle_objc_super_get_classid( super),
          _mulle_objc_super_get_methodid( super));
   
   // Check if this super is in the cache
   if( _mulle_objc_cache_probe_functionpointer( info->cache, superid))
      printf("    -> Found in cache!\n");
   else
      printf("    -> NOT in cache\n");
   
   return( mulle_objc_walk_ok);
}

@interface BaseClass
+ (void) baseMethod;
@end

@implementation BaseClass
+ (void) baseMethod
{
   printf("BaseClass baseMethod\n");
}
@end

@interface SubClass : BaseClass
+ (void) baseMethod;
+ (void) subMethod;
@end

@implementation SubClass
+ (void) baseMethod
{
   printf("SubClass baseMethod calling super\n");
   [super baseMethod];
}

+ (void) subMethod
{
   printf("SubClass subMethod\n");
}
@end

int main(void)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_infraclass *cls;
   struct _mulle_objc_metaclass  *meta;
   struct _mulle_objc_cache      *cache;
   struct _mulle_objc_super      *super_struct;
   unsigned int                  count;
   mulle_objc_superid_t          superid;
   
   universe = mulle_objc_global_get_defaultuniverse();

   assert( universe->config.preload_all_methods);
   
   cls = mulle_objc_global_lookup_infraclass_nofail(
      MULLE_OBJC_DEFAULTUNIVERSEID,
      mulle_objc_classid_from_string("SubClass"));
   
   meta = _mulle_objc_infraclass_get_metaclass(cls);
   
   // Force +initialize to run by calling a method
   [SubClass subMethod];
   
   // Get the cache after +initialize has run
   cache = _mulle_objc_class_get_impcache_cache_atomic(_mulle_objc_metaclass_as_class(meta));
   count = _mulle_objc_cache_get_count(cache);
   
   printf("Cache entries: %u\n", count);
   printf("Cache size: %u\n", _mulle_objc_cache_get_size(cache));
   
   // Check that regular methods are in the cache
   if (_mulle_objc_cache_probe_functionpointer(cache, mulle_objc_methodid_from_string("baseMethod")))
      printf("baseMethod found in cache\n");
   else
      printf("ERROR: baseMethod NOT in cache!\n");
      
   if (_mulle_objc_cache_probe_functionpointer(cache, mulle_objc_methodid_from_string("subMethod")))
      printf("subMethod found in cache\n");
   else
      printf("ERROR: subMethod NOT in cache!\n");
   
   // Check if any supers were registered and preloaded
   // The compiler registers super structs for actual [super ...] calls
   printf("\nChecking registered supers in universe:\n");
   
   struct super_walker_info   super_info;
   
   super_info.count = 0;
   super_info.cache = cache;
   
   _mulle_objc_universe_walk_supers( universe,
                                     (mulle_objc_walk_supers_callback_t) check_super,
                                     &super_info);
   
   printf("Total supers registered in universe: %d\n", super_info.count);
   printf("Cache size: %u (grew from expected 4 to accommodate supers)\n",
          _mulle_objc_cache_get_size( cache));
   
   // Test the actual super call
   printf("\nTesting super call:\n");
   [SubClass baseMethod];
   
   return 0;
}
