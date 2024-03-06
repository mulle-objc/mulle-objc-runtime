//
//  mulle_objc_class_universe.h
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
#ifndef mulle_objc_universe_class_h__
#define mulle_objc_universe_class_h__

#include "include.h"

#include "mulle-objc-uniqueid.h"
#include "mulle-objc-fastclasstable.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-object.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-version.h"


struct _mulle_objc_class;
struct _mulle_objc_universe;


/*
   @mulle-lldb@ needs to know about these:
## Universe / classid Functions

_mulle_objc_universe_inlinelookup_infraclass_nocache_nofast
_mulle_objc_universe_inlinelookup_infraclass
_mulle_objc_universe_lookup_infraclass_inline_nofail
_mulle_objc_universe_lookup_infraclass_nocache_nofail_nofast
_mulle_objc_universe_lookup_infraclass_nocache_nofast
_mulle_objc_universe_lookup_infraclass_nofail_nofast

## Universeid / classid Functions

mulle_objc_global_lookup_infraclass_inline_nofail
mulle_objc_global_lookup_infraclass_inline_nofail_nofast
mulle_objc_global_lookup_infraclass_nofail
mulle_objc_global_lookup_infraclass_nofail_nofast

## Object / universeid / classid Functions

mulle_objc_object_lookup_infraclass_inline_nofail
mulle_objc_object_lookup_infraclass_nofail
mulle_objc_object_lookup_infraclass_nofail_nofast
*/


MULLE_OBJC_RUNTIME_GLOBAL
int    mulle_objc_class_is_current_thread_registered( struct _mulle_objc_class *cls);


#pragma mark - class lookup, uncached, no callback

//
// this is useful to assert that a class is modifiable, as its not registered
// yet. (therefore it doesn't forward)
//
static inline struct _mulle_objc_infraclass *
   _mulle_objc_universe_inlinelookup_infraclass_nocache_nofast( struct _mulle_objc_universe *universe,
                                                                mulle_objc_classid_t classid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->classtable, classid));
}


#pragma mark - infraclass lookup, nocache but with callback

//
// this goes through the "slow" lookup table. Only internal code should
// use this method to not fill up the cache uselessy.
//
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_infraclass *
    _mulle_objc_universe_lookup_infraclass_nocache_nofast( struct _mulle_objc_universe *universe,
                                                           mulle_objc_classid_t classid);

//
// this looks through the uncached classtable, if nothing is found
// a delegate may provide a class "in time"
//
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_CONST_NONNULL_RETURN struct _mulle_objc_infraclass *
   _mulle_objc_universe_lookup_infraclass_nocache_nofail_nofast( struct _mulle_objc_universe *universe,
                                                                 mulle_objc_classid_t classid);


#pragma mark - infraclass lookup, cached but no fast lookup

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_infraclass  *
   _mulle_objc_universe_lookup_infraclass_nofast( struct _mulle_objc_universe *universe,
                                                  mulle_objc_classid_t classid);


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_CONST_NONNULL_RETURN struct _mulle_objc_infraclass *
    _mulle_objc_universe_lookup_infraclass_nofail_nofast( struct _mulle_objc_universe *universe,
                                                          mulle_objc_classid_t classid);


MULLE_C_NONNULL_RETURN static inline struct _mulle_objc_infraclass *
   mulle_objc_global_lookup_infraclass_inline_nofail_nofast( mulle_objc_universeid_t universeid,
                                                             mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( universeid);
   return( _mulle_objc_universe_lookup_infraclass_nofail_nofast( universe, classid));
}


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass *
   mulle_objc_global_lookup_infraclass_nofail_nofast( mulle_objc_universeid_t universeid,
                                                      mulle_objc_classid_t classid);


#pragma mark - infraclass lookup, fastclass lookup then cached

// if __MULLE_OBJC_FCS__ is disabled, this is the same as just cached

static inline struct _mulle_objc_infraclass  *
    _mulle_objc_universe_inlinelookup_infraclass( struct _mulle_objc_universe *universe,
                                                  mulle_objc_classid_t classid)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   struct _mulle_objc_infraclass  *
        _mulle_objc_universe_lookup_infraclass( struct _mulle_objc_universe *universe,
                                                mulle_objc_classid_t classid);

   assert( universe);
   assert( _mulle_objc_universe_is_messaging( universe)); // not is_initalized!!
   assert( classid);

#ifdef __MULLE_OBJC_FCS__
   {
      int                             index;
      struct _mulle_objc_infraclass   *infra;

      index = mulle_objc_get_fastclasstable_index( classid);
      if( index >= 0)
      {
         infra = mulle_objc_fastclasstable_get_infraclass( &universe->fastclasstable, index);
         return( infra);
      }
   }
#endif
   return( _mulle_objc_universe_lookup_infraclass_nofast( universe, classid));
}


static inline struct _mulle_objc_infraclass  *
    _mulle_objc_universe_lookup_infraclass_inline_nofail( struct _mulle_objc_universe *universe,
                                                          mulle_objc_classid_t classid)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   struct _mulle_objc_infraclass  *
        _mulle_objc_universe_lookup_infraclass( struct _mulle_objc_universe *universe,
                                                mulle_objc_classid_t classid);
   assert( universe);
   assert( _mulle_objc_universe_is_messaging( universe));
   assert( classid);

#ifdef __MULLE_OBJC_FCS__
   {
      int                             index;
      struct _mulle_objc_infraclass   *infra;

      index = mulle_objc_get_fastclasstable_index( classid);
      if( index >= 0)
      {
         infra = mulle_objc_fastclasstable_get_infraclass_nofail( &universe->fastclasstable, index);
         return( infra);
      }
   }
#endif
   return( _mulle_objc_universe_lookup_infraclass_nofail_nofast( universe, classid));
}


MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_infraclass  *
    _mulle_objc_universe_lookup_infraclass( struct _mulle_objc_universe *universe,
                                            mulle_objc_classid_t classid);

static inline struct _mulle_objc_infraclass  *
    mulle_objc_universe_lookup_infraclass( struct _mulle_objc_universe *universe,
                                           mulle_objc_classid_t classid)
{
   if( ! universe)
      return( NULL);
   return( _mulle_objc_universe_lookup_infraclass( universe, classid));
}


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass  *
    mulle_objc_universe_lookup_infraclass_nofail( struct _mulle_objc_universe *universe,
                                                  mulle_objc_classid_t classid);



MULLE_C_NONNULL_RETURN static inline struct _mulle_objc_infraclass *
   mulle_objc_global_lookup_infraclass_inline_nofail( mulle_objc_universeid_t universeid,
                                                      mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( universeid);
   return( _mulle_objc_universe_lookup_infraclass_inline_nofail( universe, classid));
}


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass *
   mulle_objc_global_lookup_infraclass_nofail( mulle_objc_universeid_t universeid,
                                               mulle_objc_classid_t classid);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_infraclass *
   mulle_objc_global_lookup_infraclass( mulle_objc_universeid_t universeid,
                                        mulle_objc_classid_t classid);


#pragma mark - infraclass lookup via object, fastclass lookup then cached


MULLE_C_NONNULL_RETURN static inline struct _mulle_objc_infraclass *
   mulle_objc_object_lookup_infraclass_inline_nofail( void *obj,
                                                      mulle_objc_universeid_t universeid,
                                                      mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = __mulle_objc_object_get_universe_nofail( obj, universeid);
   return( _mulle_objc_universe_lookup_infraclass_inline_nofail( universe, classid));
}


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass *
   mulle_objc_object_lookup_infraclass_nofail( void *obj,
                                               mulle_objc_universeid_t universeid,
                                               mulle_objc_classid_t classid);

MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_RETURN struct _mulle_objc_infraclass *
   mulle_objc_object_lookup_infraclass_nofail_nofast( void *obj,
                                                      mulle_objc_universeid_t universeid,
                                                      mulle_objc_classid_t classid);

// do not use, it's used by compat
MULLE_OBJC_RUNTIME_GLOBAL
void    _mulle_objc_universe_invalidate_classcache( struct _mulle_objc_universe *universe);


MULLE_C_NONNULL_RETURN static inline struct _mulle_objc_infraclass *
   mulle_objc_object_lookup_infraclass_inline_nofail_nofast( void *obj,
                                                             mulle_objc_universeid_t universeid,
                                                             mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = __mulle_objc_object_get_universe_nofail( obj, universeid);
   return( _mulle_objc_universe_lookup_infraclass_nofail_nofast( universe, classid));
}

#endif
