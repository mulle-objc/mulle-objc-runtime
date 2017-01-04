//
//  mulle_objc_object.h
//  mulle-objc
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
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

#ifndef mulle_objc_object_h__
#define mulle_objc_object_h__

#include <mulle_thread/mulle_thread.h>

#include "mulle_objc_class.h"
#include "mulle_objc_ivar.h"
#include "mulle_objc_objectheader.h"
#include "mulle_objc_runtime.h"
#include "mulle_objc_taggedpointer.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

struct _mulle_objc_method;


# pragma mark -
# pragma mark isa handling

static inline int  mulle_objc_object_get_taggedpointer_index( struct _mulle_objc_object *obj)
{
#if __MULLE_OBJC_TPS__
   return( mulle_objc_taggedpointer_get_index( obj));
#else
   return( 0);
#endif
}

//
// this defeats faults, but benchmarks much better, if put into the inline
// call routine
//
MULLE_C_ALWAYS_INLINE_NON_NULL_CONST_RETURN
static inline struct _mulle_objc_class   *_mulle_objc_object_const_get_isa( void *obj)
{
   unsigned int                 index;
   struct _mulle_objc_runtime   *runtime;

   index = mulle_objc_object_get_taggedpointer_index( obj);
   if( ! index)
      return( _mulle_objc_objectheader_get_isa( _mulle_objc_object_get_objectheader( obj)));
   
   runtime = mulle_objc_inlined_get_runtime();
   return( runtime->taggedpointers.pointerclass[ index]);
}

//
// don't use isa in most cases, use get_class (defined elsewhere)
//
MULLE_C_ALWAYS_INLINE_NON_NULL_RETURN
static inline struct _mulle_objc_class   *_mulle_objc_object_get_isa( void *obj)
{
   unsigned int                 index;
   struct _mulle_objc_runtime   *runtime;

   index = mulle_objc_object_get_taggedpointer_index( obj);
   if( __builtin_expect( ! index, 1))
      return( _mulle_objc_objectheader_get_isa( _mulle_objc_object_get_objectheader( obj)));

   runtime = mulle_objc_inlined_get_runtime();
   assert( runtime->taggedpointers.pointerclass[ index] && "tagged pointer class not configured");
   return( runtime->taggedpointers.pointerclass[ index]);
}


static inline void  _mulle_objc_object_set_isa( void *obj, struct _mulle_objc_class *cls)
{
   unsigned int   index;
   extern void    mulle_objc_raise_taggedpointer_exception( void *obj);

   index = mulle_objc_object_get_taggedpointer_index( obj);
   if( index)
      mulle_objc_raise_taggedpointer_exception( obj);
   
   _mulle_objc_objectheader_set_isa( _mulle_objc_object_get_objectheader( obj), cls);
}


// legacy support and no zone support anyway
MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_object   *_mulle_objc_object_get_zone( void *obj)
{
   return( (void *) 0);
}


#pragma mark -
#pragma mark convenience

// convenience for object
static inline struct _mulle_objc_runtime   *_mulle_objc_object_get_runtime( void *obj)
{
   struct _mulle_objc_class   *cls;
   
   cls = _mulle_objc_object_get_isa( obj);
   return( _mulle_objc_class_get_runtime( cls));
}


# pragma mark -
# pragma mark get_class

MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_class   *_mulle_objc_object_get_class( void *obj)
{
   struct _mulle_objc_class   *cls;
   
   cls = _mulle_objc_object_get_isa( obj);
   if( _mulle_objc_class_is_metaclass( cls))
      return( _mulle_objc_class_get_infraclass( cls));
   return( cls);
}


#pragma mark -
#pragma mark ivar access

// these are not calculating the size from the signature
static inline void  _mulle_objc_object_get_value_for_ivar( void *obj, struct _mulle_objc_ivar *ivar, void *buf, size_t length)
{
   memcpy( buf, &((char *) obj)[ ivar->offset], length);
}


static inline void  _mulle_objc_object_set_value_for_ivar( void *obj, struct _mulle_objc_ivar *ivar, void *buf, size_t length)
{
   memcpy( buf, &((char *) obj)[ ivar->offset], length);
}


static inline void  *_mulle_objc_object_get_pointervalue_for_ivar( void *obj, struct _mulle_objc_ivar *ivar)
{
   return( *(void **) &((char *) obj)[ ivar->offset]);
}


static inline void  _mulle_objc_object_set_pointervalue_for_ivar( void *obj, struct _mulle_objc_ivar *ivar, void *value)
{
   *(void **) &((char *) obj)[ ivar->offset] = value;
}


# pragma mark -
# pragma mark API

MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_class   *mulle_objc_object_get_isa( void *obj)
{
   return( obj ? _mulle_objc_objectheader_get_isa( _mulle_objc_object_get_objectheader( obj)) : NULL);
}

MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_class   *mulle_objc_object_get_class( void *obj)
{
   return( obj ? _mulle_objc_object_get_class( obj) : NULL);
}

// convenience for object
MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_runtime   *mulle_objc_object_get_runtime( void *obj)
{
   return( obj ? _mulle_objc_object_get_runtime( obj) : NULL);
}


MULLE_C_ALWAYS_INLINE
static inline void  mulle_objc_object_set_isa( void *obj, struct _mulle_objc_class *cls)
{
   if( obj)
      _mulle_objc_object_set_isa( obj, cls);
}

#endif
