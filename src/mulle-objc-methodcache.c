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
#include "mulle-objc-methodcache.h"

#include "include-private.h"

#include "mulle-objc-call.h"
#include "mulle-objc-class-lookup.h"
#include "mulle-objc-class-search.h"




//
// cache malloc/frees should not disturb errno, so we preserve it
//
struct _mulle_objc_methodcache
   *mulle_objc_methodcache_new( mulle_objc_cache_uint_t size,
                                struct mulle_allocator *allocator)
{
   struct _mulle_objc_methodcache  *mcache;
   int                             preserve;
   size_t                          s_cache;

   assert( allocator);
   assert( ! (sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1)));

   if( size < MULLE_OBJC_MIN_CACHE_SIZE)
      size = MULLE_OBJC_MIN_CACHE_SIZE;

   assert( ! (size & (size - 1)));          // check for tumeni bits

   preserve = errno;
   // cache struct has room for one entry already
   s_cache  = sizeof( struct _mulle_objc_methodcache) + sizeof( struct _mulle_objc_cacheentry) * (size - 1);
   mcache   = _mulle_allocator_calloc( allocator, 1, s_cache);
   errno    = preserve;

   mulle_objc_methodcache_init( mcache, size);

   return( mcache);
}


void   _mulle_objc_methodcache_free( struct _mulle_objc_methodcache *mcache,
                                     struct mulle_allocator *allocator)
{
   int   preserve;

   assert( allocator);

   preserve = errno;
   _mulle_allocator_free( allocator, mcache);
   errno = preserve;
}


void   _mulle_objc_methodcache_abafree( struct _mulle_objc_methodcache *mcache,
                                        struct mulle_allocator *allocator)
{
   int   preserve;

   assert( allocator);

   preserve = errno;
   _mulle_allocator_abafree( allocator, mcache);
   errno = preserve;
}


# pragma mark - class cache

static void
   _class_fill_inactivecache_with_preload_methodids( struct _mulle_objc_class *cls,
                                                     struct _mulle_objc_cache *cache,
                                                     mulle_objc_methodid_t *methodids,
                                                     unsigned int n)
{
   mulle_objc_methodid_t        *p;
   mulle_objc_methodid_t        *sentinel;
   struct _mulle_objc_method    *method;
   mulle_objc_implementation_t  imp;

   p        = methodids;
   sentinel = &p[ n];
   while( p < sentinel)
   {
      method = mulle_objc_class_defaultsearch_method( cls, *p++);
      if( method)
      {
         imp = _mulle_objc_method_get_implementation( method);
         _mulle_objc_cache_inactivecache_add_pointer_entry( cache,
                                                            imp,
                                                            method->descriptor.methodid);
      }
   }
}


static mulle_objc_walkcommand_t   
  preload( struct _mulle_objc_method *method,
           struct _mulle_objc_methodlist *list,
           struct _mulle_objc_class *cls,
           struct _mulle_objc_cache *cache)
{
   assert( cache);

   if( _mulle_objc_descriptor_is_preload_method( &method->descriptor))
      _mulle_objc_cache_inactivecache_add_pointer_entry( cache,
                                                         _mulle_objc_method_get_implementation( method),
                                                         method->descriptor.methodid);

   return( 0);
}


static void
   _mulle_objc_class_fill_inactivecache_with_preload_methods( struct _mulle_objc_class *cls,
                                                              struct _mulle_objc_cache *cache)
{
   unsigned int   inheritance;

   inheritance = _mulle_objc_class_get_inheritance( cls);
   _mulle_objc_class_walk_methods( cls, inheritance, (mulle_objc_method_walkcallback_t) preload, cache);
}


static inline void
   _mulle_objc_class_preload_inactivecache( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_cache *cache)
{
   struct _mulle_objc_universe   *universe;

   universe = cls->universe;
   _class_fill_inactivecache_with_preload_methodids( cls,
                                                     cache,
                                                     universe->methodidstopreload.methodids,
                                                     universe->methodidstopreload.n);
}


//
// fills the cache line with a forward: if message does not exist or
// if it exists it fills up the entry
//
struct _mulle_objc_cacheentry  empty_entry;

static MULLE_C_NEVER_INLINE
struct _mulle_objc_cacheentry   *
   _mulle_objc_class_add_cacheentry_swappmethodcache( struct _mulle_objc_class *cls,
                                                      struct _mulle_objc_cache *xcache,
                                                      struct _mulle_objc_method *method,
                                                      mulle_objc_methodid_t methodid,
                                                      enum mulle_objc_cachesizing_t strategy)
{
   struct _mulle_objc_methodcache  *mcache;
   struct _mulle_objc_methodcache  *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;
   struct _mulle_objc_universe     *universe;
   struct mulle_allocator          *allocator;
   mulle_objc_cache_uint_t         new_size;
   mulle_objc_implementation_t     imp;
   mulle_objc_methodid_t           copyid;

   old_cache = _mulle_objc_cache_get_methodcache_from_cache( xcache);

   // if the set fails, then someone else was faster
   universe = _mulle_objc_class_get_universe( cls);

   // a new beginning.. let it be filled anew
   // could ask the universe here what to do as new size

   new_size  = _mulle_objc_cache_get_resize( &old_cache->cache, strategy);
   allocator = _mulle_objc_universe_get_allocator( universe);
   mcache    = mulle_objc_methodcache_new( new_size, allocator);

   // fill it up with preload messages and place our method there too
   // now for good measure
   if( _mulle_objc_class_count_preloadmethods( cls))
      _mulle_objc_class_fill_inactivecache_with_preload_methods( cls, &mcache->cache);
   if( _mulle_objc_universe_get_numberofpreloadmethods( cls->universe))
      _mulle_objc_class_preload_inactivecache( cls, &mcache->cache);

   //
   // if someone passes in a NULL for method, empty_entry is a marker
   // for success..
   //
   entry = &empty_entry;
   if( method)
   {
      imp   = _mulle_objc_method_get_implementation( method);
      entry = _mulle_objc_cache_inactivecache_add_functionpointer_entry( &mcache->cache,
                                                                         (mulle_functionpointer_t) imp,
                                                                         methodid);
   }

   //
   // an initial_methodcache ? this is getting called too early
   // an empty_cache ? this is getting called wrong
   //
   assert( _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries)
            != universe->empty_methodcache.cache.entries);
   assert( _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries)
            != universe->initial_methodcache.cache.entries);

   if( _mulle_objc_cachepivot_atomiccas_entries( &cls->cachepivot.pivot,
                                                 mcache->cache.entries,
                                                 old_cache->cache.entries))
   {
      // cas failed, so get rid of this and punt
      _mulle_objc_methodcache_free( mcache, allocator); // sic, can be unsafe deleted now
      return( NULL);
   }

   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe,
                                 "new method cache %p "
                                 "(%u of %u used) for %s %08x \"%s\"",
                                 mcache,
                                 _mulle_objc_cache_get_count( &old_cache->cache),
                                 old_cache->cache.size,
                                 _mulle_objc_class_get_classtypename( cls),
                                 _mulle_objc_class_get_classid( cls),
                                 _mulle_objc_class_get_name( cls));

   //
   // If we repopulate the new cache with the old cache, we can then
   // determine, which methods have actually been used over the course
   // of the program by just dumping the cache contents.
   // We can do this only if the cache is growing though, or if we are
   // invalidating...
   //
   if( cls->universe->config.repopulate_caches &&
       (strategy == MULLE_OBJC_CACHESIZE_GROW ||
        strategy == MULLE_OBJC_CACHESIZE_GROW && ! method))
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
         //
         if( copyid != MULLE_OBJC_NO_METHODID)
            _mulle_objc_class_lookup_implementation_noforward( cls, copyid);
      }

      //
      // Cache might have changed again due to repopulation so pick out a
      // new entry
      //
      entry = _mulle_objc_class_lookup_cacheentry( cls, methodid);
   }

   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe, "free old method cache "
                                  "%p (%u of %u used) for %s %08x \"%s\"",
                                 old_cache,
                                  _mulle_objc_cache_get_count( &old_cache->cache),
                                  old_cache->cache.size,
                                 _mulle_objc_class_get_classtypename( cls),
                                 _mulle_objc_class_get_classid( cls),
                                 _mulle_objc_class_get_name( cls));

   _mulle_objc_methodcache_abafree( old_cache, allocator);

   return( entry);
}


// methodid can also be a superid!
MULLE_C_NEVER_INLINE void
    _mulle_objc_class_fill_methodcache_with_method( struct _mulle_objc_class *cls,
                                                    struct _mulle_objc_method *method,
                                                    mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_infraclass   *infra;
   mulle_objc_implementation_t     imp;

   assert( cls);
   assert( method);

   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return;

   // need to check that we are initialized
   //
   universe = _mulle_objc_class_get_universe( cls);
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

   imp = _mulle_objc_method_get_implementation( method);
   //
   //  try to get most up to date value
   //
   do
   {
      cache = _mulle_objc_cachepivot_atomicget_cache( &cls->cachepivot.pivot);
      if( _mulle_objc_universe_should_grow_cache( universe, cache))
         entry = _mulle_objc_class_add_cacheentry_swappmethodcache( cls,
                                                                    cache,
                                                                    method,
                                                                    uniqueid,
                                                                    MULLE_OBJC_CACHESIZE_GROW);
      else
         entry = _mulle_objc_cache_add_functionpointer_entry( cache,
                                                              (mulle_functionpointer_t) imp,
                                                              uniqueid);
   }
   while( ! entry);
}


//
// pass methodid = 0, to invalidate all
//
int   _mulle_objc_class_invalidate_methodcacheentry( struct _mulle_objc_class *cls,
                                                     mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;
   struct _mulle_objc_cache        *cache;

   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return( 0);

   cache = _mulle_objc_class_get_methodcache( cls);
   if( ! _mulle_atomic_pointer_read( &cache->n))
      return( 0);

   if( methodid != MULLE_OBJC_NO_METHODID)
   {
      assert( mulle_objc_uniqueid_is_sane( methodid));

      offset = _mulle_objc_cache_find_entryoffset( cache, methodid);
      entry  = (void *) &((char *) cache->entries)[ offset];

      // no entry is matching, fine
      if( ! entry->key.uniqueid)
         return( 0);
   }

   //
   // if we get NULL, from _mulle_objc_class_add_cacheentry_by_swapping_caches
   // someone else recreated the cache, fine by us!
   //
   for(;;)
   {
      // always break regardless of return value
      _mulle_objc_class_add_cacheentry_swappmethodcache( cls,
                                                         cache,
                                                         NULL,
                                                         MULLE_OBJC_NO_METHODID,
                                                         MULLE_OBJC_CACHESIZE_STAGNATE);
      break;
   }

   return( 0x1);
}
