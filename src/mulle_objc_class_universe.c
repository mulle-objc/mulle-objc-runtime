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
#include "mulle_objc_class_universe.h"

#include "mulle_objc_class.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_universe.h"
#include <mulle_aba/mulle_aba.h>


MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_infraclass  *
   _mulle_objc_universe_unfailinglookup_infraclass( struct _mulle_objc_universe *universe,
                                                    mulle_objc_classid_t classid);


//
// only NSAutoreleasePool 0x5b791fc6 and 0xNSThread are allowed to be called
// in a global universe configuration, so they can set up a proper NSThread
// object
//
int    mulle_objc_class_is_current_thread_registered( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_garbagecollection   *gc;
   struct _mulle_objc_universe            *universe;

#if __MULLE_OBJC_TRT__
   if( cls && (cls->classid == 0x5b791fc6 || cls->classid == 0x645eeb40))
      return( -1);
#endif
   universe = mulle_objc_inlined_get_universe();
   assert( _mulle_objc_universe_is_initialized( universe));

   gc      = _mulle_objc_universe_get_garbagecollection( universe);
   return( _mulle_aba_is_current_thread_registered( &gc->aba));
}


#pragma mark - class cache

static struct _mulle_objc_cacheentry
   *_mulle_objc_universe_add_classcacheentry_by_swapping_caches( struct _mulle_objc_universe *universe,
                                                                 struct _mulle_objc_cache *cache,
                                                                 struct _mulle_objc_class *cls)
{
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *p;
   mulle_objc_cache_uint_t         new_size;
   struct _mulle_objc_cacheentry   *sentinel;
   mulle_objc_classid_t            classid;

   old_cache = cache;

   // a new beginning.. let it be filled anew
   new_size  = old_cache->size * 2;
   cache     = mulle_objc_cache_new( new_size, &cls->universe->memory.allocator);
   if( ! cache)
      return( NULL);

   entry = _mulle_objc_cache_inactivecache_add_pointer_entry( cache, cls, cls->classid);
   
   if( _mulle_objc_cachepivot_atomic_set_entries( &universe->cachepivot, cache->entries, old_cache->entries))
   {
      // trace future!

      _mulle_objc_cache_free( cache, &cls->universe->memory.allocator);
      return( NULL);
   }

   if( universe->debug.trace.class_cache)
   {
      fprintf( stderr, "mulle_objc_universe %p trace: set new class cache %p with %u entries\n",
              universe,
              cache,
              cache->size);
      fprintf( stderr, "mulle_objc_universe %p trace: added class %08x \"%s\" to class cache %p\n",
              universe,
              _mulle_objc_class_get_classid( cls),
              _mulle_objc_class_get_name( cls),
              cache);
   }
   
   if( &old_cache->entries[ 0] != &universe->empty_cache.entries[ 0])
   {
      //
      // if we repopulate the new cache with the old cache, we can then
      // determine, which classes have actually been used over the course
      // of the program by just dumping the cache contents
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
               (void) _mulle_objc_universe_unfailinglookup_infraclass( universe, classid);
         }
         
         // entry is still valid, even if the cache has been blown away in the
         // meantime (aba)
      }

      if( universe->debug.trace.class_cache)
         fprintf( stderr, "mulle_objc_universe %p trace: frees class cache %p with %u entries\n",
                 universe,
                 old_cache,
                 old_cache->size);
      
      _mulle_objc_cache_abafree( old_cache, &cls->universe->memory.allocator);
   }

   return( entry);
}


MULLE_C_NON_NULL_RETURN
static struct _mulle_objc_cacheentry   *
   _mulle_objc_universe_fill_classcache_with_infraclass( struct _mulle_objc_universe *universe,
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
      cache = _mulle_objc_cachepivot_atomic_get_cache( &universe->cachepivot);
      if( _mulle_objc_cache_should_grow( cache))
      {
         entry = _mulle_objc_universe_add_classcacheentry_by_swapping_caches( universe, cache, _mulle_objc_infraclass_as_class( infra));
         if( ! entry)
            continue;
         break;
      }

      entry = _mulle_objc_cache_add_pointer_entry( cache, infra, _mulle_objc_infraclass_get_classid( infra));
      if( entry)
      {
         // trace here so we output the proper cache
         if( universe->debug.trace.class_cache)
            fprintf( stderr, "mulle_objc_universe %p trace: added class %08x \"%s\" to class cache %p\n",
                    universe,
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

   infra = _mulle_objc_universe_uncachedlookup_infraclass( universe, classid);
   if( ! infra)
      return( NULL);

   entry = _mulle_objc_universe_fill_classcache_with_infraclass( universe, infra);
   assert( entry);
   return( entry);
}


MULLE_C_CONST_NON_NULL_RETURN
static struct _mulle_objc_cacheentry   *
    _mulle_objc_universe_unfailingfill_classcache( struct _mulle_objc_universe *universe,
                                                   mulle_objc_classid_t classid)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_cacheentry   *entry;

   infra   = _mulle_objc_universe_unfailinguncachedlookup_infraclass( universe, classid);
   assert( mulle_objc_class_is_current_thread_registered( (void *)  infra));

   entry = _mulle_objc_universe_fill_classcache_with_infraclass( universe, infra);
   return( entry);
}


#pragma mark - class lookup, uncached

struct _mulle_objc_infraclass   *
   _mulle_objc_universe_uncachedlookup_infraclass( struct _mulle_objc_universe *universe,
                                                   mulle_objc_classid_t classid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->classtable, classid));
}


MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_infraclass   *
   _mulle_objc_universe_unfailinguncachedlookup_infraclass( struct _mulle_objc_universe *universe,
                                                            mulle_objc_classid_t classid)
{
   struct _mulle_objc_infraclass   *infra;
   int                              retry;
   
   assert( classid != MULLE_OBJC_INVALID_CLASSID && classid != MULLE_OBJC_NO_CLASSID);
   
   retry = 1;
   for(;;)
   {
      infra = _mulle_concurrent_hashmap_lookup( &universe->classtable, classid);
      if( infra)
      {
         assert( _mulle_objc_infraclass_get_classid( infra) == classid);
         return( infra);
      }
      
      if( retry)
      {
         retry = 0;
         (*universe->classdefaults.class_is_missing)( universe, classid);
         continue;
      }
      
      _mulle_objc_universe_raise_class_not_found_exception( universe, classid);
   }
}


//
// try to hide those uglies as much as possible :)
//
#pragma mark - class lookup, cached but no fastclass

//
// will place class into cache, will not check for fastclass
//
struct _mulle_objc_infraclass  *
    _mulle_objc_universe_lookup_infraclass( struct _mulle_objc_universe *universe,
                                           mulle_objc_classid_t classid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   entries = _mulle_objc_cachepivot_atomic_get_entries( &universe->cachepivot);
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
MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_infraclass  *
    _mulle_objc_universe_unfailinglookup_infraclass( struct _mulle_objc_universe *universe,
                                                     mulle_objc_classid_t classid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   entries = _mulle_objc_cachepivot_atomic_get_entries( &universe->cachepivot);
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
         entry = _mulle_objc_universe_unfailingfill_classcache( universe, classid);
         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));
      }

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


#pragma mark - class lookup, cached and fastclass

MULLE_C_NON_NULL_RETURN
struct _mulle_objc_infraclass   *mulle_objc_unfailingfastlookup_infraclass( mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;
   
   universe = mulle_objc_inlined_get_universe();
   return( _mulle_objc_universe_unfailingfastlookup_infraclass( universe, classid));
}

