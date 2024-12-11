//
//  mulle_objc_object.h
//  mulle-objc-runtime
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
#ifndef mulle_objc_object_h__
#define mulle_objc_object_h__

#include "include.h"

#include "mulle-objc-class.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-ivar.h"
#include "mulle-objc-objectheader.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-taggedpointer.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

struct _mulle_objc_method;


#ifndef MULLE_OBJC_CALL_PREFER_TPS
# define MULLE_OBJC_CALL_PREFER_TPS  1
#endif

//
// an object can be both a instance or a _class, so it should be typed as
// void *. Possibly should renamed struct _mulle_objc_object to
// struct _mulle_objc_instance...
//
// Functions that only deal with instances (like alloc/free) are
// named _mulle_objc_instance...
//

# pragma mark - isa handling

MULLE_C_ALWAYS_INLINE MULLE_C_CONST_RETURN
static inline int  mulle_objc_object_get_taggedpointerindex( void *obj)
{
#ifdef __MULLE_OBJC_TPS__
   return( mulle_objc_taggedpointer_get_index( obj));
#else
   return( 0);
#endif
}


//
// this defeats faults, but benchmarks much better, if put into the inline
// call routine
//
MULLE_C_ALWAYS_INLINE_NONNULL_CONST_RETURN
static inline struct _mulle_objc_class   *_mulle_objc_object_const_get_isa( void *obj)
{
   unsigned int                     index;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_infraclass   *infra;

   // compiler should notice that #ifdef __MULLE_OBJC_NO_TPS__ index is always 0
   index = mulle_objc_object_get_taggedpointerindex( obj);
   if( ! index)
      return( _mulle_objc_objectheader_get_isa( _mulle_objc_object_get_objectheader( obj)));

   universe = mulle_objc_global_get_universe_inline( MULLE_OBJC_DEFAULTUNIVERSEID);
   infra    = universe->taggedpointers.pointerclass[ index];
   return( _mulle_objc_infraclass_as_class( infra));
}


//
// don't use isa in most cases, use get_class (defined elsewhere)
//
MULLE_C_ALWAYS_INLINE_NONNULL_RETURN
static inline struct _mulle_objc_class *
   _mulle_objc_object_get_isa( void *obj)
{
   unsigned int                    index;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_infraclass   *infra;

   // compiler should notice that #ifdef __MULLE_OBJC_NO_TPS__ index is always 0
   index = mulle_objc_object_get_taggedpointerindex( obj);
   if( MULLE_C_EXPECT( ! index, MULLE_OBJC_CALL_PREFER_TPS)) // prefer tagged pointers path
      return( _mulle_objc_objectheader_get_isa( _mulle_objc_object_get_objectheader( obj)));

   universe = mulle_objc_global_get_universe_inline( MULLE_OBJC_DEFAULTUNIVERSEID);
   assert( universe->taggedpointers.pointerclass[ index] && "Tagged pointer class not configured. Is your object properly aligned ?");
   infra    = universe->taggedpointers.pointerclass[ index];
   return( _mulle_objc_infraclass_as_class( infra));
}



//
// don't use isa in most cases, use get_class (defined elsewhere)
//
MULLE_C_ALWAYS_INLINE_NONNULL_RETURN
static inline struct _mulle_objc_class *
   _mulle_objc_object_get_non_tps_isa( void *obj)
{
   struct _mulle_objc_objectheader  *header;

   assert( mulle_objc_object_get_taggedpointerindex( obj) == 0);
   header = _mulle_objc_object_get_objectheader( obj);
   return( _mulle_objc_objectheader_get_isa( header));
}


//
// don't use isa in most cases, use get_class (defined elsewhere)
//
MULLE_C_ALWAYS_INLINE_NONNULL_RETURN
static inline struct _mulle_objc_class *
   _mulle_objc_object_get_isa_universe( void *obj, struct _mulle_objc_universe *universe)
{
   unsigned int                    index;
   struct _mulle_objc_infraclass   *infra;

   // compiler should notice that #ifdef __MULLE_OBJC_NO_TPS__ index is always 0
   index = mulle_objc_object_get_taggedpointerindex( obj);
   if( MULLE_C_EXPECT( ! index, MULLE_OBJC_CALL_PREFER_TPS)) // prefer tagged pointers path
      return( _mulle_objc_objectheader_get_isa( _mulle_objc_object_get_objectheader( obj)));

   assert( universe->taggedpointers.pointerclass[ index] && "Tagged pointer class not configured. Is your object properly aligned ?");
   infra = universe->taggedpointers.pointerclass[ index];
   return( _mulle_objc_infraclass_as_class( infra));
}


static inline void  _mulle_objc_object_set_isa( void *obj, struct _mulle_objc_class *cls)
{
   unsigned int                      index;
   struct _mulle_objc_objectheader   *header;

   // compiler should notice that #ifdef __MULLE_OBJC_NO_TPS__ index is always 0
   index = mulle_objc_object_get_taggedpointerindex( obj);
   if( index)
      mulle_objc_universe_fail_inconsistency( NULL, "set isa on tagged pointer %p", obj);

   // need low level access, as object may not have been initialized
   header = __mulle_objc_object_get_objectheader( obj);
   _mulle_objc_objectheader_set_isa( header, cls);
}


static inline void  *_mulle_objc_object_get_extra( void *obj)
{
   struct _mulle_objc_class          *cls;
   size_t                            size;
   struct _mulle_objc_objectheader   *header;

   assert( ! mulle_objc_object_get_taggedpointerindex( obj));

   header = _mulle_objc_object_get_objectheader( obj);
   cls    = _mulle_objc_objectheader_get_isa( header);
   size   = _mulle_objc_class_get_allocationsize( cls) - sizeof( struct _mulle_objc_objectheader);
   return(  &((char *) obj)[ size]);
}


// legacy support and no zone support anyway
MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_object   *
   _mulle_objc_object_get_zone( void *obj)
{
   return( (void *) 0);
}


# pragma mark - convenience

MULLE_C_CONST_RETURN MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_class *
	__mulle_objc_object_get_isa_notps( void *obj)
{
	struct _mulle_objc_class  *cls;

   assert( ! mulle_objc_object_get_taggedpointerindex( obj));

   cls = _mulle_objc_objectheader_get_isa( _mulle_objc_object_get_objectheader( obj));
   return( cls);
}


MULLE_C_CONST_RETURN MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_universe *
	__mulle_objc_object_get_universe_notps( void *obj)
{
	struct _mulle_objc_class     *cls;

   cls = __mulle_objc_object_get_isa_notps( obj);
   return( _mulle_objc_class_get_universe( cls));
}


/* obj can not be nil here */
MULLE_C_CONST_RETURN MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_universe *
	_mulle_objc_object_get_universe( void *obj)
{
   assert( obj);

	if( mulle_objc_object_get_taggedpointerindex( obj))
      return( mulle_objc_global_get_universe_inline( MULLE_OBJC_DEFAULTUNIVERSEID));
   return( __mulle_objc_object_get_universe_notps( obj));
}


/*
 * this function also deals with obj being NULL
 * therefore we need the universeid
 */
MULLE_C_CONST_NONNULL_RETURN MULLE_C_ALWAYS_INLINE
static inline struct _mulle_objc_universe *
	__mulle_objc_object_get_universe_nofail( void *obj,
	       							        		  mulle_objc_universeid_t universeid)
{
	if( ! obj ||  mulle_objc_object_get_taggedpointerindex( obj))
      return( mulle_objc_global_get_universe_inline( universeid));
   return( __mulle_objc_object_get_universe_notps( obj));
}


# pragma mark - get_class

MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_infraclass   *
   _mulle_objc_object_get_infraclass( void *obj)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_object_get_isa( obj);
   if( cls->infraclass)
      return( cls->infraclass);
   return( (struct _mulle_objc_infraclass *) cls);
}




MULLE_C_CONST_RETURN
MULLE_C_ALWAYS_INLINE static inline mulle_thread_t
   _mulle_objc_object_get_thread( struct _mulle_objc_object *obj)
{
#ifdef __MULLE_OBJC_TAO__
   unsigned int                      index;
   struct _mulle_objc_objectheader   *header;

   // TPS are not thread affine always
   index = mulle_objc_object_get_taggedpointerindex( obj);
   if( index)
      return( 0);

   header = _mulle_objc_object_get_objectheader( obj);
   return( _mulle_objc_objectheader_get_thread( header));
#else
   return( 0);
#endif
}


#define mulle_objc_object_is_threadsafe   ((mulle_thread_t) 0)
#define mulle_objc_object_has_no_thread   ((mulle_thread_t) -1)


MULLE_C_ALWAYS_INLINE static inline void
   _mulle_objc_object_set_thread( struct _mulle_objc_object *obj, mulle_thread_t thread)
{
#ifdef __MULLE_OBJC_TAO__
   unsigned int                      index;
   struct _mulle_objc_objectheader   *header;
   struct _mulle_objc_universe       *universe;

   // TPS are not thread affine always
   index = mulle_objc_object_get_taggedpointerindex( obj);
   if( index)
      return;

   header   = _mulle_objc_object_get_objectheader( obj);
   universe = _mulle_objc_object_get_universe( obj);
   if( universe->debug.trace.tao)
   {
      mulle_objc_universe_trace( universe, "Object <%s %p> changes thread affinity to \"%p\"",
                                           _mulle_objc_class_get_name( header->_isa),
                                           obj,
                                           thread);
   }

   _mulle_objc_objectheader_set_thread( header, thread);
#endif
}



# pragma mark - ivar access

//
// these are not calculating the size from the signature
// (could work for classes too...)
//
static inline void
   _mulle_objc_object_get_value_for_ivar( void *obj,
                                          struct _mulle_objc_ivar *ivar,
                                          void *buf,
                                          size_t length)
{
   memcpy( buf, &((char *) obj)[ ivar->offset], length);
}


static inline void
   _mulle_objc_object_set_value_for_ivar( void *obj,
                                          struct _mulle_objc_ivar *ivar,
                                          void *buf,
                                          size_t length)
{
   memcpy( buf, &((char *) obj)[ ivar->offset], length);
}


static inline void  *
   _mulle_objc_object_get_pointervalue_for_ivar( void *obj,
                                                 struct _mulle_objc_ivar *ivar)
{
   return( *(void **) &((char *) obj)[ ivar->offset]);
}


static inline void
   _mulle_objc_object_set_pointervalue_for_ivar( void *obj,
                                                 struct _mulle_objc_ivar *ivar,
                                                 void *value)
{
   *(void **) &((char *) obj)[ ivar->offset] = value;
}


# pragma mark - API

MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_class   *
   mulle_objc_object_get_isa( void *obj)
{
   return( obj ? _mulle_objc_object_get_isa( obj)  : NULL);
}

MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_infraclass   *
   mulle_objc_object_get_infraclass( void *obj)
{
   return( obj ? _mulle_objc_object_get_infraclass( obj) : NULL);
}

// convenience for object, will return null if its a tagged pointer though
MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_universe   *
   mulle_objc_object_get_universe( void *obj)
{
   return( obj ? _mulle_objc_object_get_universe( obj) : NULL);
}


MULLE_C_ALWAYS_INLINE static inline void
   mulle_objc_object_set_isa( void *obj, struct _mulle_objc_class *cls)
{
   if( obj)
      _mulle_objc_object_set_isa( obj, cls);
}


MULLE_C_ALWAYS_INLINE static inline void  *
   mulle_objc_object_get_extra( void *obj)
{
   return( obj ? _mulle_objc_object_get_extra( obj) : NULL);
}

#endif
