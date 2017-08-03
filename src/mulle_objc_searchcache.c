//
//  mulle_objc_searchcache.c
//  mulle-objc-runtime
//
//  Created by Nat! on 17/08/03.
//  Copyright (c) 2017 Nat! - Mulle kybernetiK.
//  Copyright (c) 2017 Codeon GmbH.
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
#include "mulle_objc_searchcache.h"

#include <mulle_thread/mulle_thread.h>
#include <assert.h>
#include <string.h>


#pragma mark - methodcache

struct _mulle_objc_searchcache   *
    mulle_objc_searchcache_new( mulle_objc_searchcache_uint_t size,
                                struct mulle_allocator *allocator)
{
   struct _mulle_objc_searchcache  *cache;
   
   assert( allocator);
   // must be maskable
   assert( ! (sizeof( struct _mulle_objc_searchcacheentry) & (sizeof( struct _mulle_objc_searchcacheentry) - 1)));
   
   if( size < MULLE_OBJC_MIN_SEARCHCACHE_SIZE)
      size = MULLE_OBJC_MIN_SEARCHCACHE_SIZE;
   
   assert( ! (size & (size - 1)));          // check for tumeni bits
   
   cache = _mulle_allocator_calloc( allocator, 1, sizeof( struct _mulle_objc_searchcache) + sizeof( struct _mulle_objc_searchcacheentry) * (size - 1));
   
   cache->size = size;
   // myhardworkbythesewordsguardedpleasedontsteal © Nat!
   cache->mask = (size - 1) * sizeof( struct _mulle_objc_searchcacheentry);    // preshift
   
   return( cache);
}


void   *
   _mulle_objc_searchcache_lookup_pointer( struct _mulle_objc_searchcache *cache,
                                           struct _mulle_objc_searchargumentscachable *args)
{
   struct _mulle_objc_searchcacheentry   *entries;
   struct _mulle_objc_searchcacheentry   *entry;
   mulle_objc_searchcache_uint_t         offset;
   mulle_objc_searchcache_uint_t         mask;
   
   assert( cache);
   assert( args);
   assert( _mulle_objc_is_cacheablesearchmode( args->mode));
   
   entries = cache->entries;
   mask    = cache->mask;
   
   offset  = (mulle_objc_searchcache_uint_t) _mulle_objc_searchargumentscachable_hash( args);
   for(;;)
   {
      offset = (mulle_objc_searchcache_uint_t) offset & mask;
      entry = (void *) &((char *) entries)[ offset];
      if( _mulle_objc_searchargumentscachable_equals( &entry->key, args))
         return( _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));
      
      if( ! _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer))
         return( NULL);
      
      offset += sizeof( struct _mulle_objc_searchcacheentry);
   }
}


mulle_functionpointer_t
   _mulle_objc_searchcache_lookup_functionpointer( struct _mulle_objc_searchcache *cache,
                                                   struct _mulle_objc_searchargumentscachable *args)
{
   struct _mulle_objc_searchcacheentry   *entries;
   struct _mulle_objc_searchcacheentry   *entry;
   mulle_objc_searchcache_uint_t         offset;
   mulle_objc_searchcache_uint_t         mask;
   
   assert( cache);
   assert( args);
   assert( _mulle_objc_is_cacheablesearchmode( args->mode));
   
   entries = cache->entries;
   mask    = cache->mask;
   
   offset  = (mulle_objc_searchcache_uint_t) _mulle_objc_searchargumentscachable_hash( args);
   for(;;)
   {
      offset = (mulle_objc_searchcache_uint_t) offset & mask;
      entry = (void *) &((char *) entries)[ offset];
      if( _mulle_objc_searchargumentscachable_equals( &entry->key, args))
         return( _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer));
      
      if( ! _mulle_atomic_functionpointer_nonatomic_read( &entry->value.functionpointer))
         return( NULL);
      
      offset += sizeof( struct _mulle_objc_searchcacheentry);
   }
}


int   _mulle_objc_searchcache_find_entryindex( struct _mulle_objc_searchcache *cache,
                                               struct _mulle_objc_searchargumentscachable *args)
{
   int                                   index;
   struct _mulle_objc_searchcacheentry   *entry;
   struct _mulle_objc_searchcacheentry   *entries;
   mulle_objc_searchcache_uint_t         mask;
   mulle_objc_searchcache_uint_t         offset;
   
   assert( cache);
   assert( args);
   assert( _mulle_objc_is_cacheablesearchmode( args->mode));
   
   entries = cache->entries;
   mask    = cache->mask;
   index   = 0;
   
   offset  = (mulle_objc_searchcache_uint_t) _mulle_objc_searchargumentscachable_hash( args);
   for(;;)
   {
      offset = offset & mask;
      entry  = (void *) &((char *) entries)[ offset];
      if( _mulle_objc_searchargumentscachable_equals( &entry->key, args))
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
      
      offset += sizeof( struct _mulle_objc_searchcacheentry);
      ++index;
   }
}


//
// find a slot, where either the uniqueid matches, or where the slot is free
// (at least at the moment)
//
mulle_objc_searchcache_uint_t
   _mulle_objc_searchcache_find_entryoffset( struct _mulle_objc_searchcache *cache,
                                             struct _mulle_objc_searchargumentscachable *args)
{
   struct _mulle_objc_searchcacheentry   *entries;
   struct _mulle_objc_searchcacheentry   *entry;
   mulle_objc_searchcache_uint_t         offset;
   mulle_objc_searchcache_uint_t         mask;
   
   assert( cache);
   assert( args);
   assert( _mulle_objc_is_cacheablesearchmode( args->mode));
   
   entries = cache->entries;
   mask    = cache->mask;
   
   offset  = (mulle_objc_searchcache_uint_t) _mulle_objc_searchargumentscachable_hash( args);
   for(;;)
   {
      offset = offset & mask;
      entry  = (void *) &((char *) entries)[ offset];
      if( _mulle_objc_searchargumentscachable_equals( &entry->key, args))
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
      
      
      offset += sizeof( struct _mulle_objc_searchcacheentry);
   }
}


unsigned int
   mulle_objc_searchcache_calculate_fillpercentage( struct _mulle_objc_searchcache *cache)
{
   intptr_t   n;
   
   if( ! cache || ! cache->size)
      return( 0);
   
   n = (intptr_t) _mulle_atomic_pointer_read( &cache->n);
   return( (unsigned int)  (n * 100 + (cache->size / 2)) / cache->size);
}


unsigned int
    mulle_objc_searchcache_calculate_hitpercentage(
                                     struct _mulle_objc_searchcache *cache,
                                     unsigned int *percentages,
                                     unsigned int size)
{
   unsigned int                          i, n;
   unsigned int                          total;
   struct _mulle_objc_searchcacheentry   *p;
   struct _mulle_objc_searchcacheentry   *sentinel;
   
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
      if( p->key.mode)
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
struct _mulle_objc_searchcacheentry   *
   _mulle_objc_searchcache_inactivecache_add_functionpointer_entry(
                              struct _mulle_objc_searchcache *cache,
                              mulle_functionpointer_t pointer,
                              struct _mulle_objc_searchargumentscachable *args)
{
   struct _mulle_objc_searchcacheentry   *entry;
   mulle_objc_uniqueid_t                 offset;
   
   assert( _mulle_objc_searchcache_should_grow( cache));
   
   offset = _mulle_objc_searchcache_find_entryoffset( cache, args);
   entry  = (void *) &((char *) cache->entries)[ offset];
   
   assert( ! entry->key.mode);  // if it's not, it's not inactive!
   assert( ! _mulle_atomic_pointer_nonatomic_read( &entry->value.pointer));
   
   entry->key = *args;
   _mulle_atomic_functionpointer_nonatomic_write( &entry->value.functionpointer, pointer);
   
   _mulle_atomic_pointer_increment( &cache->n);
   
   return( entry);
}


struct _mulle_objc_searchcacheentry   *
   _mulle_objc_searchcache_add_functionpointer_entry( struct _mulle_objc_searchcache *cache,
                                                      mulle_functionpointer_t pointer,
                                                      struct _mulle_objc_searchargumentscachable *args)
{
   struct _mulle_objc_searchcacheentry   *entry;
   mulle_objc_uniqueid_t           offset;
   
   assert( cache);
   assert( args);
   assert( pointer);
   assert( _mulle_objc_is_cacheablesearchmode( args->mode));
   
   //
   // entries pointer never changes in cache..
   //
   offset = _mulle_objc_searchcache_find_entryoffset( cache, args);
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
      // if that guy is done writing the args check if it matches ours, then fine!
      //
      if( _mulle_atomic_pointer_read( &entry->key.pointer) == (void *) args->pointer)
      {
         // since the memory barrier was set up by the writer
         // the whole entry must be consistent (and immutable) now
         if( _mulle_objc_searchargumentscachable_equals( &entry->key, args))
            return( entry);
      }
      
      return( NULL);
   }
   
   // increment first to keep cach fill <= 25%
   _mulle_atomic_pointer_increment( &cache->n);
   
   assert( ! entry->key.pointer);

   entry->key.categoryid = args->categoryid;
   entry->key.classid    = args->classid;
   entry->key.methodid   = args->methodid;
   
   // assume key.pointer minimally writes mode
   // at this point args->key.mode must still be zero, so any search will
   // not be able to match. Now ensure this incomplete stuff is flushed
   mulle_atomic_memory_barrier();
   
   // now we atomically write down the pointer which sets
   // entry->key.methodid
   _mulle_atomic_pointer_write( &entry->key.pointer, (void *) args->pointer);
   
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
