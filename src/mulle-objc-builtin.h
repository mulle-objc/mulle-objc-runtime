//
//  mulle_objc_builtin.h
//  mulle-objc-runtime
//
//  Created by Nat! on 28.01.16.
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
#ifndef mulle_objc_builtin_h__
#define mulle_objc_builtin_h__

#include "include.h"

#include "mulle-objc-call.h"
#include "mulle-objc-retain-release.h"

#include <assert.h>

# pragma mark - compiler support

//
// use partial inline calls, to reduce code size
// for accessors, calls.
// TODO: code a generic mulle_objc_object_call function that uses various
//       callers dependening on optimization setting
//
static inline void   *mulle_objc_object_call_copy( void *self)
{
   return( mulle_objc_object_call_inline_partial( self,
                                                  MULLE_OBJC_COPY_METHODID,
                                                  self));
}


static inline void   *mulle_objc_object_call_mutablecopy( void *self)
{
   return( mulle_objc_object_call_inline_partial( self,
                                                  MULLE_OBJC_MUTABLECOPY_METHODID,
                                                  self));
}


static inline void   *mulle_objc_object_call_mulle_allocator( void *self)
{
   return( mulle_objc_object_call_inline_partial( self,
                                                  MULLE_OBJC_MULLE_ALLOCATOR_METHODID,
                                                  self));
}


//
// this is used by some property code
//
static inline void   *mulle_objc_object_call_autorelease( void *self)
{
   return( mulle_objc_object_call_inline_partial( self,
                                                  MULLE_OBJC_AUTORELEASE_METHODID,
                                                  self));
}


static inline void   mulle_objc_object_call_willchange( void *self)
{
   mulle_objc_object_call_inline_partial( self,
                                          MULLE_OBJC_WILLCHANGE_METHODID,
                                          self);
}


static inline void   *mulle_objc_object_call_willreadrelationship( void *self,
                                                                   void *value)
{
   return( mulle_objc_object_call_inline_partial( self,
                                                  MULLE_OBJC_WILLREADRELATIONSHIP_METHODID,
                                                  value));
}


static inline void   mulle_objc_object_call_addobject( void *self, void *value)
{
   mulle_objc_object_call_inline_partial( self,
                                          MULLE_OBJC_ADDOBJECT_METHODID,
                                          value);
}


static inline void   mulle_objc_object_call_removeobject( void *self, void *value)
{
   mulle_objc_object_call_inline_partial( self,
                                          MULLE_OBJC_REMOVEOBJECT_METHODID,
                                          value);
}


//
// this is called by the compiler for retain or copy of objects only
// TODO: conceivably move this out of the runtime. This should optimize
// nicely, as is_copy/is_atomic are usually constants
//
static inline void
   mulle_objc_object_set_property_value( void *self,
                                         mulle_objc_methodid_t _cmd,
                                         ptrdiff_t offset,
                                         struct _mulle_objc_object *value,
                                         char is_atomic,
                                         char is_copy)
{
   void   **p_ivar;
   void   *old;

   if( ! self)
      return;

   if( is_copy)
   {
      if( is_copy == 2)  // da apple way
         value = mulle_objc_object_call_mutablecopy( value);
      else
         value = mulle_objc_object_call_copy( value);
   }
   else
      mulle_objc_object_retain( value);

   assert( ! value || mulle_objc_object_call_mulle_allocator( value) ==
                      mulle_objc_object_call_mulle_allocator( self));
   p_ivar = (void **) &((char *) self)[ offset];
   if( is_atomic)
   {
      old = _mulle_atomic_pointer_set( (mulle_atomic_pointer_t *) p_ivar, value);
      mulle_objc_object_call_autorelease( old);
   }
   else
   {
      mulle_objc_object_call_autorelease( *p_ivar);
      *p_ivar = value;
   }
}


static inline void   *
   mulle_objc_object_get_property_value( void *self,
                                         mulle_objc_methodid_t _cmd,
                                         ptrdiff_t offset,
                                         char is_atomic)
{
   void  **p_ivar;

   if( ! self)
      return( NULL);

   p_ivar = (void **) &((char *) self)[ offset];
   if( is_atomic)
      return( _mulle_atomic_pointer_read( (mulle_atomic_pointer_t *) p_ivar));
   return( *p_ivar);
}


static inline void   mulle_objc_object_add_to_container( void *self,
                                                         ptrdiff_t offset,
                                                         void *value)
{
   void   **p_ivar;

   p_ivar = (void **) &((char *) self)[ offset];
   assert( ! value || mulle_objc_object_call_mulle_allocator( value) ==
                      mulle_objc_object_call_mulle_allocator( self));
   mulle_objc_object_call_addobject( *p_ivar, value);
}


static inline void   mulle_objc_object_remove_from_container( void *self,
                                                              ptrdiff_t offset,
                                                              void *value)
{
   void   **p_ivar;

   p_ivar = (void **) &((char *) self)[ offset];
   mulle_objc_object_call_removeobject( *p_ivar, value);
}


static inline void   mulle_objc_object_will_change( void *self)
{
   mulle_objc_object_call_willchange( self);
}


static inline void   mulle_objc_object_will_read_relationship( void *self,
                                                               ptrdiff_t offset)
{
   void   **p_ivar;

   p_ivar  = (void **) &((char *) self)[ offset];
   *p_ivar = mulle_objc_object_call_willreadrelationship( self, *p_ivar);
}

#endif
