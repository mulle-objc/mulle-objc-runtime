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
#include "mulle_objc_class_universe.h"
#include "mulle_objc_methodlist.h"
#include "mulle_objc_object.h"
#include "mulle_objc_universe.h"

#include <mulle_aba/mulle_aba.h>
#include <mulle_concurrent/mulle_concurrent.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>



static void   *_mulle_objc_object_call_uncached_class( void *obj,
                                                       mulle_objc_methodid_t methodid,
                                                       void *parameter,
                                                       struct _mulle_objc_class *cls);

static void   *_mulle_objc_object_call2_empty_cache( void *obj,
                                                     mulle_objc_methodid_t methodid,
                                                     void *parameter);
static void   *_mulle_objc_object_call2( void *obj,
                                         mulle_objc_methodid_t methodid,
                                         void *parameter);

static void   *_mulle_objc_object_call_class( void *obj,
                                              mulle_objc_methodid_t methodid,
                                              void *parameter,
                                              struct _mulle_objc_class *cls);

static void   *_mulle_objc_object_unfailing_call_methodid( void *obj,
                                                           mulle_objc_methodid_t methodid,
                                                           void *parameter,
                                                           struct  _mulle_objc_class *cls);

# pragma mark - class cache

static unsigned int   _mulle_objc_class_count_noninheritedmethods( struct _mulle_objc_class *cls)
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


// TODO: investigate a better and simpler formula or use a sensible constant
static mulle_objc_cache_uint_t  _mulle_objc_class_convenient_methodcache_size( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class   *next;
   mulle_objc_cache_uint_t    count;
   mulle_objc_cache_uint_t    pow2;
   mulle_objc_cache_uint_t    total;
   mulle_objc_cache_uint_t    preloads;

   // these are definitely in the cache
   preloads = _mulle_objc_class_count_preloadmethods( cls) + _mulle_objc_universe_number_of_preloadmethods( cls->universe);

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


static void   _mulle_objc_class_fill_inactivecache_with_preload_array_of_methodids( struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache, mulle_objc_methodid_t *methodids, unsigned int n)
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


static int  preload( struct _mulle_objc_method *method,
                     struct _mulle_objc_class *cls,
                    struct _mulle_objc_cache *cache)
{
   assert( cache);

   if( _mulle_objc_methoddescriptor_is_preload_method( &method->descriptor))
      _mulle_objc_cache_inactivecache_add_pointer_entry( cache, _mulle_objc_method_get_implementation( method), method->descriptor.methodid);

   return( 0);
}


static void   _mulle_objc_class_fill_inactivecache_with_preloadmethods( struct _mulle_objc_class *cls,
                                                                 struct _mulle_objc_cache *cache)
{
   _mulle_objc_class_walk_methods( cls, _mulle_objc_class_get_inheritance( cls), (int(*)()) preload, cache);
}


static inline void   _mulle_objc_class_preload_inactivecache( struct _mulle_objc_class *cls, struct _mulle_objc_cache *cache)
{
   struct _mulle_objc_universe   *universe;

   universe = cls->universe;
   _mulle_objc_class_fill_inactivecache_with_preload_array_of_methodids( cls, cache, universe->methodidstopreload.methodids, universe->methodidstopreload.n);
}



//
// fills the cache line with a forward: if message does not exist or
// if it exists it fills up the entry
//
struct _mulle_objc_cacheentry  empty_entry;

MULLE_C_NEVER_INLINE
struct _mulle_objc_cacheentry   *
   _mulle_objc_class_add_entry_by_swapping_caches( struct _mulle_objc_class *cls,
                                                   struct _mulle_objc_cache *cache,
                                                   struct _mulle_objc_method *method,
                                                   mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;
   struct _mulle_objc_universe      *universe;
   mulle_objc_cache_uint_t         new_size;
   
   old_cache = cache;

   // a new beginning.. let it be filled anew
   new_size = cache->size * 2;
   cache    = mulle_objc_cache_new( new_size, &cls->universe->memory.allocator);

   // fill it up with preload messages and place our method there too
   // now for good measure

   if( _mulle_objc_class_count_preloadmethods( cls))
      _mulle_objc_class_fill_inactivecache_with_preloadmethods( cls, cache);
   if( _mulle_objc_universe_number_of_preloadmethods( cls->universe))
      _mulle_objc_class_preload_inactivecache( cls, cache);

   //
   // if someone passes in a NULL for method, empty_entry is a marker
   // for success..
   //
   entry = &empty_entry;
   if( method)
      entry = _mulle_objc_cache_inactivecache_add_functionpointer_entry( cache, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method), methodid);

   //
   // an empty_cache ? this is getting called to early
   //
   assert( _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries) != mulle_objc_get_universe()->empty_cache.entries);

   // if the set fails, then someone else was faster
   universe = _mulle_objc_class_get_universe( cls);
   if( _mulle_objc_cachepivot_atomic_set_entries( &cls->cachepivot.pivot, cache->entries, old_cache->entries))
   {
      _mulle_objc_cache_free( cache, &cls->universe->memory.allocator); // sic, can be unsafe deleted now
      return( NULL);
   }

   if( universe->debug.trace.method_caches)
      fprintf( stderr, "mulle_objc_universe %p trace: new method cache %p for %s %08x \"%s\" with %u entries\n",
              universe,
              cache,
              _mulle_objc_class_get_classtypename( cls),
              _mulle_objc_class_get_classid( cls),
              _mulle_objc_class_get_name( cls),
              cache->size);

   if( &old_cache->entries[ 0] != &cls->universe->empty_cache.entries[ 0])
   {
      //
      // if we repopulate the new cache with the old cache, we can then
      // determine, which methods have actually been used over the course
      // of the program by just dumping the cache contents
      //
      if( cls->universe->config.repopulate_caches)
      {
         p        = &old_cache->entries[ 0];
         sentinel = &p[ old_cache->size];
         while( p < sentinel)
         {
            methodid = (mulle_objc_methodid_t) (intptr_t) _mulle_atomic_pointer_read( &p->key.pointer);
            ++p;
            
            // place it back into cache
            if( methodid != MULLE_OBJC_NO_METHODID)
               _mulle_objc_class_lookup_methodimplementation( cls, methodid);
         }
      }
      
      if( universe->debug.trace.method_caches)
         fprintf( stderr, "mulle_objc_universe %p trace: free old method cache %p for %s %08x \"%s\" with %u entries\n",
                 universe,
                 cache,
                 _mulle_objc_class_get_classtypename( cls),
                 _mulle_objc_class_get_classid( cls),
                 _mulle_objc_class_get_name( cls),
                 cache->size);
      
      _mulle_objc_cache_abafree( old_cache, &cls->universe->memory.allocator);
   }
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
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
   {
      _mulle_objc_universe_raise_inconsistency_exception( cls->universe, "call comes to early, the %s \"%s\" hasn't been initialized yet.", _mulle_objc_class_get_classtypename( cls), cls->name);
   }

   // when we trace method calls, we don't cache ever
   if( _mulle_objc_class_get_universe( cls)->debug.trace.method_calls)
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
   struct _mulle_objc_universe            *universe;

   universe = _mulle_objc_class_get_universe( cls);
   desc    = _mulle_objc_universe_lookup_methoddescriptor( universe, methodid);

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
   struct _mulle_objc_universe          *universe;
   
   imp = _mulle_objc_class_unfailing_lookup_methodimplementation( cls, methodid);
   
   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.method_calls)
      mulle_objc_class_trace_method_call( cls, methodid, obj, parameter, imp);

   /*->*/
   return( (*imp)( obj, methodid, parameter));
}



#pragma mark - method cache searches

// only searches cache, returns what there
mulle_objc_methodimplementation_t
   _mulle_objc_class_lookup_cached_methodimplementation( struct _mulle_objc_class *cls,
                                                         mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *entries;
   mulle_objc_cache_uint_t         offset;

   assert( methodid != MULLE_OBJC_NO_METHODID && methodid != MULLE_OBJC_INVALID_METHODID);

   cache   = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   return( (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer));
}


// does not update the cache, no forward
mulle_objc_methodimplementation_t
   _mulle_objc_class_lookup_or_search_methodimplementation_no_forward( struct _mulle_objc_class *cls,
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
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];

   imp = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
   {
      if( _mulle_objc_class_is_forwardmethodimplementation( cls, imp))
         return( 0);
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
// updates the cache, no forward
//
mulle_objc_methodimplementation_t
   _mulle_objc_class_lookup_methodimplementation_no_forward( struct _mulle_objc_class *cls,
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
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
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


// does not fill the cache but does forward
mulle_objc_methodimplementation_t
   _mulle_objc_class_lookup_or_search_methodimplementation( struct _mulle_objc_class *cls,
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
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
      return( imp);

   method = _mulle_objc_class_search_method( cls,
                                             methodid,
                                             NULL,
                                             MULLE_OBJC_ANY_OWNER,
                                             _mulle_objc_class_get_inheritance( cls),
                                             NULL);
   if( ! method)
   {
      method = _mulle_objc_class_getorsearch_forwardmethod( cls);
      if( ! method)
         return( imp);
   }

   imp = _mulle_objc_method_get_implementation( method);
   return( imp);
}


// fills the cache and does forward
mulle_objc_methodimplementation_t
    _mulle_objc_class_lookup_methodimplementation( struct _mulle_objc_class *cls,
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
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_methodimplementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
      return( imp);

   method = _mulle_objc_class_search_method( cls,
                                            methodid,
                                            NULL,
                                            MULLE_OBJC_ANY_OWNER,
                                            _mulle_objc_class_get_inheritance( cls),
                                            NULL);
   if( ! method)
      method = _mulle_objc_class_getorsearch_forwardmethod( cls);

   if( method)
   {
      imp = _mulle_objc_method_get_implementation( method);
      _mulle_objc_class_fill_cache_with_method( cls, method, methodid);
   }
   return( imp);
}



//
// fills the cache and does forward, will raise if no method
// it knows about traces and the empty cache bit
//
mulle_objc_methodimplementation_t
   _mulle_objc_class_unfailing_lookup_methodimplementation( struct _mulle_objc_class *cls,
                                                          mulle_objc_methodid_t methodid)
{
   mulle_objc_methodimplementation_t   imp;
   struct _mulle_objc_universe          *universe;
   struct _mulle_objc_method           *method;
   struct _mulle_objc_cacheentry       *entry;
   
   method  = _mulle_objc_class_unfailing_search_method( cls, methodid);
   imp     = _mulle_objc_method_get_implementation( method);
   
   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.method_calls)
      return( imp);
   // some special classes may choose to never cache
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return( imp);
   
   entry = _mulle_objc_class_fill_cache_with_method( cls, method, methodid);
   if( entry)
      entry->key.uniqueid = methodid; // overwrite in forward case
   
   return( imp);
}


//
// fills the cache and does forward, will raise if no method
// it knows about traces and the empty cache bit
// also checks and raises on empty class and wrong methodid
//
mulle_objc_methodimplementation_t
   mulle_objc_class_unfailing_lookup_methodimplementation( struct _mulle_objc_class *cls,
                                                           mulle_objc_methodid_t methodid)
{
   if( ! cls || methodid == MULLE_OBJC_NO_METHODID || methodid == MULLE_OBJC_INVALID_METHODID)
   {
      errno = EINVAL;
      //  _mulle_objc_universe_raise_fail_errno_exception( mulle_objc_get_universe());
      mulle_objc_raise_fail_errno_exception();
   }

   return( _mulle_objc_class_unfailing_lookup_methodimplementation( cls, methodid));
}


# pragma mark - calls

void   *_mulle_objc_object_call_class( void *obj,
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
void   *_mulle_objc_object_call2( void *obj, mulle_objc_methodid_t methodid, void *parameter)
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


static void   *_mulle_objc_object_call_uncached_class( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;

   method = mulle_objc_class_search_method( cls, methodid);
   if( ! method)
      method = _mulle_objc_class_unfailing_getorsearch_forwardmethod( cls, methodid);
   return( (*_mulle_objc_method_get_implementation( method))( obj, methodid, parameter));
}


# pragma mark - cache

static void   mulle_objc_class_setup_initial_cache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_cache      *cache;
   mulle_objc_cache_uint_t       n_entries;
   
   // now setup the cache and let it rip, except when we don't ever want one
   universe = _mulle_objc_class_get_universe( cls);
   
   // your chance to change the cache algorithm and initital size
   n_entries = _mulle_objc_class_convenient_methodcache_size( cls);
   if( universe->callbacks.will_init_cache)
      n_entries = (*universe->callbacks.will_init_cache)( universe, cls, n_entries);
 
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
   {
      cache = mulle_objc_cache_new( n_entries, &cls->universe->memory.allocator);
      
      assert( cache);
      assert( _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries) ==
              mulle_objc_get_universe()->empty_cache.entries);
      
      _mulle_atomic_pointer_nonatomic_write( &cls->cachepivot.pivot.entries, cache->entries);
      cls->cachepivot.call2 = _mulle_objc_object_call2;
      cls->call             = _mulle_objc_object_call_class;
      
      if( universe->debug.trace.method_caches)
         fprintf( stderr, "mulle_objc_universe %p trace: new initial cache %p "
                 "on %s %08x \"%s\" (%p) with %u entries\n",
                 universe,
                 cache,
                 _mulle_objc_class_get_classtypename( cls),
                 _mulle_objc_class_get_classid( cls),
                 _mulle_objc_class_get_name( cls),
                 cls,
                 cache->size);
   }
   else
   {
      cls->cachepivot.call2 = _mulle_objc_object_call2_empty_cache;
      cls->call             = _mulle_objc_object_call_uncached_class;
      
      if( universe->debug.trace.method_caches)
         fprintf( stderr, "mulle_objc_universe %p trace: use "
                 "\"always empty cache\" on %s %08x \"%s\" (%p)\n",
                 universe,
                 _mulle_objc_class_get_classtypename( cls),
                 _mulle_objc_class_get_classid( cls),
                 _mulle_objc_class_get_name( cls),
                 cls);

      //
      // count #caches, if there are zero caches yet, the universe can be much
      // faster adding methods.
      //
      _mulle_atomic_pointer_increment( &cls->universe->cachecount_1);
   }
   
   // this marks the class as ready to run
   _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY);
}

# pragma mark - +initialize

static void   _mulle_objc_class_wait_for_setup(  struct _mulle_objc_class *cls)
{
   /* same thread ? we are single threaded! */
   if( _mulle_atomic_pointer_read( &cls->thread) != (void *) mulle_thread_self())
   {
      while( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_INITIALIZE_DONE))
         mulle_thread_yield();
   }
}


static void   _mulle_objc_infraclass_call_initialize( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_method           *initialize;
   struct _mulle_objc_universe         *universe;
   struct _mulle_objc_metaclass        *meta;
   mulle_objc_methodimplementation_t   imp;
   int                                 flag;

   meta = _mulle_objc_infraclass_get_metaclass( infra);

   assert( ! _mulle_objc_metaclass_get_state_bit( meta, MULLE_OBJC_METACLASS_INITIALIZE_DONE));
   assert( ! _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE));
   
   // grab code from superclass
   // this is useful for MulleObjCSingleton
   initialize = _mulle_objc_class_search_method( &meta->base,
                                                 MULLE_OBJC_INITIALIZE_METHODID,
                                                 NULL,
                                                 MULLE_OBJC_ANY_OWNER,
                                                 meta->base.inheritance,
                                                 NULL);

   universe = _mulle_objc_infraclass_get_universe( infra);
   if( initialize)
   {
      if( universe->debug.trace.initialize)
         fprintf( stderr, "mulle_objc_universe %p trace: call +[%s initialize]\n",
                 universe, _mulle_objc_metaclass_get_name( meta));
      
      imp   = _mulle_objc_method_get_implementation( initialize);
      (*imp)( (struct _mulle_objc_object *) infra, MULLE_OBJC_INITIALIZE_METHODID, NULL);
   }
   else
   {
      if( universe->debug.trace.initialize)
         fprintf( stderr, "mulle_objc_universe %p trace: "
                          "no +[%s initialize] found\n",
                 universe, _mulle_objc_metaclass_get_name( meta));
   }

   // must be 1 as a return value
   flag = _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE);
   assert( flag == 1);
   flag = _mulle_objc_metaclass_set_state_bit( meta, MULLE_OBJC_METACLASS_INITIALIZE_DONE);
   assert( flag == 1);
}


static int   _mulle_objc_class_lock_for_setup( struct _mulle_objc_class *cls)
{
   assert( mulle_objc_class_is_current_thread_registered( cls));
   
   //
   // An uninitialized class has the empty_cache as the cache. It also has
   // `cls->thread` NULL. This methods is therefore usually called twice
   // once for the meta class and once for the instance. Regardless in both
   // cases, it is checked if +initialize needs to run. But this is only
   // flagged in the meta class.
   //
   // If another thread enters here, it will expect `cls->thread` to be NULL.
   // If it isn't it waits for MULLE_OBJC_CACHE_INITIALIZE_DONE to go up.
   //
   // what is tricky is, that cls and metaclass are executing this
   // singlethreaded, but still cls and metaclass could be in different threads
   //
   
   if( ! _mulle_atomic_pointer_compare_and_swap( &cls->thread, (void *) mulle_thread_self(), NULL))
      return( 0);
   return( 1);
}


static void  _mulle_objc_class_setup( struct _mulle_objc_class *cls);

static void   _mulle_objc_infraclass_initialize_if_needed( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_infraclass  *superclass;
   struct _mulle_objc_metaclass   *supermeta;

   if( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE))
      return;

   superclass = infra;
   while( superclass = _mulle_objc_infraclass_get_superclass( superclass))
   {
      supermeta = _mulle_objc_infraclass_get_metaclass( superclass);
      
      // call #initialize on superclass if needed
      _mulle_objc_class_setup( _mulle_objc_metaclass_as_class( supermeta));
   }
   
   _mulle_objc_infraclass_call_initialize( infra);
}



static void   _mulle_objc_class_initialize_if_needed( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_infraclass   *infra;
   
   // initialize is called only once though
   if( _mulle_objc_class_is_infraclass( cls))
      infra = _mulle_objc_class_as_infraclass( cls);
   else
      infra = _mulle_objc_class_get_infraclass( cls);
      
   _mulle_objc_infraclass_initialize_if_needed( infra);
}


static void  _mulle_objc_class_setup( struct _mulle_objc_class *cls)
{
   // this is always done
   if( _mulle_objc_class_lock_for_setup( cls))
   {
      mulle_objc_class_setup_initial_cache( cls);
      _mulle_objc_class_initialize_if_needed( cls);
   }
   else
      _mulle_objc_class_wait_for_setup( cls);
}


void   *_mulle_objc_object_call_class_needs_cache( void *obj,
                                                   mulle_objc_methodid_t methodid,
                                                   void *parameter,
                                                   struct _mulle_objc_class *cls)
{
   _mulle_objc_class_setup( cls);
   
   return( (*cls->call)( obj, methodid, parameter, cls));
}


void   *_mulle_objc_object_call2_needs_cache( void *obj,
                                              mulle_objc_methodid_t methodid,
                                              void *parameter);

void   *_mulle_objc_object_call2_needs_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter)
{
   struct _mulle_objc_class   *cls;
   
   cls = _mulle_objc_object_get_isa( (struct _mulle_objc_object *) obj);
   return( _mulle_objc_object_call_class_needs_cache( obj, methodid, parameter, cls));
}


#pragma mark - empty cache calls

static void   *_mulle_objc_object_call2_empty_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter)
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


#pragma mark - multiple objects call

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

# pragma mark - class super call
void   *mulle_objc_infraclass_metacall_classid( struct _mulle_objc_infraclass *infra,
                                                mulle_objc_methodid_t methodid,
                                                void *parameter,
                                                mulle_objc_classid_t classid)
{
   if( ! infra)
      return( infra);
   return( _mulle_objc_infraclass_inline_metacall_classid( infra, methodid, parameter, classid));
}


#pragma mark - instance super call
void   *mulle_objc_object_call_classid( void *obj,
                                        mulle_objc_methodid_t methodid,
                                        void *parameter,
                                        mulle_objc_classid_t classid)
{
   if( ! obj)
      return( 0);
   return( _mulle_objc_object_inline_call_classid( obj, methodid, parameter, classid));
}

