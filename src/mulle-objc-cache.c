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
#include "mulle-objc-cache.h"

#include "include-private.h"
#include <assert.h>
#include <string.h>


// no good for the various leak checkers
#ifdef MULLE_TEST
//# define MULLE_TEST_STATIC_CACHE_DISCARDS
#endif
#pragma mark - cache


//
// cache malloc/frees should not disturb errno, so we preserve it
//
struct _mulle_objc_cache   *mulle_objc_cache_new( mulle_objc_cache_uint_t size,
                                                  struct mulle_allocator *allocator)
{
   struct _mulle_objc_cache  *cache;
   int                       preserve;
   size_t                    s_cache;

   assert( allocator);
   assert( ! (sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1)));

   if( size < MULLE_OBJC_MIN_CACHE_SIZE)
      size = MULLE_OBJC_MIN_CACHE_SIZE;

   assert( ! (size & (size - 1)));          // check for tumeni bits

   preserve = errno;
   // cache struct has room for one entry already
   s_cache  = sizeof( struct _mulle_objc_cache) + sizeof( struct _mulle_objc_cacheentry) * (size - 1);
   cache    = _mulle_allocator_calloc( allocator, 1, s_cache);
   errno    = preserve;

   _mulle_objc_cache_init( cache, size);

   return( cache);
}


#ifdef MULLE_TEST_STATIC_CACHE_DISCARDS

struct _mulle_objc_cache   *mulle_objc_discarded_caches[ 0x8000];
mulle_atomic_pointer_t     n_mulle_objc_discarded_caches;

static void   discard_cache( struct _mulle_objc_cache *cache)
{
   intptr_t   n;

   n = (intptr_t) _mulle_atomic_pointer_increment( &n_mulle_objc_discarded_caches);
   if( ! n || n >= sizeof( mulle_objc_discarded_caches) / sizeof( void *))
      abort();
   mulle_objc_discarded_caches[ n - 1] = cache;
}
#endif


void   _mulle_objc_cache_free( struct _mulle_objc_cache *cache,
                               struct mulle_allocator *allocator)
{
   int   preserve;

   assert( allocator);

   preserve = errno;
#ifdef MULLE_TEST_STATIC_CACHE_DISCARDS
   discard_cache( cache);
#else
   _mulle_allocator_free( allocator, cache);
#endif
   errno    = preserve;
}


void   _mulle_objc_cache_abafree( struct _mulle_objc_cache *cache,
                                  struct mulle_allocator *allocator)
{
   int   preserve;

   assert( allocator);

   preserve = errno;
#ifdef MULLE_TEST_STATIC_CACHE_DISCARDS
   discard_cache( cache);
#else
   _mulle_allocator_abafree( allocator, cache);
#endif

   errno    = preserve;
}


// used by benchmark code
int   _mulle_objc_cache_probe_entryindex( struct _mulle_objc_cache *cache,
                                         mulle_objc_uniqueid_t uniqueid)
{
   int                             index;
   struct _mulle_objc_cacheentry   *entry;
   struct _mulle_objc_cacheentry   *entries;
   mulle_objc_cache_uint_t         mask;
   mulle_objc_cache_uint_t         offset;

   assert( cache);
   assert( mulle_objc_uniqueid_is_sane( uniqueid));

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
         if( ! _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer))
            return( -1);

      offset += sizeof( struct _mulle_objc_cacheentry);
      ++index;
   }
}


//
// find a slot, where either the uniqueid matches, or where the slot is free
// (at least at the moment)
//
mulle_objc_cache_uint_t
   _mulle_objc_cache_probe_entryoffset( struct _mulle_objc_cache *cache,
                                       mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entries;
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_cache_uint_t         offset;
   mulle_objc_cache_uint_t         mask;

   assert( cache);
   assert( mulle_objc_uniqueid_is_sane( uniqueid));

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
         if( ! _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer))
            return( offset);

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


int   _mulle_objc_cache_should_grow( struct _mulle_objc_cache *cache,
                                    unsigned int fillrate)
{
   size_t   used;
   size_t   size;

   used = (size_t) _mulle_atomic_pointer_read( &cache->n);
   size = cache->size;

   if( ! fillrate)
      return( used * 3 >= size);  // have cache filled to a third

   assert( fillrate >= 1 && fillrate <= 90);

   return( used * 100 >= size * fillrate);
}


unsigned int   mulle_objc_cache_calculate_fillpercentage( struct _mulle_objc_cache *cache)
{
   intptr_t   n;

   if( ! cache || ! cache->size)
      return( 0);

   n = (intptr_t) _mulle_atomic_pointer_read( &cache->n);
   return( (unsigned int) (n * 100 + (cache->size / 2)) / cache->size);
}


unsigned int   mulle_objc_cache_calculate_hits( struct _mulle_objc_cache *cache,
                                                unsigned int counts[ 4])
{
   unsigned int                     total;
   mulle_objc_cache_uint_t          offset;
   mulle_objc_cache_uint_t          mask;
   mulle_objc_cache_uint_t          steps;
   mulle_objc_cache_uint_t          expected;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;

   memset( counts, 0, sizeof( unsigned int) * 4);

   if( ! cache || ! cache->size)
      return( 0);

   if( ! _mulle_atomic_pointer_read( &cache->n))
      return( 0);

   p        = cache->entries;
   sentinel = &p[ cache->size];
   mask     = cache->mask;
   total    = 0;
   offset   = 0;

   for( ; p < sentinel; p++)
   {
      if( p->key.uniqueid)
      {
         expected = (p->key.uniqueid & mask);
         if( expected > offset) // wrap around
         {
            steps  = cache->size * sizeof( struct _mulle_objc_cacheentry) - expected;
            steps += offset;
         }
         else
            steps = (offset - expected);
         steps /= sizeof( struct _mulle_objc_cacheentry);
         if( steps > 3)
            steps = 3;
         counts[ steps]++;
         ++total;
      }

      offset += sizeof( struct _mulle_objc_cacheentry);
   }

   return( total);
}


unsigned int   mulle_objc_cache_calculate_percentage( struct _mulle_objc_cache *cache,
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

//
// this only works for a cache, that isn't active in the universe yet and that
// has enough space (!)
// If an entry is already occupied, this does nothing (properly observing
// inheritance therefore)
//
struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_pointer_inactive( struct _mulle_objc_cache *cache,
                                            void *pointer,
                                            mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;

   offset = _mulle_objc_cache_probe_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   // if we have a
   if( _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer))
      return( entry);

   entry->key.uniqueid = uniqueid;
   _mulle_atomic_pointer_write_nonatomic( &entry->value.pointer, pointer);

   _mulle_atomic_pointer_increment( &cache->n);
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
   entry->thread = 0;
#endif
   return( entry);
}


struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_functionpointer_inactive( struct _mulle_objc_cache *cache,
                                                   mulle_functionpointer_t pointer,
                                                   mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;

   offset = _mulle_objc_cache_probe_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   // can happen if we added the method already (due to overloading, we might
   // want to try the same uniqueid twice, but the first will be the best)
   if( entry->key.uniqueid)
   {
      assert( entry->key.uniqueid == uniqueid);
      return( entry);
   }
   assert( ! _mulle_atomic_pointer_read_nonatomic( &entry->value.pointer));

   entry->key.uniqueid = uniqueid;
   _mulle_atomic_functionpointer_nonatomic_write( &entry->value.functionpointer, pointer);

   _mulle_atomic_pointer_increment( &cache->n);
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
   entry->thread = 0;
#endif
   return( entry);
}



struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_pointer_entry( struct _mulle_objc_cache *cache,
                                        void *pointer,
                                        mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;

   assert( cache);
   assert( pointer);
   assert( mulle_objc_uniqueid_is_sane( uniqueid));

   //
   // entries pointer never changes in cache..
   //
   offset = _mulle_objc_cache_probe_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   //
   // We expect this entry to be 0,0 and we write 0,x first.
   // As 0,x it will still be ignored by other readers. if successful we put
   // in the methodid and increment ->n.
   // There is a time when the cache has actually n entries, but it's count
   // is < n. that's not fatal but one needs to know this
   //
   if( ! _mulle_atomic_pointer_cas( &entry->value.pointer, pointer, NULL))
   {
      //
      // implementation set by someone else...
      // if that guy is done writing the uniqueid and it's ours, then fine!
      // #1#
      if( _mulle_atomic_pointer_read( &entry->key.pointer) == (void *) (uintptr_t) uniqueid)
      {
         assert( _mulle_atomic_pointer_read( &entry->value.pointer) == pointer);
         return( entry);
      }
      return( NULL);
   }

   // increment first to keep cach fill <= 25%
   _mulle_atomic_pointer_increment( &cache->n);

   assert( ! entry->key.uniqueid);
   _mulle_atomic_pointer_write( &entry->key.pointer, (void *) (uintptr_t) uniqueid);
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
   entry->thread = mulle_thread_self();
#endif
   return( entry);
}


//
// this will return an entry, except if for some reason the CAS didn't work
// in which case you should retry
//
struct _mulle_objc_cacheentry   *
   _mulle_objc_cache_add_functionpointer_entry( struct _mulle_objc_cache *cache,
                                                mulle_functionpointer_t pointer,
                                                mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_cacheentry   *entry;
   mulle_objc_uniqueid_t           offset;

   assert( cache);
   assert( pointer);
   assert( mulle_objc_uniqueid_is_sane( uniqueid));

   //
   // entries pointer never changes in cache..
   //
   offset = _mulle_objc_cache_probe_entryoffset( cache, uniqueid);
   entry  = (void *) &((char *) cache->entries)[ offset];

   //
   // We expect this entry to be 0,0 and we write 0,x first.
   // As 0,x it will still be ignored by other readers. If successful, we put
   // in the methodid and increment ->n.
   // There is a time when the cache has actually n entries, but it's count
   // is < n. that's not fatal but one needs to know this.
   //
   if( ! _mulle_atomic_functionpointer_cas( &entry->value.functionpointer, pointer, NULL))
   {
      //
      // implementation set by someone else...
      // if that guy is done writing the uniqueid and it's ours, then fine!
      // #1#
      if( _mulle_atomic_pointer_read( &entry->key.pointer) == (void *) (uintptr_t) uniqueid)
      {
         assert( _mulle_atomic_functionpointer_read( &entry->value.functionpointer) == pointer);
         return( entry);
      }

      // So we failed, because someone else added something. bail and let
      // the caller retry
      return( NULL);
   }

   // increment first to keep cache fill <= 25%
   _mulle_atomic_pointer_increment( &cache->n);

   assert( ! entry->key.uniqueid);
   _mulle_atomic_pointer_write( &entry->key.pointer, (void *) (uintptr_t) uniqueid);
#ifdef MULLE_OBJC_CACHEENTRY_REMEMBERS_THREAD_CLASS
   entry->thread = mulle_thread_self();
#endif
   return( entry);
}


int   _mulle_objc_cachepivot_swap( struct _mulle_objc_cachepivot *pivot,
                                   struct _mulle_objc_cache *cache,
                                   struct _mulle_objc_cache *old_cache,
                                   struct mulle_allocator *allocator)
{
   //
   // an initial_impcache ? this is getting called too early
   // an empty_cache ? this is getting called wrong
   //
   if( _mulle_objc_cachepivot_cas_entries( pivot,
                                           cache->entries,
                                           old_cache ? old_cache->entries : NULL))
   {
      // cas failed, so get rid of this and punt
      _mulle_objc_cache_free( cache, allocator); // sic, can be unsafe deleted now
      return( -1);
   }


   _mulle_objc_cache_abafree( old_cache, allocator);
   return( 0);
}


struct _mulle_objc_cache *
   _mulle_objc_cache_grow_with_strategy( struct _mulle_objc_cache  *old_cache,
                                         enum mulle_objc_cachesizing_t strategy,
                                         struct mulle_allocator *allocator)
{
   struct _mulle_objc_cache  *cache;
   mulle_objc_cache_uint_t   new_size;

   // a new beginning.. let it be filled anew
   // could ask the universe here what to do as new size

   new_size  = _mulle_objc_cache_get_resize( old_cache, strategy);
   cache     = mulle_objc_cache_new( new_size, allocator);

   return( cache);
}



// uniqueid can be a methodid or superid!
struct _mulle_objc_cacheentry *
    _mulle_objc_cachepivot_fill_functionpointer( struct _mulle_objc_cachepivot *pivot,
                                                 mulle_functionpointer_t imp,
                                                 mulle_objc_uniqueid_t uniqueid,
                                                 unsigned int fillrate,
                                                 struct mulle_allocator *allocator)
{
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_cache        *old_cache;
   struct _mulle_objc_cacheentry   *entry;

   assert( pivot);
   assert( imp);

   //
   // try to get most up to date value
   //
   for(;;)
   {
      cache = _mulle_objc_cachepivot_get_cache_atomic( pivot);
      if( _mulle_objc_cache_should_grow( cache, fillrate))
      {
         old_cache = cache;
         cache     = _mulle_objc_cache_grow_with_strategy( old_cache,
                                                           MULLE_OBJC_CACHESIZE_GROW,
                                                           allocator);

         // doesn't really matter, if this fails or succeeds we just try
         // again
         _mulle_objc_cachepivot_swap( pivot, cache, old_cache, allocator);
         continue;
      }

      entry = _mulle_objc_cache_add_functionpointer_entry( cache,
                                                           (mulle_functionpointer_t) imp,
                                                           uniqueid);
      if( entry)
         break;
   }

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
