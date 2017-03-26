//
//  mulle_objc_kvccache.c
//  mulle-objc
//
//  Created by Nat! on 18.07.16.
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
#include "mulle_objc_kvccache.h"

#include "mulle_objc_class.h"
#include "mulle_objc_runtime.h"
#include "mulle_objc_signature.h"
#include "mulle_objc_uniqueid.h"

#include <mulle_allocator/mulle_allocator.h>
#include <string.h>


struct _mulle_objc_kvcinfo  *_mulle_objc_kvcinfo_new( char *cKey,
                                                      struct mulle_allocator *allocator)
{
   struct _mulle_objc_kvcinfo   *entry;
   size_t                       len;

   len   = strlen( cKey); // cKey is dimension [1], so no + 1 here
   entry = mulle_allocator_calloc( allocator, 1, len + sizeof( struct _mulle_objc_kvcinfo));
   entry->valueType = _C_ID;
   memcpy( entry->cKey, cKey, len);
   return( entry);
}


# pragma mark - cache

static void  _mulle_objc_kvccache_abafree( struct _mulle_objc_kvccache *cache,
                                           struct mulle_allocator *allocator)
{
   // walk through entries, free them. This can only be done
   // if the cache entries have been successfully atomically exchanged
   struct _mulle_objc_cacheentry      *start;
   struct _mulle_objc_cacheentry      *sentinel;
   struct _mulle_objc_kvccacheinfo    *info;

   start    = cache->base.entries;
   sentinel = &start[ cache->base.size];
   while( start < sentinel)
   {
      info = _mulle_atomic_pointer_read( &start->value.pointer);
      mulle_allocator_abafree( allocator, info);
      ++start;
   }

   mulle_allocator_abafree( allocator, cache);
}


static inline  void  _mulle_objc_kvccache_free( struct _mulle_objc_kvccache *cache,
                                                struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, cache);
}


static struct _mulle_objc_cacheentry
*_mulle_objc_kvccachepivot_add_cacheentry_by_swapping_caches( struct _mulle_objc_kvccachepivot *pivot,
                                                              struct _mulle_objc_kvccache *cache,
                                                              struct _mulle_objc_kvccache  *empty_cache,
                                                              struct _mulle_objc_kvcinfo *info,
                                                              mulle_objc_uniqueid_t keyid,
                                                              struct mulle_allocator *allocator)
{
   struct _mulle_objc_kvccache     *old_cache;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         new_size;

   old_cache = cache;

   // a new beginning.. let it be filled anew
   new_size  = old_cache->base.size * 2;
   cache     = mulle_objc_kvccache_new( new_size, allocator);
   if( ! cache)
      return( NULL);

   entry = _mulle_objc_kvccache_inactivecache_add_entry( cache, info, keyid);
   if( _mulle_objc_kvccachepivot_atomic_set_entries( pivot, cache->base.entries, old_cache->base.entries))
   {
      _mulle_objc_kvccache_free( cache, allocator);
      return( NULL);
   }

   if( &old_cache->base.entries[ 0] != &empty_cache->base.entries[ 0])
      _mulle_objc_kvccache_abafree( old_cache, allocator);

   return( entry);
}


int    _mulle_objc_kvccachepivot_set_kvcinfo( struct _mulle_objc_kvccachepivot *pivot,
                                              struct _mulle_objc_kvcinfo *info,
                                              struct _mulle_objc_kvccache *empty_cache,
                                              struct mulle_allocator *allocator)
{
   struct _mulle_objc_cacheentry      *entry;
   struct _mulle_objc_kvccache        *cache;
   mulle_objc_uniqueid_t              keyid;

   keyid = mulle_objc_uniqueid_from_string( info->cKey);

   for(;;)
   {
      cache = _mulle_objc_kvccachepivot_atomic_get_cache( pivot);
      if( (size_t) _mulle_atomic_pointer_read( &cache->base.n) >= (cache->base.size >> 1))  // 50% fill rate should be OK
      {
         entry = _mulle_objc_kvccachepivot_add_cacheentry_by_swapping_caches( pivot, cache, empty_cache, info, keyid, allocator);
         if( entry)
            return( 0);
         continue;
      }

      //
      // when we have a conflict, there are two possibilties
      // recreate the cache and use the new value
      // bail: I think bailing is better, because we can't get into thrashing
      // when two keys match and are used often
      //
      entry = _mulle_objc_kvccache_add_entry( cache, info, keyid);
      if( entry)
         return( 0);
      return( -1);  // conflict
   }
}


//
// the kvcinfo will be for this keyid, but there could be duplicates
//
struct _mulle_objc_kvcinfo  *_mulle_objc_kvccache_lookup_kvcinfo( struct _mulle_objc_kvccache *cache,
                                                                  char *key)
{
   mulle_objc_uniqueid_t           keyid;
   struct _mulle_objc_kvcinfo      *info;

   keyid = mulle_objc_uniqueid_from_string( key);
   info  = _mulle_objc_cache_lookup_pointer(  (struct _mulle_objc_cache *) cache, keyid);
   if( ! info)
      return( NULL);

   if( strcmp( info->cKey, key))
      return( MULLE_OBJC_KVCINFO_CONFLICT);
   return( info);
}


int   _mulle_objc_kvccachepivot_invalidate( struct _mulle_objc_kvccachepivot *pivot,
                                            struct _mulle_objc_kvccache *empty_cache,
                                            struct mulle_allocator *allocator)
{
   struct _mulle_objc_kvccache    *old_cache;
   struct _mulle_objc_cacheentry  *old_entries;

   do
   {
      old_entries = _mulle_objc_kvccachepivot_atomic_get_entries( pivot);
      if( old_entries == &empty_cache->base.entries[ 0])
         return( 0);
   }
   while( _mulle_objc_kvccachepivot_atomic_set_entries( pivot, &empty_cache->base.entries[ 0], old_entries));

   old_cache = (struct _mulle_objc_kvccache*) _mulle_objc_cacheentry_get_cache_from_entries( old_entries);
   _mulle_objc_kvccache_abafree( old_cache, allocator);
   return( 1);
}
