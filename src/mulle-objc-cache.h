//
//  mulle_objc_cache.h
//  mulle-objc-runtime
//
//  Created by Nat! on 15.09.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
//  All rights reserved.
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
#ifndef mulle_objc_cache_h__
#define mulle_objc_cache_h__

#include "mulle-objc-uniqueid.h"

#include "include.h"
#include <stddef.h>
#include <assert.h>


# pragma mark - method cache


#define MULLE_OBJC_MIN_CACHE_SIZE  4

typedef mulle_objc_uniqueid_t   mulle_objc_cache_uint_t;


enum mulle_objc_cachesizing_t
{
   MULLE_OBJC_CACHESIZE_SHRINK   = -1,
   MULLE_OBJC_CACHESIZE_STAGNATE = 0,
   MULLE_OBJC_CACHESIZE_GROW     = 2
};


//
// this sizeof() must be a power of 2 else stuff fails
// because technically a functionpointer need not be the same size as a
// void *, we differentiate here. Though when will this ever be useful in
// real life ?
//
struct _mulle_objc_cacheentry
{
   union
   {
      mulle_objc_uniqueid_t         uniqueid;
      mulle_atomic_pointer_t        pointer;
   } key;
   union
   {
      mulle_atomic_functionpointer_t   functionpointer;
      mulle_atomic_pointer_t           pointer;
   } value;
};


struct _mulle_objc_cache
{
   mulle_atomic_pointer_t          n;
   mulle_objc_cache_uint_t         size;  // don't optimize away (alignment!)
   mulle_objc_cache_uint_t         mask;
   struct _mulle_objc_cacheentry   entries[ 1];
};


// incoming cache must have been zero filled already
static inline void   mulle_objc_cache_init( struct _mulle_objc_cache *cache,
                                            mulle_objc_cache_uint_t size)
{
   assert( ! (sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1)));

   // must be zero filled, don't want to assert everything though
   assert( ! _mulle_atomic_pointer_read( &cache->n));

   cache->size = size;
   // myhardworkbythesewordsguardedpleasedontsteal © Nat!
   cache->mask = (size - 1) * sizeof( struct _mulle_objc_cacheentry);    // preshift
}



static inline size_t   _mulle_objc_cache_get_resize( struct _mulle_objc_cache *cache,
                                                     enum mulle_objc_cachesizing_t strategy)
{
   switch( strategy)
   {
   case MULLE_OBJC_CACHESIZE_STAGNATE :
      return( cache->size);

   case MULLE_OBJC_CACHESIZE_SHRINK :
      return( cache->size <= MULLE_OBJC_MIN_CACHE_SIZE * 2
                  ? MULLE_OBJC_MIN_CACHE_SIZE
                  : cache->size >> 1);
   default :
      return( cache->size * 2);
   }
}


MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_cache  *
    _mulle_objc_cacheentry_get_cache_from_entries( struct _mulle_objc_cacheentry *entries)
{
   return( (void *) &((char *) entries)[ -(int)  offsetof( struct _mulle_objc_cache, entries)]);
}


struct _mulle_objc_cachepivot
{
   mulle_atomic_pointer_t   entries; // for atomic XCHG with pointer indirection
};


MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_cacheentry  *
   _mulle_objc_cachepivot_nonatomicget_entries( struct _mulle_objc_cachepivot *p)
{
   return( _mulle_atomic_pointer_nonatomic_read( &p->entries));
}


MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_cache *
   _mulle_objc_cachepivot_nonatomicget_cache( struct _mulle_objc_cachepivot *p)
{
   struct _mulle_objc_cacheentry  *entries;

   entries = _mulle_objc_cachepivot_nonatomicget_entries( p);
   return( _mulle_objc_cacheentry_get_cache_from_entries( entries));
}


MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_cacheentry *
   _mulle_objc_cachepivot_atomicget_entries( struct _mulle_objc_cachepivot *p)
{
   return( (void *) _mulle_atomic_pointer_read( &p->entries));
}


static inline struct _mulle_objc_cacheentry  *
   _mulle_objc_cachepivot_atomicset_entries( struct _mulle_objc_cachepivot *p,
                                             struct _mulle_objc_cacheentry *new_entries)
{
   return( _mulle_atomic_pointer_set( &p->entries, new_entries));
}


// will return 0 if successful
static inline int
   _mulle_objc_cachepivot_atomiccas_entries( struct _mulle_objc_cachepivot *p,
                                             struct _mulle_objc_cacheentry *new_entries,
                                             struct _mulle_objc_cacheentry *old_entries)
{
   assert( old_entries != new_entries);
   return( ! _mulle_atomic_pointer_cas( &p->entries, new_entries, old_entries));
}


static inline struct _mulle_objc_cacheentry  *
   _mulle_objc_cachepivot_atomiccweakcas_entries( struct _mulle_objc_cachepivot *p,
                                                  struct _mulle_objc_cacheentry *new_entries,
                                                  struct _mulle_objc_cacheentry *old_entries)
{
   assert( old_entries != new_entries);
   return( __mulle_atomic_pointer_weakcas( &p->entries, new_entries, old_entries));
}


static inline struct _mulle_objc_cache  *
   _mulle_objc_cachepivot_atomicget_cache( struct _mulle_objc_cachepivot *p)
{
   struct _mulle_objc_cacheentry   *entries;

   entries = _mulle_objc_cachepivot_atomicget_entries( p);
   return( _mulle_objc_cacheentry_get_cache_from_entries( entries));
}


# pragma mark - cache petty accessors

static inline mulle_objc_cache_uint_t
    _mulle_objc_cache_get_count( struct _mulle_objc_cache *cache)
{
   // yay double cast, how C like...
   return( (mulle_objc_cache_uint_t) (uintptr_t) _mulle_atomic_pointer_read( &cache->n));
}


static inline mulle_objc_uniqueid_t
    _mulle_objc_cache_get_size( struct _mulle_objc_cache *cache)
{
   return( cache->size);
}


static inline mulle_objc_cache_uint_t
    _mulle_objc_cache_get_mask( struct _mulle_objc_cache *cache)
{
   return( cache->mask);
}


# pragma mark - cache allocation

struct _mulle_objc_cache   *mulle_objc_cache_new( mulle_objc_cache_uint_t size,
                                                  struct mulle_allocator *allocator);

void   _mulle_objc_cache_free( struct _mulle_objc_cache *cache,
                               struct mulle_allocator *allocator);
void   _mulle_objc_cache_abafree( struct _mulle_objc_cache *cache,
                                  struct mulle_allocator *allocator);


# pragma mark - cache add entry

struct _mulle_objc_cacheentry   *
    _mulle_objc_cache_inactivecache_add_pointer_entry( struct _mulle_objc_cache *cache,
                                                       void *pointer,
                                                       mulle_objc_uniqueid_t uniqueid);

struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_inactivecache_add_functionpointer_entry( struct _mulle_objc_cache *cache,
                                                              mulle_functionpointer_t pointer,
                                                              mulle_objc_uniqueid_t uniqueid);

// returns null if cache is full
struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_pointer_entry( struct _mulle_objc_cache *cache,
                                        void *pointer,
                                        mulle_objc_uniqueid_t uniqueid);
struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_functionpointer_entry( struct _mulle_objc_cache *cache,
                                                mulle_functionpointer_t pointer,
                                                mulle_objc_uniqueid_t uniqueid);


# pragma mark - cache method lookup

void   *_mulle_objc_cache_lookup_pointer( struct _mulle_objc_cache *cache,
                                          mulle_objc_uniqueid_t uniqueid);
mulle_functionpointer_t
   _mulle_objc_cache_lookup_functionpointer( struct _mulle_objc_cache *cache,
                                             mulle_objc_uniqueid_t uniqueid);

mulle_objc_cache_uint_t
   _mulle_objc_cache_find_entryoffset( struct _mulle_objc_cache *cache,
                                       mulle_objc_uniqueid_t uniqueid);


# pragma mark - cache utilitites

unsigned int   mulle_objc_cache_calculate_fillpercentage( struct _mulle_objc_cache *cache);

// gives percentage of relative indexes high percentages[ 0] is good. size must be > 1
unsigned int  mulle_objc_cache_calculate_hitpercentage( struct _mulle_objc_cache *cache,
                                                        unsigned int *percentages,
                                                        unsigned int size);

int   _mulle_objc_cache_find_entryindex( struct _mulle_objc_cache *cache,
                                         mulle_objc_uniqueid_t uniqueid);

#endif /* defined(__MULLE_OBJC__mulle_objc_cache__) */
