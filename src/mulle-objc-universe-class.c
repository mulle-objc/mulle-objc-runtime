//
//  mulle_objc_class_universe.c
//  mulle-objc-runtime
//
//  Created by Nat! on 10.07.16.
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
#include "mulle-objc-universe-class.h"

#include "mulle-objc-class.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-universe.h"
#include "include-private.h"


int    mulle_objc_class_is_current_thread_registered( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_garbagecollection   *gc;
   struct _mulle_objc_universe            *universe;

   if( ! cls)
      return( 0);

   universe = mulle_objc_class_get_universe( cls);
   assert( ! _mulle_objc_universe_is_uninitialized( universe));

   gc = _mulle_objc_universe_get_gc( universe);
   return( _mulle_aba_is_current_thread_registered( &gc->aba));
}


#pragma mark - class cache

static struct _mulle_objc_cacheentry *
   _mulle_objc_universe_add_classcacheentry_swapcaches( struct _mulle_objc_universe *universe,
                                                        struct _mulle_objc_cache *cache,
                                                        struct _mulle_objc_class *cls)
{
   mulle_objc_cache_uint_t         new_size;
   mulle_objc_classid_t            classid;
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;
   struct mulle_allocator          *allocator;

   old_cache = cache;

   allocator = _mulle_objc_universe_get_allocator( universe);
   // a new beginning.. let it be filled anew
   new_size  = old_cache->size * 2;
   cache     = mulle_objc_cache_new( new_size, allocator);
   if( ! cache)
      return( NULL);

   entry = _mulle_objc_cache_inactivecache_add_pointer_entry( cache, cls, cls->classid);

   if( _mulle_objc_cachepivot_atomiccas_entries( &universe->cachepivot, cache->entries, old_cache->entries))
   {
      // trace future!

      _mulle_objc_cache_free( cache, allocator);
      return( NULL);
   }

   if( universe->debug.trace.class_cache)
   {
      mulle_objc_universe_trace( universe,
                                 "set new class cache %p with %u entries",
                                 cache,
                                 cache->size);
      mulle_objc_universe_trace( universe,
                                 "added class %08x \"%s\" to class cache %p",
                                 _mulle_objc_class_get_classid( cls),
                                 _mulle_objc_class_get_name( cls),
                                 cache);
   }

   if( &old_cache->entries[ 0] != &universe->empty_cache.entries[ 0])
   {
      //
      // if we repopulate the new cache with the old cache, we can then
      // determine, which classes have actually been used over the course
      // of the program by just dumping the cache contents.
      //
      if( universe->config.repopulate_caches)
      {
         p        = &old_cache->entries[ 0];
         sentinel = &p[ old_cache->size];
         while( p < sentinel)
         {
            classid = (mulle_objc_classid_t)(intptr_t) _mulle_atomic_pointer_read( &p->key.pointer);
            ++p;

            // place it into cache
            if( classid != MULLE_OBJC_NO_CLASSID)
               (void) mulle_objc_universe_lookup_infraclass_nofail( universe, classid);
         }

         // entry is still valid, even if the cache has been blown away in the
         // meantime (aba)
      }

      if( universe->debug.trace.class_cache)
         mulle_objc_universe_trace( universe,
                                    "frees class cache %p with %u entries\n",
                                    old_cache,
                                    old_cache->size);

      _mulle_objc_cache_abafree( old_cache, allocator);
   }

   return( entry);
}


MULLE_C_NONNULL_RETURN static struct _mulle_objc_cacheentry *
   _mulle_objc_universe_fill_classcache_class( struct _mulle_objc_universe *universe,
                                               struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;

   assert( infra);
   assert( universe);

   //
   // here try to get most up to date value
   //
   for(;;)
   {
      cache = _mulle_objc_cachepivot_atomicget_cache( &universe->cachepivot);
      if( _mulle_objc_universe_should_grow_cache( universe, cache))
      {
         entry = _mulle_objc_universe_add_classcacheentry_swapcaches( universe,
                                                                      cache,
                                                                      _mulle_objc_infraclass_as_class( infra));
         if( ! entry)
            continue;
         break;
      }

      entry = _mulle_objc_cache_add_pointer_entry( cache,
                                                   infra,
                                                   _mulle_objc_infraclass_get_classid( infra));
      if( entry)
      {
         // trace here so we output the proper cache
         if( universe->debug.trace.class_cache)
            mulle_objc_universe_trace( universe,
                                       "added class %08x \"%s\" to "
                                       "class cache %p\n",
                                       _mulle_objc_infraclass_get_classid( infra),
                                       _mulle_objc_infraclass_get_name( infra),
                                       cache);
         break;
      }
   }

   return( entry);
}


static struct _mulle_objc_cacheentry   *
   _mulle_objc_universe_fill_classcache( struct _mulle_objc_universe *universe,
                                         mulle_objc_classid_t classid)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_cacheentry   *entry;

   infra = _mulle_objc_universe_lookup_infraclass_nocache_nofast( universe, classid);
   if( ! infra)
      return( NULL);

   entry = _mulle_objc_universe_fill_classcache_class( universe, infra);
   assert( entry);
   return( entry);
}


void   _mulle_objc_universe_invalidate_classcache( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_cacheentry   *expect_entries;
   struct _mulle_objc_cacheentry   *old_entries;
   struct _mulle_objc_cache        *old_cache;
   struct mulle_allocator          *allocator;

   old_entries = NULL;
   do
   {
      expect_entries = old_entries;
      old_entries    = _mulle_objc_cachepivot_atomiccweakcas_entries( &universe->cachepivot,
                                                                      universe->empty_cache.entries,
                                                                      old_entries);
      if( old_entries == universe->empty_cache.entries)
         return;
   }
   while( old_entries != expect_entries);

   allocator = _mulle_objc_universe_get_allocator( universe);
   old_cache = _mulle_objc_cacheentry_get_cache_from_entries( old_entries);
   _mulle_objc_cache_abafree( old_cache, allocator);
}



MULLE_C_CONST_NONNULL_RETURN static struct _mulle_objc_cacheentry *
    _mulle_objc_universe_fill_classcache_nofail( struct _mulle_objc_universe *universe,
                                                   mulle_objc_classid_t classid)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_cacheentry   *entry;

   infra   = _mulle_objc_universe_lookup_infraclass_nocache_nofail_nofast( universe, classid);
   assert( mulle_objc_class_is_current_thread_registered( (void *)  infra));

   entry = _mulle_objc_universe_fill_classcache_class( universe, infra);
   return( entry);
}


#pragma mark - infraclass lookup, cached but no fast lookup

struct _mulle_objc_infraclass   *
   _mulle_objc_universe_lookup_infraclass_nocache_nofast( struct _mulle_objc_universe *universe,
                                                          mulle_objc_classid_t classid)
{
   struct _mulle_objc_infraclass    *infra;
   int                              retry;
   int                              preserve;


   retry = 1;
   for(;;)
   {
      infra = _mulle_objc_universe_inlinelookup_infraclass_nocache_nofast( universe, classid);
      if( infra)
      {
         assert( _mulle_objc_infraclass_get_classid( infra) == classid);
         return( infra);
      }

      if( ! retry)
        return( NULL);

      retry = 0;

      // preserve errno for user
      preserve = errno;
      (*universe->classdefaults.class_is_missing)( universe, classid);
      errno    = preserve;
   }
}


MULLE_C_CONST_NONNULL_RETURN struct _mulle_objc_infraclass   *
   _mulle_objc_universe_lookup_infraclass_nocache_nofail_nofast( struct _mulle_objc_universe *universe,
                                                                 mulle_objc_classid_t classid)
{
   struct _mulle_objc_infraclass   *infra;

   assert( mulle_objc_uniqueid_is_sane( classid));

   infra = _mulle_objc_universe_lookup_infraclass_nocache_nofast( universe, classid);
   if( infra)
      return( infra);

   mulle_objc_universe_fail_classnotfound( universe, classid);
}


// used by the debugger
MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass *
   mulle_objc_global_lookup_infraclass_nofail_nofast( mulle_objc_universeid_t universeid,
                                                            mulle_objc_classid_t classid)
{
   return( mulle_objc_global_inlinelookup_infraclass_nofail_nofast( universeid, classid));
}



//
// try to hide those uglies as much as possible :)
//
#pragma mark - infraclass lookup, cached but no fast lookup

//
// will place class into cache, will not check for fastclass
//
struct _mulle_objc_infraclass  *
    _mulle_objc_universe_lookup_infraclass_nofast( struct _mulle_objc_universe *universe,
                                                   mulle_objc_classid_t classid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   entries = _mulle_objc_cachepivot_atomicget_entries( &universe->cachepivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) classid;
   for(;;)
   {
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == classid)
         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));

      if( ! entry->key.uniqueid)
      {
         entry = _mulle_objc_universe_fill_classcache( universe, classid);
         if( ! entry)
            return( NULL);

         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));
      }

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


//
// class must exist, otherwise pain
// will place class into cache, will not check for fastclass
//
MULLE_C_CONST_NONNULL_RETURN struct _mulle_objc_infraclass  *
    _mulle_objc_universe_lookup_infraclass_nofail_nofast( struct _mulle_objc_universe *universe,
                                                          mulle_objc_classid_t classid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   entries = _mulle_objc_cachepivot_atomicget_entries( &universe->cachepivot);
   cache   = _mulle_objc_cacheentry_get_cache_from_entries( entries);
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) classid;
   for(;;)
   {
      offset  = offset & mask;
      entry   = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == classid)
         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));

      if( ! entry->key.uniqueid)
      {
         entry = _mulle_objc_universe_fill_classcache_nofail( universe, classid);
         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));
      }

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


#pragma mark - infraclass lookup, fastclass lookup then cached


struct _mulle_objc_infraclass  *
    _mulle_objc_universe_lookup_infraclass( struct _mulle_objc_universe *universe,
                                            mulle_objc_classid_t classid)
{
   if( universe->config.no_fast_call)
      return( _mulle_objc_universe_lookup_infraclass_nofast( universe, classid));
   return( _mulle_objc_universe_inlinelookup_infraclass( universe, classid));
}


MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass  *
    mulle_objc_universe_lookup_infraclass_nofail( struct _mulle_objc_universe *universe,
                                                   mulle_objc_classid_t classid)
{
   assert( universe);

   if( universe->config.no_fast_call)
      return( _mulle_objc_universe_lookup_infraclass_nofail_nofast( universe, classid));
   return( _mulle_objc_universe_inlinelookup_infraclass_nofail( universe, classid));
}


// This is called by the compiler for [Foo class] to lookup Foo.
//
// We could "hardcode" the universeid with
//    mulle_objc_global_defaultlookup_infraclass_nofail
// and
//    mulle_objc_global_vfl1848lookup_infraclass_nofail
// for each universe, (autogenerate the function call) via cpp and
// #__MULLE_OBJC_UNIVERSEID__
//
// I am not sure that this translates into much of a win though.

// we could create mulle_objc_object_lookup_infraclass_nofail
MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass  *
   mulle_objc_global_lookup_infraclass_nofail( mulle_objc_universeid_t universeid,
                                               mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_inlineget_universe( universeid);
   return( mulle_objc_universe_lookup_infraclass_nofail( universe, classid));
}


#pragma mark - infraclass lookup via object, fastclass lookup then cached

MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass *
   mulle_objc_object_lookup_infraclass_nofail( void *obj,
                                               mulle_objc_universeid_t universeid,
                                               mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = __mulle_objc_object_get_universe_nofail( obj, universeid);
   return( mulle_objc_universe_lookup_infraclass_nofail( universe,
                                                         classid));
}


MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass *
   mulle_objc_object_lookup_infraclass_nofail_nofast( void *obj,
                                                      mulle_objc_universeid_t universeid,
                                                      mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = __mulle_objc_object_get_universe_nofail( obj, universeid);
   return( _mulle_objc_universe_lookup_infraclass_nofail_nofast( universe,
                                                                 classid));
}
