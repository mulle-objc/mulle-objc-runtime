//
//  mulle_objc_universe_global.c
//  mulle-objc
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
//
#ifndef mulle_objc_universe_global_h__
#define mulle_objc_universe_global_h__

#include "mulle_objc_universe_struct.h"

#include <mulle_c11/mulle_c11.h>


#ifndef MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
# define MULLE_OBJC_RUNTIME_EXTERN_GLOBAL    MULLE_C_EXTERN_GLOBAL
#endif

#define MULLE_OBJC_RUNTIME_GLOBAL            MULLE_C_GLOBAL


#ifdef __MULLE_OBJC_NO_TRT__

MULLE_C_CONST_NON_NULL_RETURN  // always returns same value (in same thread)
static inline struct _mulle_objc_universe  *mulle_objc_get_global_universe( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL 
      struct _mulle_objc_universe   mulle_objc_global_universe;

   assert( ! _mulle_objc_universe_is_uninitialized( &mulle_objc_global_universe) && "universe not initialized yet");
   return( &mulle_objc_global_universe);
}


// only __mulle_objc_get_universe should use this
static inline struct _mulle_objc_universe  *__mulle_objc_get_global_universe( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL 
      struct _mulle_objc_universe   mulle_objc_global_universe;

   return( &mulle_objc_global_universe);
}


static inline int _mulle_objc_is_global_universe( struct _mulle_objc_universe *universe)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL 
      struct _mulle_objc_universe   mulle_objc_global_universe;

   return( universe == &mulle_objc_global_universe);
}


#else


static inline int _mulle_objc_is_global_universe( struct _mulle_objc_universe *universe)
{
   return( 0);
}

#endif


//
// this is used by NSAutoreleasePool only
// to lazily setup a universe thread if missing
//
static inline int   mulle_objc_thread_key_is_intitialized( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL 
      mulle_thread_tss_t   mulle_objc_thread_key;

   return( mulle_objc_thread_key != (mulle_thread_tss_t) -1);
}


static inline struct _mulle_objc_threadconfig  *_mulle_objc_get_threadconfig( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL 
      mulle_thread_tss_t   mulle_objc_thread_key;
   struct _mulle_objc_threadconfig  *config;

   /* if you crash here [^1] */
   assert( mulle_objc_thread_key_is_intitialized());
   config = mulle_thread_tss_get( mulle_objc_thread_key);
   return( config);
}


MULLE_C_CONST_NON_NULL_RETURN  // always returns same value (in same thread)
static inline struct _mulle_objc_threadconfig  *mulle_objc_get_threadconfig( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL 
      mulle_thread_tss_t   mulle_objc_thread_key;
   struct _mulle_objc_threadconfig  *config;

   /* if you crash here [^1] */
   assert( mulle_objc_thread_key_is_intitialized());
   config = mulle_thread_tss_get( mulle_objc_thread_key);
   assert( config);
   return( config);
}

#endif
