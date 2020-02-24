//
//  test_message_sending.c
//  mulle-objc-runtime
//
//  Created by Nat! on 13.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include "test_runtime_ids.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


//
// this doesn't check inheritance, it just creates a class dynamically
// adds methods to it and checks that the cache is OK
//

// load 1000 autogenerated functions
#include "f.c"


struct _1000_8_chars
{
   char   names[ 1000][ 8];
};


void   test_message_sending()
{
   struct _1000_8_chars            *storage;
   struct _mulle_objc_class        *A_cls;
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_infraclass   *A_infra;
   struct _mulle_objc_metaclass    *A_meta;
   struct _mulle_objc_descriptor   *desc;
   struct _mulle_objc_methodlist   *methodlist;
   struct _mulle_objc_object       *A_obj;
   struct _mulle_objc_universe     *universe;
   unsigned int                    i;
   void                            *rval;

   universe = mulle_objc_global_register_universe( MULLE_OBJC_DEFAULTUNIVERSEID, NULL);

   pair = mulle_objc_universe_new_classpair( universe, A_classid, "A", 0, 0, NULL);
   assert( pair);
   A_infra = _mulle_objc_classpair_get_infraclass( pair);
   A_meta  = _mulle_objc_classpair_get_metaclass( pair);

   // now fix up classes with empty method lists, so they are OK to be added
   mulle_objc_metaclass_add_methodlist_nofail( A_meta, NULL);

   storage = mulle_objc_universe_calloc( universe, 1, sizeof( struct _1000_8_chars));

   srand( 0x1848);

   methodlist = mulle_objc_universe_alloc_methodlist( universe, 1000);

   // create 1000 methods names, unlikely that their are duplicates
   for( i = 0; i < 1000; i++)
   {
      sprintf( storage->names[ i], "f%d", i);

      _mulle_atomic_functionpointer_nonatomic_write( &methodlist->methods[ i].implementation,
                                                     (mulle_functionpointer_t) f[ i]);

      methodlist->methods[ i].descriptor.name      = storage->names[ i];
      methodlist->methods[ i].descriptor.methodid  = mulle_objc_methodid_from_string( storage->names[ i]);
      methodlist->methods[ i].descriptor.signature = "@:";

      //printf( "#%04d: %s -> %lld\n", i, methods[ i].descriptor.name, (int64_t) methods[ i].descriptor.methodid);
   }
   methodlist->n_methods = 1000;

   mulle_objc_methodlist_sort( methodlist);

   // mark this one as preload
   methodlist->methods[ 500].descriptor.bits = _mulle_objc_method_preload;

   // for( i = 0; i < 1000; i++)
   //   printf( "#%04d: %s -> %llx\n", i, methodlist->methods[ i]->descriptor.name, (int64_t) methodlist->methods[ i]->descriptor.methodid);

   methodlist->n_methods = 1000;
   mulle_objc_infraclass_add_methodlist_nofail( A_infra, methodlist);
   mulle_objc_metaclass_add_methodlist_nofail( A_meta, NULL);
   mulle_objc_infraclass_add_ivarlist_nofail( A_infra, NULL);
   mulle_objc_infraclass_add_propertylist_nofail( A_infra, NULL);
   mulle_objc_universe_add_infraclass_nofail( universe, A_infra);

   A_obj = mulle_objc_infraclass_alloc_instance( A_infra);
   A_cls = _mulle_objc_infraclass_as_class( A_infra);
   for( i = 0; i < 1000; i++)
   {
      assert( i == 500 || ! _mulle_objc_class_lookup_implementation_cacheonly( (void *) A_cls, methodlist->methods[ i].descriptor.methodid));
      desc = _mulle_objc_universe_lookup_descriptor( universe, methodlist->methods[ i].descriptor.methodid);
      assert( desc == &methodlist->methods[ i].descriptor);
      rval = mulle_objc_object_call( A_obj, methodlist->methods[ i].descriptor.methodid, NULL);
      assert( _mulle_objc_class_lookup_implementation_cacheonly( (void *) A_infra, methodlist->methods[ i].descriptor.methodid));
      assert( rval == (void *) (uintptr_t) methodlist->methods[ i].descriptor.methodid);
   }

#if DEBUG
   {
      size_t                     old;
      unsigned int               percentages[ 16];
      unsigned int               i, n;
      struct _mulle_objc_cache   *A_cache;

      A_cache = _mulle_objc_class_get_methodcache( A_cls);

      // run it again to get cache size stable
      do
      {
         old = _mulle_objc_cache_size( A_cache);

         for( i = 0; i < 1000; i++)
         {
            rval = mulle_objc_object_call( A_obj, methodlist->methods[ i].descriptor.methodid, NULL);
            assert( rval == (void *) methodlist->methods[ i].descriptor.methodid);
         }
      }
      while( _mulle_objc_class_get_methodcache( A_cls)->size != old);

      printf( "cache size  : %d\n", (int) _mulle_objc_cache_size( ));
      printf( "cache mask  : 0x%llx\n", (long long) _mulle_objc_cache_mask( A_cache));
      printf( "cache count : %d\n", (int) _mulle_objc_cache_count( A_cache));
      printf( "cache fill  : %d%%\n", mulle_objc_cache_fill_percentage( A_cache));

      /* by chance expected would be
           79%: 1
           10%: 2
            1%: 3
       */
      n = mulle_objc_cache_hit_percentage( A_cache, percentages, 16);
      for( i = 0; i < n; i++)
      {
         printf( "\t#%02d: %d%%\n", i, percentages[ i]);
      }
   }
#endif
   mulle_objc_instance_free( A_obj);
}
