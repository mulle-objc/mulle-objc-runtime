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
#ifndef mulle_objc_impcache_h__
#define mulle_objc_impcache_h__

#include "include.h"

#include "mulle-objc-cache.h"
#include "mulle-objc-uniqueid.h"
#include "mulle-objc-method.h"


struct _mulle_objc_class;
struct _mulle_objc_universe;

//
// as we don't want to pollute other caches with this, have a separate
// cache struct for methods. MEMO: the method cache is not like
// mulle_pointermap, where the keys and values are in separate array, rather
// here the key/values are together. This is presumed to be better for
// locality of reference, since we expect very little looping in most cases.
// But this has never been tested.
//
struct _mulle_objc_impcache
{
   //
   // standard call vector, its assumed you already did the relatively
   // expensive isa(), so pass it as fourth parameter
   //
   void   *(*call)( void *, mulle_objc_methodid_t, void *, struct _mulle_objc_class *);

   // continuation call, when the cache missed the first entry, we don't pass
   // cls here to match IMP. Slight hope that placement as second entry is
   // useful.
   void   *(*call2)( void *, mulle_objc_methodid_t, void *);

   //
   // TODO: for complete inlining code, it would be good to have call3
   //       where the callback does not search through the cache again
   //

   // super calls
   void   *(*supercall)( void *, mulle_objc_methodid_t, void *,
                         mulle_objc_superid_t, struct _mulle_objc_class *);

   void   *(*supercall2)( void *, mulle_objc_methodid_t, void *,
                          mulle_objc_superid_t, struct _mulle_objc_class *);

   struct _mulle_objc_cache      cache;
};



//
// the icache must have been properly allocated and zeroed before you call
// this method (i.e. don't use it, use mulle_objc_impcache_new)
//
static inline void   mulle_objc_impcache_init( struct _mulle_objc_impcache *icache,
                                               mulle_objc_cache_uint_t size)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   void  _mulle_objc_impcache_init_normal_callbacks( struct _mulle_objc_impcache *p);

   mulle_objc_cache_init( &icache->cache, size);

   _mulle_objc_impcache_init_normal_callbacks( icache);
}



// these functions don't return errno, though they allocate
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_impcache   *mulle_objc_impcache_new( mulle_objc_cache_uint_t size,
                                                        struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_impcache_free( struct _mulle_objc_impcache *cache,
                                  struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_impcache_abafree( struct _mulle_objc_impcache *cache,
                                     struct mulle_allocator *allocator);



MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_impcache  *
    _mulle_objc_cache_get_impcache_from_cache( struct _mulle_objc_cache *cache)
{
   return( (void *) &((char *) cache)[ -(int) offsetof( struct _mulle_objc_impcache, cache)]);
}


//
// the order of these structure elements is architecture dependent
// the trick is, that the mask is "behind" the buckets in malloced
// space. the vtab is in there, because universe and class share this
// (why not make the universe a first class object, because then
// the access to classes slows. We could put the universe into another object
//

struct _mulle_objc_impcachepivot
{
   struct _mulle_objc_cachepivot   pivot; // for atomic XCHG with pointer indirection
};


MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_cache  *
   _mulle_objc_impcachepivot_get_impcache_cache( struct _mulle_objc_impcachepivot *cachepivot)
{
   struct _mulle_objc_cacheentry     *entries;
   struct _mulle_objc_cache          *cache;

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cachepivot->pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   return( cache);
}


MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_impcache  *
   _mulle_objc_impcachepivot_get_impcache( struct _mulle_objc_impcachepivot *cachepivot)
{
   struct _mulle_objc_cache      *cache;
   struct _mulle_objc_impcache   *impcache;

   cache    = _mulle_objc_impcachepivot_get_impcache_cache( cachepivot);
   impcache = _mulle_objc_cache_get_impcache_from_cache( cache);
   return( impcache);
}


MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_impcachepivot_swap( struct _mulle_objc_impcachepivot *pivot,
                                      struct _mulle_objc_impcache *icache,
                                      struct _mulle_objc_impcache *old_cache,
                                      struct mulle_allocator *allocator);


MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cacheentry *
    _mulle_objc_impcachepivot_fill( struct _mulle_objc_impcachepivot *cachepivot,
                                    mulle_objc_implementation_t imp,
                                    mulle_objc_uniqueid_t uniqueid,
                                    unsigned int fillrate,
                                    struct mulle_allocator *allocator);



//
// this method does not use any callbacks and loops, so its fairly hefty
//
MULLE_C_ALWAYS_INLINE static inline 
mulle_objc_implementation_t
   _mulle_objc_impcachepivot_probe_inline( struct _mulle_objc_impcachepivot *cachepivot,
                                           mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;
   mulle_functionpointer_t          p;

   assert( mulle_objc_uniqueid_is_sane( uniqueid));

   // MEMO: When inlining, we are "fine" if the cache is stale
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cachepivot->pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries
   offset  = (mulle_objc_cache_uint_t) uniqueid;
   do
   {
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == uniqueid)
      {
         p = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         return( (mulle_objc_implementation_t) p);
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }
   while( entry->key.uniqueid);

   return( 0);
}


MULLE_C_CONST_RETURN
MULLE_C_NONNULL_RETURN
MULLE_C_ALWAYS_INLINE static inline 
mulle_objc_implementation_t
   _mulle_objc_impcachepivot_lookup_inline( struct _mulle_objc_impcachepivot *cachepivot,
                                            mulle_objc_uniqueid_t uniqueid)
{
   mulle_objc_implementation_t      f;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_impcache      *icache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   assert( mulle_objc_uniqueid_is_sane( uniqueid));

   // MEMO: When inlining, we are "fine" if the cache is stale
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cachepivot->pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) uniqueid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( MULLE_C_LIKELY( entry->key.uniqueid == uniqueid))
      f = (mulle_objc_implementation_t) _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer);
   else
   {
      icache = _mulle_objc_cache_get_impcache_from_cache( cache);
      f      = icache->call2;
   }
   return( f);
}


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_FIRST
struct _mulle_objc_cacheentry *
    _mulle_objc_class_add_imp_to_impcachepivot( struct _mulle_objc_class *cls,
                                                struct _mulle_objc_impcachepivot *cachepivot,
                                                mulle_objc_implementation_t imp,
                                                mulle_objc_uniqueid_t uniqueid);


#endif
