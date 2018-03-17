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
#ifndef mulle_objc_class_universe_h__
#define mulle_objc_class_universe_h__

#include "mulle-objc-uniqueid.h"
#include "mulle-objc-fastclasstable.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-object.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-version.h"


struct _mulle_objc_class;
struct _mulle_objc_universe;


int    mulle_objc_class_is_current_thread_registered( struct _mulle_objc_class *cls);


#pragma mark - class lookup, uncached

//
// this goes through the "slow" lookup table. Only internal code should
// use this method to not fill up the cache uselessy.
//
struct _mulle_objc_infraclass   *
    _mulle_objc_universe_uncachedlookup_infraclass( struct _mulle_objc_universe *universe,
                                                    mulle_objc_classid_t classid);

//
// this looks through the uncached classtable, if nothing is found
// a delegate may provide a class "in time"
//
MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_infraclass *
   _mulle_objc_universe_unfailinguncachedlookup_infraclass( struct _mulle_objc_universe *universe,
                                                            mulle_objc_classid_t classid);


struct _mulle_objc_infraclass  *
   _mulle_objc_universe_lookup_infraclass( struct _mulle_objc_universe *universe,
                                           mulle_objc_classid_t classid);


#pragma mark - class lookup, cached and fastclass

static inline struct _mulle_objc_infraclass  *
    _mulle_objc_universe_fastlookup_infraclass( struct _mulle_objc_universe *universe,
                                               mulle_objc_classid_t classid)
{
   int                             index;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_infraclass  *
        _mulle_objc_universe_lookup_infraclass( struct _mulle_objc_universe *universe,
                                               mulle_objc_classid_t classid);

   assert( universe);
   assert( ! _mulle_objc_universe_is_uninitialized( universe)); // not is_initalized!!
   assert( classid);

   index = mulle_objc_get_fastclasstable_index( classid);
   if( index >= 0)
   {
      infra = mulle_objc_fastclasstable_get_infraclass( &universe->fastclasstable, index);
      return( infra);
   }
   return( _mulle_objc_universe_lookup_infraclass( universe, classid));
}


// this is the method to use for looking up classes
MULLE_C_CONST_NON_NULL_RETURN
static inline struct _mulle_objc_infraclass *
   _mulle_objc_universe_unfailingfastlookup_infraclass( struct _mulle_objc_universe *universe,
                                                           mulle_objc_classid_t classid)
{
   int                             index;
   struct _mulle_objc_infraclass   *infra;
   MULLE_C_CONST_NON_NULL_RETURN struct _mulle_objc_infraclass *
       _mulle_objc_universe_unfailinglookup_infraclass( struct _mulle_objc_universe *universe,
                                                        mulle_objc_classid_t classid);

   assert( universe);
   assert( ! _mulle_objc_universe_is_uninitialized( universe));  // not is_initalized!!
   assert( classid);

   index = mulle_objc_get_fastclasstable_index( classid);
   if( index >= 0)
   {
      infra = mulle_objc_fastclasstable_unfailingget_infraclass( &universe->fastclasstable, index);
      return( infra);
   }
   return( _mulle_objc_universe_unfailinglookup_infraclass( universe, classid));
}


MULLE_C_NON_NULL_RETURN
static inline struct _mulle_objc_infraclass   *mulle_objc_inline_unfailingfastlookup_infraclass( mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_inlined_get_universe();
   return( _mulle_objc_universe_unfailingfastlookup_infraclass( universe, classid));
}


MULLE_C_NON_NULL_RETURN
struct _mulle_objc_infraclass   *mulle_objc_unfailingfastlookup_infraclass( mulle_objc_classid_t classid);

#endif
