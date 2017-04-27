//
//  mulle_objc_class_runtime.h
//  mulle-objc
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
#ifndef mulle_objc_class_runtime_h__
#define mulle_objc_class_runtime_h__


#include "mulle_objc_uniqueid.h"
#include "mulle_objc_fastclasstable.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_object.h"
#include "mulle_objc_runtime.h"
#include "mulle_objc_version.h"


struct _mulle_objc_class;
struct _mulle_objc_runtime;


int    mulle_objc_class_is_current_thread_registered( struct _mulle_objc_class *cls);


#pragma mark - class lookup

MULLE_C_CONST_RETURN
struct _mulle_objc_infraclass  *_mulle_objc_runtime_lookup_infraclass( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid);


//
// this goes through the cache, but ignores fastclasstable, useful if classid
// is not a constant
//
MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_infraclass   *_mulle_objc_runtime_unfailing_lookup_infraclass( struct _mulle_objc_runtime *runtime,
                                                                        mulle_objc_classid_t classid);

//
// this goes through the "slow" lookup table. Only internal code should
// use this method to not fill up the cache uselessy.
//
MULLE_C_CONST_RETURN
struct _mulle_objc_infraclass   *_mulle_objc_runtime_lookup_uncached_infraclass( struct _mulle_objc_runtime *runtime,
                                                                       mulle_objc_classid_t classid);


MULLE_C_CONST_NON_NULL_RETURN
static inline struct _mulle_objc_infraclass   *_mulle_objc_runtime_unfailing_get_or_lookup_infraclass( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   int                             index;
   struct _mulle_objc_infraclass   *infra;

   assert( runtime);
   assert( runtime->version == MULLE_OBJC_RUNTIME_VERSION);
   assert( classid);

   index = mulle_objc_get_fastclasstable_index( classid);
   if( index >= 0)
   {
      infra = mulle_objc_fastclasstable_unfailing_get_infraclass( &runtime->fastclasstable, index);
      return( infra);
   }
   return( _mulle_objc_runtime_unfailing_lookup_infraclass( runtime, classid));
}


MULLE_C_NON_NULL_RETURN
static inline struct _mulle_objc_infraclass   *mulle_objc_inline_unfailing_get_or_lookup_infraclass( mulle_objc_classid_t classid)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   return( _mulle_objc_runtime_unfailing_get_or_lookup_infraclass( runtime, classid));
}


MULLE_C_NON_NULL_RETURN
struct _mulle_objc_infraclass   *mulle_objc_unfailing_get_or_lookup_infraclass( mulle_objc_classid_t classid);


//
// This is not a way to get isa. It's an indirect way of calling
// mulle_objc_runtime_unfailing_lookup_infraclass
//
MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_infraclass   *_mulle_objc_object_unfailing_lookup_infraclass( void *obj,
                                                                       mulle_objc_classid_t classid);


MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_infraclass   *
   _mulle_objc_runtime_unfailing_lookup_uncached_infraclass( struct _mulle_objc_runtime *runtime,
                                                              mulle_objc_classid_t classid);


#ifndef MULLE_OBJC_NO_CONVENIENCES

MULLE_C_NON_NULL_RETURN
struct _mulle_objc_infraclass   *mulle_objc_unfailing_lookup_infraclass( mulle_objc_classid_t classid);

#endif



#pragma mark - API

MULLE_C_CONST_RETURN
static inline struct _mulle_objc_infraclass  *mulle_objc_runtime_lookup_infraclass( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   return( runtime ? _mulle_objc_runtime_lookup_infraclass( runtime, classid) : NULL);
}

#endif
