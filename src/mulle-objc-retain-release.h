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

#include "include.h"

#include "mulle-objc-method.h"
#include "mulle-objc-object.h"
#include "mulle-objc-taggedpointer.h"
#include "mulle-objc-call.h"

#include <limits.h>
#include <assert.h>


#define MULLE_OBJC_SLOW_RELEASE   INTPTR_MAX
#define MULLE_OBJC_NEVER_RELEASE  (INTPTR_MAX-1)
// if an inline rc count manages to pass MULLE_OBJC_INLINE_RELEASE and hit
// MULLE_OBJC_NEVER_RELEASE, the release counting stops there and we have a
// leak, but not a crash
#define MULLE_OBJC_INLINE_RELEASE  (INTPTR_MAX-2)

// retaincount_1 is in the object header
//
//    0       -> INTPTR_MAX-2 :: normal retain counting, actual retainCount is retaincount_1 + 1
//    -1                      :: released
//    INTPTR_MIN              :: retainCount 0, during finalizing
//    INTPTR_MIN -> -2        :: other threads retaining during finalization
//
// you can use MULLE_OBJC_NEVER_RELEASE to create static objects
// you can use MULLE_OBJC_SLOW_RELEASE to force use of -release, but
// obviously you need to keep the referenceCount elsewhere
//
// like f.e.
// { MULLE_OBJC_NEVER_RELEASE, @selector( NSConstantString), "VfL Bochum 1848" };
//



//
// Your root class MUST implement -finalize and -dealloc.
//
MULLE_OBJC_RUNTIME_GLOBAL
void  _mulle_objc_object_tryfinalizetrydealloc( void *obj);

MULLE_OBJC_RUNTIME_GLOBAL
void  _mulle_objc_object_perform_finalize( void *obj);


static inline int   _mulle_objc_object_is_finalized( void *obj)
{
   intptr_t                          retaincount_1;
   struct _mulle_objc_objectheader   *header;

   header        = _mulle_objc_object_get_objectheader( obj);
   retaincount_1 = (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1);
   return( retaincount_1 < 0);
}


static inline void   mulle_objc_object_perform_finalize( void *obj)
{
   if( obj)
      _mulle_objc_object_perform_finalize( obj);
}



// this function will create an "adjusted" always positive value
// if you want the raw number, check the header function retaincount_1
MULLE_OBJC_RUNTIME_GLOBAL
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


// TODO: getting a bit too fat for inlining or ?
static inline void   _mulle_objc_object_increment_retaincount( void *obj)
{
   struct _mulle_objc_objectheader    *header;
   intptr_t                           rc;

   if( MULLE_C_LIKELY( mulle_objc_taggedpointer_get_index( obj)))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   rc     = (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1);
   if( MULLE_C_LIKELY( rc <= MULLE_OBJC_INLINE_RELEASE))
   {
      _mulle_atomic_pointer_increment( &header->_retaincount_1); // atomic increment needed
      return;
   }

   if( rc == MULLE_OBJC_SLOW_RELEASE)
      mulle_objc_object_call_inline_partial( obj, MULLE_OBJC_RETAIN_METHODID, obj);
}


static inline int   _mulle_objc_object_decrement_retaincount_waszero( void *obj)
{
   struct _mulle_objc_objectheader    *header;
   intptr_t                           rc;

   if( MULLE_C_LIKELY( mulle_objc_taggedpointer_get_index( obj)))
      return( 0);

   header = _mulle_objc_object_get_objectheader( obj);
   rc     = (intptr_t) _mulle_atomic_pointer_read( &header->_retaincount_1);

   assert( rc != -1 && rc != INTPTR_MIN);
   if( MULLE_C_LIKELY( rc <= MULLE_OBJC_INLINE_RELEASE))
      return( (intptr_t) _mulle_atomic_pointer_decrement( &header->_retaincount_1) <= 0);

   //
   // this release method call may finalize/dealloc already. So we
   // return 0 as to say no, which might be a lie, but the caller which
   // is NSDecrementExtraRefCountWasZero is probably wanting to call dealloc
   // so it shouldn't with 0
   //
   if( rc == MULLE_OBJC_SLOW_RELEASE)
      mulle_objc_object_call_inline_partial( obj, MULLE_OBJC_RELEASE_METHODID, obj);
   return( 0);
}


// IDEA: make MULLE_OBJC_NEVER_RELEASE call some class callback, this class
//       callback could then possibly call -release. This would be nice for
//       classes that need to interpose -release
//
// compiler should optimize that the mulle_objc_object_try_finalize_try_dealloc
// is rarely taken, but
// MULLE_C_EXPECT( --header->retaincount_1 < 0, 0)
// didnt do anything for me
//
static inline void   _mulle_objc_object_release_inline( void *obj)
{
   if( MULLE_C_UNLIKELY( _mulle_objc_object_decrement_retaincount_waszero( obj)))
      _mulle_objc_object_tryfinalizetrydealloc( obj);
}


//
// IDEA: on transition from 0 -> 1 could "seal" an instance, as this
//       indicates setup is over. So all modifications except retain
//       counting would be invalid. Advantages: can setup as desired
//       with property accessors. Negative bi-directional retains won't
//       work. Manual sealing might be preferable.
//
static inline void   _mulle_objc_object_retain_inline( void *obj)
{
   _mulle_objc_object_increment_retaincount( obj);
}


//
// Try not to use it, it obscures bugs and could disturb leak detection.
// You should only use it when creating an object, not "on the fly"
// therefore it's nonatomic
//
static inline void   _mulle_objc_object_constantify_noatomic( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   _mulle_atomic_pointer_write_nonatomic( &header->_retaincount_1, (void *) MULLE_OBJC_NEVER_RELEASE);
}


//
// only use this when deallocating a placeholder
//
static inline void   _mulle_objc_object_deconstantify_noatomic( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   _mulle_atomic_pointer_write_nonatomic( &header->_retaincount_1, (void *) 0);
}


//
// Don't use this either, except when setting up the object
//
static inline void   _mulle_objc_object_slowify_noatomic( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   _mulle_atomic_pointer_write_nonatomic( &header->_retaincount_1, (void *) MULLE_OBJC_SLOW_RELEASE);
}


static inline void   _mulle_objc_object_deslowify_noatomic( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return;

   header = _mulle_objc_object_get_objectheader( obj);
   _mulle_atomic_pointer_write_nonatomic( &header->_retaincount_1, (void *) 0);
}


static inline int   _mulle_objc_object_is_constant( void *obj)
{
   struct _mulle_objc_objectheader    *header;

   if( mulle_objc_taggedpointer_get_index( obj))
      return( 1);

   header = _mulle_objc_object_get_objectheader( obj);
   return( _mulle_atomic_pointer_read( &header->_retaincount_1) == (void *) MULLE_OBJC_NEVER_RELEASE);
}


# pragma mark - API, compiler uses this code too

// must be void *, for compiler
static inline void   *mulle_objc_object_retain_inline( void *obj)
{
   if( obj)
      _mulle_objc_object_retain_inline( obj);
   return( obj);
}


static inline void   mulle_objc_object_release_inline( void *obj)
{
   if( ! obj)
      return;
   _mulle_objc_object_release_inline( obj);
}


// must be void *, for compiler
MULLE_OBJC_RUNTIME_GLOBAL
void   *mulle_objc_object_retain( void *obj);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_object_release( void *obj);

//
// does _mulle_objc_object_release_inline but with two parameteres
// like mulle-allocator->free
//
MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_object_release2( void *obj, void *unused);

# pragma mark - API


MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_objects_retain( void **objects, size_t n);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_objects_release( void **objects, size_t n);

MULLE_OBJC_RUNTIME_GLOBAL
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
