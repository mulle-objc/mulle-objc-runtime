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
#include "mulle-objc-impcache.h"

#include "include-private.h"

#include "mulle-objc-call.h"
#include "mulle-objc-class-lookup.h"
#include "mulle-objc-class-search.h"




//
// cache malloc/frees should not disturb errno, so we preserve it
//
struct _mulle_objc_impcache
   *mulle_objc_impcache_new( mulle_objc_cache_uint_t size,
                                struct mulle_allocator *allocator)
{
   struct _mulle_objc_impcache  *icache;
   int                             preserve;
   size_t                          s_cache;

   assert( allocator); // need this for now
   assert( ! (sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1)));

   if( size < MULLE_OBJC_MIN_CACHE_SIZE)
      size = MULLE_OBJC_MIN_CACHE_SIZE;

   assert( ! (size & (size - 1)));          // check for tumeni bits

   preserve = errno;
   // cache struct has room for one entry already
   s_cache  = sizeof( struct _mulle_objc_impcache) + sizeof( struct _mulle_objc_cacheentry) * (size - 1);
   icache   = _mulle_allocator_calloc( allocator, 1, s_cache);
   errno    = preserve;

   mulle_objc_impcache_init( icache, size);

   return( icache);
}


#ifdef MULLE_TEST
struct _mulle_objc_impcache   *mulle_objc_discarded_impcaches[ 0x8000];
mulle_atomic_pointer_t            n_mulle_objc_discarded_impcaches;

static void   discard_impcache( struct _mulle_objc_impcache *icache)
{
   intptr_t   n;

   n = (intptr_t) _mulle_atomic_pointer_increment( &n_mulle_objc_discarded_impcaches);
   if( n >= sizeof( mulle_objc_discarded_impcaches) / sizeof( void *))
      abort();
   mulle_objc_discarded_impcaches[ n - 1] = icache;
}
#endif



void   _mulle_objc_impcache_free( struct _mulle_objc_impcache *icache,
                                     struct mulle_allocator *allocator)
{
   int   preserve;

   assert( allocator);

   preserve = errno;
#ifdef MULLE_TEST
   discard_impcache( icache);
#else
   _mulle_allocator_free( allocator, icache);
#endif
   errno = preserve;
}


void   _mulle_objc_impcache_abafree( struct _mulle_objc_impcache *icache,
                                        struct mulle_allocator *allocator)
{
   int   preserve;

   assert( allocator);

   preserve = errno;
#ifdef MULLE_TEST
   discard_impcache( icache);
#else
   _mulle_allocator_free( allocator, icache);
#endif
   errno = preserve;
}



//
// fills the cache line with a forward: if message does not exist or
// if it exists it fills up the entry
//
static struct _mulle_objc_cacheentry  empty_entry;


int   _mulle_objc_impcachepivot_swap( struct _mulle_objc_impcachepivot *pivot,
                                      struct _mulle_objc_impcache *icache,
                                      struct _mulle_objc_impcache *old_cache,
                                      struct mulle_allocator *allocator)
{
   //
   // an initial_impcache ? this is getting called too early
   // an empty_cache ? this is getting called wrong
   //
   if( _mulle_objc_cachepivot_cas_entries( &pivot->pivot,
                                           icache->cache.entries,
                                           old_cache ? old_cache->cache.entries : NULL))
   {
      // cas failed, so get rid of this and punt
      _mulle_objc_impcache_free( icache, allocator); // sic, can be unsafe deleted now
      return( -1);
   }


   _mulle_objc_impcache_abafree( old_cache, allocator);
   return( 0);
}


static struct _mulle_objc_impcache   *
   _mulle_objc_impcache_grow_with_strategy( struct _mulle_objc_impcache  *old_cache,
                                            enum mulle_objc_cachesizing_t strategy,
                                            struct mulle_allocator *allocator)
{
   struct _mulle_objc_impcache  *icache;
   mulle_objc_cache_uint_t      new_size;

   // a new beginning.. let it be filled anew
   // could ask the universe here what to do as new size

   new_size  = _mulle_objc_cache_get_resize( &old_cache->cache, strategy);
   icache    = mulle_objc_impcache_new( new_size, allocator);

   // copy old possibly non-standard callbacks, if the cache isn't the empty
   //if( old_cache->cache != &universeempty_cache)
   {
      icache->call       = old_cache->call;
      icache->call2      = old_cache->call2;
      icache->supercall  = old_cache->supercall;
      icache->supercall2 = old_cache->supercall2;
   }

   return( icache);
}



// uniqueid can be a methodid or superid!
struct _mulle_objc_cacheentry *
    _mulle_objc_impcachepivot_fill( struct _mulle_objc_impcachepivot *cachepivot,
                                    mulle_objc_implementation_t imp,
                                    mulle_objc_uniqueid_t uniqueid,
                                    unsigned int fillrate,
                                    struct mulle_allocator *allocator)
{
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_impcache     *old_cache;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;

   assert( cachepivot);
   assert( imp);

   //
   // try to get most up to date value
   //
   for(;;)
   {
      cache = _mulle_objc_cachepivot_get_cache_atomic( &cachepivot->pivot);

      if( _mulle_objc_cache_should_grow( cache, fillrate))
      {
         old_cache = _mulle_objc_cache_get_impcache_from_cache( cache);
         icache    = _mulle_objc_impcache_grow_with_strategy( old_cache,
                                                                 MULLE_OBJC_CACHESIZE_GROW,
                                                                 allocator);

         // doesn't really matter, if this fails or succeeds we just try 
         // again
         _mulle_objc_impcachepivot_swap( cachepivot,
                                         icache,
                                         old_cache,
                                         allocator);
         continue; 
      }

      entry = _mulle_objc_cache_add_functionpointer_entry( cache,
                                                           (mulle_functionpointer_t) imp,
                                                           uniqueid);
      if( entry)
         break;
   }

   return( entry);
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
      method = mulle_objc_class_defaultsearch_method( cls, *p);
      if( method)
      {
         imp = _mulle_objc_method_get_implementation( method);
         _mulle_objc_cache_add_pointer_inactive( cache, imp, *p);
      }
      p++;
   }
}


static mulle_objc_walkcommand_t   
  preload( struct _mulle_objc_method *method,
           struct _mulle_objc_methodlist *list,
           struct _mulle_objc_class *cls,
           struct _mulle_objc_cache *cache)
{
   struct _mulle_objc_cacheentry   *entry;

   assert( cache);

   if( _mulle_objc_descriptor_is_preload_method( &method->descriptor))
   {
      entry = _mulle_objc_cache_add_pointer_inactive( cache,
                                                      _mulle_objc_method_get_implementation( method),
                                                      method->descriptor.methodid);
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
      entry->cls = cls;
#endif
      ((void)( entry));  // use
   }

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
   _mulle_objc_class_preload_inactivecache_with_universe_methodids( struct _mulle_objc_class *cls,
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
// do not optimize uniqueid away, it can be the superid
//
static struct _mulle_objc_cacheentry   *
   _mulle_objc_impcache_preload_inactive_with_class( struct _mulle_objc_impcache *icache,
                                                     struct _mulle_objc_impcache *old_cache,
                                                     enum mulle_objc_cachesizing_t strategy,
                                                     struct _mulle_objc_class *cls,
                                                     mulle_objc_implementation_t imp,
                                                     mulle_objc_uniqueid_t uniqueid)

{
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_universe     *universe;
   mulle_objc_methodid_t           copyid;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;

   universe  = _mulle_objc_class_get_universe( cls);

   // fill it up with preload messages and place our method there too
   // now for good measure
   if( _mulle_objc_class_count_preloadmethods( cls))
      _mulle_objc_class_fill_inactivecache_with_preload_methods( cls, &icache->cache);
   if( _mulle_objc_universe_get_numberofpreloadmethods( universe))
      _mulle_objc_class_preload_inactivecache_with_universe_methodids( cls, &icache->cache);

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
   struct _mulle_objc_impcache        *icache;
   struct _mulle_objc_impcache        *old_cache;
   struct _mulle_objc_impcachepivot   *cachepivot;
   struct _mulle_objc_cacheentry         *entry;
   struct _mulle_objc_universe           *universe;
   struct mulle_allocator                *allocator;

   cachepivot = &cls->cachepivot;
   old_cache  = _mulle_objc_impcachepivot_get_impcache( cachepivot);
   universe   = _mulle_objc_class_get_universe( cls);
   allocator  = _mulle_objc_universe_get_allocator( universe);
   // a new beginning.. let it be filled anew
   // could ask the universe here what to do as new size
   icache     = _mulle_objc_impcache_grow_with_strategy( old_cache, strategy, allocator);

   entry      = _mulle_objc_impcache_preload_inactive_with_class( icache,
                                                                    old_cache,
                                                                    strategy,
                                                                    cls,
                                                                    imp,
                                                                    uniqueid);
   // if the set fails, then someone else was faster
   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe,
                                 "new method cache %p "
                                 "(%u of %u size used) for %s %08x \"%s\"",
                                 icache,
                                 _mulle_objc_cache_get_count( &icache->cache),
                                 icache->cache.size,
                                 _mulle_objc_class_get_classtypename( cls),
                                 _mulle_objc_class_get_classid( cls),
                                 _mulle_objc_class_get_name( cls));


   assert( _mulle_atomic_pointer_read_nonatomic( &cachepivot->pivot.entries)
            != universe->empty_impcache.cache.entries);
   assert( _mulle_atomic_pointer_read_nonatomic( &cachepivot->pivot.entries)
            != universe->initial_impcache.cache.entries);

   allocator = _mulle_objc_universe_get_allocator( universe);
   if( _mulle_objc_impcachepivot_swap( cachepivot, icache, old_cache, allocator))
   {
      if( universe->debug.trace.method_cache)
         mulle_objc_universe_trace( universe,
                                    "punted tmp cache %p as a new one is available",
                                    icache);

      return( NULL);
   }

   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe,
                                 "swapped old method cache %p with new method cache %p",
                                 old_cache,
                                 icache);

   return( entry);
}


// uniqueid can also be a superid!
struct _mulle_objc_cacheentry *
    _mulle_objc_class_add_imp_to_impcachepivot( struct _mulle_objc_class *cls,
                                                struct _mulle_objc_impcachepivot *cachepivot,
                                                mulle_objc_implementation_t imp,
                                                mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_universe     *universe;
   unsigned int                    fillrate;

   assert( cls);
   assert( imp);

   if( ! cachepivot)
      cachepivot = _mulle_objc_class_get_impcachepivot( cls);

   assert( cachepivot);

   universe   = _mulle_objc_class_get_universe( cls);
   fillrate   = _mulle_objc_universe_get_cache_fillrate( universe);

   //
   // try to get most up to date value
   //
   do
   {
      cache = _mulle_objc_cachepivot_get_cache_atomic( &cachepivot->pivot);

      if( _mulle_objc_cache_should_grow( cache, fillrate))
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
MULLE_C_NEVER_INLINE void
    _mulle_objc_class_fill_impcache_method( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_impcachepivot *cachepivot,
                                            struct _mulle_objc_method *method,
                                            mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_infraclass   *infra;
   mulle_objc_implementation_t     imp;

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
   entry = _mulle_objc_class_add_imp_to_impcachepivot( cls, cachepivot, imp, uniqueid);
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
   if( entry)
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

   cache = _mulle_objc_class_get_impcache_cache( cls);
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



