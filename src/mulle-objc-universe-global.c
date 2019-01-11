//
//  mulle_objc_universe_global.h
//  mulle-objc-runtime
//
//  Created by Nat! on 07.09.16.
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
#include "mulle-objc-universe-struct.h"

#include "include-private.h"



#define MULLE_OBJC_RUNTIME_GLOBAL   MULLE_C_GLOBAL


//
// Globals I can't get rid off
// They must be initialized because of windows
//

MULLE_OBJC_RUNTIME_GLOBAL long   __mulle_objc_personality_v0 = 1848;   // no idea what this is used for

//
// the mulle_objc_defaultuniverse is special and has universeid 0
//
MULLE_OBJC_RUNTIME_GLOBAL struct _mulle_objc_universe
   mulle_objc_defaultuniverse =
{
   .version = (void *) mulle_objc_universe_is_uninitialized
};


struct init_wrapper_for_windows
{
	struct mulle_concurrent_hashmap   map;
	int                               initvalue;
};


//
// other universes are stored here
// the mulle_objc_defaultuniverse is not part of it
//
static const struct _mulle_concurrent_hashmapstorage	empty_hashmapstorage =
{
   (void *) -1,
   0,
   { { MULLE_CONCURRENT_NO_HASH, NULL } }
};


MULLE_OBJC_RUNTIME_GLOBAL struct init_wrapper_for_windows
	mulle_objc_universetable =
{
	{
	  (struct _mulle_concurrent_hashmapstorage *) &empty_hashmapstorage,
	  (struct _mulle_concurrent_hashmapstorage *) &empty_hashmapstorage
	},
	1848
};


struct _mulle_objc_universe  *
   __mulle_objc_global_register_universe( mulle_objc_universeid_t universeid,
                                          struct _mulle_objc_universe *universe)
{
   struct mulle_allocator   *allocator;

   assert( universeid != MULLE_OBJC_DEFAULTUNIVERSEID);
   allocator = _mulle_atomic_pointer_read( &mulle_objc_universetable.map.allocator);
   if( ! allocator)
   {
      //
      // use stdlib allocator for this, since we leak here
      //
	   allocator = &mulle_stdlib_allocator;
	   assert( allocator->abafree && allocator->abafree != (int (*)()) abort);

	   _mulle_atomic_pointer_cas( &mulle_objc_universetable.map.allocator, allocator, NULL);
   }
   return( mulle_concurrent_hashmap_register( &mulle_objc_universetable.map,
                                              universeid,
                                              universe));
}



void
   __mulle_objc_global_unregister_universe( mulle_objc_universeid_t universeid,
                                            struct _mulle_objc_universe *universe)
{
   assert( universeid != MULLE_OBJC_DEFAULTUNIVERSEID);
   mulle_concurrent_hashmap_remove( &mulle_objc_universetable.map,
                                    universeid,
                                    universe);
}



struct _mulle_objc_universe  *__mulle_objc_global_getany_universe( void)
{
   struct _mulle_objc_universe                 *universe;
   struct mulle_concurrent_hashmapenumerator   rover;
   intptr_t                                    hash;
   int                                         rval;

   universe = &mulle_objc_defaultuniverse;
   if( _mulle_objc_universe_is_initialized( universe))
      return( universe);

   // look through alternate universes, we don't really know what
   // the debugger is inspecting

retry:
   universe = NULL;
   rover    = mulle_concurrent_hashmap_enumerate( &mulle_objc_universetable.map);
   rval     = mulle_concurrent_hashmapenumerator_next( &rover, &hash, (void **) &universe);
   mulle_concurrent_hashmapenumerator_done( &rover);
   if( rval == EBUSY)
      goto retry;

   return( universe);
}



size_t  __mulle_objc_global_get_alluniverses( struct _mulle_objc_universe **buf,
                                              size_t n)
{
   struct _mulle_objc_universe                  *universe;
   struct _mulle_objc_universe                  **p;
   struct _mulle_objc_universe                  **sentinel;
   struct mulle_concurrent_hashmapenumerator    rover;
   intptr_t                                     hash;
   size_t                                       i;
   size_t                                       j;
   int                                          rval;

   if( ! buf && n)
      return( -1);

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

   //
   // add alternate universes they are known to be initialized if present
   //
retry:
   j = 0;
   rover = mulle_concurrent_hashmap_enumerate( &mulle_objc_universetable.map);
   for(;;)
   {
      rval = mulle_concurrent_hashmapenumerator_next( &rover, &hash, (void **) &universe);
      if( rval == 1)
      {
         if( p < sentinel)
            *p++ = universe;
         ++j;
         continue;
      }
      break;
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
   if( rval == EBUSY)
      goto retry;

   if( buf)
      memset( p, 0, (sentinel - p) * sizeof( struct _mulle_objc_universe *));

   return( i + j);
}


void   mulle_objc_global_reset_universetable( void)
{
   struct _mulle_objc_universe                 *universe;
   struct mulle_concurrent_hashmapenumerator   rover;
   intptr_t                                    hash;
   int                                         rval;

   assert( mulle_concurrent_hashmap_count( &mulle_objc_universetable.map) == 0);
   mulle_concurrent_hashmap_done( &mulle_objc_universetable.map);

   mulle_objc_universetable.map.storage.storage      =
   mulle_objc_universetable.map.next_storage.storage =
      (struct _mulle_concurrent_hashmapstorage *) &empty_hashmapstorage;
}

