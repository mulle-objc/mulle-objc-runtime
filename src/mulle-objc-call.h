//
//  mulle_objc_call.h
//  mulle-objc-runtime
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
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
#ifndef mulle_objc_call_h__
#define mulle_objc_call_h__

#include "mulle-objc-class.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-cache.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-method.h"
#include "mulle-objc-methodidconstants.h"
#include "mulle-objc-object.h"

#include "mulle-objc-universe-class.h"

#include "include.h"
#include <assert.h>


//// on x86_64 this should optimize into 30 bytes tops w/o tagged pointers
//
// When inlining, we are "fine" if the cache is stale
// Unfortunately llvm sometimes really produces pathetic code.
//
// use this for -O3
//
MULLE_C_ALWAYS_INLINE static inline void  *
   mulle_objc_object_inlinecall( void *obj,
                                 mulle_objc_methodid_t methodid,
                                 void *parameter)
{
#ifdef __MULLE_OBJC_FCS__
   int                             index;
#endif
   mulle_objc_implementation_t     f;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   //
   // with tagged pointers inlining starts to become useless, because this
   // _mulle_objc_object_get_isa function produces too much code IMO
   //
   cls = _mulle_objc_object_get_isa( obj);

#ifdef __MULLE_OBJC_FCS__
   //
   // try to simplify to return( (*cls->vtab.methods[ index])( obj, methodid, parameter)
   //
   index = mulle_objc_get_fastmethodtable_index( methodid);
   if( index >= 0)
      return( _mulle_objc_fastmethodtable_invoke( obj, methodid, parameter, &cls->vtab, index));
#endif

   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) methodid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( __builtin_expect( (entry->key.uniqueid == methodid), 1))
      f = (mulle_objc_implementation_t) _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer);
   else
      f = cls->cachepivot.call2;
   return( (*f)( obj, methodid, parameter));
}


//
// use this for -O -O1
//
MULLE_C_ALWAYS_INLINE static inline void  *
   mulle_objc_object_partialinlinecall( void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter)
{
#ifdef __MULLE_OBJC_FCS__
   int                        index;
#endif
   struct _mulle_objc_class   *cls;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   cls   = _mulle_objc_object_get_isa( obj);

#ifdef __MULLE_OBJC_FCS__
   index = mulle_objc_get_fastmethodtable_index( methodid);
   if( __builtin_expect( index >= 0, 1))
      return( _mulle_objc_fastmethodtable_invoke( obj, methodid, parameter, &cls->vtab, index));
#endif

   return( (*cls->call)( obj, methodid, parameter, cls));
}


//
// this is the method to use, when the selector is variable
//
MULLE_C_ALWAYS_INLINE static inline void  *
   mulle_objc_object_inlinecall_variablemethodid( void *obj,
                                                  mulle_objc_methodid_t methodid,
                                                  void *parameter)
{
   mulle_objc_implementation_t     f;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) methodid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( __builtin_expect( (entry->key.uniqueid == methodid), 1))
      f = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   else
      f = cls->cachepivot.call2;
   return( (*f)( obj, methodid, parameter));
}


//
// use this when you know methodid is not a constant and you want to keep
// the call size down
//
MULLE_C_ALWAYS_INLINE static inline void   *
   mulle_objc_object_call_variablemethodid( void *obj,
                                            mulle_objc_methodid_t methodid,
                                            void *parameter)
{
   struct _mulle_objc_class   *cls;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   cls   = _mulle_objc_object_get_isa( obj);
   return( (*cls->call)( obj, methodid, parameter, cls));
}


//
// this is useful for calling a list of objects efficiently, it is assumed that
// class/methods do not change during its run
//
void   mulle_objc_objects_call( void **objects,
                                unsigned int n,
                                mulle_objc_methodid_t sel,
                                void *params);


#pragma mark - calls for super

mulle_objc_implementation_t
   _mulle_objc_object_superlookup_implementation_nofail( void *obj,
                                                         mulle_objc_superid_t superid);

static inline mulle_objc_implementation_t
   _mulle_objc_object_inlinesuperlookup_implementation_nofail( void *obj,
                                                               mulle_objc_superid_t superid)
{
   struct _mulle_objc_class      *cls;
   mulle_objc_implementation_t   imp;

   cls = _mulle_objc_object_get_isa( obj);
   imp = (*cls->superlookup)( cls, superid);
   return( imp);
}

//
// this is used for calling super. It's the same for metaclasses and
// infraclasses. The superid is hash( <classname> ';' <methodname>)
// Since it is a super call, obj is known to be non-nil. So there is
// no corresponding mulle_objc_object_inline_supercall.
// Will call _mulle_objc_class_superlookup_implementation
//

static inline void   *
   _mulle_objc_object_partialinlinesupercall( void *obj,
                                              mulle_objc_methodid_t methodid,
                                              void *parameter,
                                              mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t   imp;

   imp = _mulle_objc_object_superlookup_implementation_nofail( obj, superid);
   return( (*imp)( obj, methodid, parameter));
}



MULLE_C_ALWAYS_INLINE
   static inline void  *_mulle_objc_object_inlinesupercall( void *obj,
                                                            mulle_objc_methodid_t methodid,
                                                            void *parameter,
                                                            mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t     f;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   //
   // with tagged pointers inlining starts to become useless, because this
   // _mulle_objc_object_get_isa function produces too much code IMO
   //
   cls = _mulle_objc_object_get_isa( obj);

   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) superid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( __builtin_expect( (entry->key.uniqueid == superid), 1))
      f = (mulle_objc_implementation_t) _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer);
   else
      f  = (*cls->superlookup2)( cls, superid);
   return( (*f)( obj, methodid, parameter));
}



void   *_mulle_objc_object_supercall( void *obj,
                                      mulle_objc_methodid_t methodid,
                                      void *parameter,
                                      mulle_objc_superid_t superid);

MULLE_C_CONST_RETURN MULLE_C_NON_NULL_RETURN struct _mulle_objc_method *
   _mulle_objc_class_superlookup_method_nofail( struct _mulle_objc_class *cls,
                                                mulle_objc_superid_t superid);

MULLE_C_CONST_RETURN MULLE_C_NON_NULL_RETURN mulle_objc_implementation_t
   _mulle_objc_class_superlookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                        mulle_objc_superid_t superid);


# pragma mark - API Calls

// compiler uses this for -O0, -Os, it does no fast calls
void   *mulle_objc_object_call( void *obj,
                                mulle_objc_methodid_t methodid,
                                void *parameter);


# pragma mark - special initial setup calls

void   *_mulle_objc_object_call_class_needcache( void *obj,
                                                 mulle_objc_methodid_t methodid,
                                                 void *parameter,
                                                 struct _mulle_objc_class *cls);


# pragma mark  -
# pragma mark cache support

// internal, call
struct _mulle_objc_cacheentry *
   _mulle_objc_class_add_cacheentry_swappmethodcache( struct _mulle_objc_class *cls,
                                                      struct _mulle_objc_cache *cache,
                                                      struct _mulle_objc_method *method,
                                                      mulle_objc_methodid_t methodid,
                                                      enum mulle_objc_cachesizing_t sizing);

MULLE_C_NEVER_INLINE
struct _mulle_objc_cacheentry   *
   _mulle_objc_class_add_cacheentry_swapsupercache( struct _mulle_objc_class *cls,
                                                    struct _mulle_objc_cache *cache,
                                                    struct _mulle_objc_method *method,
                                                    mulle_objc_superid_t superid,
                                                    enum mulle_objc_cachesizing_t sizing);


void   mulle_objc_class_trace_call( struct _mulle_objc_class *cls,
                                    mulle_objc_methodid_t methodid,
                                    void *obj,
                                    void *parameter,
                                    mulle_objc_implementation_t imp);



# pragma mark  -
# pragma mark method lookup

// this will not update the cache
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nocache_noforward( struct _mulle_objc_class *cls,
                                                              mulle_objc_methodid_t methodid);

mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nocache( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t methodid);

// convenience
static inline mulle_objc_implementation_t
   _mulle_objc_object_lookup_implementation_nocache_noforward( void *obj,
                                                               mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_lookup_implementation_nocache_noforward( cls, methodid));
}


// convenience
static inline mulle_objc_implementation_t
   _mulle_objc_object_lookup_implementation_nocache( void *obj,
                                                     mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_lookup_implementation_nocache( cls, methodid));
}


// goes through cache returns an implementation if cached, NULL otherwise
// will return forward: if nothing found and (!) put it into the cache
mulle_objc_implementation_t
    _mulle_objc_class_lookup_implementation( struct _mulle_objc_class *cls,
                                             mulle_objc_methodid_t methodid);


// goes through cache returns an implementation if cached, NULL otherwise
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_cacheonly( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid);

// goes through cache returns an implementation if cached, trys to fill cache otherwise
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_noforward( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid);

// knows about trace and empty cache, forwards
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                   mulle_objc_methodid_t methodid);

mulle_objc_implementation_t
    mulle_objc_class_lookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                   mulle_objc_methodid_t methodid);


mulle_objc_implementation_t
   _mulle_objc_class_superlookup_implementation( struct _mulle_objc_class *cls,
                                                 mulle_objc_superid_t superid);


#pragma mark - low level support

void   *_mulle_objc_object_call_class( void *obj,
                                       mulle_objc_methodid_t methodid,
                                       void *parameter,
                                       struct _mulle_objc_class *cls);

static inline void   _mulle_objc_object_finalize( void *obj)
{
   struct _mulle_objc_class   *isa;

   isa = _mulle_objc_object_get_isa( obj);
   assert( isa);
#ifdef __MULLE_OBJC_FCS__
   _mulle_objc_fastmethodtable_invoke( obj, MULLE_OBJC_FINALIZE_METHODID, NULL, &isa->vtab, 2);
#else
   _mulle_objc_object_call_class( obj, MULLE_OBJC_FINALIZE_METHODID, obj, isa);
#endif
}


static inline void   _mulle_objc_object_dealloc( void *obj)
{
   struct _mulle_objc_class   *isa;

   isa = _mulle_objc_object_get_isa( obj);
   assert( isa);
#ifdef __MULLE_OBJC_FCS__
   _mulle_objc_fastmethodtable_invoke( obj, MULLE_OBJC_DEALLOC_METHODID, NULL, &isa->vtab, 3);
#else
   _mulle_objc_object_call_class( obj, MULLE_OBJC_DEALLOC_METHODID, obj, isa);
#endif
}

# pragma mark - compat layer support

// don't use it yourself, it's supposed to be called automatically
void  _mulle_objc_class_setup( struct _mulle_objc_class *cls);


# pragma mark - API

// [self finalize]
static inline void   mulle_objc_object_finalize( void *obj)
{
   if( obj)
      _mulle_objc_object_finalize( obj);
}


// [self dealloc]
static inline void   mulle_objc_object_dealloc( void *obj)
{
   if( obj)
      _mulle_objc_object_dealloc( obj);
}

#endif
