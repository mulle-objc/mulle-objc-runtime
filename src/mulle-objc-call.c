//
//  mulle_objc_message.c
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
#include "mulle-objc-call.h"

#include "mulle-objc-class.h"
#include "mulle-objc-class-search.h"
#include "mulle-objc-class-universe.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-object.h"
#include "mulle-objc-universe.h"

#include "include-private.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>


static void   *_mulle_objc_object_noncachingcall_class( void *obj,
                                                       mulle_objc_methodid_t methodid,
                                                       void *parameter,
                                                       struct _mulle_objc_class *cls);

static void   *_mulle_objc_object_call2_empty_cache( void *obj,
                                                     mulle_objc_methodid_t methodid,
                                                     void *parameter);
static void   *_mulle_objc_object_call2( void *obj,
                                         mulle_objc_methodid_t methodid,
                                         void *parameter);

void   *_mulle_objc_object_call_class( void *obj,
                                       mulle_objc_methodid_t methodid,
                                       void *parameter,
                                       struct _mulle_objc_class *cls);

static void   *_mulle_objc_object_unfailingcall_methodid( void *obj,
                                                           mulle_objc_methodid_t methodid,
                                                           void *parameter,
                                                           struct  _mulle_objc_class *cls);

static mulle_objc_implementation_t
   _mulle_objc_class_unfailinglookup2_superimplementation( struct _mulle_objc_class *cls,
                                                           mulle_objc_superid_t superid);

# pragma mark - class cache

static mulle_objc_cache_uint_t  _mulle_objc_class_search_minmethodcachesize( struct _mulle_objc_class *cls)
{
   mulle_objc_cache_uint_t   preloads;

   // these are definitely in the cache
   preloads = _mulle_objc_class_count_preloadmethods( cls) + _mulle_objc_universe_get_numberofpreloadmethods( cls->universe);
   return( preloads);
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
      method = mulle_objc_class_defaultsearch_method( cls, *p++);
      if( method)
         _mulle_objc_cache_inactivecache_add_pointer_entry( cache, _mulle_objc_method_get_implementation( method), method->descriptor.methodid);
   }
}


static int  preload( struct _mulle_objc_method *method,
                     struct _mulle_objc_class *cls,
                    struct _mulle_objc_cache *cache)
{
   assert( cache);

   if( _mulle_objc_descriptor_is_preload_method( &method->descriptor))
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
   _mulle_objc_class_add_cacheentry_by_swapping_methodcaches( struct _mulle_objc_class *cls,
                                                   struct _mulle_objc_cache *cache,
                                                   struct _mulle_objc_method *method,
                                                   mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;
   struct _mulle_objc_universe     *universe;
   mulle_objc_cache_uint_t         new_size;

   old_cache = cache;

   // if the set fails, then someone else was faster
   universe = _mulle_objc_class_get_universe( cls);

   // a new beginning.. let it be filled anew
   // could ask the universe here what to do as new size

   new_size = cache->size * 2;
   cache    = mulle_objc_cache_new( new_size, &cls->universe->memory.allocator);

   // fill it up with preload messages and place our method there too
   // now for good measure

   if( _mulle_objc_class_count_preloadmethods( cls))
      _mulle_objc_class_fill_inactivecache_with_preloadmethods( cls, cache);
   if( _mulle_objc_universe_get_numberofpreloadmethods( cls->universe))
      _mulle_objc_class_preload_inactivecache( cls, cache);

   //
   // if someone passes in a NULL for method, empty_entry is a marker
   // for success..
   //
   entry = &empty_entry;
   if( method)
      entry = _mulle_objc_cache_inactivecache_add_functionpointer_entry( cache, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method), methodid);

   //
   // an empty_cache ? this is getting called too early
   //
   assert( _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries) != mulle_objc_get_universe()->empty_cache.entries);

   if( _mulle_objc_cachepivot_atomic_set_entries( &cls->cachepivot.pivot, cache->entries, old_cache->entries))
   {
      _mulle_objc_cache_free( cache, &cls->universe->memory.allocator); // sic, can be unsafe deleted now
      return( NULL);
   }

   if( universe->debug.trace.method_cache)
      fprintf( stderr, "mulle_objc_universe %p trace: new method cache %p (%u of %u used) for %s %08x \"%s\"\n",
              universe,
              cache,
              _mulle_objc_cache_get_count( cache),
              old_cache->size,
              _mulle_objc_class_get_classtypename( cls),
              _mulle_objc_class_get_classid( cls),
              _mulle_objc_class_get_name( cls));

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
               _mulle_objc_class_lookup_implementation( cls, methodid);
         }
      }

      if( universe->debug.trace.method_cache)
         fprintf( stderr, "mulle_objc_universe %p trace: free old method cache %p (%u of %u used) for %s %08x \"%s\"\n",
                 universe,
                 old_cache,
                  _mulle_objc_cache_get_count( cache),
                  old_cache->size,
                 _mulle_objc_class_get_classtypename( cls),
                 _mulle_objc_class_get_classid( cls),
                 _mulle_objc_class_get_name( cls));

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
   struct _mulle_objc_universe     *universe;

   assert( cls);
   assert( method);

   // need to check that we are initialized
   universe = _mulle_objc_class_get_universe( cls);
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
   {
      _mulle_objc_universe_raise_inconsistency_exception( universe,
               "Method call %08x \"%s\" comes too early, "
               "the cache of %s \"%s\" hasn't been initialized yet.",
               methodid, _mulle_objc_method_get_name( method),
               _mulle_objc_class_get_classtypename( cls), cls->name);
   }

   // when we trace method calls, we don't cache ever
   if( _mulle_objc_class_get_universe( cls)->debug.trace.method_call)
      return( NULL);

   //
   //  try to get most up to date value
   //
   for(;;)
   {
      cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
      if( _mulle_objc_universe_should_grow_cache( universe, cache))
      {
         entry = _mulle_objc_class_add_cacheentry_by_swapping_methodcaches( cls, cache, method, methodid);
         if( entry)
            return( entry);
         continue;
      }

      entry = _mulle_objc_cache_add_functionpointer_entry( cache, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method), methodid);
      if( entry)
         return( entry);
   }
}


MULLE_C_NEVER_INLINE
static struct _mulle_objc_cacheentry   *
 _mulle_objc_class_fill_methodcache_with_method( struct _mulle_objc_class *cls,
                                                 struct _mulle_objc_method *method,
                                                 mulle_objc_methodid_t methodid)
{
   // need to check that we are initialized
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
   {
      _mulle_objc_universe_raise_inconsistency_exception( cls->universe,
                                                         "Method call %08x \"%s\" comes too early, "
                                                         "the cache of %s \"%s\" hasn't been initialized yet.",
                                                         methodid, _mulle_objc_method_get_name( method),
                                                         _mulle_objc_class_get_classtypename( cls), cls->name);
   }

   return( _mulle_objc_class_fill_cache_with_method( cls, method, methodid));
}


#ifdef HAVE_SUPERCACHE

# pragma mark - supercache (use if methodcache sharing is not so hot)

//
// fills the cache line with a forward: if message does not exist or
// if it exists it fills up the entry
//
struct _mulle_objc_cacheentry  empty_entry;

MULLE_C_NEVER_INLINE
struct _mulle_objc_cacheentry   *
   _mulle_objc_class_add_cacheentry_by_swapping_supercaches( struct _mulle_objc_class *cls,
                                                             struct _mulle_objc_cache *cache,
                                                             struct _mulle_objc_method *method,
                                                             mulle_objc_superid_t superid)
{
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_universe     *universe;
   mulle_objc_cache_uint_t         new_size;

   old_cache = cache;

   // a new beginning.. let it be filled anew
   new_size = cache->size * 2;
   cache    = mulle_objc_cache_new( new_size, &cls->universe->memory.allocator);

   //
   // if someone passes in a NULL for method, empty_entry is a marker
   // for success..
   //
   entry = &empty_entry;
   if( method)
      entry = _mulle_objc_cache_inactivecache_add_functionpointer_entry( cache, (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method), superid);

   universe = _mulle_objc_class_get_universe( cls);

   // if the set fails, then someone else was faster
   if( _mulle_objc_cachepivot_atomic_set_entries( &cls->supercachepivot, cache->entries, old_cache->entries))
   {
      _mulle_objc_cache_free( cache, &universe->memory.allocator); // sic, can be unsafe deleted now
      return( NULL);
   }

   if( universe->debug.trace.method_cache)
      fprintf( stderr, "mulle_objc_universe %p trace: new search cache %p for %s %08x \"%s\" with %u entries\n",
              universe,
              cache,
              _mulle_objc_class_get_classtypename( cls),
              _mulle_objc_class_get_classid( cls),
              _mulle_objc_class_get_name( cls),
              cache->size);

   if( &old_cache->entries[ 0] != &universe->empty_cache.entries[ 0])
   {
      if( universe->debug.trace.method_cache)
         fprintf( stderr, "mulle_objc_universe %p trace: free old search cache %p for %s %08x \"%s\" with %u entries\n",
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
   _mulle_objc_class_fill_supercache_with_method( struct _mulle_objc_class *cls,
                                                  struct _mulle_objc_method *method,
                                                  mulle_objc_superid_t superid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   mulle_functionpointer_t         imp;
   struct _mulle_objc_universe     *universe;

   assert( cls);
   assert( method);

   // need to check that we are initialized
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_NO_SEARCH_CACHE))
      return( NULL);
   // when we trace method calls, we don't cache ever
   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.method_call)
      return( NULL);

   //
   //  try to get most up to date value
   //
   for(;;)
   {
      cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->supercachepivot);
      if( _mulle_objc_universe_should_grow_cache( universe, cache))
      {
         entry = _mulle_objc_class_add_cacheentry_by_swapping_supercaches( cls, cache, method, superid);
         if( entry)
            return( entry);
         continue;
      }

      imp   = (mulle_functionpointer_t) _mulle_objc_method_get_implementation( method);
      entry = _mulle_objc_cache_add_functionpointer_entry( cache, imp, superid);
      if( entry)
         return( entry);
   }
}

#endif


MULLE_C_CONST_RETURN MULLE_C_NON_NULL_RETURN
struct _mulle_objc_method *
   _mulle_objc_class_unfailinguncachedlookup_supermethod( struct _mulle_objc_class *cls,
                                                          mulle_objc_superid_t superid)
{
   struct _mulle_objc_method             *method;
   struct _mulle_objc_universe           *universe;
   struct _mulle_objc_searcharguments    args;
   struct _mulle_objc_super              *p;

   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   universe = _mulle_objc_class_get_universe( cls);
   p        = _mulle_objc_universe_unfailinglookup_super( universe, superid);

   _mulle_objc_searcharguments_superinit( &args, p->methodid, p->classid);
   method = mulle_objc_class_search_method( cls,
                                            &args,
                                            cls->inheritance,
                                            NULL);
   if( ! method)
      method = _mulle_objc_class_unfailinglazyget_forwardmethod( cls, args.args.methodid);
   return( method);
}


MULLE_C_CONST_RETURN MULLE_C_NON_NULL_RETURN
mulle_objc_implementation_t
   _mulle_objc_class_unfailinguncachedlookup_supermethodimplementation( struct _mulle_objc_class *cls,
                                                                        mulle_objc_superid_t superid)
{
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;

   method = _mulle_objc_class_unfailinguncachedlookup_supermethod( cls, superid);
   imp    = _mulle_objc_method_get_implementation( method);
   return( imp);
}

//
// fills the cache and does forward
//
mulle_objc_implementation_t
   _mulle_objc_class_unfailinglookup_superimplementation( struct _mulle_objc_class *cls,
                                                          mulle_objc_superid_t superid)
{
   mulle_objc_cache_uint_t               offset;
   mulle_objc_implementation_t           imp;
   struct _mulle_objc_cache              *cache;
   struct _mulle_objc_cacheentry         *entries;
   struct _mulle_objc_cacheentry         *entry;
   struct _mulle_objc_method             *method;

#ifndef HAVE_SUPERCACHE
   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
#else
   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->supercachepivot);
#endif
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   offset  = _mulle_objc_cache_find_entryoffset( cache, superid);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
      return( imp);

   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   method = _mulle_objc_class_unfailinguncachedlookup_supermethod( cls, superid);
   imp    = _mulle_objc_method_get_implementation( method);

#ifndef HAVE_SUPERCACHE
   _mulle_objc_class_fill_methodcache_with_method( cls, method, superid);
#else
   _mulle_objc_class_fill_supercache_with_method( cls, method, superid);
#endif

   return( imp);
}



//mulle_objc_implementation_t   mulle_objc_class_unfailingsearch_implementation( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid)
//{
//   struct _mulle_objc_method   *method;
//
//   method  = _mulle_objc_class_unfailingsearch_method( cls, methodid);
//   return( method->implementation);
//}


# pragma mark - trace method

void   mulle_objc_class_trace_method_call( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t methodid,
                                           void *obj,
                                           void *parameter,
                                           mulle_objc_implementation_t imp)
{
   struct _mulle_objc_descriptor   *desc;
   struct _mulle_objc_universe     *universe;

   universe = _mulle_objc_class_get_universe( cls);
   desc    = _mulle_objc_universe_lookup_descriptor( universe, methodid);

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
static void   *_mulle_objc_object_unfailingcall_methodid( void *obj,
                                                          mulle_objc_methodid_t methodid,
                                                          void *parameter,
                                                          struct  _mulle_objc_class *cls)
{
   mulle_objc_implementation_t   imp;
   struct _mulle_objc_universe         *universe;

   imp = _mulle_objc_class_unfailinglookup_implementation( cls, methodid);

   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.method_call)
      mulle_objc_class_trace_method_call( cls, methodid, obj, parameter, imp);

   /*->*/
   return( (*imp)( obj, methodid, parameter));
}



#pragma mark - method cache searches

// only searches cache, returns what there
mulle_objc_implementation_t
   _mulle_objc_class_cacheonlylookup_implementation( struct _mulle_objc_class *cls,
                                                     mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *entries;
   mulle_objc_cache_uint_t         offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cache   = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   return( (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer));
}


// does not update the cache, no forward
mulle_objc_implementation_t
   _mulle_objc_class_noncachinglookup_implementation_no_forward( struct _mulle_objc_class *cls,
                                                                 mulle_objc_methodid_t methodid)
{
   mulle_objc_implementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_method           *method;
   mulle_objc_cache_uint_t             offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cache   = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];

   imp = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
   {
      if( _mulle_objc_class_is_forwardimplementation( cls, imp))
         return( 0);
   }
   else
   {
      method = mulle_objc_class_defaultsearch_method( cls, methodid);
      if( method)
         imp = _mulle_objc_method_get_implementation( method);
   }

   return( imp);
}


//
// updates the cache, no forward
//
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_no_forward( struct _mulle_objc_class *cls,
                                                       mulle_objc_methodid_t methodid)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_method       *method;
   mulle_objc_cache_uint_t         offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cache   = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
   {
      if( _mulle_objc_class_is_forwardimplementation( cls, imp))
         return( NULL);
      return( imp);
   }

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( method)
   {
      imp = _mulle_objc_method_get_implementation( method);
      _mulle_objc_class_fill_methodcache_with_method( cls, method, methodid);
   }
   return( imp);
}


// does not fill the cache but does forward
mulle_objc_implementation_t
   _mulle_objc_class_noncachinglookup_implementation( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid)
{
   mulle_objc_cache_uint_t             offset;
   mulle_objc_implementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_method           *method;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
      return( imp);

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( ! method)
   {
      method = _mulle_objc_class_lazyget_forwardmethod( cls);
      if( ! method)
         return( imp);
   }

   imp = _mulle_objc_method_get_implementation( method);
   return( imp);
}


// fills the cache and does forward
mulle_objc_implementation_t
    _mulle_objc_class_lookup_implementation( struct _mulle_objc_class *cls,
                                                   mulle_objc_methodid_t methodid)
{
   mulle_objc_cache_uint_t             offset;
   mulle_objc_implementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_method           *method;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entry   = (void *) &((char *) entries)[ offset];
   imp     = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   if( imp)
      return( imp);

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( ! method)
      method = _mulle_objc_class_lazyget_forwardmethod( cls);

   if( method)
   {
      imp = _mulle_objc_method_get_implementation( method);
      _mulle_objc_class_fill_methodcache_with_method( cls, method, methodid);
   }
   return( imp);
}


//
// fills the cache and does forward, will raise if no method
// it knows about traces and the empty cache bit
//
mulle_objc_implementation_t
   _mulle_objc_class_unfailinglookup_implementation( struct _mulle_objc_class *cls,
                                                          mulle_objc_methodid_t methodid)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_cacheentry   *entry;

   method  = mulle_objc_class_unfailingsearch_method( cls, methodid);
   imp     = _mulle_objc_method_get_implementation( method);

   universe = _mulle_objc_class_get_universe( cls);
   // trace but don't cache it
   if( universe->debug.trace.method_call)
      return( imp);
   // some special classes may choose to never cache
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
      return( imp);

   entry = _mulle_objc_class_fill_methodcache_with_method( cls, method, methodid);
   if( entry)
      entry->key.uniqueid = methodid; // overwrite in forward case

   return( imp);
}


//
// fills the cache and does forward, will raise if no method
// it knows about traces and the empty cache bit
// also checks and raises on empty class and wrong methodid
//
mulle_objc_implementation_t
   mulle_objc_class_unfailinglookup_implementation( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t methodid)
{
   if( ! cls || ! mulle_objc_uniqueid_is_sane( methodid))
   {
      errno = EINVAL;
      //  _mulle_objc_universe_raise_errno_exception( mulle_objc_get_universe());
      mulle_objc_raise_errno_exception();
   }

   return( _mulle_objc_class_unfailinglookup_implementation( cls, methodid));
}


# pragma mark - calls

void   *_mulle_objc_object_call_class( void *obj,
                                       mulle_objc_methodid_t methodid,
                                       void *parameter,
                                       struct _mulle_objc_class *cls)
{
   mulle_objc_implementation_t         imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   mulle_objc_cache_uint_t             mask;
   mulle_objc_cache_uint_t             offset;

   assert( obj);
   assert( mulle_objc_uniqueid_is_sane( methodid));

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
         imp = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
/*->*/   return( (*imp)( obj, methodid, parameter));
      }

      if( ! entry->key.uniqueid)
/*->*/   return( _mulle_objc_object_unfailingcall_methodid( obj, methodid, parameter, cls));

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


//
// this function is called, when the first inline cache check gave a
// collision
//
void   *_mulle_objc_object_call2( void *obj, mulle_objc_methodid_t methodid, void *parameter)
{
   mulle_objc_implementation_t   imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   struct _mulle_objc_class            *cls;
   mulle_objc_cache_uint_t             mask;
   mulle_objc_cache_uint_t             offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

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
         imp = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         return( (*imp)( obj, methodid, parameter));
      }

      if( ! entry->key.uniqueid)
/*->*/   return( _mulle_objc_object_unfailingcall_methodid( obj, methodid, parameter, cls));
   }
}


//
// this function is called, when the first inline cache check gave a
// collision
//
static mulle_objc_implementation_t
   _mulle_objc_class_unfailinglookup2_superimplementation( struct _mulle_objc_class *cls,
                                                           mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t         imp;
   struct _mulle_objc_cache            *cache;
   struct _mulle_objc_cacheentry       *entries;
   struct _mulle_objc_cacheentry       *entry;
   mulle_objc_cache_uint_t             mask;
   mulle_objc_cache_uint_t             offset;

   entries = _mulle_objc_cachepivot_atomic_get_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) superid;
   for(;;)
   {
      offset += sizeof( struct _mulle_objc_cacheentry);
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == superid)
      {
         imp = (mulle_objc_implementation_t) _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         return( imp);
      }

      if( ! entry->key.uniqueid)
/*->*/   return( _mulle_objc_class_unfailinglookup_superimplementation( cls, superid));
   }
}



static void   *_mulle_objc_object_noncachingcall_class( void *obj, mulle_objc_methodid_t methodid, void *parameter, struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( ! method)
      method = _mulle_objc_class_unfailinglazyget_forwardmethod( cls, methodid);
   return( (*_mulle_objc_method_get_implementation( method))( obj, methodid, parameter));
}


# pragma mark - cache

static void   _mulle_objc_class_setup_initial_cache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_cache      *cache;
   mulle_objc_cache_uint_t       n_entries;

   // now setup the cache and let it rip, except when we don't ever want one
   universe = _mulle_objc_class_get_universe( cls);

   // your chance to change the cache algorithm and initital size
   n_entries = _mulle_objc_class_search_minmethodcachesize( cls);
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
      cls->superlookup      = _mulle_objc_class_unfailinglookup_superimplementation;
      cls->superlookup2     = _mulle_objc_class_unfailinglookup2_superimplementation;

      if( universe->debug.trace.method_cache)
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
      cls->call             = _mulle_objc_object_noncachingcall_class;
      cls->superlookup      = _mulle_objc_class_unfailinguncachedlookup_supermethodimplementation;
      cls->superlookup2     = _mulle_objc_class_unfailinguncachedlookup_supermethodimplementation;

      if( universe->debug.trace.method_cache)
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
}


static void   _mulle_objc_class_setup_initial_cache_if_needed( struct _mulle_objc_class *cls)
{
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
      return;

   _mulle_objc_class_setup_initial_cache( cls);

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
   struct _mulle_objc_method       *initialize;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_metaclass    *meta;
   mulle_objc_implementation_t     imp;
   int                             flag;

   meta = _mulle_objc_infraclass_get_metaclass( infra);

   assert( ! _mulle_objc_metaclass_get_state_bit( meta, MULLE_OBJC_METACLASS_INITIALIZE_DONE));
   assert( ! _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE));

   // grab code from superclass
   // this is useful for MulleObjCSingleton
   initialize = mulle_objc_class_defaultsearch_method( &meta->base, MULLE_OBJC_INITIALIZE_METHODID);

   universe = _mulle_objc_infraclass_get_universe( infra);
   if( initialize)
   {
      if( universe->debug.trace.initialize)
         fprintf( stderr, "mulle_objc_universe %p trace: call +[%s initialize]\n",
                 universe, _mulle_objc_metaclass_get_name( meta));

      imp   = _mulle_objc_method_get_implementation( initialize);
      if( universe->debug.trace.method_call)
         mulle_objc_class_trace_method_call( &infra->base, MULLE_OBJC_INITIALIZE_METHODID, infra, NULL, imp);
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

static void   _mulle_objc_infraclass_initialize_if_needed( struct _mulle_objc_infraclass *start)
{
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_infraclass                *infra;
   struct _mulle_objc_infraclass                *superclass;
   struct _mulle_objc_metaclass                 *meta;
   struct _mulle_objc_metaclass                 *rootmeta;
   struct _mulle_objc_metaclass                 *supermeta;
   struct _mulle_objc_protocolclassenumerator   rover;

   if( _mulle_objc_infraclass_get_state_bit( start, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE))
      return;

   rootmeta = NULL;

   pair  = _mulle_objc_infraclass_get_classpair( start);
   rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
   for(;;)
   {
      infra = _mulle_objc_protocolclassenumerator_next( &rover);
      if( ! infra)
         break;

      meta = _mulle_objc_infraclass_get_metaclass( infra);
      if( ! rootmeta)
         rootmeta = meta;

      _mulle_objc_class_setup( _mulle_objc_metaclass_as_class( meta));
   }
   _mulle_objc_protocolclassenumerator_done( &rover);


   superclass = start;
   while( superclass = _mulle_objc_infraclass_get_superclass( superclass))
   {
      supermeta = _mulle_objc_infraclass_get_metaclass( superclass);
      if( ! rootmeta)
         rootmeta = supermeta;

      // call #initialize on superclass if needed
      _mulle_objc_class_setup( _mulle_objc_metaclass_as_class( supermeta));
   }

   //
   // root class must have cache setup as well for wrap around
   //
   if( ! rootmeta)
      _mulle_objc_class_setup_initial_cache_if_needed( _mulle_objc_infraclass_as_class( start));

   _mulle_objc_infraclass_call_initialize( start);
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
      _mulle_objc_class_setup_initial_cache_if_needed( cls);
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


mulle_objc_implementation_t   _mulle_objc_class_superlookup2_needs_cache( struct _mulle_objc_class *cls,
                                                                     mulle_objc_superid_t superid);

mulle_objc_implementation_t   _mulle_objc_class_superlookup2_needs_cache( struct _mulle_objc_class *cls,
                                                                      mulle_objc_superid_t superid)
{
   _mulle_objc_class_setup( cls);

   return( (*cls->superlookup2)( cls, superid));
}


#pragma mark - empty cache calls

static void   *_mulle_objc_object_call2_empty_cache( void *obj, mulle_objc_methodid_t methodid, void *parameter)
{
   struct _mulle_objc_class    *cls;
   struct _mulle_objc_method   *method;

   cls    = _mulle_objc_object_get_isa( obj);
   method = mulle_objc_class_unfailingsearch_method( cls, methodid);
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


#pragma mark - super call

void   *_mulle_objc_object_supercall( void *obj,
                                      mulle_objc_methodid_t methodid,
                                      void *parameter,
                                      mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t   imp;

   imp = _mulle_objc_object_inline_unfailinglookup_superimplementation( obj, superid);
   return( (*imp)( obj, methodid, parameter));
}

// need to call cls->call to prepare caches


#pragma mark - multiple objects call

void   mulle_objc_objects_call( void **objects, unsigned int n, mulle_objc_methodid_t methodid, void *params)
{
   mulle_objc_implementation_t   (*lookup)( struct _mulle_objc_class *, mulle_objc_methodid_t);
   mulle_objc_implementation_t   imp;
   mulle_objc_implementation_t   lastSelIMP[ 16];
   struct _mulle_objc_class            *lastIsa[ 16];
   struct _mulle_objc_class            *thisIsa;
   unsigned int                        i;
   void                                **sentinel;
   void                                *p;

   assert( mulle_objc_uniqueid_is_sane( methodid));
   memset( lastIsa, 0, sizeof( lastIsa));

   // assume compiler can do unrolling
   lookup   = _mulle_objc_class_lookup_implementation_no_forward;
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


mulle_objc_implementation_t
   _mulle_objc_object_unfailinglookup_superimplementation( void *obj,
                                                         mulle_objc_superid_t superid)
{
   return( _mulle_objc_object_inline_unfailinglookup_superimplementation( obj, superid));
}



