//
//  mulle_objc_retain_release.h
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

#ifndef mulle_objc_retain_release_h__
#define mulle_objc_retain_release_h__

#include "mulle-objc-method.h"
#include "mulle-objc-object.h"
#include "mulle-objc-taggedpointer.h"
#include "mulle-objc-call.h"

#include "include.h"
#include <limits.h>
#include <assert.h>

//
// Your root class MUST implement -finalize and -dealloc.
//
void  _mulle_objc_object_tryfinalizetrydealloc( void *obj);
void  _mulle_objc_object_perform_finalize( void *obj);


static inline void   mulle_objc_object_perform_finalize( void *obj)
{
   if( obj)
      _mulle_objc_object_perform_finalize( obj);
}



// this function will create an "adjusted" always positive value
// if you want the raw number, check the header function retaincount_1
uintptr_t  __mulle_objc_object_get_retaincount( void *obj);


static inline intptr_t   _mulle_objc_object_get_retaincount( void *obj)
{
   if( mulle_objc_taggedpointer_get_index( obj))
      return( MULLE_OBJC_NEVER_RELEASE);
   return( __mulle_objc_object_get_retaincount( obj));
}


static inline intptr_t   mulle_objc_object_get_retaincount( void *obj)
{
   return( obj ? _mulle_objc_object_get_retaincount( obj) : 0);
}


static inline void   _mulle_objc_object_increment_retaincount( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   if( (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1) != MULLE_OBJC_NEVER_RELEASE)
      _mulle_atomic_pointer_increment( &header->_retaincount_1); // atomic increment needed
}


static inline int   _mulle_objc_object_decrement_retaincount_waszero( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return( 0);

   header = _mulle_objc_object_get_objectheader( obj);
   assert( (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1) != -1 &&
           (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1) != INTPTR_MIN);
   if( (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1) != MULLE_OBJC_NEVER_RELEASE)
   {
      if( _mulle_atomic_pointer_decrement( &header->_retaincount_1) == 0)
         return( 1);
   }
   return( 0);
}

//
// Try not to use it, it obscures bugs and will disturb leak detection
// You should only use it when creating an object, not "on the fly"
// therefore it's nonatomic
//
static inline void   _mulle_objc_object_infiniteretain_noatomic( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   _mulle_atomic_pointer_nonatomic_write( &header->_retaincount_1, (void *) MULLE_OBJC_NEVER_RELEASE);
}


static inline int   _mulle_objc_object_is_constant( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return( 1);

   header = _mulle_objc_object_get_objectheader( obj);
   return( _mulle_atomic_pointer_read( &header->_retaincount_1) == (void *) MULLE_OBJC_NEVER_RELEASE);
}



//
// compiler should optimize that the mulle_objc_object_try_finalize_try_dealloc
// is rarely taken, but
// __builtin_expect( --header->retaincount_1 < 0, 0)
// didnt do anything for me
//
static inline void   _mulle_objc_object_inlinerelease( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);

   // -1 means released, INTPTR_MIN finalizing/released
   assert( (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1) != -1 &&
           (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1) != INTPTR_MIN);

   // INTPTR_MAX means dont ever free
   if( (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1) != MULLE_OBJC_NEVER_RELEASE)
   {
      if( __builtin_expect( (intptr_t) _mulle_atomic_pointer_decrement( &header->_retaincount_1) <= 0, 0)) // atomic decrement needed
         _mulle_objc_object_tryfinalizetrydealloc( obj);
   }
}


static inline void   _mulle_objc_object_inlineretain( void *obj)
{
   _mulle_objc_object_increment_retaincount( obj);
}


# pragma mark - API, compiler uses this code too

// must be void *, for compiler
static inline void   *mulle_objc_object_inlineretain( void *obj)
{
   if( obj)
      _mulle_objc_object_inlineretain( obj);
   return( obj);
}


static inline void   mulle_objc_object_inlinerelease( void *obj)
{
   if( ! obj)
      return;
   _mulle_objc_object_inlinerelease( obj);
}


// must be void *, for compiler
void   *mulle_objc_object_retain( void *obj);
void   mulle_objc_object_release( void *obj);


# pragma mark - API


void   _mulle_objc_objects_retain( void **objects, size_t n);
void   _mulle_objc_objects_release( void **objects, size_t n);
void   _mulle_objc_objects_releaseandzero( void **objects, size_t n);


static inline void   mulle_objc_objects_retain( void **objects, size_t n)
{
   if( ! objects)
   {
      assert( ! n);
      return;
   }
   _mulle_objc_objects_retain( objects, n);
}


static inline void   mulle_objc_objects_release( void **objects, size_t n)
{
   if( ! objects)
   {
      assert( ! n);
      return;
   }
   _mulle_objc_objects_release( objects, n);
}


static inline void   mulle_objc_objects_releaseandzero( void **objects, size_t n)
{
   if( ! objects)
   {
      assert( ! n);
      return;
   }
   _mulle_objc_objects_releaseandzero( objects, n);
}

#endif
