//
//  mulle_objc_builtin.h
//  mulle-objc
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

#include "mulle_objc_call.h"
#include "mulle_objc_retain_release.h"

#include <assert.h>


# pragma mark -
# pragma mark compiler support

static inline void   *mulle_objc_object_copy( void *self)
{
   return( mulle_objc_object_inline_constant_methodid_call( self, MULLE_OBJC_COPY_METHODID, self));
}


//
// this is used by some property code
//
static inline void   *mulle_objc_object_autorelease( void *self)
{
   return( mulle_objc_object_inline_constant_methodid_call( self, MULLE_OBJC_AUTORELEASE_METHODID, self));
}


//
// this is called by the compiler for retain or copy of objects only
//
static inline void   mulle_objc_object_set_property_value(
                              void *self,
                              mulle_objc_methodid_t _cmd,
                              ptrdiff_t offset,
                              struct _mulle_objc_object *value,
                              char is_atomic,
                              char is_copy)
{
   void  **ivar;
   
   assert( ! is_atomic);

   if( ! self)
      return;

   ivar = (void **) &((char *) self)[ offset];
   if( is_copy)
      value = mulle_objc_object_copy( value);
   else
      mulle_objc_object_retain( value);
   
   mulle_objc_object_autorelease( *ivar);
   *ivar = value;
}


static inline void   *mulle_objc_object_get_property_value(
                              void *self,
                              mulle_objc_methodid_t _cmd,
                              ptrdiff_t offset,
                              char is_atomic)
{
   void  **ivar;
   
   assert( ! is_atomic);
   
   if( ! self)
      return( NULL);
   
   ivar = (void **) &((char *) self)[ offset];
   return( *ivar);
}

#endif
