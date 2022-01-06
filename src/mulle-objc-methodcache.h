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

#include "mulle-objc-cache.h"
#include "mulle-objc-uniqueid.h"
#include "mulle-objc-method.h"


struct _mulle_objc_class;


//
// as we don't want to pollute other caches with this, have a separate
// cache struct for methods
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


   struct _mulle_objc_cache        cache;
};



static inline void   mulle_objc_methodcache_init( struct _mulle_objc_methodcache *mcache,
                                                  mulle_objc_cache_uint_t size)
{
   extern void  _mulle_objc_methodcache_init_normal_callbacks( struct _mulle_objc_methodcache *p);

   mulle_objc_cache_init( &mcache->cache, size);

   _mulle_objc_methodcache_init_normal_callbacks( mcache);
}



// these functions don't perturn errno, though they allocate
MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
struct _mulle_objc_methodcache   *mulle_objc_methodcache_new( mulle_objc_cache_uint_t size,
                                                              struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
void   _mulle_objc_methodcache_free( struct _mulle_objc_methodcache *cache,
                                     struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
void   _mulle_objc_methodcache_abafree( struct _mulle_objc_methodcache *cache,
                                        struct mulle_allocator *allocator);



MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_methodcache  *
    _mulle_objc_cache_get_methodcache_from_cache( struct _mulle_objc_cache *cache)
{
   return( (void *) &((char *) cache)[ -(int)  offsetof( struct _mulle_objc_methodcache, cache)]);
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



MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
void
    _mulle_objc_class_fill_methodcache_with_method( struct _mulle_objc_class *cls,
                                                    struct _mulle_objc_method *method,
                                                    mulle_objc_uniqueid_t uniqueid);

#endif
