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

#include "include.h"

#include "mulle-objc-uniqueid.h"

#include <stddef.h>
#include <assert.h>



#ifdef MULLE_TEST
//# define PEDANTIC_VERIFY
#endif

# pragma mark - method cache


#define MULLE_OBJC_MIN_CACHE_SIZE  4
#define MULLE_OBJC_MAX_CACHE_SIZE  (4*1024*1024)  // completely arbitrary


typedef mulle_objc_uniqueid_t   mulle_objc_cache_uint_t;


enum mulle_objc_cachesizing_t
{
   MULLE_OBJC_CACHESIZE_SHRINK   = -1,
   MULLE_OBJC_CACHESIZE_STAGNATE = 0,
   MULLE_OBJC_CACHESIZE_GROW     = 2
};

#ifdef MULLE_TEST
# define MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
#endif


struct _mulle_objc_cachepivot;

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
      mulle_objc_uniqueid_t            uniqueid;
      mulle_atomic_pointer_t           pointer;
   } key;
   union
   {
      mulle_atomic_functionpointer_t   functionpointer;
      mulle_atomic_pointer_t           pointer;
   } value;
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
   mulle_thread_t                      thread;
   struct _mulle_objc_class            *cls;
#endif
};


struct _mulle_objc_cache
{
   mulle_atomic_pointer_t          n;
   mulle_objc_cache_uint_t         size;  // don't optimize away (alignment!)
   mulle_objc_cache_uint_t         mask;
   struct _mulle_objc_cacheentry   entries[ 1];
};


// incoming cache must have been zero filled already
static inline void   _mulle_objc_cache_init( struct _mulle_objc_cache *cache,
                                             mulle_objc_cache_uint_t size)
{
   assert( ! (sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1)));

   // must be zero filled, don't want to assert everything though
   assert( ! _mulle_atomic_pointer_read( &cache->n));
   cache->size = size;
   // myhardworkbythesewordsguardedpleasedontsteal © Nat!
   cache->mask = (size - 1) * sizeof( struct _mulle_objc_cacheentry);    // preshift

#ifdef PEDANTIC_VERIFY
   for( unsigned char *a = (unsigned char *) cache->entries,
                      *b = (unsigned char *) &cache->entries[ cache->size];
        a < b;
        assert( ! *a), a++);
#endif
}


# pragma mark - cache allocation

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cache   *mulle_objc_cache_new( mulle_objc_cache_uint_t size,
                                                  struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_cache_free( struct _mulle_objc_cache *cache,
                               struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_cache_abafree( struct _mulle_objc_cache *cache,
                                  struct mulle_allocator *allocator);


static inline mulle_objc_cache_uint_t
   _mulle_objc_cache_get_resize( struct _mulle_objc_cache *cache,
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


MULLE_C_STATIC_ALWAYS_INLINE
struct _mulle_objc_cache  *
    _mulle_objc_cacheentry_get_cache_from_entries( struct _mulle_objc_cacheentry *entries)
{
   return( (void *) &((char *) entries)[ -(int)  offsetof( struct _mulle_objc_cache, entries)]);
}


# pragma mark - cache petty accessors

static inline mulle_objc_cache_uint_t
    _mulle_objc_cache_get_count( struct _mulle_objc_cache *cache)
{
   // yay double cast, how C like...
   return( (mulle_objc_cache_uint_t) (uintptr_t) _mulle_atomic_pointer_read( &cache->n));
}


static inline mulle_objc_cache_uint_t
    _mulle_objc_cache_get_size( struct _mulle_objc_cache *cache)
{
   return( cache->size);
}


static inline mulle_objc_cache_uint_t
    _mulle_objc_cache_get_mask( struct _mulle_objc_cache *cache)
{
   return( cache->mask);
}


static inline struct _mulle_objc_cacheentry *
    _mulle_objc_cache_get_entries( struct _mulle_objc_cache *cache)
{
   return( cache->entries);
}


MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_cache_should_grow( struct _mulle_objc_cache *cache,
                                     unsigned int fillrate);


# pragma mark - cache add entry, fails if cache too small

//
// there is no "fill" which swaps the cache with another when it gets full
//
// inactive: cache must be large enough to accomodate another entry
//           no atomicity, as the cache is inactive
//
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cacheentry   *
    _mulle_objc_cache_add_pointer_inactive( struct _mulle_objc_cache *cache,
                                             void *pointer,
                                             mulle_objc_uniqueid_t uniqueid);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_functionpointer_inactive( struct _mulle_objc_cache *cache,
                                                   mulle_functionpointer_t pointer,
                                                   mulle_objc_uniqueid_t uniqueid);

// returns null if cache is full
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_pointer_entry( struct _mulle_objc_cache *cache,
                                        void *pointer,
                                        mulle_objc_uniqueid_t uniqueid);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_functionpointer_entry( struct _mulle_objc_cache *cache,
                                                mulle_functionpointer_t pointer,
                                                mulle_objc_uniqueid_t uniqueid);


# pragma mark - cache method lookup

static inline
void   *_mulle_objc_cache_probe_pointer( struct _mulle_objc_cache *cache,
                                         mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   assert( mulle_objc_uniqueid_is_sane( uniqueid));

   entries = cache->entries;
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) uniqueid;
   for(;;)
   {
      offset = (mulle_objc_cache_uint_t) offset & mask;
      entry  = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == uniqueid)
         return( _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer));

      if( ! _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer))
         return( 0);

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}

static inline mulle_functionpointer_t
   _mulle_objc_cache_probe_functionpointer( struct _mulle_objc_cache *cache,
                                            mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   assert( mulle_objc_uniqueid_is_sane( uniqueid));

   entries = cache->entries;
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) uniqueid;
   for(;;)
   {
      offset = (mulle_objc_cache_uint_t) offset & mask;
      entry  = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == uniqueid)
         return( _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer));

      if( ! _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer))
         return( 0);

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_cache_uint_t
   _mulle_objc_cache_probe_entryoffset( struct _mulle_objc_cache *cache,
                                        mulle_objc_uniqueid_t uniqueid);

MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_cache_probe_entryindex( struct _mulle_objc_cache *cache,
                                          mulle_objc_uniqueid_t uniqueid);


# pragma mark - cache utilitites

MULLE_OBJC_RUNTIME_GLOBAL
unsigned int   mulle_objc_cache_calculate_fillpercentage( struct _mulle_objc_cache *cache);

// Checks each entry and calculates how many loops it must for a cache lookup
// returns the number of cache entries:
//
// count[ 0] : perfect hits
// count[ 1] : one step required
// count[ 2] : two steps required
// count[ 3] : three or more steps required
//
MULLE_OBJC_RUNTIME_GLOBAL
unsigned int   mulle_objc_cache_calculate_hits( struct _mulle_objc_cache *cache,
                                                unsigned int count[ 4]);




// The cache pivot indexes into the middle of the cache at the entries. The
// meta info of the cache is above. With a simple CAS the Class can exchange
// caches.
//                              +---------------------------------------------+
// +----------------+           |  Cache                                      |
// | Class.         |           |   meta info                                 |
// |    cachepivot ----entries----> entries[ 0] = { 0, 0}                     |
// +----------------+           |   entries[ 1] = { @selector( foo), foo_imp} |
//                              |   entries[ 2] = { 0, 0}                     |
//                              |   entries[ 3] = { 0, 0}                     |
//                              +---------------------------------------------+
struct _mulle_objc_cachepivot
{
   mulle_atomic_pointer_t   entries; // for atomic XCHG with pointer indirection
};


MULLE_C_STATIC_ALWAYS_INLINE
struct _mulle_objc_cacheentry  *
   _mulle_objc_cachepivot_get_entries_nonatomic( struct _mulle_objc_cachepivot *p)
{
   return( _mulle_atomic_pointer_read_nonatomic( &p->entries));
}


MULLE_C_STATIC_ALWAYS_INLINE
struct _mulle_objc_cache *
   _mulle_objc_cachepivot_get_cache_nonatomic( struct _mulle_objc_cachepivot *p)
{
   struct _mulle_objc_cacheentry  *entries;

   entries = _mulle_objc_cachepivot_get_entries_nonatomic( p);
   return( _mulle_objc_cacheentry_get_cache_from_entries( entries));
}


MULLE_C_STATIC_ALWAYS_INLINE
struct _mulle_objc_cacheentry *
   _mulle_objc_cachepivot_get_entries_atomic( struct _mulle_objc_cachepivot *p)
{
   return( (void *) _mulle_atomic_pointer_read( &p->entries));
}


static inline struct _mulle_objc_cacheentry  *
   _mulle_objc_cachepivot_set_entries_atomic( struct _mulle_objc_cachepivot *p,
                                             struct _mulle_objc_cacheentry *new_entries)
{
   return( _mulle_atomic_pointer_set( &p->entries, new_entries));
}


// will return 0 if successful
static inline int
   _mulle_objc_cachepivot_cas_entries( struct _mulle_objc_cachepivot *p,
                                       struct _mulle_objc_cacheentry *new_entries,
                                       struct _mulle_objc_cacheentry *old_entries)
{
   assert( old_entries != new_entries);
   return( ! _mulle_atomic_pointer_cas( &p->entries, new_entries, old_entries));
}


static inline struct _mulle_objc_cacheentry  *
   _mulle_objc_cachepivot_cas_weak_entries( struct _mulle_objc_cachepivot *p,
                                            struct _mulle_objc_cacheentry *new_entries,
                                            struct _mulle_objc_cacheentry *old_entries)
{
   assert( old_entries != new_entries);
   return( __mulle_atomic_pointer_cas_weak( &p->entries, new_entries, old_entries));
}


static inline struct _mulle_objc_cache  *
   _mulle_objc_cachepivot_get_cache_atomic( struct _mulle_objc_cachepivot *p)
{
   struct _mulle_objc_cacheentry   *entries;

   entries = _mulle_objc_cachepivot_get_entries_atomic( p);
   return( _mulle_objc_cacheentry_get_cache_from_entries( entries));
}


// returns -1 if failed
MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_cachepivot_swap( struct _mulle_objc_cachepivot *pivot,
                                   struct _mulle_objc_cache *cache,
                                   struct _mulle_objc_cache *old_cache,
                                   struct mulle_allocator *allocator);


MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cache *
   _mulle_objc_cache_grow_with_strategy( struct _mulle_objc_cache  *old_cache,
                                         enum mulle_objc_cachesizing_t strategy,
                                         struct mulle_allocator *allocator);

// uniqueid can be a methodid or superid!
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cacheentry *
    _mulle_objc_cachepivot_fill_functionpointer( struct _mulle_objc_cachepivot *pivot,
                                                 mulle_functionpointer_t imp,
                                                 mulle_objc_uniqueid_t uniqueid,
                                                 unsigned int fillrate,
                                                 struct mulle_allocator *allocator);

#endif /* defined(__MULLE_OBJC__mulle_objc_cache__) */
