//
//  mulle-objc-memorycache.c
//  mulle-objc-runtime
//
//  Copyright (c) 2021 Nat! - Mulle kybernetiK.
//  Copyright (c) 2021 Codeon GmbH.
//  All rights reserved.
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#include "mulle-objc-class-impcache.h"

#include "include-private.h"

#include "mulle-objc-call.h"
#include "mulle-objc-class-lookup.h"
#include "mulle-objc-class-search.h"






//
// Check if we should build a preload map for this class
//
static int   _mulle_objc_class_should_build_preload_map( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;
   
   universe = _mulle_objc_class_get_universe( cls);
   
   // disable if trace is enabled
   if( universe->debug.method_call & MULLE_OBJC_UNIVERSE_CALL_TRACE_BIT)
      return( 0);
   // Always build if preload_all_methods is enabled
   if( universe->config.preload_all_methods)
      return( 1);
      
   // Build if this class has preload methods
   if( _mulle_objc_class_count_preloadmethods( cls) > 0)
      return( 1);
      
   // Build if universe has preload methods
   if( _mulle_objc_universe_get_numberofpreloadmethods( universe) > 0)
      return( 1);
      
   return( 0);
}


//
// Context for collecting preload methods
//
struct preload_collect_context
{
   struct mulle__pointermap  *map;
   struct _mulle_objc_class  *original_cls;
};


//
// Callback to collect preload methods during methodlist walk
//
static mulle_objc_walkcommand_t   collect_preload_methods( struct _mulle_objc_class *cls,
                                                           struct _mulle_objc_methodlist *list,
                                                           void *userinfo)
{
   struct preload_collect_context           *context;
   struct _mulle_objc_methodlistenumerator  rover;
   struct _mulle_objc_method                *method;
   struct _mulle_objc_method                *resolved_method;
   struct _mulle_objc_universe              *universe;
   mulle_objc_methodid_t                    methodid;
   
   context  = (struct preload_collect_context *) userinfo;
   universe = _mulle_objc_class_get_universe( context->original_cls);
   
   rover = _mulle_objc_methodlist_enumerate( list);
   while( (method = _mulle_objc_methodlistenumerator_next( &rover)))
   {
      // Check if method should be preloaded
      if( ! _mulle_objc_universe_is_preload_method( universe, &method->descriptor))
         continue;
         
      methodid = _mulle_objc_method_get_methodid( method);
      
      // Resolve method through proper search
      resolved_method = mulle_objc_class_defaultsearch_method( context->original_cls, methodid);
      if( ! resolved_method)
         continue;
         
      // Add to map: methodid -> method
      mulle__pointermap_set( context->map, (void *) (uintptr_t) methodid, resolved_method, 
                            _mulle_objc_universe_get_allocator( universe));
      
      if( universe->debug.trace.preload)
      {
         mulle_objc_universe_trace( universe,
                                    "collected preload method %c%s",
                                    _mulle_objc_class_is_metaclass( context->original_cls) ? '+' : '-',
                                    _mulle_objc_method_get_name( resolved_method));
      }
   }
   _mulle_objc_methodlistenumerator_done( &rover);
   
   return( mulle_objc_walk_ok);
}


//
// Context for collecting super calls
//
struct super_collect_context
{
   struct mulle__pointermap  *map;
   struct _mulle_objc_class  *original_cls;
};


//
// Callback to collect super calls
//
static mulle_objc_walkcommand_t   collect_super_calls( struct _mulle_objc_universe *universe,
                                                       struct _mulle_objc_super *super,
                                                       void *userinfo)
{
   struct super_collect_context    *context;
   struct _mulle_objc_method       *method;
   mulle_objc_superid_t            superid;
   
   context = (struct super_collect_context *) userinfo;
   
   superid = _mulle_objc_super_get_superid( super);
   
   // Resolve super call
   method = _mulle_objc_class_supersearch_method( context->original_cls, superid);
   if( ! method)
      return( mulle_objc_walk_ok);
   
   // Add to map: superid -> method
   mulle__pointermap_set( context->map, (void *) (uintptr_t) superid, method,
                         _mulle_objc_universe_get_allocator( universe));
   
   if( universe->debug.trace.preload)
   {
      mulle_objc_universe_trace( universe,
                                 "collected preload super method %s for superid %08x",
                                 _mulle_objc_method_get_name( method),
                                 superid);
   }
   
   return( mulle_objc_walk_ok);
}


//
// Build preload map using methodlist walk for accurate traversal
//
static void   _mulle_objc_class_build_preload_map( struct _mulle_objc_class *cls,
                                                   struct mulle__pointermap *map)
{
   struct _mulle_objc_universe        *universe;
   struct preload_collect_context     method_context;
   struct super_collect_context       super_context;
   
   if( ! _mulle_objc_class_should_build_preload_map( cls))
      return;
   
   universe = _mulle_objc_class_get_universe( cls);
   
   if( universe->debug.trace.preload)
   {
      mulle_objc_universe_trace( universe,
                                 "building preload map for %sclass %s",
                                 _mulle_objc_class_is_metaclass( cls) ? "meta" : "infra",
                                 _mulle_objc_class_get_name( cls));
   }
   
   // Collect preload methods using methodlist walk
   method_context.map         = map;
   method_context.original_cls = cls;
   
   mulle_objc_class_methodlist_walk( cls, collect_preload_methods, &method_context);
   
   // Collect super calls if preload_all_methods is enabled
   if( universe->config.preload_all_methods)
   {
      super_context.map         = map;
      super_context.original_cls = cls;
      
      _mulle_objc_universe_walk_supers( universe, collect_super_calls, &super_context);
   }
}


//
// Fill cache from preload map with TAO checks
//
static void   fill_cache_from_preload_map( struct _mulle_objc_impcache *icache,
                                          struct mulle__pointermap *map,
                                          struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           uniqueid;
   void                            *key;
   void                            *value;
   
   universe = _mulle_objc_class_get_universe( cls);
   
   mulle__pointermap_for( map, key, value)
   {
      uniqueid = (mulle_objc_uniqueid_t) (uintptr_t) key;
      method   = (struct _mulle_objc_method *) value;
      
      // Apply TAO check before filling cache
      if( universe->debug.method_call & MULLE_OBJC_UNIVERSE_CALL_TAO_BIT)
      {
         if( _mulle_objc_class_is_threadaffine( cls) &&
             _mulle_objc_method_is_threadaffine( method))
         {
            if( universe->debug.trace.preload)
            {
               mulle_objc_universe_trace( universe,
                                          "skipped preload method %s (TAO thread unsafe)",
                                          _mulle_objc_method_get_name( method));
            }
            continue;  // Skip - not threadsafe under TAO
         }
      }
      
      // Add to cache
      entry = _mulle_objc_cache_add_pointer_inactive( &icache->cache,
                                                      _mulle_objc_method_get_implementation( method),
                                                      uniqueid);
      if( universe->debug.trace.preload)
      {
         mulle_objc_universe_trace( universe,
                                    "preloaded method %s",
                                    _mulle_objc_method_get_name( method));
      }
      
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
      if( entry && ! entry->cls)
         entry->cls = cls;
#else
      MULLE_C_UNUSED( entry);
#endif
   }
}


//
// Public function to preload methods into a cache
//
void   _mulle_objc_impcache_preload_methods( struct _mulle_objc_impcache *icache,
                                             struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;
   struct mulle__pointermap      map;
   struct mulle_allocator        *allocator;

   universe = _mulle_objc_class_get_universe( cls);

   // Use new preload map approach for comprehensive preloading
   if( _mulle_objc_class_should_build_preload_map( cls) || 
       _mulle_objc_universe_get_numberofpreloadmethods( universe))
   {
      allocator = _mulle_objc_universe_get_allocator( universe);
      _mulle__pointermap_init( &map, 0, allocator);
      
      _mulle_objc_class_build_preload_map( cls, &map);
      fill_cache_from_preload_map( icache, &map, cls);
      
      _mulle__pointermap_done( &map, allocator);
   }
}


//
// do not optimize uniqueid away, it can be the superid
//
static struct _mulle_objc_cacheentry   *
   _mulle_objc_class_preload_inactive_impcache( struct _mulle_objc_class *cls,
                                                struct _mulle_objc_impcache *icache,
                                                struct _mulle_objc_impcache *old_cache,
                                                enum mulle_objc_cachesizing_t strategy,
                                                mulle_objc_implementation_t imp,
                                                mulle_objc_uniqueid_t uniqueid)

{
   static struct _mulle_objc_cacheentry   empty_entry;
   struct _mulle_objc_cacheentry          *entry;
   struct _mulle_objc_universe            *universe;
   mulle_objc_methodid_t                  copyid;
   struct _mulle_objc_cacheentry          *p;
   struct _mulle_objc_cacheentry          *sentinel;

   universe  = _mulle_objc_class_get_universe( cls);

   // fill it up with preload messages and place our method there too
   // now for good measure. It is known that the cache is large enough
   // to hold the preload methods and one additional functionpointer.
   // Why ?
   _mulle_objc_impcache_preload_methods( icache, cls);

   //
   // if someone passes in a NULL for method, empty_entry is a marker
   // for success..
   //
   entry = &empty_entry;

   // only when invalidating will we not get a new entry, cache is still
   // fresh and single-thread here
   if( imp)
   {
      entry = _mulle_objc_cache_add_functionpointer_inactive( &icache->cache,
                                                              (mulle_functionpointer_t) imp,
                                                              uniqueid);
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
      if( ! entry->cls)
         entry->cls = cls;
#endif
   }

   //
   // If we repopulate the new cache with the old cache, we can then
   // determine, which methods have actually been used over the course
   // of the program by just dumping the cache contents.
   // We can do this only if the cache is growing though, or if we are
   // invalidating (when method == NULL)
   //
   if( universe->config.repopulate_caches &&
       (strategy == MULLE_OBJC_CACHESIZE_GROW || ! imp))
   {
      p        = &old_cache->cache.entries[ 0];
      sentinel = &p[ old_cache->cache.size];
      while( p < sentinel)
      {
         copyid = (mulle_objc_methodid_t) (intptr_t) _mulle_atomic_pointer_read( &p->key.pointer);
         ++p;

         //
         // Place it back into cache.
         // The copyid can also be a superid! In this case, it will not be
         // found. We leave it empty and don't place a forward into it.
         // I think we generally want to do a new lookup here, since we could
         // be invalidating..
         //
         if( copyid != MULLE_OBJC_NO_METHODID)
         {
            imp = _mulle_objc_class_lookup_implementation_noforward_nofill( cls, copyid);
            if( imp)
               _mulle_objc_cache_add_functionpointer_entry( &icache->cache,
                                                            (mulle_functionpointer_t) imp,
                                                            copyid);
         }
      }

      //
      // Cache might have changed again due to repopulation so pick out a
      // new entry (unless invalidating)
      //
      if( imp)
         entry = _mulle_objc_class_probe_cacheentry( cls, uniqueid);
   }
   return( entry);
}



static MULLE_C_NEVER_INLINE
struct _mulle_objc_cacheentry *
   _mulle_objc_class_convenient_swap_impcache( struct _mulle_objc_class *cls,
                                               mulle_objc_implementation_t imp,
                                               mulle_objc_uniqueid_t uniqueid,
                                               enum mulle_objc_cachesizing_t strategy)
{
   struct _mulle_objc_impcache       *icache;
   struct _mulle_objc_impcache       *old_cache;
   struct _mulle_objc_impcachepivot  *cachepivot;
   struct _mulle_objc_cacheentry     *entry;
   struct _mulle_objc_universe       *universe;
   struct mulle_allocator            *allocator;

   cachepivot = &cls->cachepivot;
   old_cache  = _mulle_objc_impcachepivot_get_impcache_atomic( cachepivot);
   universe   = _mulle_objc_class_get_universe( cls);
   allocator  = _mulle_objc_universe_get_allocator( universe);

   // a new beginning.. let it be filled anew. Calculate the minimum size
   // needed, can be less than the actual cache size now

   icache  = _mulle_objc_impcache_grow_with_strategy( old_cache, strategy, allocator);
   entry   = _mulle_objc_class_preload_inactive_impcache( cls,
                                                          icache,
                                                          old_cache,
                                                          strategy,
                                                          imp,
                                                          uniqueid);

   // if the set fails, then someone else was faster
   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe,
                                 "new grown method cache %p "
                                 "(%u of %u size used) for %s %08x \"%s\"",
                                 icache,
                                 _mulle_objc_cache_get_count( &icache->cache),
                                 icache->cache.size,
                                 _mulle_objc_class_get_classtypename( cls),
                                 _mulle_objc_class_get_classid( cls),
                                 _mulle_objc_class_get_name( cls));

   return( _mulle_objc_impcachepivot_convenient_swap( cachepivot,
                                                      icache,
                                                      universe)
           ? entry
           : NULL);
}






// uniqueid can also be a superid!
static struct _mulle_objc_cacheentry *
    _mulle_objc_class_fill_impcachepivot_imp( struct _mulle_objc_class *cls,
                                              mulle_objc_implementation_t imp,
                                              mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cache          *cache;
   struct _mulle_objc_cacheentry     *entry;
   struct _mulle_objc_universe       *universe;
   struct _mulle_objc_impcachepivot  *cachepivot;

   assert( cls);
   assert( imp);

   universe = _mulle_objc_class_get_universe( cls);

   //
   // try to get most up to date value
   //
   cachepivot = _mulle_objc_class_get_impcachepivot( cls);
   do
   {
      cache = _mulle_objc_cachepivot_get_cache_atomic( &cachepivot->pivot);

      if( _mulle_objc_universe_cache_should_grow( universe, cache))
      {
         entry = _mulle_objc_class_convenient_swap_impcache( cls,
                                                             imp,
                                                             uniqueid,
                                                             MULLE_OBJC_CACHESIZE_GROW);
      }
      else
      {
         entry = _mulle_objc_cache_add_functionpointer_entry( cache,
                                                              (mulle_functionpointer_t) imp,
                                                              uniqueid);
      }
   }
   while( ! entry);

   return( entry);
}





// uniqueid can also be a superid!, so don't pull it from method
// TODO: use imp instead of method or have second function
MULLE_C_NEVER_INLINE
void
    _mulle_objc_class_fill_impcache_method( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_method *method,
                                            mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_classpair      *pair;
   struct _mulle_objc_cacheentry     *entry;
   struct _mulle_objc_universe       *universe;
   struct _mulle_objc_infraclass     *infra;
   mulle_objc_implementation_t       imp;

   assert( cls);
   assert( method);

   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return;

   universe = _mulle_objc_class_get_universe( cls);
   // when we trace method calls, we don't cache ever
   if( universe->debug.method_call & MULLE_OBJC_UNIVERSE_CALL_TRACE_BIT)
      return;

   if( universe->debug.method_call & MULLE_OBJC_UNIVERSE_CALL_TAO_BIT)
   {
      // check if method is threadsafe, if not, it won't get into the cache
      // when tao checking is enabled, if the class is not, then all methods
      // are fine.
      //
      // | class_is_threadaffine | method_is_threadaffine | threadsafe
      // |-----------------------|------------------------|------------
      // | NO                    | NO                     | YES
      // | NO                    | YES                    | YES
      // | YES                   | NO                     | YES
      // | YES                   | YES                    | NO
      //
      if( _mulle_objc_class_is_threadaffine( cls) &&
          _mulle_objc_method_is_threadaffine( method))
         return;
   }

   // need to check that we are initialized
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
   {
      pair  = _mulle_objc_class_get_classpair( cls);
      infra = _mulle_objc_classpair_get_infraclass( pair);
      if( ! _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZING))
         mulle_objc_universe_fail_inconsistency( universe,
                  "Method call %08x \"%s\" comes too early, "
                  "the cache of %s \"%s\" hasn't been initialized yet.",
                  uniqueid, _mulle_objc_method_get_name( method),
                  _mulle_objc_class_get_classtypename( cls), cls->name);
      // otherwise its ok, just don't fill
      return;
   }

   //
   // try to get most up to date value
   //
   imp   = _mulle_objc_method_get_implementation( method);
   entry = _mulle_objc_class_fill_impcachepivot_imp( cls, imp, uniqueid);
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
   if( entry && ! entry->cls)
      entry->cls = cls;
#else
   MULLE_C_UNUSED( entry);
#endif
}


//
// pass uniqueid = MULLE_OBJC_NO_UNIQUEID, to invalidate all, otherwise we
// only invalidate all, if uniqueid is found
//
int   _mulle_objc_class_invalidate_impcacheentry( struct _mulle_objc_class *cls,
                                                 mulle_objc_methodid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cache        *cache;
   mulle_objc_uniqueid_t           offset;

   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return( 0);

   cache = _mulle_objc_class_get_impcache_cache_atomic( cls);
   if( ! _mulle_atomic_pointer_read( &cache->n))
      return( 0);

   if( uniqueid != MULLE_OBJC_NO_METHODID)
   {
      assert( mulle_objc_uniqueid_is_sane( uniqueid));

      offset = _mulle_objc_cache_probe_entryoffset( cache, uniqueid);
      entry  = (void *) &((char *) cache->entries)[ offset];

      // no entry is matching, fine
      if( ! entry->key.uniqueid)
         return( 0);
   }

   //
   // if we get NULL, from _mulle_objc_class_add_cacheentry_by_swapping_caches
   // someone else recreated the cache, fine by us!
   //
   _mulle_objc_class_convenient_swap_impcache( cls,
                                               NULL,
                                               MULLE_OBJC_NO_METHODID,
                                               MULLE_OBJC_CACHESIZE_STAGNATE);

   return( 0x1);
}

