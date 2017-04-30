//
//  mulle_objc_message.c
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

#include "mulle_objc_call.h"

#include "mulle_objc_class.h"
#include "mulle_objc_class_runtime.h"
#include "mulle_objc_methodlist.h"
#include "mulle_objc_object.h"
#include "mulle_objc_runtime.h"

#include <mulle_aba/mulle_aba.h>
#include <mulle_concurrent/mulle_concurrent.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>


# pragma mark - class cache

unsigned int   _mulle_objc_class_count_noninheritedmethods( struct _mulle_objc_class *cls)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_methodlist                    *list;
   unsigned int                                     count;

   count = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &cls->methodlists);
   while( list = _mulle_concurrent_pointerarrayenumerator_next( &rover))
      count += list->n_methods;
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   return( count);
}


mulle_objc_cache_uint_t  _mulle_objc_class_convenient_methodcache_size( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class   *next;
   mulle_objc_cache_uint_t    count;
   mulle_objc_cache_uint_t    pow2;
   mulle_objc_cache_uint_t    total;
   mulle_objc_cache_uint_t    preloads;

   // these are definitely in the cache
   preloads = _mulle_objc_class_count_preloadmethods( cls) + _mulle_objc_runtime_number_of_preloadmethods( cls->runtime);

   // just a heuristic
   total  = 0;
   next   = cls->superclass;
   for(;;)
   {
      count  = _mulle_objc_class_count_noninheritedmethods( cls);
      total += count;

      cls = next;
      if( ! cls)
         break;

      next = cls->superclass;

      // root class as superclass, just too big to be interesting
      if( ! next)
         break;
   }

   // heuristic time
   // Assume of all the methods available, called we only need 1/8,
   // but cache should be only 25% full

   // put preloads are a given
   total = (total >> 3) << 2;
   total += preloads;

   // calculate a nice power of 2 for cache size
   pow2 = 4;
   while( total > pow2)
      pow2 += pow2;

   return( pow2);
}


static inline void   _mulle_objc_class_preload_inactivecache( struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = cls->runtime;
   _mulle_objc_class_fill_inactivecache_with_preload_array_of_methodids( cls, cache, runtime->methodidstopreload.methodids, runtime->methodidstopreload.n);
}



//
// fills the cache line with a forward: if message does not exist or
// if it exists it fills up the entry
//
MULLE_C_NEVER_INLINE
struct _mulle_objc_cacheentry  *_mulle_objc_class_add_entry_by_swapping_caches( struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache, struct _mulle_objc_method *method, mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         new_size;

   old_cache = cache;

   // a new beginning.. let it be filled anew
   new_size = cache->size * 2;
   cache    = mulle_objc_cache_new( new_size, &cls->runtime->memory.allocator);

   // fill it up with preload messages and place our method there too
   // now for good measure

   if( _mulle_objc_class_count_preloadmethods( cls))
      _mulle_objc_class_fill_inactivecache_with_preloadmethods( cls, cache);
   if( _mulle_objc_runtime_number_of_preloadmethods( cls->runtime))
      _mulle_objc_class_preload_inactivecache( cls, cache);

   entry = NULL;
   if( method)
      entry = _mulle_objc_cache_inactivecache_add_functionpointer_entry( cache, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method), methodid);

   //
   // an empty_cache ? this is getting called to early
   //
   assert( _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries) != mulle_objc_get_runtime()->empty_cache.entries);

   // if the set fails, then someone else was faster
   if( _mulle_objc_cachepivot_atomic_set_entries( &cls->cachepivot.pivot, cache->entries, old_cache->entries))
   {
      struct _mulle_objc_runtime   *runtime;

      runtime = _mulle_objc_class_get_runtime( cls);
      if( runtime->debug.trace.method_caches)
         fprintf( stderr, "mulle_objc_runtime %p trace: increased cache of %s %08x \"%s\" to %u entries\n",
                 runtime,
                 _mulle_objc_class_get_classtypename( cls),
                 _mulle_objc_class_get_classid( cls),
                 _mulle_objc_class_get_name( cls),
                 cache->size);
      _mulle_objc_cache_free( cache, &cls->runtime->memory.allocator); // sic, can be unsafe deleted now
      return( NULL);
   }

   if( &old_cache->entries[ 0] != &cls->runtime->empty_cache.entries[ 0])
      _mulle_objc_cache_abafree( old_cache, &cls->runtime->memory.allocator);

   return( entry);
}


MULLE_C_NEVER_INLINE
static struct _mulle_objc_cacheentry   *
    _mulle_objc_class_fill_cache_with_method( struct _mulle_objc_class *cls,
                                              struct _mulle_objc_method *method,
                                              mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;

   assert( cls);
   assert( method);

   // need to check that we are initialized
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CACHE_INITIALIZED))
   {
      _mulle_objc_runtime_raise_inconsistency_exception( cls->runtime, "call comes to early, the %s \"%s\" hasn't been initialized yet.", _mulle_objc_class_get_classtypename( cls), cls->name);
   }

   // when we trace method calls, we don't cache ever
   if( _mulle_objc_class_get_runtime( cls)->debug.trace.method_calls)
      return( NULL);

   //
   //  try to get most up to date value
   //
   for(;;)
   {
      cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
      if( (size_t) _mulle_atomic_pointer_read( &cache->n) >= (cache->size >> 2))  // 25% fill rate is nicer
      {
         entry = _mulle_objc_class_add_entry_by_swapping_caches( cls, cache, method, methodid);
         if( entry)
            return( entry);
         continue;
      }

      entry = _mulle_objc_cache_add_functionpointer_entry( cache, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method), methodid);
      if( entry)
         return( entry);
   }
}



//mulle_objc_methodimplementation_t   mulle_objc_class_unfailing_search_methodimplementation( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid)
//{
//   struct _mulle_objc_method   *method;
//
//   method  = _mulle_objc_class_unfailing_search_method( cls, methodid);
//   return( method->implementation);
//}


# pragma mark - trace method

void   mulle_objc_class_trace_method_call( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t methodid,
                                           void *obj,
                                           void *parameter,
                                           mulle_objc_methodimplementation_t imp)
{
   struct _mulle_objc_methoddescriptor   *desc;
   struct _mulle_objc_runtime            *runtime;

   runtime = _mulle_objc_class_get_runtime( cls);
   desc    = _mulle_objc_runtime_lookup_methoddescriptor( runtime, methodid);

   // [::] is just there to grep it
   if( desc)
      fprintf( stderr, "[::] %c[%s %s] (%s@%p)\n",
            _mulle_objc_class_is_metaclass( cls) ? '+' : '-',
            _mulle_objc_class_get_name( _mulle_objc_object_get_isa( obj)),
            desc->name,
            _mulle_objc_class_get_name( cls),
            imp);
   else
      fprintf( stderr, "[::] %c[%s #%08x] (%s@%p)\n",
            _mulle_objc_class_is_metaclass( cls) ? '+' : '-',
            _mulle_objc_class_get_name( _mulle_objc_object_get_isa( obj)),
            methodid,
            _mulle_objc_class_get_name( cls),
            imp);
}


//
// not inlining this is better, because when inlined clang pushes too many
// registers on the stack, what otherwise would be a slim function for the
// most general case
//
MULLE_C_NEVER_INLINE
static void   *_mulle_objc_object_unfailing_call_methodid( void *obj,
                                                           mulle_objc_methodid_t methodid,
                                                           void *parameter,
                                                           struct  _mulle_objc_class *cls)
{
   mulle_objc_methodimplementation_t   imp;
   struct _mulle_objc_runtime          *runtime;
   struct _mulle_objc_method           *method;
   struct _mulle_objc_cacheentry       *entry;

   method  = _mulle_objc_class_unfailing_search_method( cls, methodid);
   imp     = _mulle_objc_method_get_implementation( method);

   runtime = _mulle_objc_class_get_runtime( cls);
   if( runtime->debug.trace.method_calls)
   {
      mulle_objc_class_trace_method_call( cls, methodid, obj, parameter, imp);
   }
   else
   {
      // some special classes may choose to never cache
      if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_ALWAYS_EMPTY_CACHE))
      {
         entry = _mulle_objc_class_fill_cache_with_method( cls, method, methodid);
         if( entry)
            entry->key.uniqueid = methodid; // overwrite in forward case
      }
   }

/*->*/
   return( (*imp)( obj, methodid, parameter));
}


static int  preload( struct _mulle_objc_method *method, struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache)
{
   assert( cache);

   if( _mulle_objc_methoddescriptor_is_preload_method( &method->descriptor))
      _mulle_objc_cache_inactivecache_add_pointer_entry( cache, _mulle_objc_method_get_implementation( method), method->descriptor.methodid);

   return( 0);
}



void   _mulle_objc_class_fill_inactivecache_with_preloadmethods( struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache)
{
   _mulle_objc_class_walk_methods( cls, _mulle_objc_class_get_inheritance( cls), (int(*)()) preload, cache);
}


void   _mulle_objc_class_fill_inactivecache_with_preload_array_of_methodids( struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache, mulle_objc_methodid_t *methodids, unsigned int n)
{
   mulle_objc_methodid_t      *p;
   mulle_objc_methodid_t      *sentinel;
   struct _mulle_objc_method   *method;

   p        = methodids;
   sentinel = &p[ n];
   while( p < sentinel)
   {
      method = mulle_objc_class_search_method( cls, *p++);
      if( method)
         _mulle_objc_cache_inactivecache_add_pointer_entry( cache, _mulle_objc_method_get_implementation( method), method->descriptor.methodid);
   }
}



# pragma mark - object

mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_cached_methodimplementation( struct _mulle_objc_class *cls,
                                                                                         mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *entries;
   mulle_objc_cache_uint_t         offset;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   cache   = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_offset_for_uniqueid( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   return( (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer));
}


mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_or_search_methodimplementation_no_forward( struct _mulle_objc_class *cls,
                                                                                             mulle_objc_methodid_t methodid)
{
   mulle_objc_methodimplementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_method           *method;
   mulle_objc_cache_uint_t             offset;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   cache   = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_offset_for_uniqueid( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];

   imp = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
   {
      if( _mulle_objc_class_is_forwardmethodimplementation( cls, imp))
         imp = NULL;
   }
   else
   {
      method = mulle_objc_class_search_method( cls, methodid);
      if( method)
         imp = _mulle_objc_method_get_implementation( method);
   }

   return( imp);
}


//
// this updates the cache, if nothing is there yet
// will not return "forward though"...
// because it updates the cache, it's not lookup_or_search but just lookup
//
mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_methodimplementation_no_forward( struct _mulle_objc_class *cls,
                                                                                                mulle_objc_methodid_t methodid)
{
   mulle_objc_methodimplementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_method           *method;
   mulle_objc_cache_uint_t             offset;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   cache   = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_offset_for_uniqueid( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
   {
      if( _mulle_objc_class_is_forwardmethodimplementation( cls, imp))
         return( NULL);
      return( imp);
   }

   method = mulle_objc_class_search_method( cls, methodid);
   if( method)
   {
      imp = _mulle_objc_method_get_implementation( method);
      _mulle_objc_class_fill_cache_with_method( cls, method, methodid);
   }
   return( imp);
}


mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_or_search_methodimplementation( struct _mulle_objc_class *cls,
                                                                                  mulle_objc_methodid_t methodid)
{
   mulle_objc_cache_uint_t             offset;
   mulle_objc_methodimplementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_method           *method;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   offset  = _mulle_objc_cache_offset_for_uniqueid( cache, methodid);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
      return( imp);

   method = mulle_objc_class_search_method( cls, methodid);
   if( ! method)
   {
      method = _mulle_objc_class_getorsearch_forwardmethod( cls);
      if( ! method)
         return( NULL);
   }
   return( _mulle_objc_method_get_implementation( method));
}


mulle_objc_methodimplementation_t   _mulle_objc_class_lookup_methodimplementation( struct _mulle_objc_class *cls,
                                                                                     mulle_objc_methodid_t methodid)
{
   mulle_objc_cache_uint_t             offset;
   mulle_objc_methodimplementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_method           *method;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   offset  = _mulle_objc_cache_offset_for_uniqueid( cache, methodid);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
      return( imp);

   method = mulle_objc_class_search_method( cls, methodid);
   if( ! method)
      method = _mulle_objc_class_getorsearch_forwardmethod( cls);

   if( method)
   {
      imp = _mulle_objc_method_get_implementation( method);
      _mulle_objc_class_fill_cache_with_method( cls, method, methodid);
   }
   return( imp);
}


mulle_objc_methodimplementation_t   mulle_objc_class_unfailing_lookup_methodimplementation( struct _mulle_objc_class *cls,
                                                                                     mulle_objc_methodid_t methodid)
{
   mulle_objc_methodimplementation_t   imp;

   // this is compatible with apple and doesn't really hurt
   // does it crash with unfailing ? not really
   if( ! cls)
      return( 0);

   imp = _mulle_objc_class_lookup_methodimplementation( cls, methodid);
   if( ! imp)
      _mulle_objc_runtime_raise_fail_exception( cls->runtime, "no forward: method found");
   return( imp);
}


# pragma mark - lldb support
mulle_objc_methodimplementation_t   mulle_objc_lldb_lookup_methodimplementation( void *obj,
                                                                                 mulle_objc_methodid_t methodid,
                                                                                 void *cls_or_classid,
                                                                                 int is_classid,
                                                                                 int is_meta,
                                                                                 int debug)
{
   struct _mulle_objc_class            *cls;
   struct _mulle_objc_metaclass        *meta;
   struct _mulle_objc_runtime          *runtime;
   struct _mulle_objc_infraclass       *found;
   struct _mulle_objc_class            *call_cls;
   mulle_objc_methodimplementation_t   imp;

   if( debug)
      fprintf( stderr, "lookup %p %08x %p (%d)\n", obj, methodid, cls_or_classid, is_classid);

   if( ! obj || methodid == MULLE_OBJC_NO_METHODID || methodid == MULLE_OBJC_INVALID_METHODID)
      return( 0);

   // ensure class init
   cls  = is_meta ? obj : _mulle_objc_object_get_isa( obj);
   meta = _mulle_objc_class_is_metaclass( cls)
               ? (struct _mulle_objc_metaclass *) cls
               : _mulle_objc_class_get_metaclass( cls);
   if( ! _mulle_objc_metaclass_get_state_bit( meta, MULLE_OBJC_META_INITIALIZE_DONE))
      mulle_objc_object_call( cls, MULLE_OBJC_CLASS_METHODID, NULL);

   if( is_classid)
   {
      runtime  = _mulle_objc_class_get_runtime( cls);
      found    = _mulle_objc_runtime_unfailing_get_or_lookup_infraclass( runtime,
                                (mulle_objc_classid_t) (uintptr_t) cls_or_classid);
      if( is_meta)
         call_cls = _mulle_objc_metaclass_as_class( _mulle_objc_infraclass_get_metaclass( found));
      else
         call_cls = _mulle_objc_infraclass_as_class( found);
   }
   else
      call_cls = cls_or_classid;


   imp = _mulle_objc_class_lookup_or_search_methodimplementation( call_cls, methodid);
   if( debug)
   {
      char   buf[ s_mulle_objc_sprintf_functionpointer_buffer];
      mulle_objc_sprintf_functionpointer( buf, (mulle_functionpointer_t) imp);
      fprintf( stderr, "resolved to %p -> %s\n", call_cls, buf);
   }
   return( imp);
}


# pragma mark - calls

void   *mulle_objc_object_call_class( void *obj,
                                      mulle_objc_methodid_t methodid,
                                      void *parameter,
                                      struct _mulle_objc_class *cls)
{
   mulle_objc_methodimplementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   mulle_objc_cache_uint_t             mask;
   mulle_objc_cache_uint_t             offset;

   assert( obj);
   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   for(;;)
   {
      offset = offset & mask;
      entry = (void *) &((char *) entries)[ offset];

      if( entry->key.uniqueid == methodid)
      {
         imp = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
/*->*/   return( (*imp)( obj, methodid, parameter));
      }

      if( ! entry->key.uniqueid)
/*->*/   return( _mulle_objc_object_unfailing_call_methodid( obj, methodid, parameter, cls));

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


//
// this function is called, when the first inline cache check gave a
// collision
//
void   *mulle_objc_object_call2( void *obj, mulle_objc_methodid_t methodid, void *parameter)
{
   mulle_objc_methodimplementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_class            *cls;
   mulle_objc_cache_uint_t             mask;
   mulle_objc_cache_uint_t             offset;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   for(;;)
   {
      offset += sizeof( struct _mulle_objc_cacheentry);
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == methodid)
      {
         imp = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         return( (*imp)( obj, methodid, parameter));
      }

      if( ! entry->key.uniqueid)
      /*->*/   return( _mulle_objc_object_unfailing_call_methodid( obj, methodid, parameter, cls));
   }
}


void   *mulle_objc_object_call_uncached_class( void *obj,  mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;

   method = mulle_objc_class_search_method( cls, methodid);
   if( ! method)
      method = _mulle_objc_class_unfailing_getorsearch_forwardmethod( cls, methodid);
   return( (*_mulle_objc_method_get_implementation( method))( obj, methodid, parameter));
}


//
// specialized function when cache is empty
//
void   *mulle_objc_object_call_class_empty_cache( void *obj,
                                                  mulle_objc_methodid_t methodid,
                                                  void *parameter,
                                                  struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;

   method = _mulle_objc_class_unfailing_search_method( cls, methodid);
   return( (*_mulle_objc_method_get_implementation( method))( obj, methodid, parameter));
}


void   *mulle_objc_object_call2_empty_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter)
{
   struct _mulle_objc_class    *cls;
   struct _mulle_objc_method   *method;

   cls    = _mulle_objc_object_get_isa( obj);
   method = _mulle_objc_class_unfailing_search_method( cls, methodid);
   return( (*_mulle_objc_method_get_implementation( method))( obj, methodid, parameter));
}


MULLE_C_NEVER_INLINE
void   *mulle_objc_object_call( void *obj,
                                mulle_objc_methodid_t methodid,
                                void *parameter)
{
   struct _mulle_objc_class   *cls;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   cls = _mulle_objc_object_get_isa( obj);
   return( (*cls->call)( obj, methodid, parameter, cls));
}


void   mulle_objc_objects_call( void **objects, unsigned int n, mulle_objc_methodid_t methodid, void *params)
{
   mulle_objc_methodimplementation_t   (*lookup)( struct _mulle_objc_class *, mulle_objc_methodid_t);
   mulle_objc_methodimplementation_t   imp;
   mulle_objc_methodimplementation_t   lastSelIMP[ 16];
   struct _mulle_objc_class            *lastIsa[ 16];
   struct _mulle_objc_class            *thisIsa;
   unsigned int                        i;
   void                                **sentinel;
   void                                *p;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);
   memset( lastIsa, 0, sizeof( lastIsa));

   // assume compiler can do unrolling
   lookup   = _mulle_objc_class_lookup_methodimplementation_no_forward;
   sentinel = &objects[ n];

   while( objects < sentinel)
   {
      p = *objects++;
      if( ! p)
         continue;

      // our IMP cacheing thing
      thisIsa = _mulle_objc_object_get_isa( p);
      assert( thisIsa);

      i = ((uintptr_t) thisIsa >> sizeof( uintptr_t)) & 15;

      if( lastIsa[ i] != thisIsa)
      {
         imp            = (*lookup)( thisIsa, methodid);
         lastIsa[ i]    = thisIsa;
         lastSelIMP[ i] = imp;

         assert( imp);
      }

      (*lastSelIMP[ i])( p, methodid, params);
   }
}


void   *mulle_objc_infraclass_metacall_classid( struct _mulle_objc_infraclass *infra,
                                                mulle_objc_methodid_t methodid,
                                                void *parameter,
                                                mulle_objc_classid_t classid)
{
   if( ! infra)
      return( infra);
   return( _mulle_objc_infraclass_inline_metacall_classid( infra, methodid, parameter, classid));
}


void   *mulle_objc_object_call_classid( void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter,
                                        mulle_objc_classid_t classid)
{
   if( ! obj)
      return( 0);
   return( _mulle_objc_object_inline_call_classid( obj, methodid, parameter, classid));
}

