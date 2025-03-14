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


#ifdef MULLE_TEST
# define KEEP_IMPCACHES_FOR_TEST
#endif

#ifdef KEEP_IMPCACHES_FOR_TEST
//# define TRACE_IMPCACHES_FOR_TEST
#endif


//
// MEMO: not really sure why this was useful ?
//
#ifdef KEEP_IMPCACHES_FOR_TEST
struct _mulle_objc_impcache   *mulle_objc_allocated_impcaches[ 0x8000];
mulle_atomic_pointer_t        n_mulle_objc_allocated_impcaches;
struct _mulle_objc_impcache   *mulle_objc_freed_impcaches[ 0x8000];
mulle_atomic_pointer_t        n_mulle_objc_freed_impcaches;

static void   assert_allocated_impcache( struct _mulle_objc_impcache *icache)
{
   intptr_t                      n;
   struct _mulle_objc_impcache   **p;
   struct _mulle_objc_impcache   **sentinel;

   n        = (intptr_t) _mulle_atomic_pointer_read( &n_mulle_objc_allocated_impcaches);
   p        = mulle_objc_allocated_impcaches;
   sentinel = &p[ n];
   while( p < sentinel)
   {
      if( *p == icache)
         return;
      ++p;
   }

   mulle_fprintf( stderr, "address %p is garbage, not an allocated impcache\n");
   abort();
}

static void   assert_nonfreed_impcache( struct _mulle_objc_impcache *icache)
{
   intptr_t                      n;
   struct _mulle_objc_impcache   **p;
   struct _mulle_objc_impcache   **sentinel;

   n        = (intptr_t) _mulle_atomic_pointer_read( &n_mulle_objc_freed_impcaches);
   p        = mulle_objc_freed_impcaches;
   sentinel = &p[ n];
   while( p < sentinel)
   {
      if( *p == icache)
      {
         mulle_fprintf( stderr, "impcaches %p already freed\n");
         abort();
      }
      ++p;
   }
}

static void   assert_valid_impcache( struct _mulle_objc_impcache *icache)
{
   assert_nonfreed_impcache( icache);
   assert_allocated_impcache( icache);
}

#endif


static struct _mulle_objc_impcache   *allocate_impcache( struct mulle_allocator *allocator, size_t s_cache)
{
   struct _mulle_objc_impcache   *icache;

   icache = _mulle_allocator_calloc( allocator, 1, s_cache);
#ifdef KEEP_IMPCACHES_FOR_TEST
   intptr_t   n;

   n = (intptr_t) _mulle_atomic_pointer_increment( &n_mulle_objc_allocated_impcaches);
   if( n >= sizeof( mulle_objc_allocated_impcaches) / sizeof( void *))
   {
      mulle_fprintf( stderr, "too many impcaches allocated for KEEP_IMPCACHES_FOR_TEST\n");
      abort();
   }
   mulle_objc_allocated_impcaches[ n] = icache;
#endif
#ifdef TRACE_IMPCACHES_FOR_TEST
   mulle_fprintf( stderr, "allocated impcache %p\n", icache);
#endif
   return( icache);
}


static void   free_impcache( struct mulle_allocator *allocator,
                             struct _mulle_objc_impcache *icache)
{
#ifdef KEEP_IMPCACHES_FOR_TEST
   if( icache)
   {
      intptr_t   n;

      assert_valid_impcache( icache);
      n = (intptr_t) _mulle_atomic_pointer_increment( &n_mulle_objc_freed_impcaches);
      if( n >= sizeof( mulle_objc_freed_impcaches) / sizeof( void *))
      {
         mulle_fprintf( stderr, "too many impcaches freed for KEEP_IMPCACHES_FOR_TEST\n");
         abort();
      }
      mulle_objc_freed_impcaches[ n] = icache;
   }
#else
   _mulle_allocator_free( allocator, icache);
#endif

#ifdef TRACE_IMPCACHES_FOR_TEST
   if( icache)
      mulle_fprintf( stderr, "freed impcache %p\n", icache);
#endif
}


static void   abafree_impcache( struct mulle_allocator *allocator,
                                    struct _mulle_objc_impcache *icache)
{
#ifdef KEEP_IMPCACHES_FOR_TEST
   if( icache)
   {
      intptr_t   n;

      assert_valid_impcache( icache);
      n = (intptr_t) _mulle_atomic_pointer_increment( &n_mulle_objc_freed_impcaches);
      if( n >= sizeof( mulle_objc_freed_impcaches) / sizeof( void *))
      {
         mulle_fprintf( stderr, "too many impcaches freed for KEEP_IMPCACHES_FOR_TEST\n");
         abort();
      }
      mulle_objc_freed_impcaches[ n] = icache;
   }
#else
   _mulle_allocator_abafree( allocator, icache);
#endif

#ifdef TRACE_IMPCACHES_FOR_TEST
   if( icache)
      mulle_fprintf( stderr, "abafreed impcache %p\n", icache);
#endif
}



//
// cache malloc/frees should not disturb errno, so we preserve it
//
struct _mulle_objc_impcache *
   mulle_objc_impcache_new( mulle_objc_cache_uint_t size,
                            struct _mulle_objc_impcache_callback *callback,
                            struct mulle_allocator *allocator)
{
   struct _mulle_objc_impcache  *icache;
   int                          preserve;
   size_t                       s_cache;

   assert( allocator); // need this for now
   assert( ! (sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1)));

   if( size < MULLE_OBJC_MIN_CACHE_SIZE)
      size = MULLE_OBJC_MIN_CACHE_SIZE;
   if( size > MULLE_OBJC_MAX_CACHE_SIZE)
      size = MULLE_OBJC_MAX_CACHE_SIZE;

   if( size & (size - 1))          // check for tumeni bits
   {
      size  = size - 1;             // make it a power of 3
      size |= size >> 1;
      size |= size >> 2;
      size |= size >> 4;
      size |= size >> 8;
      size |= size >> 16;
      ++size;
   }

   preserve = errno;
   // cache struct has room for one entry already
   s_cache  = sizeof( struct _mulle_objc_impcache) + sizeof( struct _mulle_objc_cacheentry) * (size - 1);
   icache   = allocate_impcache( allocator, s_cache);
   errno    = preserve;

   _mulle_objc_impcache_init( icache, callback, size);

   return( icache);
}


void   _mulle_objc_impcache_free( struct _mulle_objc_impcache *icache,
                                  struct mulle_allocator *allocator)
{
   int   preserve;

   assert( allocator);

   preserve = errno;
   free_impcache( allocator, icache);
   errno = preserve;
}


void   _mulle_objc_impcache_abafree( struct _mulle_objc_impcache *icache,
                                     struct mulle_allocator *allocator)
{
   int   preserve;

   assert( allocator);

   preserve = errno;
   abafree_impcache( allocator, icache);
   errno = preserve;
}


//
// fills the cache line with a forward: if message does not exist or
// if it exists it fills up the entry
//
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


int   _mulle_objc_impcachepivot_convenient_swap( struct _mulle_objc_impcachepivot *cachepivot,
                                                 struct _mulle_objc_impcache *new_cache,
                                                 struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_impcache   *old_cache;
   struct mulle_allocator        *allocator;

   assert( _mulle_atomic_pointer_read_nonatomic( &cachepivot->pivot.entries)
            != universe->empty_impcache.cache.entries);
   assert( _mulle_atomic_pointer_read_nonatomic( &cachepivot->pivot.entries)
            != universe->initial_impcache.cache.entries);

   old_cache = _mulle_objc_impcachepivot_get_impcache_atomic( cachepivot);
   allocator = _mulle_objc_universe_get_allocator( universe);

   // if the set fails, then someone else was faster
   if( _mulle_objc_impcachepivot_swap( cachepivot, new_cache, old_cache, allocator))
   {
      if( universe->debug.trace.method_cache)
         mulle_objc_universe_trace( universe,
                                    "punted tmp impcache %p as a new one is available",
                                    new_cache);

      return( 0);
   }

   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe,
                                 "swapped old impcache %p with new method cache %p",
                                 old_cache,
                                 new_cache);
   return( 1);
}


struct _mulle_objc_cacheentry *
    _mulle_objc_impcachepivot_fill( struct _mulle_objc_impcachepivot *cachepivot,
                                    mulle_objc_implementation_t imp,
                                    mulle_objc_uniqueid_t uniqueid,
                                    unsigned int strategy,
                                    struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cacheentry   *entry;
   struct mulle_allocator          *allocator;

   assert( cachepivot);
   assert( imp);

   do
   {
      // try to get most up to date value
      icache = _mulle_objc_impcachepivot_get_impcache_atomic( cachepivot);
      if( _mulle_objc_universe_cache_should_grow( universe, &icache->cache))
      {
         allocator  = _mulle_objc_universe_get_allocator( universe);
         icache     = _mulle_objc_impcache_grow_with_strategy( icache, strategy, allocator);
         _mulle_objc_impcachepivot_convenient_swap( cachepivot,
                                                    icache,
                                                    universe);
         continue;
      }

      entry = _mulle_objc_cache_add_functionpointer_entry( &icache->cache,
                                                           (mulle_functionpointer_t) imp,
                                                           uniqueid);
   }
   while( ! entry);

   return( entry);
}
