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

#include "include.h"

#include "mulle-objc-class.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-cache.h"
#include "mulle-objc-impcache.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-method.h"
#include "mulle-objc-methodidconstants.h"
#include "mulle-objc-object.h"
#include "mulle-objc-fastmethodtable.h"
#include "mulle-objc-universe-class.h"

#include <assert.h>

// so the general idea here is, that the compiler even in debug mode will
// produce a JMP to the method and not a JSR, which is beneficial when
// stepping with the debugger through the code
// Unfortunately clang can not do this, so useless. RATS!
//
// #ifndef MULLE_OBJC_OPTIMIZE_CALL
// # if defined( __GNUC__) && ! defined( MULLE_OBJC_DEBUG_METHOD_CALLS)
// #  define MULLE_OBJC_OPTIMIZE_CALL   __attribute__((optimize("O3")))
// # else
// #  define MULLE_OBJC_OPTIMIZE_CALL
// # endif
// #endif


#ifndef MULLE_OBJC_CALL_PREFER_FCS
# define MULLE_OBJC_CALL_PREFER_FCS   0
#endif



MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_implementation_trace( mulle_objc_implementation_t imp,
                                        void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter,
                                        struct _mulle_objc_class *cls);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_object_taocheck_call( void *obj,
                                        mulle_objc_methodid_t methodid);


static inline mulle_objc_implementation_t
   _mulle_objc_implementation_debug( mulle_objc_implementation_t imp,
                                     void *obj,
                                     mulle_objc_methodid_t methodid,
                                     void *parameter,
                                     struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_object_get_universe( cls);
   if( universe->debug.method_call & MULLE_OBJC_UNIVERSE_CALL_TRACE_BIT)
      mulle_objc_implementation_trace( imp, obj, methodid, parameter, cls);
   if( universe->debug.method_call & MULLE_OBJC_UNIVERSE_CALL_TAO_BIT)
      mulle_objc_object_taocheck_call( obj, methodid);
   return( imp);
}


static inline
void   mulle_objc_implementation_debug( mulle_objc_implementation_t imp,
                                        void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter)

{
   struct _mulle_objc_class     *cls;

   cls = _mulle_objc_object_get_isa( obj);
   _mulle_objc_implementation_debug( imp, obj, methodid, parameter, cls);
}


MULLE_C_STATIC_ALWAYS_INLINE
void  *
   _mulle_objc_implementation_invoke( mulle_objc_implementation_t imp,
                                      void *self,
                                      mulle_objc_methodid_t sel,
                                      void *parameter)
{
   return( (*imp)( self, sel, parameter));
}



MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_implementation_invoke( mulle_objc_implementation_t imp,
                                     void *self,
                                     mulle_objc_methodid_t sel,
                                     void *parameter)
{
   // this is slow
#if defined( DEBUG) || defined( MULLE_OBJC_INCLUDE_TRACE_CODE)
   mulle_objc_implementation_debug( imp, self, sel, parameter);
#endif
   return( (*imp)( self, sel, parameter));
}



//
// calls are always "nofail", whereas lookup can fail (and return nil)
// calls can return nil too, often, do but that's not a fail

// clang and gcc complain when artificial functions aren't inlineable :(


# pragma mark - API Calls


// compiler uses this for -O0, -Os, it does no fast calls
// TODO: 1. rename to mulle_objc_object_standardcall
//       2. create mulle_objc_object_call, that uses any of the three call
//          types based on optimization level
//

//MULLE_C_ARTIFICIAL
MULLE_OBJC_RUNTIME_GLOBAL
void   *mulle_objc_object_call( void *obj,
                                mulle_objc_methodid_t methodid,
                                void *parameter);

MULLE_OBJC_RUNTIME_GLOBAL
void   *_mulle_objc_object_call( void *obj,
                                 mulle_objc_methodid_t methodid,
                                 void *parameter);

//
// use this for -O -O2
//
MULLE_C_STATIC_ALWAYS_INLINE
void *
   mulle_objc_object_call_inline_minimal( void *obj,
                                          mulle_objc_methodid_t methodid,
                                          void *parameter)
{
   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   return( _mulle_objc_object_call( obj, methodid, parameter));
}


#ifdef __MULLE_OBJC_FCS__

MULLE_C_STATIC_ALWAYS_INLINE
void *
   _mulle_objc_fastmethodtable_get_imp( struct _mulle_objc_fastmethodtable *table,
                                        unsigned int index)
{
   mulle_objc_implementation_t   imp;

   imp = (mulle_objc_implementation_t) _mulle_atomic_pointer_read( &table->methods[ index].pointer);
   return( imp);
}


MULLE_C_STATIC_ALWAYS_INLINE
void *
   _mulle_objc_fastmethodtable_invoke( void *obj,
                                       mulle_objc_methodid_t methodid,
                                       void *param,
                                       struct _mulle_objc_fastmethodtable *table,
                                       unsigned int index)
{
   mulle_objc_implementation_t   imp;

   imp = (mulle_objc_implementation_t) _mulle_atomic_pointer_read( &table->methods[ index].pointer);
   return( mulle_objc_implementation_invoke( imp, obj, methodid, param));
}

#endif


//
// use this for -O3. -O3 is the cmake default. But the "full" inline
// is a bit much for default release IMO
//
MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_object_call_inline_partial( void *obj,
                                          mulle_objc_methodid_t methodid,
                                          void *parameter)
{
#ifdef __MULLE_OBJC_FCS__
   int                             index;
#endif
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_impcache     *icache;

   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   cls = _mulle_objc_object_get_isa( obj);

#ifdef __MULLE_OBJC_FCS__
   index = mulle_objc_get_fastmethodtable_index( methodid);
   if( MULLE_C_EXPECT( index >= 0, MULLE_OBJC_CALL_PREFER_FCS))
      return( _mulle_objc_fastmethodtable_invoke( obj, methodid, parameter, &cls->vtab, index));
#endif

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   icache  = _mulle_objc_cache_get_impcache_from_cache( cache);
   return( (*icache->callback.call)( obj, methodid, parameter, cls));
}



//// on x86_64 this should optimize into 30 bytes tops w/o tagged pointers
//// Unfortunately llvm sometimes really produces pathetic code.
//
//
// use this for -fobjc-inline-method-calls=5 == full (loop completely inline)
// use this for -fobjc-inline-method-calls=4 == standard
// use this for -fobjc-inline-method-calls=3 == partial
// use this for -fobjc-inline-method-calls=2 == partial
// use this for -fobjc-inline-method-calls=1 == minimal, keep in sync with -Olevel
// use this for -fobjc-inline-method-calls=0 == none
//
//
MULLE_C_STATIC_ALWAYS_INLINE
void  *
   _mulle_objc_object_call_inline( void *obj,
                                   mulle_objc_methodid_t methodid,
                                   void *parameter)
{
#ifdef __MULLE_OBJC_FCS__
   int                             index;
#endif
   mulle_objc_implementation_t     f;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

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
   if( MULLE_C_EXPECT( index >= 0, MULLE_OBJC_CALL_PREFER_FCS)) // prefer fast methods path
      return( _mulle_objc_fastmethodtable_invoke( obj, methodid, parameter, &cls->vtab, index));
#endif

   assert( methodid);

   // MEMO: When inlining, we are "fine" if the cache is stale
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) methodid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( MULLE_C_LIKELY( entry->key.uniqueid == methodid))
      f = (mulle_objc_implementation_t) _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer);
   else
   {
      icache = _mulle_objc_cache_get_impcache_from_cache( cache);
      f      = icache->callback.call_cache_collision;
   }
   // don't use invoke, because the checking will be done in icache->call_cache_collision
   return( (*f)( obj, methodid, parameter));
}


MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_object_call_inline( void *obj,
                                  mulle_objc_methodid_t methodid,
                                  void *parameter)
{
   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   return( _mulle_objc_object_call_inline( obj, methodid, parameter));
}


MULLE_C_STATIC_ALWAYS_INLINE
void  *
   _mulle_objc_object_get_imp_inline_full_no_fcs( void *obj,
                                                  mulle_objc_methodid_t methodid)
{
   mulle_objc_implementation_t     f;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   assert( methodid);

   //
   // with tagged pointers inlining starts to become useless, because this
   // _mulle_objc_object_get_isa function produces too much code IMO
   //
   cls     = _mulle_objc_object_get_isa( obj);

   // MEMO: When inlining, we are "fine" if the cache is stale
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries
   offset  = (mulle_objc_cache_uint_t) methodid;
   for(;;)
   {
      offset  = (mulle_objc_cache_uint_t) offset & mask;
      entry   = (void *) &((char *) entries)[ offset];

      if( MULLE_C_LIKELY( entry->key.uniqueid == methodid))
      {
         f = (mulle_objc_implementation_t) _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer);
         break;
      }

      if( ! entry->key.uniqueid)
      {
         icache = _mulle_objc_cache_get_impcache_from_cache( cache);
         f      = icache->callback.call_cache_miss;
         break;
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }

   // don't use invoke, because the checking will be done in icache->call_cache_miss
   return( f);
}



//
// same as above, but we don't stop at the first miss. This adds some looping
// code to the inlined function, you might or might not deep acceptable. So
// if we miss the cache, we got to call_cache_miss.
//
MULLE_C_STATIC_ALWAYS_INLINE

void  *
   _mulle_objc_object_call_inline_full( void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter)
{
#ifdef __MULLE_OBJC_FCS__
   int                             index;
#endif
   mulle_objc_implementation_t     f;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

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
   if( MULLE_C_EXPECT( index >= 0, MULLE_OBJC_CALL_PREFER_FCS)) // prefer fast methods path
      return( _mulle_objc_fastmethodtable_invoke( obj, methodid, parameter, &cls->vtab, index));
#endif

   assert( methodid);

   // MEMO: When inlining, we are "fine" if the cache is stale
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries
   offset  = (mulle_objc_cache_uint_t) methodid;
   for(;;)
   {
      offset  = (mulle_objc_cache_uint_t) offset & mask;
      entry   = (void *) &((char *) entries)[ offset];

      if( MULLE_C_LIKELY( entry->key.uniqueid == methodid))
      {
         f = (mulle_objc_implementation_t) _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer);
         break;
      }

      if( ! entry->key.uniqueid)
      {
         icache = _mulle_objc_cache_get_impcache_from_cache( cache);
         f      = icache->callback.call_cache_miss;
         break;
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }
   // don't use invoke, because the checking will be done in icache->call_cache_miss
   return( (*f)( obj, methodid, parameter));
}


MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_object_call_inline_full( void *obj,
                                       mulle_objc_methodid_t methodid,
                                       void *parameter)
{
   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   return( _mulle_objc_object_call_inline_full( obj, methodid, parameter));
}


//
// this is the method to use, when the selector is variable. This doesn't
// do or attempt FCS, which would be too slow if the selector is not a
// constant.
//
MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_object_call_inline_variable( void *obj,
                                           mulle_objc_methodid_t methodid,
                                           void *parameter)
{
   mulle_objc_implementation_t     f;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   assert( methodid);

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) methodid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( MULLE_C_LIKELY( entry->key.uniqueid == methodid))
      f = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   else
   {
      icache = _mulle_objc_cache_get_impcache_from_cache( cache);
      f      = entry->key.uniqueid
               ? icache->callback.call_cache_collision
               : icache->callback.call_cache_miss;
   }
   // don't use invoke, because the checking will be done in icache->call_cache_...
   return( (*f)( obj, methodid, parameter));
}


// old mistyped name
MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_object_call_variable_inline( void *obj,
                                           mulle_objc_methodid_t methodid,
                                           void *parameter)
{
   return( mulle_objc_object_call_inline_variable( obj, methodid, parameter));
}


//
// this is the method to use, when the selector is variable. This doesn't
// do or attempt FCS, which would be too slow if the selector is not a
// constant.
//
MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_object_call_inline_full_variable( void *obj,
                                                mulle_objc_methodid_t methodid,
                                                void *parameter)
{
   mulle_objc_implementation_t     f;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   assert( methodid);

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries
   offset  = (mulle_objc_cache_uint_t) methodid;

   for(;;)
   {
      offset  = (mulle_objc_cache_uint_t) offset & mask;
      entry   = (void *) &((char *) entries)[ offset];

      if( MULLE_C_LIKELY( entry->key.uniqueid == methodid))
      {
         f = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         break;
      }

      if( ! entry->key.uniqueid)
      {
         icache = _mulle_objc_cache_get_impcache_from_cache( cache);
         f      = icache->callback.call_cache_miss;
         break;
      }
      offset += sizeof( struct _mulle_objc_cacheentry);
   }
   // don't use invoke, because the checking will be done in icache->call_cache_miss
   return( (*f)( obj, methodid, parameter));
}



//
// this is just an alias for mulle_objc_object_call, there is no difference
// in code/semantics
//
MULLE_C_STATIC_ALWAYS_INLINE
void   *mulle_objc_object_call_variable( void *obj,
                                         mulle_objc_methodid_t methodid,
                                         void *parameter)
{
   return( mulle_objc_object_call( obj, methodid, parameter));
}

#pragma mark - calls for super


// MULLE_C_ARTIFICIAL
// same as above just not inlining
MULLE_OBJC_RUNTIME_GLOBAL
void   *mulle_objc_object_call_super( void *obj,
                                      mulle_objc_methodid_t methodid,
                                      void *parameter,
                                      mulle_objc_superid_t superid);



MULLE_C_STATIC_ALWAYS_INLINE
void   *
   mulle_objc_object_call_super_inline_partial( void *obj,
                                                mulle_objc_methodid_t methodid,
                                                void *parameter,
                                                mulle_objc_superid_t superid)
{
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_impcache     *icache;

   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   icache  = _mulle_objc_cache_get_impcache_from_cache( cache);

   return( (*icache->callback.supercall)( obj, methodid, parameter, superid, cls));
}



MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_object_call_super_inline( void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter,
                                        mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t      f;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_impcache      *icache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_class         *cls;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;
   void                             *(*callback)( void *,
                                                  mulle_objc_methodid_t,
                                                  void *,
                                                  mulle_objc_superid_t,
                                                  struct _mulle_objc_class *);
   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   //
   // with tagged pointers inlining starts to become useless, because this
   // _mulle_objc_object_get_isa function produces too much code IMO
   //
   cls = _mulle_objc_object_get_isa( obj);

   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // preshifted so we can just AND it to entries

   offset  = (mulle_objc_cache_uint_t) superid & mask;
   entry   = (void *) &((char *) entries)[ offset];

   if( MULLE_C_EXPECT( (entry->key.uniqueid == superid), 1))
   {
      f = (mulle_objc_implementation_t) _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer);
         // don't use invoke, because its in the cache
      return( (*f)( obj, methodid, parameter));
   }

   icache   = _mulle_objc_cache_get_impcache_from_cache( cache);
   callback = entry->key.uniqueid
              ? icache->callback.supercall_cache_collision
              : icache->callback.supercall_cache_miss;
   return( (*callback)( obj, methodid, parameter, superid, cls));
}


MULLE_C_STATIC_ALWAYS_INLINE
void  *
   mulle_objc_object_call_super_inline_full( void *obj,
                                             mulle_objc_methodid_t methodid,
                                             void *parameter,
                                             mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t     f;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_impcache     *icache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_class        *cls;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   if( MULLE_C_UNLIKELY( ! obj))
      return( obj);

   //
   // with tagged pointers inlining starts to become useless, because this
   // _mulle_objc_object_get_isa function produces too much code IMO
   //
   cls = _mulle_objc_object_get_isa( obj);

   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_get_entries_atomic( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;  // pre-shifted so we can just AND it to entries
   offset  = (mulle_objc_cache_uint_t) superid;
   for(;;)
   {
      offset  = (mulle_objc_cache_uint_t) offset & mask;
      entry   = (void *) &((char *) entries)[ offset];

      if( MULLE_C_EXPECT( (entry->key.uniqueid == superid), 1))
      {
         f = (mulle_objc_implementation_t) _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer);
         // don't use invoke, because its in the cache
         return( (*f)( obj, methodid, parameter));
      }

      if( ! entry->key.uniqueid)
      {
         icache = _mulle_objc_cache_get_impcache_from_cache( cache);
         return( (*icache->callback.supercall_cache_miss)( obj, methodid, parameter, superid, cls));
      }

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


// this is useful for calling a list of objects efficiently, it is assumed that
// class/methods do not change during its run
//
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_objects_call( void **objects,
                                unsigned int n,
                                mulle_objc_methodid_t sel,
                                void *params);

#endif
