//
//  mulle_objc_version.h
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
#ifndef mulle_objc_version_h__
#define mulle_objc_version_h__

#include "include.h"

#include <stdint.h>

//
// up the major if the compiler output is incompatible
// up the minor for added features
// up the patch for bugfixes
//
// Change the values below also to match.
//
// *** DONT FORGET TO EDIT mulle-objc-jit.inc TOO***
//
#define MULLE_OBJC_RUNTIME_VERSION  ((0UL << 20) | (23 << 8) | 0)

//
// these three values are read by the compiler(!)
// only use integers and no expressions
//
#define MULLE_OBJC_RUNTIME_VERSION_MAJOR  0   // max 511
#define MULLE_OBJC_RUNTIME_VERSION_MINOR  23  // max 1023
#define MULLE_OBJC_RUNTIME_VERSION_PATCH  0   // max 255


static inline unsigned int  mulle_objc_version_get_major( uint32_t version)
{
   assert( (MULLE_OBJC_RUNTIME_VERSION >> 20) == MULLE_OBJC_RUNTIME_VERSION_MAJOR);

   return( (unsigned int) (version >> 20));
}


static inline unsigned int  mulle_objc_version_get_minor( uint32_t version)
{
   assert( ((MULLE_OBJC_RUNTIME_VERSION >> 8) & (1024 - 1)) == MULLE_OBJC_RUNTIME_VERSION_MINOR);

   return( (unsigned int) ((version >> 8) & (1024 - 1)));
}


static inline unsigned int  mulle_objc_version_get_patch( uint32_t version)
{
   assert( (MULLE_OBJC_RUNTIME_VERSION_PATCH & 255) == MULLE_OBJC_RUNTIME_VERSION_PATCH);

   return( (unsigned int) (version & 255));
}

#endif
