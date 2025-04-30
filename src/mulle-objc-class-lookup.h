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


MULLE_C_STATIC_ALWAYS_INLINE
struct _mulle_objc_cacheentry *
   _mulle_objc_class_probe_cacheentry_inline( struct _mulle_objc_class *cls,
                                              mulle_objc_superid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   do
   {
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == methodid)
         return( entry);
      offset += sizeof( struct _mulle_objc_cacheentry);
   }
   while( entry->key.uniqueid);

   return( NULL);
}


MULLE_C_STATIC_ALWAYS_INLINE
mulle_objc_implementation_t
   _mulle_objc_class_probe_implementation_inline( struct _mulle_objc_class *cls,
                                                  mulle_objc_superid_t methodid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_implementation_t     imp;

   entry = _mulle_objc_class_probe_cacheentry_inline( cls, methodid);
   imp   = entry
           ? (mulle_objc_implementation_t) _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer)
           : 0;

   return( imp);
}


MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_cacheentry *
   _mulle_objc_class_probe_cacheentry( struct _mulle_objc_class *cls,
                                       mulle_objc_methodid_t methodid);

// only searches cache, returns what there
MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_probe_implementation( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t methodid);


# pragma mark - method lookup

// goes through cache returns an implementation if cached, NULL otherwise
// will return forward: if nothing found and (!) put it into the cache

MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
    _mulle_objc_class_lookup_implementation( struct _mulle_objc_class *cls,
                                             mulle_objc_methodid_t methodid);


// this will not fail if class has no forward method declared (who needs this ?)
MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                   mulle_objc_methodid_t methodid);

// used by NSObject, which can't be sure that the cache is not yet properly
// set up yet
MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofill( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid);

// goes through cache returns an implementation if cached, tries to fill cache otherwise
MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_noforward( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid);

static inline mulle_objc_implementation_t
   mulle_objc_class_lookup_implementation_noforward( struct _mulle_objc_class *cls,
                                                     mulle_objc_methodid_t methodid)
{
   return( cls ? _mulle_objc_class_lookup_implementation_noforward( cls, methodid) : 0);
}

// this will not update the cache
MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_noforward_nofill( struct _mulle_objc_class *cls,
                                                              mulle_objc_methodid_t methodid);

MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofail_nofill( struct _mulle_objc_class *cls,
                                                             mulle_objc_methodid_t methodid);


# pragma mark - impcache refresh


// refresh functions, do not check the cache, do the slow search immediately
// and then update cache. If you don't want to update the cache use a
// search directly.
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_method *
   _mulle_objc_class_refresh_supermethod_nofail( struct _mulle_objc_class *cls,
                                                 mulle_objc_superid_t superid);


MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_refresh_superimplementation_nofail( struct _mulle_objc_class *cls,
                                                         mulle_objc_superid_t superid);

MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   _mulle_objc_class_refresh_implementation_nofail( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t methodid);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_method *
   _mulle_objc_class_refresh_method_nofail( struct _mulle_objc_class *cls,
                                            mulle_objc_methodid_t methodid);

# pragma mark - super lookup

MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
static inline struct _mulle_objc_super *
   _mulle_objc_class_lookup_super_nofail( struct _mulle_objc_class *cls,
                                          mulle_objc_superid_t superid)
{
   struct _mulle_objc_universe  *universe;
   struct _mulle_objc_super     *p;

   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   universe = _mulle_objc_class_get_universe( cls);
   p        = _mulle_objc_universe_lookup_super_nofail( universe, superid);

   return( p);
}


# pragma mark - method super lookup



MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
static inline mulle_objc_implementation_t
   _mulle_objc_class_lookup_superimplementation_inline_nofail( struct _mulle_objc_class *cls,
                                                               mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t   imp;

   imp = _mulle_objc_class_probe_implementation_inline( cls, superid);
   if( ! imp)
      imp = _mulle_objc_class_refresh_superimplementation_nofail( cls, superid);
   return( imp);
}



MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
struct _mulle_objc_method *
   _mulle_objc_class_search_supermethod_nofail( struct _mulle_objc_class *cls,
                                                mulle_objc_superid_t superid);


MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
static inline mulle_objc_implementation_t
   _mulle_objc_class_search_superimplementation_nofail( struct _mulle_objc_class *cls,
                                                        mulle_objc_superid_t superid)
{
   struct _mulle_objc_method            *method;
   mulle_objc_implementation_t          imp;

   method = _mulle_objc_class_search_supermethod_nofail( cls, superid);
   imp    = _mulle_objc_method_get_implementation( method);
   return( imp);
}


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
mulle_objc_implementation_t
   _mulle_objc_class_lookup_superimplementation_nofail( struct _mulle_objc_class *cls,
                                                        mulle_objc_superid_t superid);

// this is just the non-inline variant of above
MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
static inline mulle_objc_implementation_t
   _mulle_objc_object_lookup_superimplementation_inline_nofail( void *obj,
                                                                mulle_objc_superid_t superid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_lookup_superimplementation_inline_nofail( cls, superid));
}


// used by debugger code
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_CONST_RETURN
mulle_objc_implementation_t
   _mulle_objc_object_lookup_superimplementation_noforward_nofill( void *obj,
                                                                      mulle_objc_superid_t superid);

// this is just the non-inline variant of above
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN
mulle_objc_implementation_t
   _mulle_objc_object_lookup_superimplementation_nofail( void *obj,
                                                         mulle_objc_superid_t superid);


#endif
