//
//  mulle_objc_kvccache.h
//  mulle-objc
//
//  Created by Nat! on 18.07.16.
//  Copyright (c) 2016 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
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
#ifndef mulle_objc_kvccache_h__
#define mulle_objc_kvccache_h__

#include "mulle_objc_cache.h"

#include "mulle_objc_method.h"

#include <string.h>

//
// storage mechanism for KVC caches
//
// 0 : get        1: take
// 2 : storedGet  3: storedTake
//
//
struct _mulle_objc_kvcinfo
{
   mulle_objc_methodimplementation_t   implementation[ 4];
   mulle_objc_methodid_t               methodid[ 4];
   ptrdiff_t                           offset;
   char                                valueType;
   char                                cKey[1];  // flexible
};


struct _mulle_objc_kvcinfo   *_mulle_objc_kvcinfo_new( char *cKey,
                                                       struct mulle_allocator *allocator);

static inline void  _mulle_objc_kvcinfo_free( struct _mulle_objc_kvcinfo *entry,
                                              struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, entry);
}


static inline int  _mulle_objc_kvcinfo_equals( struct _mulle_objc_kvcinfo *entry,
                                               struct _mulle_objc_kvcinfo *other)
{
   return( ! strcmp( entry->cKey, other->cKey));
}


#pragma mark -
#pragma mark _mulle_objc_kvccache

struct _mulle_objc_kvccache
{
   struct _mulle_objc_cache    base;
};

#define MULLE_OBJC_KVCINFO_CONFLICT   ((struct _mulle_objc_kvcinfo *) -1)


static inline struct _mulle_objc_kvccache  *
   mulle_objc_kvccache_new( mulle_objc_cache_uint_t size,
                         struct mulle_allocator *allocator)
{
   return( (struct _mulle_objc_kvccache *) mulle_objc_cache_new( size, allocator));
}



static inline struct _mulle_objc_cacheentry  *
_mulle_objc_kvccache_add_entry( struct _mulle_objc_kvccache *cache,
                                struct _mulle_objc_kvcinfo *info,
                                mulle_objc_uniqueid_t keyid)
{
   return( _mulle_objc_cache_add_pointer_entry( (struct _mulle_objc_cache *) cache, info, keyid));
}


static inline struct _mulle_objc_cacheentry  *
   _mulle_objc_kvccache_inactivecache_add_entry( struct _mulle_objc_kvccache *cache,
                                                 struct _mulle_objc_kvcinfo *info,
                                                 mulle_objc_uniqueid_t keyid)
{
   return( _mulle_objc_cache_inactivecache_add_pointer_entry( (struct _mulle_objc_cache *) cache, info, keyid));
}


struct _mulle_objc_kvcinfo  *_mulle_objc_kvccache_lookup_kvcinfo( struct _mulle_objc_kvccache *cache,
                                                                 char *key);


#pragma mark -
#pragma mark _mulle_objc_kvccachepivot

struct _mulle_objc_kvccachepivot
{
   mulle_atomic_pointer_t   entries; // for atomic XCHG with pointer indirection
};


static inline struct _mulle_objc_kvccache  *_mulle_objc_kvccachepivot_atomic_get_cache( struct _mulle_objc_kvccachepivot *pivot)
{
   return( (struct _mulle_objc_kvccache *) _mulle_objc_cachepivot_atomic_get_cache( (struct _mulle_objc_cachepivot *) pivot));
}


MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_cacheentry  *_mulle_objc_kvccachepivot_atomic_get_entries( struct _mulle_objc_kvccachepivot *p)
{
   return( _mulle_objc_cachepivot_atomic_get_entries( (struct _mulle_objc_cachepivot *) p));
}



static inline int
   _mulle_objc_kvccachepivot_atomic_set_entries( struct _mulle_objc_kvccachepivot *p,
                                                 struct _mulle_objc_cacheentry  *new_entries,
                                                 struct _mulle_objc_cacheentry  *old_entries)
{
   return( _mulle_objc_cachepivot_atomic_set_entries( (struct _mulle_objc_cachepivot *) p, new_entries, old_entries));
}


int   _mulle_objc_kvccachepivot_invalidate( struct _mulle_objc_kvccachepivot *pivot,
                                            struct _mulle_objc_kvccache *empty_cache,
                                            struct mulle_allocator *allocator);



int    _mulle_objc_kvccachepivot_set_kvcinfo( struct _mulle_objc_kvccachepivot *pivot,
                                             struct _mulle_objc_kvcinfo *info,
                                             struct _mulle_objc_kvccache *empty_cache,
                                             struct mulle_allocator *allocator);

#endif /* mulle_objc_kvccache_h */
