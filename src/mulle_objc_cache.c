//
//  mulle_objc_cache.c
//  mulle-objc-runtime
//
//  Created by Nat! on 15.09.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
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
#include "mulle_objc_cache.h"

#include <mulle_thread/mulle_thread.h>
#include <assert.h>
#include <string.h>


#pragma mark - methodcache

struct _mulle_objc_cache   *mulle_objc_cache_new( mulle_objc_cache_uint_t size, struct mulle_allocator *allocator)
{
   struct _mulle_objc_cache  *cache;

   assert( allocator);
   assert( ! (sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1)));

   if( size < MULLE_OBJC_MIN_CACHE_SIZE)
      size = MULLE_OBJC_MIN_CACHE_SIZE;

   assert( ! (size & (size - 1)));          // check for tumeni bits

   cache = _mulle_allocator_calloc( allocator, 1, sizeof( struct _mulle_objc_cache) + sizeof( struct _mulle_objc_cacheentry) * (size - 1));

   cache->size = size;
   // myhardworkbythesewordsguardedpleasedontsteal © Nat!
   cache->mask = (size - 1) * sizeof( struct _mulle_objc_cacheentry);    // preshift

   return( cache);
}


void   *_mulle_objc_cache_lookup_pointer( struct _mulle_objc_cache *cache,
                                          mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   assert( uniqueid != MULLE_OBJC_NO_UNIQUEID && uniqueid != MULLE_OBJC_INVALID_UNIQUEID);

   entries = cache->entries;
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) uniqueid;
   for(;;)
   {
      offset = (mulle_objc_cache_uint_t) offset & mask;
      entry = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == uniqueid)
         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));

      if( ! _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer))
         return( NULL);

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


mulle_functionpointer_t  _mulle_objc_cache_lookup_functionpointer( struct _mulle_objc_cache *cache,
                                                                   mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   assert( uniqueid != MULLE_OBJC_NO_UNIQUEID && uniqueid != MULLE_OBJC_INVALID_UNIQUEID);

   entries = cache->entries;
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) uniqueid;
   for(;;)
   {
      offset = (mulle_objc_cache_uint_t) offset & mask;
      entry = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == uniqueid)
         return( _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer));

      if( ! _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer))
         return( NULL);

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


int   _mulle_objc_cache_find_entryindex( struct _mulle_objc_cache *cache, mulle_objc_uniqueid_t uniqueid)
{
   int                             index;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *entries;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   assert( cache);
   assert( uniqueid != MULLE_OBJC_NO_UNIQUEID && uniqueid != MULLE_OBJC_INVALID_UNIQUEID);

   entries = cache->entries;
   mask    = cache->mask;
   index   = 0;

   offset  = (mulle_objc_cache_uint_t) uniqueid;
   for(;;)
   {
      offset = offset & mask;
      entry  = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == uniqueid)
         return( index);

      // prefer larger of the two for NULL check read
      if( sizeof( mulle_functionpointer_t) > sizeof( void *))
      {
         if( ! _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer))
            return( -1);
      }
      else
         if( ! _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer))
            return( -1);

      offset += sizeof( struct _mulle_objc_cacheentry);
      ++index;
   }
}


//
// find a slot, where either the uniqueid matches, or where the slot is free
// (at least at the moment)
//
mulle_objc_cache_uint_t   _mulle_objc_cache_find_entryoffset( struct _mulle_objc_cache *cache, mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   assert( cache);
   assert( uniqueid != MULLE_OBJC_NO_UNIQUEID && uniqueid != MULLE_OBJC_INVALID_UNIQUEID);

   entries = cache->entries;
   mask    = cache->mask;

   offset  = (mulle_objc_cache_uint_t) uniqueid;
   for(;;)
   {
      offset = offset & mask;
      entry  = (void *) &((char *) entries)[ offset];
      if( entry->key.uniqueid == uniqueid)
         return( offset);

      // prefer larger of the two for NULL check read
      if( sizeof( mulle_functionpointer_t) > sizeof( void *))
      {
         if( ! _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer))
            return( offset);
      }
      else
         if( ! _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer))
            return( offset);


      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


unsigned int  mulle_objc_cache_calculate_fillpercentage( struct _mulle_objc_cache *cache)
{
   intptr_t   n;

   if( ! cache || ! cache->size)
      return( 0);

   n = (intptr_t) _mulle_atomic_pointer_read( &cache->n);
   return( (unsigned int)  (n * 100 + (cache->size / 2)) / cache->size);
}


unsigned int   mulle_objc_cache_calculate_hitpercentage( struct _mulle_objc_cache *cache,
                                                unsigned int *percentages,
                                                unsigned int size)
{
   unsigned int                     i, n;
   unsigned int                     total;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;

   if( ! percentages || size <= 1)
      return( 0);
   if( ! cache || ! cache->size)
      return( 0);

   if( ! _mulle_atomic_pointer_read( &cache->n))
      return( 0);

   memset( percentages, 0, sizeof( unsigned int) * size);

   i        = 0;
   total    = 0;
   p        = cache->entries;
   sentinel = &p[ cache->size];

   while( p < sentinel)
   {
      if( p->key.uniqueid)
      {
         ++percentages[ i < size ? i : size - 1];
         ++i;
         ++total;
      }
      else
         i = 0;
      ++p;
   }

   assert( total);
   n = 0;
   for( i = 0; i < size; i++)
   {
      if( percentages[ i])
         n = i + 1;

      percentages[ i] = (percentages[ i] * 100 + (total >> 1)) / total;
   }
   return( n);
}


# pragma mark - add

// this only works for a cache, that isn't active in the universe yet and that
// has enough space (!)

struct _mulle_objc_cacheentry   *_mulle_objc_cache_inactivecache_add_pointer_entry( struct _mulle_objc_cache *cache, void *pointer, mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;

   assert( ! _mulle_objc_cache_should_grow( cache));

   offset = _mulle_objc_cache_find_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   assert( ! entry->key.uniqueid);  // if it's not, it's not inactive!
   assert( ! _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));

   entry->key.uniqueid = uniqueid;
   _mulle_atomic_pointer_nonatomic_write( &entry->value.pointer, pointer);

   _mulle_atomic_pointer_increment( &cache->n);

   return( entry);
}


struct _mulle_objc_cacheentry   *_mulle_objc_cache_inactivecache_add_functionpointer_entry( struct _mulle_objc_cache *cache, mulle_functionpointer_t pointer, mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;

   assert( ! _mulle_objc_cache_should_grow( cache));

   offset = _mulle_objc_cache_find_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   assert( ! entry->key.uniqueid);  // if it's not, it's not inactive!
   assert( ! _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));

   entry->key.uniqueid = uniqueid;
   _mulle_atomic_functionpointer_nonatomic_write( &entry->value.functionpointer, pointer);

   _mulle_atomic_pointer_increment( &cache->n);

   return( entry);
}



struct _mulle_objc_cacheentry   *_mulle_objc_cache_add_pointer_entry( struct _mulle_objc_cache *cache,
                                                                      void *pointer,
                                                                      mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;

   assert( cache);
   assert( pointer);
   assert( uniqueid != MULLE_OBJC_NO_UNIQUEID && uniqueid != MULLE_OBJC_INVALID_UNIQUEID);

   //
   // entries pointer never changes in cache..
   //
   offset = _mulle_objc_cache_find_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   //
   // We expect this entry to be 0,0 and we write 0,x first.
   // As 0,x it will still be ignored by other readers. if successful we put
   // in the methodid and increment ->n.
   // There is a time when the cache has actually n entries, but it's count
   // is < n. that's not fatal but one needs to know this
   //
   if( ! _mulle_atomic_pointer_compare_and_swap( &entry->value.pointer, pointer, NULL))
   {
      //
      // implementation set by someone else...
      // if that guy is done writing the uniqueid and it's ours, then fine!
      // #1#
      if( _mulle_atomic_pointer_read( &entry->key.pointer) == (void *) (uintptr_t) uniqueid)
         return( entry);

      return( NULL);
   }

   // increment first to keep cach fill <= 25%
   _mulle_atomic_pointer_increment( &cache->n);

   assert( ! entry->key.uniqueid);
   _mulle_atomic_pointer_write( &entry->key.pointer, (void *) (uintptr_t) uniqueid);

   return( entry);
}


struct _mulle_objc_cacheentry   *_mulle_objc_cache_add_functionpointer_entry( struct _mulle_objc_cache *cache,
                                                              mulle_functionpointer_t pointer,
                                                              mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;

   assert( cache);
   assert( pointer);
   assert( uniqueid != MULLE_OBJC_NO_UNIQUEID && uniqueid != MULLE_OBJC_INVALID_UNIQUEID);

   //
   // entries pointer never changes in cache..
   //
   offset = _mulle_objc_cache_find_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   //
   // We expect this entry to be 0,0 and we write 0,x first.
   // As 0,x it will still be ignored by other readers. if successful we put
   // in the methodid and increment ->n.
   // There is a time when the cache has actually n entries, but it's count
   // is < n. that's not fatal but one needs to know this
   //
   if( ! _mulle_atomic_functionpointer_compare_and_swap( &entry->value.functionpointer, pointer, NULL))
   {
      //
      // implementation set by someone else...
      // if that guy is done writing the uniqueid and it's ours, then fine!
      // #1#
      if( _mulle_atomic_pointer_read( &entry->key.pointer) == (void *) (uintptr_t) uniqueid)
         return( entry);

      return( NULL);
   }

   // increment first to keep cach fill <= 25%
   _mulle_atomic_pointer_increment( &cache->n);

   assert( ! entry->key.uniqueid);
   _mulle_atomic_pointer_write( &entry->key.pointer, (void *) (uintptr_t) uniqueid);

   return( entry);
}

// #1#
// the atomicity of this.
//
// * the call does not read it atomically
// * this could fail if 32 bit is written in 2 16 bit words and the parts of
//   the selectors of either word are entirely zero
// * mitigation: ensure that at least one bit is set in either word of the
//   selector. We know the 32 bit value prior contents have to be zero. Either
//   16 bit write could accidentally produce a valid selector, that is being
//   searched. But if the requirement is, that a selector must have at least
//   one bit set in both 16 bit words, a single 16 bit write can not produce
//   a valid selector. The worst that can happen then, is that duplicate cache
//   entries could be produced.
//
