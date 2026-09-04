//
//  mulle-objc-universe-global.c
//  mulle-objc-runtime
//
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//
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
#include "mulle-objc-universe-global.h"

#include "include-private.h"


//
// Globals I can't get rid off
// They must be initialized because of windows
//

long   __mulle_objc_personality_v0 = 1848;   // no idea what this is used for

//
// the mulle_objc_defaultuniverse is special and has universeid 0
//
MULLE_OBJC_RUNTIME_GLOBAL_VAR
struct _mulle_objc_universe   mulle_objc_defaultuniverse =
{
   .version = (void *) mulle_objc_universe_is_uninitialized
};


//
// other universes are stored here
// the mulle_objc_defaultuniverse is not part of it
//
// The tables are initialized lazily. The init state is separate from the
// hashtable itself so readers never observe a partially initialized table.
MULLE_OBJC_RUNTIME_GLOBAL_VAR
struct mulle_objc_universe_init_wrapper_for_windows   mulle_objc_universetable =
{
   .initvalue = 1848
};


struct _mulle_objc_universe  *
   __mulle_objc_global_register_universe( mulle_objc_universeid_t universeid,
                                          struct _mulle_objc_universe *universe)
{
   struct mulle_concurrent_hashtable      *table;
   void                                   *old;

   assert( universeid != MULLE_OBJC_DEFAULTUNIVERSEID);
   _mulle_objc_global_ensure_universe_table();
   table = _mulle_objc_global_get_universe_table();

   old = NULL;
   if( mulle_concurrent_hashtable_register( table,
                                            _mulle_objc_global_get_universe_hash( universeid),
                                            universe,
                                            &old))
      return( NULL);
   return( old);
}



void
   __mulle_objc_global_unregister_universe( mulle_objc_universeid_t universeid,
                                            struct _mulle_objc_universe *universe)
{
   struct mulle_concurrent_hashtable   *table;

   assert( universeid != MULLE_OBJC_DEFAULTUNIVERSEID);
   _mulle_objc_global_ensure_universe_table();
   table = _mulle_objc_global_get_universe_table();
   mulle_concurrent_hashtable_remove( table,
                                      _mulle_objc_global_get_universe_hash( universeid),
                                      universe);
}


size_t  __mulle_objc_global_get_alluniverses( struct _mulle_objc_universe **buf,
                                              size_t n)
{
   struct _mulle_objc_universe                  *universe;
   struct _mulle_objc_universe                  **p;
   struct _mulle_objc_universe                  **sentinel;
   struct mulle_concurrent_hashtableenumerator  rover;
   intptr_t                                     hash;
   size_t                                       i;
   size_t                                       j;
   int                                          rval;

   if( ! buf && n)
      return( -1);

   _mulle_objc_global_ensure_universe_table();

retry:
   p        = buf;
   sentinel = &p[ n];
   i        = 0;
   universe = &mulle_objc_defaultuniverse;
   if( _mulle_objc_universe_is_initialized( universe))
   {
      if( p < sentinel)
         *p++ = universe;
      ++i;
   }

   j     = 0;
   rover = mulle_concurrent_hashtable_enumerate( &mulle_objc_universetable.map);
   for(;;)
   {
      rval = mulle_concurrent_hashtableenumerator_next( &rover,
                                                        &hash,
                                                        (void **) &universe);
      if( rval == 1)
      {
         if( p < sentinel)
            *p++ = universe;
         ++j;
         continue;
      }
      break;
   }
   mulle_concurrent_hashtableenumerator_done( &rover);
   if( rval == ECANCELED)
      goto retry;

   if( buf)
      memset( p, 0, (sentinel - p) * sizeof( struct _mulle_objc_universe *));

   return( i + j);
}


static void
   _mulle_objc_global_reset_universe_table( struct mulle_concurrent_hashtable *table,
                                            mulle_atomic_pointer_t *initvalue)
{
   if( _mulle_atomic_pointer_read( initvalue) != (void *) 2)
      return;

   assert( mulle_concurrent_hashtable_count( table) == 0);
   mulle_concurrent_hashtable_done( table);
   _mulle_atomic_pointer_nonatomic_write( initvalue, (void *) 0);
}


void   mulle_objc_global_reset_universetable( void)
{
   _mulle_objc_global_reset_universe_table( &mulle_objc_universetable.map,
                                            &mulle_objc_universetable.mapinitvalue);
}

