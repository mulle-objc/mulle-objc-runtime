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
#include "mulle-objc-universe-class.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-object.h"
#include "mulle-objc-universe.h"

#include "include-private.h"
#include <assert.h>
#include <stdlib.h>


//
// MEMO: during messaging operation (i.e. -[foo bar] ) the runtime must not set "errno"
//       this could happen, if malloc sets errno even on success. It's
//       assumed mulle_malloc shields us from that on misbehaving platforms.
//       The same should hold true for class lookups like +[NSArray array]

static void   *_mulle_objc_object_call_class_nocache( void *obj,
                                                      mulle_objc_methodid_t methodid,
                                                      void *parameter,
                                                      struct _mulle_objc_class *cls);

static void   *_mulle_objc_object_call2_emptycache( void *obj,
                                                    mulle_objc_methodid_t methodid,
                                                    void *parameter);
static void   *_mulle_objc_object_call2( void *obj,
                                         mulle_objc_methodid_t methodid,
                                         void *parameter);

void   *_mulle_objc_object_call_class( void *obj,
                                       mulle_objc_methodid_t methodid,
                                       void *parameter,
                                       struct _mulle_objc_class *cls);

static void   *_mulle_objc_object_call_class_nofail( void *obj,
                                                     mulle_objc_methodid_t methodid,
                                                     void *parameter,
                                                     struct  _mulle_objc_class *cls);

static mulle_objc_implementation_t
   _mulle_objc_class_superlookup2_implementation_nofail( struct _mulle_objc_class *cls,
                                                         mulle_objc_superid_t superid);

#pragma mark - method cache searches

// only searches cache, returns what there
static struct _mulle_objc_cacheentry *
   _mulle_objc_class_lookup_cacheentry( struct _mulle_objc_class *cls,
                                        mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *entries;
   mulle_objc_cache_uint_t         offset;
   mulle_functionpointer_t         p;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cache   = _mulle_objc_cachepivot_atomicget_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   return( entry);
}


// only searches cache, returns what there
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_cacheonly( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *entries;
   mulle_objc_cache_uint_t         offset;
   mulle_functionpointer_t         p;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cache   = _mulle_objc_cachepivot_atomicget_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   return( (mulle_objc_implementation_t) p);
}


# pragma mark - class cache

static mulle_objc_cache_uint_t
   _class_search_minmethodcachesize( struct _mulle_objc_class *cls)
{
   mulle_objc_cache_uint_t   preloads;

   // these are definitely in the cache
   preloads = _mulle_objc_class_count_preloadmethods( cls) +
              _mulle_objc_universe_get_numberofpreloadmethods( cls->universe);
   return( preloads);
}


static void
   _class_fill_inactivecache_with_preload_methodids( struct _mulle_objc_class *cls,
                                                     struct _mulle_objc_cache *cache,
                                                     mulle_objc_methodid_t *methodids,
                                                     unsigned int n)
{
   mulle_objc_methodid_t        *p;
   mulle_objc_methodid_t        *sentinel;
   struct _mulle_objc_method    *method;
   mulle_objc_implementation_t  imp;

   p        = methodids;
   sentinel = &p[ n];
   while( p < sentinel)
   {
      method = mulle_objc_class_defaultsearch_method( cls, *p++);
      if( method)
      {
         imp = _mulle_objc_method_get_implementation( method);
         _mulle_objc_cache_inactivecache_add_pointer_entry( cache,
                                                            imp,
                                                            method->descriptor.methodid);
      }
   }
}


static int  preload( struct _mulle_objc_method *method,
                     struct _mulle_objc_methodlist *list,
                     struct _mulle_objc_class *cls,
                     struct _mulle_objc_cache *cache)
{
   assert( cache);

   if( _mulle_objc_descriptor_is_preload_method( &method->descriptor))
      _mulle_objc_cache_inactivecache_add_pointer_entry( cache,
                                                         _mulle_objc_method_get_implementation( method),
                                                         method->descriptor.methodid);

   return( 0);
}


static void
   _class_fill_inactivecache_with_preload_methods( struct _mulle_objc_class *cls,
                                                  struct _mulle_objc_cache *cache)
{
   unsigned int   inheritance;

   inheritance = _mulle_objc_class_get_inheritance( cls);
   _mulle_objc_class_walk_methods( cls, inheritance, (int(*)()) preload, cache);
}


static inline void
   _mulle_objc_class_preload_inactivecache( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_cache *cache)
{
   struct _mulle_objc_universe   *universe;

   universe = cls->universe;
   _class_fill_inactivecache_with_preload_methodids( cls, cache, universe->methodidstopreload.methodids, universe->methodidstopreload.n);
}


//
// fills the cache line with a forward: if message does not exist or
// if it exists it fills up the entry
//
struct _mulle_objc_cacheentry  empty_entry;

MULLE_C_NEVER_INLINE struct _mulle_objc_cacheentry   *
   _mulle_objc_class_add_cacheentry_swappmethodcache( struct _mulle_objc_class *cls,
                                                      struct _mulle_objc_cache *cache,
                                                      struct _mulle_objc_method *method,
                                                      mulle_objc_methodid_t methodid,
                                                      enum mulle_objc_cachesizing_t strategy)
{
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;
   struct _mulle_objc_universe     *universe;
   struct mulle_allocator          *allocator;
   mulle_objc_cache_uint_t         new_size;
   mulle_objc_implementation_t     imp;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_methodid_t           copyid;

   old_cache = cache;

   // if the set fails, then someone else was faster
   universe = _mulle_objc_class_get_universe( cls);

   // a new beginning.. let it be filled anew
   // could ask the universe here what to do as new size

   new_size  = _mulle_objc_cache_get_resize( old_cache, strategy);
   allocator = _mulle_objc_universe_get_allocator( universe);
   cache     = mulle_objc_cache_new( new_size, allocator);

   // fill it up with preload messages and place our method there too
   // now for good measure

   if( _mulle_objc_class_count_preloadmethods( cls))
      _class_fill_inactivecache_with_preload_methods( cls, cache);
   if( _mulle_objc_universe_get_numberofpreloadmethods( cls->universe))
      _mulle_objc_class_preload_inactivecache( cls, cache);

   //
   // if someone passes in a NULL for method, empty_entry is a marker
   // for success..
   //
   entry = &empty_entry;
   if( method)
   {
      imp   =  _mulle_objc_method_get_implementation( method);
      entry = _mulle_objc_cache_inactivecache_add_functionpointer_entry( cache,
                                                                         (mulle_functionpointer_t) imp,
                                                                         methodid);
   }

   //
   // an empty_cache ? this is getting called too early
   //
   assert( _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries)
            != universe->empty_methodcache.entries);

   if( _mulle_objc_cachepivot_atomiccas_entries( &cls->cachepivot.pivot,
                                                 cache->entries,
                                                 old_cache->entries))
   {
      // cas failed, so get rid of this and punt
      _mulle_objc_cache_free( cache, allocator); // sic, can be unsafe deleted now
      return( NULL);
   }

   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe,
                                 "new method cache %p "
                                 "(%u of %u used) for %s %08x \"%s\"",
                                 cache,
                                 _mulle_objc_cache_get_count( cache),
                                 old_cache->size,
                                 _mulle_objc_class_get_classtypename( cls),
                                 _mulle_objc_class_get_classid( cls),
                                 _mulle_objc_class_get_name( cls));

   // ??? isn't this checked in the assert above already ?
   if( &old_cache->entries[ 0] == &cls->universe->empty_methodcache.entries[ 0])
      return( entry);

   //
   // If we repopulate the new cache with the old cache, we can then
   // determine, which methods have actually been used over the course
   // of the program by just dumping the cache contents.
   // We can do this only if the cache is growing though, or if we are
   // invalidating...
   //
   if( cls->universe->config.repopulate_caches &&
       (strategy == MULLE_OBJC_CACHESIZE_GROW ||
        strategy == MULLE_OBJC_CACHESIZE_GROW && ! method))
   {
      p        = &old_cache->entries[ 0];
      sentinel = &p[ old_cache->size];
      while( p < sentinel)
      {
         copyid = (mulle_objc_methodid_t) (intptr_t) _mulle_atomic_pointer_read( &p->key.pointer);
         ++p;

         //
         // Place it back into cache.
         // The copyid can also be a superid! In this case, it will not be
         // found. We leave it empty and don't place a forward into it.
         //
         if( copyid != MULLE_OBJC_NO_METHODID)
            _mulle_objc_class_lookup_implementation_noforward( cls, copyid);
      }

      //
      // Cache might have changed again due to repopulation so pick out a
      // new entry
      //
      entry = _mulle_objc_class_lookup_cacheentry( cls, methodid);
   }

   if( universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe, "free old method cache "
                                  "%p (%u of %u used) for %s %08x \"%s\"",
                                 old_cache,
                                  _mulle_objc_cache_get_count( cache),
                                  old_cache->size,
                                 _mulle_objc_class_get_classtypename( cls),
                                 _mulle_objc_class_get_classid( cls),
                                 _mulle_objc_class_get_name( cls));

   _mulle_objc_cache_abafree( old_cache, allocator);

   return( entry);
}


MULLE_C_NEVER_INLINE
static struct _mulle_objc_cacheentry   *
    __mulle_objc_class_fill_methodcache_with_method( struct _mulle_objc_class *cls,
                                                     struct _mulle_objc_method *method,
                                                     mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_universe     *universe;
   mulle_objc_implementation_t     imp;

   assert( cls);
   assert( method);

   // need to check that we are initialized
   universe = _mulle_objc_class_get_universe( cls);
   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
   {
      mulle_objc_universe_fail_inconsistency( universe,
               "Method call %08x \"%s\" comes too early, "
               "the cache of %s \"%s\" hasn't been initialized yet.",
               methodid, _mulle_objc_method_get_name( method),
               _mulle_objc_class_get_classtypename( cls), cls->name);
   }

   // when we trace method calls, we don't cache ever
   if( _mulle_objc_class_get_universe( cls)->debug.trace.method_call)
      return( NULL);

   imp = _mulle_objc_method_get_implementation( method);
   //
   //  try to get most up to date value
   //
   for(;;)
   {
      cache = _mulle_objc_cachepivot_atomicget_cache( &cls->cachepivot.pivot);
      if( _mulle_objc_universe_should_grow_cache( universe, cache))
      {
         entry = _mulle_objc_class_add_cacheentry_swappmethodcache( cls,
                                                                    cache,
                                                                    method,
                                                                    methodid,
                                                                    MULLE_OBJC_CACHESIZE_GROW);
         if( entry)
            return( entry);
         continue;
      }

      entry = _mulle_objc_cache_add_functionpointer_entry( cache,
                                                           (mulle_functionpointer_t) imp,
                                                           methodid);
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
   // there is no difference anymore between
   // __mulle_objc_class_fill_methodcache_with_method
   // and
   // _mulle_objc_class_fill_methodcache_with_method
   return( __mulle_objc_class_fill_methodcache_with_method( cls, method, methodid));
}


/*
 * super calls
 */


MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN struct _mulle_objc_method *
   _mulle_objc_class_superlookup_method_nocache_nofail( struct _mulle_objc_class *cls,
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
   p        = _mulle_objc_universe_lookup_super_nofail( universe, superid);

   _mulle_objc_searcharguments_superinit( &args, p->methodid, p->classid);
   method = mulle_objc_class_search_method( cls,
                                            &args,
                                            cls->inheritance,
                                            NULL);
   if( ! method)
      method = _mulle_objc_class_get_forwardmethod_lazy_nofail( cls, args.args.methodid);
   return( method);
}


MULLE_C_CONST_RETURN MULLE_C_NONNULL_RETURN mulle_objc_implementation_t
   _mulle_objc_class_superlookup_implementation_nocache_nofail( struct _mulle_objc_class *cls,
                                                        mulle_objc_superid_t superid)
{
   struct _mulle_objc_method     *method;
   mulle_objc_implementation_t   imp;

   method = _mulle_objc_class_superlookup_method_nocache_nofail( cls, superid);
   imp    = _mulle_objc_method_get_implementation( method);
   return( imp);
}

//
// fills the cache
//
mulle_objc_implementation_t
   _mulle_objc_class_superlookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                        mulle_objc_superid_t superid)
{
   mulle_objc_cache_uint_t         offset;
   mulle_objc_implementation_t     imp;
   mulle_functionpointer_t         p;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_methodcache  *mcache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_method       *method;

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   offset  = _mulle_objc_cache_find_entryoffset( cache, superid);
   entry   = (void *) &((char *) entries)[ offset];
   p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   imp     = (mulle_objc_implementation_t) p;
   if( imp)
      return( imp);

   //
   // since "previous_method" in args will not be accessed" this is OK to cast
   // and obviously cheaper than making a copy
   //
   mcache = _mulle_objc_cache_get_methodcache_from_cache( cache);
   imp    = (*mcache->superlookup)( cls, superid);

   return( imp);
}


//mulle_objc_implementation_t   mulle_objc_class_search_implementation_unfail( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid)
//{
//   struct _mulle_objc_method   *method;
//
//   method  = _mulle_objc_class_search_method_nofail( cls, methodid);
//   return( method->implementation);
//}

# pragma mark - trace method

void   mulle_objc_class_trace_call( struct _mulle_objc_class *cls,
                                    void *obj,
                                    mulle_objc_methodid_t methodid,
                                    void *parameter,
                                    mulle_objc_implementation_t imp)
{
   struct _mulle_objc_descriptor        *desc;
   struct _mulle_objc_universe          *universe;
   struct _mulle_objc_method            *method;
   struct _mulle_objc_class             *isa;
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   unsigned int                         inheritance;
   char                                 *name;
   int                                  frames;

   universe = _mulle_objc_class_get_universe( cls);
   //
   // What is basically wrong here is, that we should be searching for the
   // class that implements the method, not the called class
   // This is fairly expensive though...
   //
   inheritance = _mulle_objc_class_get_inheritance( cls);
   _mulle_objc_searcharguments_impinit( &search, imp);
   method = mulle_objc_class_search_method( cls,
                                            &search,
                                            inheritance,
                                            &result);

   mulle_thread_mutex_lock( &universe->debug.lock);
   {
      mulle_objc_universe_trace_preamble( universe);

      fprintf( stderr, "[::] ");
      frames = (*universe->debug.count_stackdepth)();
      while( frames--)
         fputc( ' ', stderr);
      fprintf( stderr, "%c[", _mulle_objc_class_is_metaclass( cls) ? '+' : '-');

      if( method)
      {
         fprintf( stderr, "%s", _mulle_objc_class_get_name( result.class));
         name = mulle_objc_methodlist_get_categoryname( result.list);
         if( name)
            fprintf( stderr, "(%s)", name);
         fprintf( stderr, " %s]", _mulle_objc_method_get_name( method));
      }
      else
      {
         // fallback in case...
         fprintf( stderr, "?%s", _mulle_objc_class_get_name( cls));
         desc     = _mulle_objc_universe_lookup_descriptor( universe, methodid);
         if( desc)
            fprintf( stderr, " %s]", desc->name);
         else
            fprintf( stderr, " #%08x]", methodid);
      }

      isa = _mulle_objc_object_get_isa_universe( obj, universe);
      fprintf( stderr, " @%p %s (%p, %x, %p)\n",
               imp,
               _mulle_objc_class_get_name( isa),
               obj,
               methodid,
               parameter);
   }
   mulle_thread_mutex_unlock( &universe->debug.lock);
}



//
// not inlining this is better, because when inlined clang pushes too many
// registers on the stack, what otherwise would be a slim function for the
// most general case
//
MULLE_C_NEVER_INLINE static void *
   _mulle_objc_object_call_class_nofail( void *obj,
                                         mulle_objc_methodid_t methodid,
                                         void *parameter,
                                         struct  _mulle_objc_class *cls)
{
   mulle_objc_implementation_t   imp;
   struct _mulle_objc_universe   *universe;

   imp      = _mulle_objc_class_lookup_implementation_nofail( cls, methodid);
   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.method_call)
      mulle_objc_class_trace_call( cls, obj, methodid, parameter, imp);
   /*->*/
   return( (*imp)( obj, methodid, parameter));
}


# pragma mark -

// does not update the cache, no forward
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nocache_noforward( struct _mulle_objc_class *cls,
                                                              mulle_objc_methodid_t methodid)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_method       *method;
   mulle_objc_cache_uint_t         offset;
   mulle_functionpointer_t         p;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cache   = _mulle_objc_cachepivot_atomicget_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];

   p   = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   imp = (mulle_objc_implementation_t) p;
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
   _mulle_objc_class_lookup_implementation_noforward( struct _mulle_objc_class *cls,
                                                      mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_method        *method;
   mulle_objc_cache_uint_t          offset;
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cache   = _mulle_objc_cachepivot_atomicget_cache( &cls->cachepivot.pivot);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entries = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   entry   = (void *) &((char *) entries)[ offset];
   p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   imp     = (mulle_objc_implementation_t) p;
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
      if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
         _mulle_objc_class_fill_methodcache_with_method( cls, method, methodid);
   }
   return( imp);
}


// does not fill the cache but does forward
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nocache( struct _mulle_objc_class *cls,
                                                    mulle_objc_methodid_t methodid)
{
   mulle_objc_cache_uint_t          offset;
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_method        *method;
   int                              error;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entry   = (void *) &((char *) entries)[ offset];
   p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   imp     = (mulle_objc_implementation_t) p;
   if( imp)
   {
      assert( entry->key.uniqueid == methodid);
      return( imp);
   }

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( ! method)
   {
      method = _mulle_objc_class_lazyget_forwardmethod( cls, &error);
      if( ! method)
         return( imp);  // known to be NULL
   }

   imp = _mulle_objc_method_get_implementation( method);
   assert( _mulle_objc_method_get_methodid( method) == methodid);
   return( imp);
}


// fills the cache and does forward
mulle_objc_implementation_t
    _mulle_objc_class_lookup_implementation( struct _mulle_objc_class *cls,
                                             mulle_objc_methodid_t methodid)
{
   mulle_objc_cache_uint_t          offset;
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_method        *method;
   int                              error;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   offset  = _mulle_objc_cache_find_entryoffset( cache, methodid);
   entry   = (void *) &((char *) entries)[ offset];
   p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
   imp     = (mulle_objc_implementation_t) p;
   if( imp)
      return( imp);

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( ! method)
      method = _mulle_objc_class_lazyget_forwardmethod( cls, &error);

   if( method)
   {
      imp = _mulle_objc_method_get_implementation( method);
      if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
         __mulle_objc_class_fill_methodcache_with_method( cls, method, methodid);
   }
   return( imp);
}


//
// fills the cache and does forward, will raise if no method
// it knows about traces and the empty cache bit
//
mulle_objc_implementation_t
   _mulle_objc_class_lookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                   mulle_objc_methodid_t methodid)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_method       *method;
   struct _mulle_objc_cacheentry   *entry;

   method   = mulle_objc_class_search_method_nofail( cls, methodid);
   imp      = _mulle_objc_method_get_implementation( method);
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
   mulle_objc_class_lookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                  mulle_objc_methodid_t methodid)
{
   if( ! cls || ! mulle_objc_uniqueid_is_sane( methodid))
   {
      errno = EINVAL;  // BAD FAIL so errno is OK
      //  mulle_objc_universe_fail_errno( mulle_objc_global_get_universe());
      mulle_objc_universe_fail_errno( NULL);
   }

   return( _mulle_objc_class_lookup_implementation_nofail( cls, methodid));
}


# pragma mark - calls

void   *_mulle_objc_object_call_class( void *obj,
                                       mulle_objc_methodid_t methodid,
                                       void *parameter,
                                       struct _mulle_objc_class *cls)
{
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   assert( obj);
   assert( mulle_objc_uniqueid_is_sane( methodid));

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   for(;;)
   {
      offset = offset & mask;
      entry  = (void *) &((char *) entries)[ offset];

      if( entry->key.uniqueid == methodid)
      {
         p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp     = (mulle_objc_implementation_t) p;
/*->*/   return( (*imp)( obj, methodid, parameter));
      }

      if( ! entry->key.uniqueid)
         break;

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
/*->*/
   return( _mulle_objc_object_call_class_nofail( obj, methodid, parameter, cls));
}


//
// this function is called, when the first inline cache check gave a
// collision
//
void   *_mulle_objc_object_call2( void *obj,
                                  mulle_objc_methodid_t methodid,
                                  void *parameter)
{
   mulle_objc_implementation_t      imp;
   mulle_functionpointer_t          p;
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *entries;
   struct _mulle_objc_cacheentry    *entry;
   struct _mulle_objc_class         *cls;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          offset;

   assert( mulle_objc_uniqueid_is_sane( methodid));

   cls     = _mulle_objc_object_get_isa( obj);
   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) methodid;
   do
   {
      offset += sizeof( struct _mulle_objc_cacheentry);
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == methodid)
      {
         p       = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp     = (mulle_objc_implementation_t) p;
/*->*/
         return( (*imp)( obj, methodid, parameter));
      }
   }
   while( entry->key.uniqueid);
/*->*/
   return( _mulle_objc_object_call_class_nofail( obj, methodid, parameter, cls));
}


//
// this function is called, when the first inline cache check gave a
// collision
//
static mulle_objc_implementation_t
   _mulle_objc_class_superlookup2_implementation_nofail( struct _mulle_objc_class *cls,
                                                         mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;
   mulle_functionpointer_t         p;

   entries = _mulle_objc_cachepivot_atomicget_entries( &cls->cachepivot.pivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) superid;
   do
   {
      offset += sizeof( struct _mulle_objc_cacheentry);
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == superid)
      {
         p   = _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer);
         imp = (mulle_objc_implementation_t) p;
/*->*/   return( imp);
      }
   }
   while( entry->key.uniqueid);
/*->*/
   return( _mulle_objc_class_superlookup_implementation_nofail( cls, superid));
}


static void   *
   _mulle_objc_object_call_class_nocache( void *obj,
                                          mulle_objc_methodid_t methodid,
                                          void *parameter,
                                          struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method   *method;

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   if( ! method)
      method = _mulle_objc_class_get_forwardmethod_lazy_nofail( cls, methodid);
   return( (*_mulle_objc_method_get_implementation( method))( obj, methodid, parameter));
}


# pragma mark - cache

// this runs when the class is locked,
static void   _mulle_objc_class_setup_initial_cache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_cache      *cache;
   struct mulle_allocator        *allocator;
   mulle_objc_cache_uint_t       n_entries;
   void                          *found;

   // now setup the cache and let it rip, except when we don't ever want one
   universe  = _mulle_objc_class_get_universe( cls);

   // your chance to change the cache algorithm and initital size
   n_entries = _class_search_minmethodcachesize( cls);
   if( universe->callbacks.will_init_cache)
      n_entries = (*universe->callbacks.will_init_cache)( universe, cls, n_entries);

   if( ! _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE))
   {
      allocator = _mulle_objc_universe_get_allocator( universe);
      cache     = mulle_objc_cache_new( n_entries, allocator);

      assert( cache);

      // trace this before the switch
      if( universe->debug.trace.method_cache)
         mulle_objc_universe_trace( universe, "new initial cache %p "
                                    "on %s %08x \"%s\" (%p) with %u entries",
                                    cache,
                                    _mulle_objc_class_get_classtypename( cls),
                                    _mulle_objc_class_get_classid( cls),
                                    _mulle_objc_class_get_name( cls),
                                    cls,
                                    cache->size);
      found = __mulle_atomic_pointer_cas( &cls->cachepivot.pivot.entries,
                                          cache->entries,
                                          universe->empty_methodcache.entries);

      // TODO: here

      cls->superlookup      = _mulle_objc_class_superlookup_implementation_nofail;
      cls->superlookup2     = _mulle_objc_class_superlookup2_implementation_nofail;
      //
      // right after we do these switches, a competing thread may run unimpeded
      // MEMO: put these in atomically ?
      cls->cachepivot.call2 = _mulle_objc_object_call2;
      cls->call             = _mulle_objc_object_call_class;

      assert( found == universe->empty_methodcache.entries);
   }
   else
   {
      cls->superlookup      = _mulle_objc_class_superlookup_implementation_nocache_nofail;
      cls->superlookup2     = _mulle_objc_class_superlookup_implementation_nocache_nofail;

      cls->call             = _mulle_objc_object_call_class_nocache;
      cls->cachepivot.call2 = _mulle_objc_object_call2_emptycache;

      if( universe->debug.trace.method_cache)
         mulle_objc_universe_trace( universe, "use \"always empty cache\" on "
                                    "%s %08x \"%s\" (%p)",
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


static void
   _mulle_objc_class_setup_initial_cache_if_needed( struct _mulle_objc_class *cls)
{
   if( _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY))
      return;

   _mulle_objc_class_setup_initial_cache( cls);

   // this marks the class as ready to run
   _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_CLASS_CACHE_READY);
}


# pragma mark - +initialize


MULLE_C_NEVER_INLINE
void   _mulle_objc_class_warn_recursive_initialize( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   if( universe->debug.trace.initialize)
   {
      mulle_objc_universe_trace( universe, "recursive +[%s initialize] ignored.\n"
                     "break on _mulle_objc_class_warn_recursive_initialize to debug.",
              _mulle_objc_class_get_name( cls));
      mulle_objc_universe_maybe_hang_or_abort( universe);
   }
}


static void
   _mulle_objc_infraclass_call_initialize( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_method       *initialize;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_metaclass    *meta;
   mulle_objc_implementation_t     imp;
   int                             flag;
   char                            *name;
   int                             preserve;

   preserve = errno;

   universe = _mulle_objc_infraclass_get_universe( infra);
   meta     = _mulle_objc_infraclass_get_metaclass( infra);
   name     = _mulle_objc_infraclass_get_name( infra);

   // grab code from superclass
   // this is useful for MulleObjCSingleton
   initialize = mulle_objc_class_defaultsearch_method( &meta->base,
                                                       MULLE_OBJC_INITIALIZE_METHODID);
   if( ! initialize)
   {
      if( universe->debug.trace.initialize)
         mulle_objc_universe_trace( universe, "no +[%s initialize] found", name);

      errno = preserve;
      return;
   }

   if( universe->debug.trace.initialize)
      mulle_objc_universe_trace( universe,
                                 "call +[%s initialize]",
                                 _mulle_objc_metaclass_get_name( meta));

   imp   = _mulle_objc_method_get_implementation( initialize);
   if( universe->debug.trace.method_call)
      mulle_objc_class_trace_call( &infra->base,
                                   infra,
                                   MULLE_OBJC_INITIALIZE_METHODID,
                                   NULL,
                                   imp);
   (*imp)( (struct _mulle_objc_object *) infra,
           MULLE_OBJC_INITIALIZE_METHODID,
           NULL);

   if( universe->debug.trace.initialize)
      mulle_objc_universe_trace( universe,
                                 "done +[%s initialize]",
                                 _mulle_objc_metaclass_get_name( meta));
   errno = preserve;
}


static void  _mulle_objc_infraclass_setup_superclasses( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_protocolclassenumerator   rover;
   struct _mulle_objc_infraclass                *protocolclass;
   struct _mulle_objc_infraclass                *superclass;
   struct _mulle_objc_class                     *cls;

   /*
    * Ensure protocol classes are there
    */
   pair  = _mulle_objc_infraclass_get_classpair( infra);
   rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
   while( protocolclass = _mulle_objc_protocolclassenumerator_next( &rover))
      _mulle_objc_infraclass_setup_if_needed( protocolclass);
   _mulle_objc_protocolclassenumerator_done( &rover);

   /*
    * Ensure superclass is there
    */
   superclass = _mulle_objc_infraclass_get_superclass( infra);
   if( superclass)
      _mulle_objc_infraclass_setup_if_needed( superclass);
}


static void  _mulle_objc_metaclass_setup_superclass( struct _mulle_objc_metaclass *meta)
{
   struct _mulle_objc_metaclass    *superclass;
   struct _mulle_objc_class        *cls;

   /*
    * Ensure superclass is there (infraclass will do protocolclasses)
    */
   superclass = _mulle_objc_metaclass_get_superclass( meta);
   if( superclass)
   {
      cls = _mulle_objc_metaclass_as_class( superclass);
      _mulle_objc_class_setup( cls);
   }
}


int   _mulle_objc_class_setup( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_classpair    *pair;
   mulle_thread_mutex_t            *initialize_lock;
   mulle_thread_t                  current_thread;

   assert( mulle_objc_class_is_current_thread_registered( cls));

   pair           = _mulle_objc_class_get_classpair( cls);
   infra          = _mulle_objc_classpair_get_infraclass( pair);
   current_thread = mulle_thread_self();
   //
   // Allow recursion to same class in same thread
   // if a second thread is incoming we want to lock, so that the other
   // thread can finish up.
   // With the new code, we shouldn't really be getting here anymore
   //
   if( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZING))
   {
      // if we are the same thread, we do the slow call here
      if( (mulle_thread_t) _mulle_atomic_pointer_read( &pair->thread) == current_thread)
      {
         // is this still true ?
         if( cls->superclass)
            _mulle_objc_class_warn_recursive_initialize( cls);  // hmmm
         return( 1);  // go slow
      }
   }

   meta            = _mulle_objc_classpair_get_metaclass( pair);
   initialize_lock = _mulle_objc_classpair_get_lock( pair);
   mulle_thread_mutex_lock( initialize_lock);
   {
      // this has to be set before initializing
      // we make this atomic, just to be sure that it's set before
      // initializing...
      _mulle_atomic_pointer_write( &pair->thread, (void *) current_thread);
      if( _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZING))
      {
         // #>>> PROBLEMATIC #

         // As soon as we setup a method cache, the class is free to be
         // messaged by other threads. This means we have to run +initialize
         // without a cache. Other threads must be blocked in the
         // initialize_lock.
         //
         _mulle_objc_metaclass_setup_superclass( meta);
         _mulle_objc_infraclass_setup_superclasses( infra);

         // MEMO: we are in state MULLE_OBJC_INFRACLASS_INITIALIZING, but
         //       not yet MULLE_OBJC_INFRACLASS_INITIALIZE_DONE. The
         //       superclasses are setup already though. And they ran
         //       (or are running (!)) +initialize.
         //       It is only guaranteed that +initialize on the superclass is
         //       messaged before the subclass, it isn't guaranteed that it has
         //       completed.
         _mulle_objc_infraclass_call_initialize( infra);

         _mulle_objc_infraclass_set_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE);

         // now we can let it rip
         _mulle_objc_class_setup_initial_cache_if_needed( _mulle_objc_metaclass_as_class( meta));
         _mulle_objc_class_setup_initial_cache_if_needed( _mulle_objc_infraclass_as_class( infra));
      }
      else
      {
         // looks like someone else was quicker, fine
         assert( _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE));
      }
   }
   mulle_thread_mutex_unlock( initialize_lock);
   return( 0);
}


void   *_mulle_objc_object_call_class_needcache( void *obj,
                                                 mulle_objc_methodid_t methodid,
                                                 void *parameter,
                                                 struct _mulle_objc_class *cls)
{
   _mulle_objc_class_setup( cls);
   return( _mulle_objc_object_call2_emptycache( obj, methodid, parameter));
//   return( (*cls->call)( obj, methodid, parameter, cls));
}


void   *_mulle_objc_object_call_needcache( void *obj,
                                            mulle_objc_methodid_t methodid,
                                            void *parameter);

void   *_mulle_objc_object_call_needcache( void *obj,
                                            mulle_objc_methodid_t methodid,
                                            void *parameter)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( (struct _mulle_objc_object *) obj);
   return( _mulle_objc_object_call_class_needcache( obj, methodid, parameter, cls));
}


#pragma mark - empty cache calls

static void   *_mulle_objc_object_call2_emptycache( void *obj,
                                                    mulle_objc_methodid_t methodid,
                                                    void *parameter)
{
   struct _mulle_objc_class    *cls;
   struct _mulle_objc_method   *method;

   cls    = _mulle_objc_object_get_isa( obj);
   method = mulle_objc_class_search_method_nofail( cls, methodid);
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


mulle_objc_implementation_t
   _mulle_objc_class_superlookup_needcache( struct _mulle_objc_class *cls,
                                             mulle_objc_superid_t superid);

mulle_objc_implementation_t
   _mulle_objc_class_superlookup_needcache( struct _mulle_objc_class *cls,
                                             mulle_objc_superid_t superid)
{
   // happens when we do +[super initialize] in +initialize
   _mulle_objc_class_setup( cls);
   // this is slow and uncached as we need it
   return( _mulle_objc_class_superlookup_implementation_nocache_nofail( cls, superid));
}


void   *mulle_objc_object_supercall( void *obj,
                                     mulle_objc_methodid_t methodid,
                                     void *parameter,
                                     mulle_objc_superid_t superid)
{
   mulle_objc_implementation_t   imp;

   if( __builtin_expect( ! obj, 0))
      return( obj);

   imp = _mulle_objc_object_superlookup_implementation_inline_nofail( obj, superid);
   return( (*imp)( obj, methodid, parameter));
}


// need to call cls->call to prepare caches

#pragma mark - multiple objects call

void   mulle_objc_objects_call( void **objects,
                                unsigned int n,
                                mulle_objc_methodid_t methodid,
                                void *params)
{
   mulle_objc_implementation_t   (*lookup)( struct _mulle_objc_class *, mulle_objc_methodid_t);
   mulle_objc_implementation_t   imp;
   mulle_objc_implementation_t   lastSelIMP[ 16];
   struct _mulle_objc_class      *lastIsa[ 16];
   struct _mulle_objc_class      *thisIsa;
   unsigned int                  i;
   void                          **sentinel;
   void                          *p;

   assert( mulle_objc_uniqueid_is_sane( methodid));
   memset( lastIsa, 0, sizeof( lastIsa));

   // assume compiler can do unrolling
   lookup   = _mulle_objc_class_lookup_implementation;
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
   _mulle_objc_object_superlookup_implementation_nofail( void *obj,
                                                         mulle_objc_superid_t superid)
{
   return( _mulle_objc_object_superlookup_implementation_inline_nofail( obj, superid));
}
