//
//  mulle_objc_call.h
//  mulle-objc
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

#include "mulle_objc_class.h"
#include "mulle_objc_cache.h"
#include "mulle_objc_method.h"
#include "mulle_objc_object.h"

#include "mulle_objc_class_runtime.h"

#include <mulle_aba/mulle_aba.h>
#include <mulle_c11/mulle_c11.h>
#include <assert.h>

//
// all stuff concerned with "calling" methods
//
# pragma mark  -
# pragma mark internal calls

void   *mulle_objc_object_call_class( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls);
void   *mulle_objc_object_call_uncached_class( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls);
void   *mulle_objc_object_call2( void *obj, mulle_objc_methodid_t methodid, void *parameter);

void   *mulle_objc_object_call2_empty_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter);
void   *mulle_objc_object_call_class_empty_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls);


//// on x86_64 this should optimize into 30 bytes tops w/o tagged pointers
//
// When inlining, we are "fine" if the cache is stale
// Unfortunately llvm sometimes really produces pathetic code.
//
// use this for -O3
//
MULLE_C_ALWAYS_INLINE
static inline void  *mulle_objc_object_inline_constant_methodid_call( void *obj,
                                                                      mulle_objc_methodid_t methodid,
                                                                      void *parameter)
{
   int                                 index;
   mulle_objc_methodimplementation_t   f;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_class            *cls;
   mulle_objc_cache_uint_t             mask;
   mulle_objc_cache_uint_t             offset;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   //
   // with tagged pointers inlining starts to become useless, because this
   // _mulle_objc_object_get_isa function produces too much code IMO
   //
   cls = _mulle_objc_object_get_isa( obj);

   //
   // try to simplify to return( (*cls->vtab.methods[ index])( obj, methodid, parameter)
   //
   index = mulle_objc_get_fastmethodtable_index( methodid);
   if( index >= 0)
      return( _mulle_objc_fastmethodtable_invoke( obj, methodid, parameter, &cls->vtab, index));

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) methodid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( __builtin_expect( (entry->key.uniqueid == methodid), 1))
      f = (mulle_objc_methodimplementation_t) _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer);
   else
      f = cls->cachepivot.call2;
   return( (*f)( obj, methodid, parameter));
}


//
// use this for -O -O1
//
MULLE_C_ALWAYS_INLINE
static inline void   *mulle_objc_object_constant_methodid_call( void *obj,
                                                                mulle_objc_methodid_t methodid,
                                                                void *parameter)
{
   struct _mulle_objc_class   *cls;
   int                        index;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   cls   = _mulle_objc_object_get_isa( obj);
   index = mulle_objc_get_fastmethodtable_index( methodid);
   if( index >= 0)
      return( _mulle_objc_fastmethodtable_invoke( obj, methodid, parameter, &cls->vtab, index));

   return( (*cls->call)( obj, methodid, parameter, cls));
}


//
// this is the default method to use, when you used to call objc_msgSend
// and he selector is variable
//
MULLE_C_ALWAYS_INLINE
static inline void  *mulle_objc_object_inline_variable_methodid_call( void *obj,
                                                                      mulle_objc_methodid_t methodid,
                                                                      void *parameter)
{
   mulle_objc_methodimplementation_t   f;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_class            *cls;
   mulle_objc_cache_uint_t             mask;
   mulle_objc_cache_uint_t             offset;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) methodid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( __builtin_expect( (entry->key.uniqueid == methodid), 1))
      f = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   else
      f = cls->cachepivot.call2;
   return( (*f)( obj, methodid, parameter));
}


//
// use this when you know methodid is not a constant and you want to keep it
// down
//
MULLE_C_ALWAYS_INLINE
static inline void   *mulle_objc_object_variable_methodid_call( void *obj,
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
// class method implementations do not change during its run
//
void   mulle_objc_objects_call( void **objects, unsigned int n, mulle_objc_methodid_t sel, void *params);



#pragma mark - calls for super

#if 0 // unused apparently
static inline void   *_mulle_objc_class_call_classid( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid,
                                                      void *parameter,
                                                      mulle_objc_classid_t classid)
{
   struct _mulle_objc_class   *call_cls;
   struct _mulle_objc_runtime *runtime;

   runtime  = _mulle_objc_class_get_runtime( cls);
   call_cls = _mulle_objc_runtime_unfailing_get_or_lookup_class( runtime, classid);
   // need to call cls->call to prepare caches
   return( (*call_cls->call)( cls, methodid, parameter, call_cls));
}


// this is used for calling super on classes, the classid is determined by
// the compiler since it is a super call, self is known to be non-nil.
//
static inline void   *mulle_objc_class_call_classid( struct _mulle_objc_class *cls,
                                                     mulle_objc_methodid_t methodid,
                                                     void *parameter,
                                                     mulle_objc_classid_t classid)
{

   if( ! cls)
      return( cls);
   return( _mulle_objc_class_call_classid( cls, methodid, parameter, classid));
}

#endif


//
// this is used for calling super on class methods, the classid is determined by
// the compiler since it is a super call, self is known to be non-nil.
//
static inline void   *_mulle_objc_class_inline_metacall_classid( struct _mulle_objc_class *cls,
                                                                 mulle_objc_methodid_t methodid,
                                                                 void *parameter,
                                                                 mulle_objc_classid_t classid)
{
   struct _mulle_objc_class     *call_cls;
   struct _mulle_objc_runtime   *runtime;

   runtime = _mulle_objc_class_get_runtime( cls);
   call_cls = (cls->superclassid == classid)
               ? cls->superclass
               : _mulle_objc_runtime_unfailing_get_or_lookup_class( runtime, classid);
   call_cls = _mulle_objc_class_get_metaclass( call_cls);
   // need to call cls->call to prepare caches
   return( (*call_cls->call)( cls, methodid, parameter, call_cls));
}


static inline void   *mulle_objc_class_inline_metacall_classid( struct _mulle_objc_class *cls,
                                                                mulle_objc_methodid_t methodid,
                                                                void *parameter,
                                                                mulle_objc_classid_t classid)
{

   if( ! cls)
      return( cls);
   return( _mulle_objc_class_inline_metacall_classid( cls, methodid, parameter, classid));
}


void   *mulle_objc_class_metacall_classid( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t methodid,
                                           void *parameter,
                                           mulle_objc_classid_t classid);


//
// this is used for calling super on instances, the classid is determined by
// the compiler since it is a super call, self is known to be non-nil.
//
static inline void   *_mulle_objc_object_inline_call_classid( void *obj,
                                                              mulle_objc_methodid_t methodid,
                                                              void *parameter,
                                                              mulle_objc_classid_t classid)
{
   struct _mulle_objc_class     *call_cls;
   struct _mulle_objc_class     *cls;
   struct _mulle_objc_runtime   *runtime;

   cls      = _mulle_objc_object_get_isa( obj);
   runtime  = _mulle_objc_class_get_runtime( cls);
   call_cls = (cls->superclassid == classid)
               ? cls->superclass
               : _mulle_objc_runtime_unfailing_get_or_lookup_class( runtime, classid);
   // need to call cls->call to prepare caches
   return( (*call_cls->call)( obj, methodid, parameter, call_cls));
}



# pragma mark  -
# pragma mark cache support

unsigned int  _mulle_objc_class_count_noninheritedmethods( struct _mulle_objc_class *cls);
unsigned int  _mulle_objc_class_convenient_methodcache_size( struct _mulle_objc_class *cls);
void          _mulle_objc_class_fill_inactivecache_with_preload_methods( struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache);
void          _mulle_objc_class_fill_inactivecache_with_preload_array_of_methodids( struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache, mulle_objc_methodid_t *methodids, unsigned int n);

// internal, call
struct _mulle_objc_cacheentry
   *_mulle_objc_class_add_entry_by_swapping_caches( struct _mulle_objc_class *cls,
                                                    struct _mulle_objc_cache *cache,
                                                    struct _mulle_objc_method *method,
                                                    mulle_objc_methodid_t methodid);

void   mulle_objc_class_trace_method_call( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t methodid,
                                           void *obj,
                                           void *parameter,
                                           mulle_objc_methodimplementation_t imp);


# pragma mark  -
# pragma mark method lookup

// this will not update the cache
mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_or_search_methodimplementation_no_forward( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);

mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_or_search_methodimplementation( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);

// convenience
static inline mulle_objc_methodimplementation_t   _mulle_objc_object_lookup_or_search_methodimplementation_no_forward( struct _mulle_objc_object *obj, mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_lookup_or_search_methodimplementation_no_forward( cls, methodid));
}

// convenience
static inline mulle_objc_methodimplementation_t   _mulle_objc_object_lookup_or_search_methodimplementation( struct _mulle_objc_object *obj, mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_lookup_or_search_methodimplementation( cls, methodid));
}


// goes through cache returns an implementation if cached, NULL otherwise
// will return forward:: if nothing found and (!) put it into the cache
mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_methodimplementation( struct _mulle_objc_class *cls,
                                                                          mulle_objc_methodid_t methodid);

mulle_objc_methodimplementation_t   mulle_objc_class_unfailing_lookup_methodimplementation( struct _mulle_objc_class *cls,
                                                                                    mulle_objc_methodid_t methodid);

// goes through cache returns an implementation if cached, NULL otherwise
mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_cached_methodimplementation( struct _mulle_objc_class *cls,
                                                                              mulle_objc_methodid_t methodid);

// goes through cache returns an implementation if cached, trys to fill cache otherwise
mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_methodimplementation_no_forward( struct _mulle_objc_class *cls,
                                                                         mulle_objc_methodid_t methodid);

#pragma mark - lldb support

mulle_objc_methodimplementation_t   mulle_objc_lldb_lookup_methodimplementation( void *object,
                                                                                 mulle_objc_methodid_t sel,
                                                                                 void *cls_or_classid,
                                                                                 int is_classid,
                                                                                 int is_meta,
                                                                                 int debug);

#pragma mark - low level support

static inline void   _mulle_objc_object_finalize( void *obj)
{
   struct _mulle_objc_class   *isa;

   isa = _mulle_objc_object_get_isa( obj);
   assert( isa);
   _mulle_objc_fastmethodtable_invoke( obj, MULLE_OBJC_FINALIZE_METHODID, NULL, &isa->vtab, 2);
}


static inline void   _mulle_objc_object_dealloc( void *obj)
{
   struct _mulle_objc_class   *isa;

   isa = _mulle_objc_object_get_isa( obj);
   assert( isa);
   _mulle_objc_fastmethodtable_invoke( obj, MULLE_OBJC_DEALLOC_METHODID, NULL, &isa->vtab, 3);
}


/* if you need to "manually" call a MetaABI function with a _param block
   use mulle_objc_metaabi_param_block to generate it.

   ex.

   mulle_objc_metaabi_param_block( NSRange, NSUInteger)   _param;

   _param.p = NSMakeRange( 1, 1);
   mulle_objc_object_call( obj, sel, &_param);
   return( _param.rval);
*/

#define mulle_objc_void_5_pointers( size)  \
   (((size) +  sizeof( void *[ 5]) - 1) /  sizeof( void *[ 5]))

#define mulle_objc_metaabi_param_block( param_type, rval_type) \
   union                   \
   {                       \
      rval_type    r;      \
      param_type   p;      \
      void         *space[ 5][ sizeof( rval_type) > sizeof( param_type)        \
                         ?  mulle_objc_void_5_pointers( sizeof( rval_type))    \
                         :  mulle_objc_void_5_pointers( sizeof( param_type))]; \
   }

#define mulle_objc_metaabi_param_block_void_return( param_type) \
   mulle_objc_metaabi_param_block( param_type, void *)


# pragma mark - API

static inline void   mulle_objc_object_finalize( void *obj)
{
   if( obj)
      _mulle_objc_object_finalize( obj);
}


static inline void   mulle_objc_object_dealloc( void *obj)
{
   if( obj)
      _mulle_objc_object_dealloc( obj);
}


# pragma mark  -
# pragma mark API Calls

// compiler uses this for -O0, -Os, it does no fast calls
void   *mulle_objc_object_call( void *obj,
                               mulle_objc_methodid_t methodid,
                               void *parameter);



// this is used for calling super on classes, the classid is determined by
// the compiler since it is a super call, self is known to be non-nil.
//
static inline void   *mulle_objc_object_inline_call_classid( void *obj,
                                                             mulle_objc_methodid_t methodid,
                                                             void *parameter,
                                                             mulle_objc_classid_t classid)
{
   if( ! obj)
      return( obj);
   return( _mulle_objc_object_inline_call_classid( obj, methodid, parameter, classid));
}


void   *mulle_objc_object_call_classid( void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter,
                                        mulle_objc_classid_t classid);

#endif
