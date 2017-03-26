//
//  mulle_objc_retain_release.h
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

#ifndef mulle_objc_retain_release_h__
#define mulle_objc_retain_release_h__

#include "mulle_objc_method.h"
#include "mulle_objc_object.h"
#include "mulle_objc_taggedpointer.h"
#include "mulle_objc_call.h"

#include <mulle_thread/mulle_thread.h>
#include <limits.h>
#include <assert.h>

//
// Your root class MUST implement -finalize and -dealloc.
//
void  _mulle_objc_object_try_finalize_try_dealloc( void *obj);
void  _mulle_objc_object_perform_finalize( void *obj);


static inline void   mulle_objc_object_perform_finalize( void *obj)
{
   if( obj)
      _mulle_objc_object_perform_finalize( obj);
}


#define MULLE_OBJC_NEVER_RELEASE   INTPTR_MAX

// retaincount_1
//    0       -> INTPTR_MAX-1 :: normal retain counting, actual retainCount is retaincount_1 + 1
//    -1                      :: released
//    INTPTR_MIN              :: retainCount 0, when finalizing
//    INTPTR_MIN -> -2        :: finalizing/finalized retainCount
//
// you can use INTPTR_MAX to create static objects
//
// like f.e.
// { INTPTR_MAX, MULLE_OBJC_METHODID( 0x423aeba60e4eb3be), "VfL Bochum 1848" }; // 0x423aeba60e4eb3be == NXConstantString
//
intptr_t  _mulle_objc_object_get_retaincount( void *obj);


static inline intptr_t   mulle_objc_object_get_retaincount( void *obj)
{
   if( mulle_objc_taggedpointer_get_index( obj))
      return( MULLE_OBJC_NEVER_RELEASE);

   if( obj)
      return( _mulle_objc_object_get_retaincount( obj));
   return( 0);
}


static inline void   _mulle_objc_object_increment_retaincount( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   if( (intptr_t) _mulle_atomic_pointer_nonatomic_read( &header->_retaincount_1) != MULLE_OBJC_NEVER_RELEASE)
      _mulle_atomic_pointer_increment( &header->_retaincount_1); // atomic increment needed
}


static inline int   _mulle_objc_object_decrement_retaincount_was_zero( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return( 0);

   header = _mulle_objc_object_get_objectheader( obj);
   assert( (intptr_t) _mulle_atomic_pointer_nonatomic_read( &header->_retaincount_1) != -1 &&
           (intptr_t) _mulle_atomic_pointer_nonatomic_read( &header->_retaincount_1) != INTPTR_MIN);
   if( (intptr_t) _mulle_atomic_pointer_nonatomic_read( &header->_retaincount_1) != MULLE_OBJC_NEVER_RELEASE)
   {
      if( _mulle_atomic_pointer_decrement( &header->_retaincount_1) == 0)
         return( 1);
   }
   return( 0);
}

// try not to use it, it obscures bugs and will disturb leak detection
static inline void   _mulle_objc_object_infinite_retain( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   _mulle_atomic_pointer_nonatomic_write( &header->_retaincount_1, (void *) MULLE_OBJC_NEVER_RELEASE);
}



//
// compiler should optimize that the mulle_objc_object_try_finalize_try_dealloc
// is rarely taken, but
// __builtin_expect( --header->retaincount_1 < 0, 0)
// didnt do anything for me
//
static inline void   _mulle_objc_object_release( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);

   // -1 means released, INTPTR_MIN finalizing/released
   assert( (intptr_t) _mulle_atomic_pointer_nonatomic_read( &header->_retaincount_1) != -1 &&
           (intptr_t) _mulle_atomic_pointer_nonatomic_read( &header->_retaincount_1) != INTPTR_MIN);

   // INTPTR_MAX means dont ever free
   if( (intptr_t) _mulle_atomic_pointer_nonatomic_read( &header->_retaincount_1) != MULLE_OBJC_NEVER_RELEASE)
   {
      if( __builtin_expect( (intptr_t) _mulle_atomic_pointer_decrement( &header->_retaincount_1) <= 0, 0)) // atomic decrement needed
         _mulle_objc_object_try_finalize_try_dealloc( obj);
   }
}


static inline void   _mulle_objc_object_retain( void *obj)
{
   _mulle_objc_object_increment_retaincount( obj);
}


void   _mulle_objc_objects_call_retain( void **objects, size_t n);
void   _mulle_objc_objects_call_release( void **objects, size_t n);
void   _mulle_objc_objects_call_release_and_zero( void **objects, size_t n);

# pragma mark - API, compiler uses this code too

// must be void *, for compiler
static inline void   *mulle_objc_object_retain( void *obj)
{
   if( obj)
      _mulle_objc_object_retain( obj);
   return( obj);
}


static inline void   mulle_objc_object_release( void *obj)
{
   if( ! obj)
      return;
#if DEBUG
   struct _mulle_objc_class  *cls;

   cls = _mulle_objc_object_get_isa( obj);
   (*cls->call)( obj, MULLE_OBJC_RELEASE_METHODID, NULL, cls);
#else
   _mulle_objc_object_release( obj);
#endif
}


# pragma mark - API


static inline void   mulle_objc_objects_call_retain( void **objects, size_t n)
{
   if( ! objects)
   {
      assert( ! n);
      return;
   }
   _mulle_objc_objects_call_retain( objects, n);
}


static inline void   mulle_objc_objects_call_release( void **objects, size_t n)
{
   if( ! objects)
   {
      assert( ! n);
      return;
   }
   _mulle_objc_objects_call_release( objects, n);
}


static inline void   mulle_objc_objects_call_release_and_zero( void **objects, size_t n)
{
   if( ! objects)
   {
      assert( ! n);
      return;
   }
   _mulle_objc_objects_call_release_and_zero( objects, n);
}

#endif
