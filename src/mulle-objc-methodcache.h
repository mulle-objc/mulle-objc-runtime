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
#ifndef mulle_objc_methodcache_h__
#define mulle_objc_methodcache_h__

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
struct _mulle_objc_methodcache
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
   // standard lookup call for super calls, will return the implementation,
   // does not pass over the first entry in the cache
   mulle_objc_implementation_t   (*superlookup)( struct _mulle_objc_class *, mulle_objc_superid_t);

   // as above, but passes over the first entry in the cache
   mulle_objc_implementation_t   (*superlookup2)( struct _mulle_objc_class *, mulle_objc_superid_t);


   struct _mulle_objc_cache      cache;
};



//
// the mcache must have been properly allocated and zeroed before you call
// this method (i.e. don't use it, use mulle_objc_methodcache_new)
//
static inline void   mulle_objc_methodcache_init( struct _mulle_objc_methodcache *mcache,
                                                  mulle_objc_cache_uint_t size)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   void  _mulle_objc_methodcache_init_normal_callbacks( struct _mulle_objc_methodcache *p);

   mulle_objc_cache_init( &mcache->cache, size);

   _mulle_objc_methodcache_init_normal_callbacks( mcache);
}



// these functions don't return errno, though they allocate
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_methodcache   *mulle_objc_methodcache_new( mulle_objc_cache_uint_t size,
                                                              struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_methodcache_free( struct _mulle_objc_methodcache *cache,
                                     struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_methodcache_abafree( struct _mulle_objc_methodcache *cache,
                                        struct mulle_allocator *allocator);



MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_methodcache  *
    _mulle_objc_cache_get_methodcache_from_cache( struct _mulle_objc_cache *cache)
{
   return( (void *) &((char *) cache)[ -(int) offsetof( struct _mulle_objc_methodcache, cache)]);
}


//
// the order of these structure elements is architecture dependent
// the trick is, that the mask is "behind" the buckets in malloced
// space. the vtab is in there, because universe and class share this
// (why not make the universe a first class object, because then
// the access to classes slows. We could put the universe into another object
//

struct _mulle_objc_methodcachepivot
{
   struct _mulle_objc_cachepivot   pivot; // for atomic XCHG with pointer indirection
};


MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_methodcache  *
   _mulle_objc_methodcachepivot_get_methodcache( struct _mulle_objc_methodcachepivot *cachepivot)
{
   struct _mulle_objc_cacheentry     *entries;
   struct _mulle_objc_cache          *cache;
   struct _mulle_objc_methodcache    *methodcache;

   entries     = _mulle_objc_cachepivot_atomicget_entries( &cachepivot->pivot);
   cache       = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   methodcache = _mulle_objc_cache_get_methodcache_from_cache( cache);
   return( methodcache);
}


MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_methodcachepivot_swap( struct _mulle_objc_methodcachepivot *pivot,
                                         struct _mulle_objc_methodcache *mcache,
                                         struct _mulle_objc_methodcache *old_cache,
                                         struct mulle_allocator *allocator);


MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cacheentry *
    _mulle_objc_methodcachepivot_add_method( struct _mulle_objc_methodcachepivot *cachepivot,
                                             struct _mulle_objc_method *method,
                                             mulle_objc_uniqueid_t uniqueid,
                                             unsigned int fillrate,
                                             struct mulle_allocator *allocator);



//
// this method does not use any callbacks and loops, so its fairly hefty
//
MULLE_C_ALWAYS_INLINE static inline 
mulle_objc_implementation_t
   mulle_objc_methodcachepivot_lookup_inline_null( struct _mulle_objc_methodcachepivot *cachepivot,
                                                   mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;
   mulle_functionpointer_t          p;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   // MEMO: When inlining, we are "fine" if the cache is stale
   entries = _mulle_objc_cachepivot_atomicget_entries( &cachepivot->pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries
   offset  = (mulle_objc_cache_uint_t) methodid;
   do
   {
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == methodid)
      {
         p = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         return( (mulle_objc_implementation_t) p);
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }
   while( entry->key.uniqueid);

   return( 0);
}


MULLE_C_ALWAYS_INLINE static inline 
mulle_objc_implementation_t
   mulle_objc_methodcachepivot_lookup_inline( struct _mulle_objc_methodcachepivot *cachepivot,
                                              mulle_objc_methodid_t methodid)
{
   mulle_objc_implementation_t      f;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_methodcache   *mcache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   // MEMO: When inlining, we are "fine" if the cache is stale
   entries = _mulle_objc_cachepivot_atomicget_entries( &cachepivot->pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) methodid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( MULLE_C_LIKELY( entry->key.uniqueid == methodid))
      f = (mulle_objc_implementation_t) _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer);
   else
   {
      mcache = _mulle_objc_cache_get_methodcache_from_cache( cache);
      f      = mcache->call2;
   }
   return( f);
}



// Conveniences for class
MULLE_OBJC_RUNTIME_GLOBAL
void
    _mulle_objc_class_fill_methodcache_with_method( struct _mulle_objc_class *cls,
                                                    struct _mulle_objc_method *method,
                                                    mulle_objc_uniqueid_t uniqueid);

#endif
