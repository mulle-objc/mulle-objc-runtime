//
//  mulle_objc_class_runtime.c
//  mulle-objc
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
#include "mulle_objc_class_runtime.h"

#include "mulle_objc_class.h"
#include "mulle_objc_runtime.h"
#include <mulle_aba/mulle_aba.h>


//
// only NSAutoreleasePool 0x5b791fc6 and 0xNSThread are allowed to be called
// in a global runtime configuration, so they can set up a proper NSThread
// object
//
int    mulle_objc_class_is_current_thread_registered( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_garbagecollection   *gc;
   struct _mulle_objc_runtime             *runtime;

#ifndef __MULLE_OBJC_TRT__
   if( cls && (cls->classid == 0x5b791fc6 || cls->classid == 0x645eeb40))
      return( -1);
#endif
   runtime = mulle_objc_inlined_get_runtime();
   gc      = _mulle_objc_runtime_get_garbagecollection( runtime);
   return( _mulle_aba_is_current_thread_registered( &gc->aba));
}


#pragma mark -
#pragma mark class lookup

MULLE_C_NON_NULL_RETURN
struct _mulle_objc_class   *mulle_objc_unfailing_get_or_lookup_class( mulle_objc_classid_t classid)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   return( _mulle_objc_runtime_unfailing_get_or_lookup_class( runtime, classid));
}


struct _mulle_objc_class   *_mulle_objc_runtime_lookup_uncached_class( struct _mulle_objc_runtime *runtime,
                                                                       mulle_objc_classid_t classid)
{
   return( _mulle_concurrent_hashmap_lookup( &runtime->classtable, classid));
}


struct _mulle_objc_class   *_mulle_objc_runtime_unfailing_lookup_uncached_class( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   struct _mulle_objc_class   *cls;
   int                        retry;
   
   assert( classid != MULLE_OBJC_INVALID_CLASSID && classid != MULLE_OBJC_NO_CLASSID);
   
   retry = 1;
   for(;;)
   {
      cls = _mulle_concurrent_hashmap_lookup( &runtime->classtable, classid);
      if( cls)
      {
         assert( cls->classid == classid);
         return( cls);
      }
      
      if( retry)
      {
         retry = 0;
         (*runtime->classdefaults.class_is_missing)( runtime, classid);
         continue;
      }
      
      _mulle_objc_runtime_raise_class_not_found_exception( runtime, classid);
   }
}


#pragma mark -
#pragma mark class cache

static struct _mulle_objc_cacheentry
   *_mulle_objc_runtime_add_classcacheentry_by_swapping_caches( struct _mulle_objc_runtime *runtime, struct _mulle_objc_cache *cache, struct _mulle_objc_class *cls)
{
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t          new_size;
   
   old_cache = cache;
   
   // a new beginning.. let it be filled anew
   new_size  = old_cache->size * 2;
   cache     = mulle_objc_cache_new( new_size, &cls->runtime->memory.allocator);
   if( ! cache)
      return( NULL);
   
   entry = _mulle_objc_cache_inactivecache_add_pointer_entry( cache, cls, cls->classid);
   
   if( _mulle_objc_cachepivot_atomic_set_entries( &runtime->cachepivot, cache->entries, old_cache->entries))
   {
      _mulle_objc_cache_free( cache, &cls->runtime->memory.allocator);
      return( NULL);
   }
   
   if( &old_cache->entries[ 0] != &runtime->empty_cache.entries[ 0])
      _mulle_objc_cache_abafree( old_cache, &cls->runtime->memory.allocator);
   
   return( entry);
}


static struct _mulle_objc_cacheentry   *_mulle_objc_runtime_fill_classcache_with_class( struct _mulle_objc_runtime *runtime, struct _mulle_objc_class *cls)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entry;
   
   assert( cls && ! _mulle_objc_class_is_metaclass( cls));
   assert( runtime);
   
   //
   // here try to get most up to date value
   //
   for(;;)
   {
      cache = _mulle_objc_cachepivot_atomic_get_cache( &runtime->cachepivot);
      if( (size_t) _mulle_atomic_pointer_read( &cache->n) >= (cache->size >> 2))  // 25% fill rate is nicer
      {
         entry = _mulle_objc_runtime_add_classcacheentry_by_swapping_caches( runtime, cache, cls);
         if( entry)
            return( entry);
         continue;
      }

      entry = _mulle_objc_cache_add_pointer_entry( cache, cls, cls->classid);
      if( entry)
         return( entry);
   }
}


static struct _mulle_objc_cacheentry   *_mulle_objc_runtime_fill_classcache( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_cacheentry   *entry;
   
   cls = _mulle_objc_runtime_lookup_uncached_class( runtime, classid);
   if( ! cls)
      return( NULL);
   
   entry = _mulle_objc_runtime_fill_classcache_with_class( runtime, cls);
   assert( entry);
   return( entry);
}


static struct _mulle_objc_cacheentry   *_mulle_objc_runtime_unfailing_fill_classcache( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   struct _mulle_objc_class        *cls;
   struct _mulle_objc_cacheentry   *entry;
   
   cls   = _mulle_objc_runtime_unfailing_lookup_uncached_class( runtime, classid);
   assert( mulle_objc_class_is_current_thread_registered( cls));
   
   entry = _mulle_objc_runtime_fill_classcache_with_class( runtime, cls);
   return( entry);
}


#pragma mark -
#pragma mark class lookup

MULLE_C_CONST_RETURN
struct _mulle_objc_class  *_mulle_objc_runtime_lookup_class( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;
   
   entries = _mulle_objc_cachepivot_atomic_get_entries( &runtime->cachepivot);
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
         entry = _mulle_objc_runtime_fill_classcache( runtime, classid);
         if( ! entry)
            return( NULL);
         
         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));
      }

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_class  *_mulle_objc_runtime_unfailing_lookup_class( struct _mulle_objc_runtime *runtime,
                                                                       mulle_objc_classid_t classid)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;
   
   entries = _mulle_objc_cachepivot_atomic_get_entries( &runtime->cachepivot);
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
         entry = _mulle_objc_runtime_unfailing_fill_classcache( runtime, classid);
         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));
      }

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}
