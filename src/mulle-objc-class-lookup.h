//
//  mulle-objc-class-lookup.c
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
#ifndef mulle_objc_class_lookup_h__
#define mulle_objc_class_lookup_h__

#include "include.h"


// lookup functions, check the cache and if they don't find anything
// in the cache do the slow search and then update the cache, unless
// no_cache is given

#include "mulle-objc-class-struct.h"
#include "mulle-objc-uniqueid.h"
#include "mulle-objc-object.h"


#pragma mark - cache support

static inline struct _mulle_objc_cacheentry *
   _mulle_objc_class_lookup_cacheentry( struct _mulle_objc_class *cls,
                                        mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *entries;
   mulle_objc_cache_uint_t         offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cache   = _mulle_objc_cachepivot_atomicget_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   return( entry);
}


// only searches cache, returns what there
static inline mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_cacheonly( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_functionpointer_t         p;

   entry   = _mulle_objc_class_lookup_cacheentry( cls, methodid);
   p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   return( (mulle_objc_implementation_t) p);
}


# pragma mark - method lookup

// goes through cache returns an implementation if cached, NULL otherwise
// will return forward: if nothing found and (!) put it into the cache
MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
mulle_objc_implementation_t
    _mulle_objc_class_lookup_implementation( struct _mulle_objc_class *cls,
                                             mulle_objc_methodid_t methodid);


// knows about trace and empty cache, forwards
MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                   mulle_objc_methodid_t methodid);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nocache( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t methodid);

// goes through cache returns an implementation if cached, trys to fill cache otherwise
MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_noforward( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid);

// this will not update the cache
MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nocache_noforward( struct _mulle_objc_class *cls,
                                                              mulle_objc_methodid_t methodid);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nocache_nofail( struct _mulle_objc_class *cls,
                                                           mulle_objc_methodid_t methodid);

# pragma mark - method super lookup


MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
static inline mulle_objc_implementation_t
   _mulle_objc_object_superlookup_implementation_inline_nofail( void *obj,
                                                                mulle_objc_superid_t superid)
{
   struct _mulle_objc_class        *cls;
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_methodcache  *mcache;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;

   //
   // never forget, the superid contains the methodid and the classid
   //
   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mcache  = _mulle_objc_cache_get_methodcache_from_cache( cache);

   imp     = (*mcache->superlookup)( cls, superid);
   return( imp);
}

// this is just the non-inline variant of above
MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
mulle_objc_implementation_t
   _mulle_objc_object_superlookup_implementation_nofail( void *obj,
                                                         mulle_objc_superid_t superid);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
mulle_objc_implementation_t
   _mulle_objc_class_superlookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                        mulle_objc_superid_t superid);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
mulle_objc_implementation_t
   _mulle_objc_class_superlookup_implementation_nocache_nofail( struct _mulle_objc_class *cls,
                                                                mulle_objc_superid_t superid);


# pragma mark - methodcache refresh


// refresh functions, do not check the cache, do the slow search immediately
// and then update cache. If you don't want to update the cache use a
// search directly.


MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_superrefresh_implementation_nofail( struct _mulle_objc_class *cls,
                                                         mulle_objc_superid_t superid);

MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_refresh_implementation_nofail( struct _mulle_objc_class *cls,
                                                     mulle_objc_methodid_t methodid);

#endif
