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
#include "mulle_objc_object.h"
#include "mulle_objc_runtime.h"
#include "mulle_objc_version.h"


struct _mulle_objc_class;
struct _mulle_objc_runtime;


int    mulle_objc_class_is_current_thread_registered( struct _mulle_objc_class *cls);


#pragma mark -
#pragma mark class lookup

MULLE_C_CONST_RETURN
struct _mulle_objc_class  *_mulle_objc_runtime_lookup_class( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid);


//
// this goes through the cache, but ignores fastclasstable, useful if classid
// is not a constant
//
MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_class   *_mulle_objc_runtime_unfailing_lookup_class( struct _mulle_objc_runtime *runtime,
                                                                        mulle_objc_classid_t classid);

//
// this goes through the "slow" lookup table. Only internal code should
// use this method to not fill up the cache uselessy.
//
MULLE_C_CONST_RETURN
struct _mulle_objc_class   *_mulle_objc_runtime_lookup_uncached_class( struct _mulle_objc_runtime *runtime,
                                                                       mulle_objc_classid_t classid);


MULLE_C_CONST_NON_NULL_RETURN
static inline struct _mulle_objc_class   *_mulle_objc_runtime_unfailing_get_or_lookup_class( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   int                        index;
   struct _mulle_objc_class   *cls;
   
   assert( runtime);
   assert( runtime->version == MULLE_OBJC_RUNTIME_VERSION);
   assert( classid);

   index = mulle_objc_get_fastclasstable_index( classid);
   if( index >= 0)
   {
      cls = mulle_objc_fastclasstable_unfailing_get( &runtime->fastclasstable, index);
      return( _mulle_objc_class_unfailing_get_infraclass( cls));
   }
   return( _mulle_objc_runtime_unfailing_lookup_class( runtime, classid));
}


MULLE_C_NON_NULL_RETURN
static inline struct _mulle_objc_class   *mulle_objc_inline_unfailing_get_or_lookup_class( mulle_objc_classid_t classid)
{
   struct _mulle_objc_runtime   *runtime;
   
   runtime = mulle_objc_inlined_get_runtime();
   return( _mulle_objc_runtime_unfailing_get_or_lookup_class( runtime, classid));
}


MULLE_C_NON_NULL_RETURN
struct _mulle_objc_class   *mulle_objc_unfailing_get_or_lookup_class( mulle_objc_classid_t classid);


//
// This is not a way to get isa. It's an indirect way of calling
// mulle_objc_runtime_unfailing_lookup_class
//
MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_class   *_mulle_objc_object_unfailing_lookup_class( void *obj,
                                                                       mulle_objc_classid_t classid);


MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_class   *_mulle_objc_runtime_unfailing_lookup_uncached_class( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid);


MULLE_C_NON_NULL_RETURN  // always returns same value (in same thread)
static inline struct _mulle_objc_class   *_mulle_objc_object_unfailing_uncached_lookup_class( void *obj,
                                                                                mulle_objc_classid_t classid)
{
   struct _mulle_objc_class     *cls;
   struct _mulle_objc_runtime   *runtime;
   
   // super calls can not have obj = nil
   assert( obj);
   
   cls     = _mulle_objc_object_get_isa( obj);
   runtime = _mulle_objc_class_get_runtime( cls);
   
   return( _mulle_objc_runtime_unfailing_lookup_uncached_class( runtime, classid));
}



MULLE_C_NON_NULL_RETURN
struct _mulle_objc_class   *mulle_objc_runtime_unfailing_lookup_class( struct _mulle_objc_runtime *runtime,
                                                                      mulle_objc_classid_t classid);


#ifndef MULLE_OBJC_NO_CONVENIENCES

MULLE_C_NON_NULL_RETURN
struct _mulle_objc_class   *mulle_objc_unfailing_lookup_class( mulle_objc_classid_t classid);

#endif


#pragma mark -
#pragma mark instance creation

static inline struct _mulle_objc_object   *_mulle_objc_class_alloc_instance( struct _mulle_objc_class *cls, struct mulle_allocator *allocator)
{
   struct _mulle_objc_objectheader  *header;
   struct _mulle_objc_object        *obj;

   header = mulle_allocator_calloc( allocator, 1, _mulle_objc_class_get_instance_and_header_size( cls));
   obj    = _mulle_objc_objectheader_get_object( header);
   _mulle_objc_object_set_isa( obj, cls);
   return( obj);
}


static inline struct _mulle_objc_object   *_mulle_objc_class_alloc_instance_extra( struct _mulle_objc_class *cls, size_t extra, struct mulle_allocator *allocator)
{
   struct _mulle_objc_objectheader  *header;
   struct _mulle_objc_object        *obj;
   
   header = mulle_allocator_calloc( allocator, 1, _mulle_objc_class_get_instance_and_header_size( cls) + extra);
   if( ! header)
      return( NULL);
   
   obj = _mulle_objc_objectheader_get_object( header);
   _mulle_objc_object_set_isa( obj, cls);
   return( obj);
}


static inline void   _mulle_objc_object_free( struct _mulle_objc_object *obj, struct mulle_allocator *allocator)
{
   struct _mulle_objc_objectheader  *header;
   
   header = _mulle_objc_object_get_objectheader( obj);
   mulle_allocator_free( allocator, header);
}


static inline struct _mulle_objc_object   *mulle_objc_class_alloc_instance( struct _mulle_objc_class *cls, struct mulle_allocator *allocator)
{
   if( ! cls)
      abort();
   if( ! allocator)
      allocator = &mulle_default_allocator;
   return( _mulle_objc_class_alloc_instance_extra( cls, 0, allocator));
}


static inline struct _mulle_objc_object   *mulle_objc_class_alloc_instance_extra( struct _mulle_objc_class *cls, size_t extra, struct mulle_allocator *allocator)
{
   if( ! cls)
      abort();
   if( ! allocator)
      allocator = &mulle_default_allocator;
   return( _mulle_objc_class_alloc_instance_extra( cls, extra, allocator));
}


static inline void   mulle_objc_object_free( struct _mulle_objc_object *obj, struct mulle_allocator *allocator)
{
   if( ! obj)
      return;
   if( ! allocator)
      allocator = &mulle_default_allocator;
   _mulle_objc_object_free( obj, allocator);
}


#pragma mark -
#pragma mark API

MULLE_C_CONST_RETURN
static inline struct _mulle_objc_class  *mulle_objc_runtime_lookup_class( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   return( runtime ? _mulle_objc_runtime_lookup_class( runtime, classid) : NULL);
}

#endif
