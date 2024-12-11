//
//  mulle_objc_objectheader.h
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
#ifndef mulle_objc_objectheader_h__
#define mulle_objc_objectheader_h__

#include "include.h"

#include <assert.h>

#include "mulle-objc-class-struct.h"
#include "mulle-objc-taggedpointer.h"
struct _mulle_objc_object;


// do it like this, so we can easily test compile without header
// for TAO canary test
#ifndef MULLE_OBJC_TAO_OBJECT_HEADER
# if (defined( __MULLE_OBJC_TAO__) || defined( MULLE_TEST) ) && ! defined( MULLE_OBJC_NO_TAO_OBJECT_HEADER)
#  define MULLE_OBJC_TAO_OBJECT_HEADER  1
# else
#  define MULLE_OBJC_TAO_OBJECT_HEADER  0
# endif
#endif



#ifndef NDEBUG
static inline void
   mulle_objc_object_assert_tao_object_header_no_tps( struct _mulle_objc_object *obj,
                                                      int define)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   void   _mulle_objc_object_assert_tao_object_header_no_tps( struct _mulle_objc_object *obj,
                                                              int define);

   if( obj)
      _mulle_objc_object_assert_tao_object_header_no_tps( obj, define);
}
#else
static inline void
   mulle_objc_object_assert_tao_object_header_no_tps( struct _mulle_objc_object *obj,
                                                      int define)
{
   MULLE_C_UNUSED( obj);
   MULLE_C_UNUSED( define);
}
#endif


//
// this is ahead of the actual instance
// isa must be underscored
// It is important, that on 64 bit it's 16 byte size aligned, because then a
// following class can provide an isa pointer with 4 zero ls bits, which is
// needed for __MULLE_TPS_.
// If the class has "headerextrasize" then there is space ahead of the header
// still, which leads us to the alloc.
//
struct _mulle_objc_objectheader
{
   //
   // in tests its inconvenient to have variable sizes, since it breaks stuff
   //
#if MULLE_OBJC_TAO_OBJECT_HEADER
   void                        *_align;
   mulle_atomic_pointer_t      _thread;
#endif
   mulle_atomic_pointer_t      _retaincount_1;  // negative means finalized
   struct _mulle_objc_class    *_isa;
};


MULLE_C_ALWAYS_INLINE static inline void *
   _mulle_objc_objectheader_get_alloc( struct _mulle_objc_objectheader *header,
                                       uintptr_t headerextrasize)
{
   return( &((char *) header)[ - (intptr_t) headerextrasize]);
}


MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_objectheader *
   _mulle_objc_alloc_get_objectheader( void *alloc,
                                       uintptr_t headerextrasize)
{
   return( (struct _mulle_objc_objectheader *) &((char *) alloc)[ headerextrasize]);
}


// same as below but no header checker test for true low level code
MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_objectheader *
   __mulle_objc_object_get_objectheader( void *obj)
{
   struct _mulle_objc_objectheader   *header;

   assert( obj);
   assert( ! ((uintptr_t) obj & mulle_objc_get_taggedpointer_mask()) && "tagged pointer");
   header = (void *) &((char *) obj)[ - (int) sizeof( struct _mulle_objc_objectheader)];
   return( header);
}


MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_objectheader *
   _mulle_objc_object_get_objectheader( void *obj)
{
   struct _mulle_objc_objectheader   *header;

   assert( obj);
   assert( ! ((uintptr_t) obj & mulle_objc_get_taggedpointer_mask()) && "tagged pointer");
   mulle_objc_object_assert_tao_object_header_no_tps( obj, MULLE_OBJC_TAO_OBJECT_HEADER);
   header = (void *) &((char *) obj)[ - (int) sizeof( struct _mulle_objc_objectheader)];
   return( header);
}


MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_object *
   _mulle_objc_objectheader_get_object( struct _mulle_objc_objectheader *header)
{
   struct _mulle_objc_object *obj;

   assert( header);

   obj = (void *) (header + 1);

   // object should be pointer aligned, could awry if the metaextrasize is
   // wrong
   assert( ((uintptr_t) obj & (sizeof( void *) - 1)) == 0);
   return( obj);
}


static inline intptr_t
   _mulle_objc_objectheader_get_retaincount_1( struct _mulle_objc_objectheader *header)
{
   return( (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1));
}


# pragma mark - object / header

MULLE_C_ALWAYS_INLINE static inline struct _mulle_objc_class *
   _mulle_objc_objectheader_get_isa( struct _mulle_objc_objectheader *header)
{
   return( header->_isa);
}


MULLE_C_CONST_RETURN
MULLE_C_ALWAYS_INLINE static inline mulle_thread_t
   _mulle_objc_objectheader_get_thread( struct _mulle_objc_objectheader *header)
{
#if MULLE_OBJC_TAO_OBJECT_HEADER
   return( (mulle_thread_t) _mulle_atomic_pointer_read( &header->_thread));
#else
   return( 0);
#endif
}


//
// this need not be atomic, it would be one object setting the affinity
// and then handing it over to another thread
//
// | executing thread |   old  |   new  | Description
// |------------------|--------|--------|-------------
// |   a              |  NULL  |    a   | from multi-threaded to single-threaded (hmm)
// |   a              |    a   |    b   |
// |   a              |    b   |    a   |
// |   a              |    b   |  NULL  |
// |   b              |  NULL  |    b   | from multi-threaded to single-threaded (hmm)
// |   b              |    a   |    b   |
// |   b              |    b   |    a   |
// |   a              |    b   |  NULL  |

MULLE_C_ALWAYS_INLINE static inline void
   _mulle_objc_objectheader_set_thread( struct _mulle_objc_objectheader *header,
                                        mulle_thread_t thread)
{
#if MULLE_OBJC_TAO_OBJECT_HEADER
   _mulle_atomic_pointer_write( &header->_thread, (void *) thread);
#endif
}



//
// this is not atomic! only do this when noone else has
// access (like during initialization)
//
static inline void
   _mulle_objc_objectheader_set_isa( struct _mulle_objc_objectheader *header,
                                     struct _mulle_objc_class *cls)
{
   header->_isa = cls;
}


# pragma mark - init


enum
{
   _mulle_objc_memory_is_not_zeroed = 0,
   _mulle_objc_memory_is_zeroed     = 1
};

static inline void
   _mulle_objc_objectheader_init( struct _mulle_objc_objectheader *header,
                                  struct _mulle_objc_class *cls,
                                  uintptr_t headerextrasize,
                                  int is_zeroed)
{
   // clean out extra meta memory and the retain count
   if( ! is_zeroed)
      memset( &((char *) header)[ - (intptr_t) headerextrasize],
              0,
              sizeof( *header) - sizeof( struct _mulle_objc_class *) + headerextrasize);
   _mulle_objc_objectheader_set_isa( header, cls);
#if MULLE_OBJC_TAO_OBJECT_HEADER
   if( _mulle_objc_class_is_threadaffine( cls))
      _mulle_objc_objectheader_set_thread( header, mulle_thread_self());
#endif
}

#endif
