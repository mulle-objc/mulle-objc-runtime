//
//  mulle_objc_universe_global.c
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
#ifndef mulle_objc_universe_global_h__
#define mulle_objc_universe_global_h__

#include "mulle-objc-universe-struct.h"

#include "include.h"


#ifndef MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
# define MULLE_OBJC_RUNTIME_EXTERN_GLOBAL    MULLE_C_EXTERN_GLOBAL
#endif


#pragma mark - default universe (0)

// always returns same value (in same thread)
MULLE_C_CONST_NON_NULL_RETURN static inline struct _mulle_objc_universe *
   mulle_objc_global_get_defaultuniverse( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
      struct _mulle_objc_universe   mulle_objc_defaultuniverse;

   assert( ! _mulle_objc_universe_is_uninitialized( &mulle_objc_defaultuniverse) \
               && "The universe not initialized yet.\nIs a C function - possibly __attribute__((constructor)) - calling Objective-C prematurely? ");
   return( &mulle_objc_defaultuniverse);
}

// only __mulle_objc_global_get_universe should use this
MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_universe  *
   __mulle_objc_global_get_defaultuniverse( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
      struct _mulle_objc_universe   mulle_objc_defaultuniverse;

   return( &mulle_objc_defaultuniverse);
}

#pragma mark - named universes

// only __mulle_objc_global_get_universe should use this
static inline struct _mulle_objc_universe  *
   __mulle_objc_global_lookup_universe( mulle_objc_universeid_t universeid)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
      struct mulle_concurrent_hashmap   mulle_objc_universetable;

   return( mulle_concurrent_hashmap_lookup( &mulle_objc_universetable, universeid));
}


static inline struct _mulle_objc_universe  *
   mulle_objc_global_lookup_universe( mulle_objc_universeid_t universeid)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
      struct mulle_concurrent_hashmap   mulle_objc_universetable;
   struct _mulle_objc_universe  *universe;

   assert( universeid && "lookup would fail for default universe (use mulle_objc_global_get_universe)");

   universe = mulle_concurrent_hashmap_lookup( &mulle_objc_universetable, universeid);
   assert( ! universe || ! _mulle_objc_universe_is_uninitialized( universe) \
               && "universe not initialized yet");
   return( universe);
}


struct _mulle_objc_universe  *
   __mulle_objc_global_register_universe( mulle_objc_universeid_t universeid,
                                          struct _mulle_objc_universe *universe);


void
   __mulle_objc_global_unregister_universe( mulle_objc_universeid_t universeid,
                                            struct _mulle_objc_universe *universe);

MULLE_C_ALWAYS_INLINE static inline int
   _mulle_objc_universe_is_default( struct _mulle_objc_universe *universe)
{
   return( universe->universeid == MULLE_OBJC_DEFAULTUNIVERSEID);
}


//
// only call this before exiting main, if you use valgrind and dislike the
// leak, when using universes other than MULLE_OBJC_DEFAULTUNIVERSEID
// Remove the universe before calling this!
//
void   mulle_objc_global_reset_universetable( void);


MULLE_OBJC_RUNTIME_EXTERN_GLOBAL long   __mulle_objc_personality_v0;   // no idea what this is used for

size_t  __mulle_objc_global_get_alluniverses( struct _mulle_objc_universe **p,
                                              size_t n);

#endif
