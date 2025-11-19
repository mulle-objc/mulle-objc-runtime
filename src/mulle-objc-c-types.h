//
//  mulle-objc-c-types.h
//  mulle-objc-runtime
//
//  Copyright (c) 2025 Nat! - Mulle kybernetiK.
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
#ifndef mulle_objc_c_types_h__
#define mulle_objc_c_types_h__

// THIS FILE NEEDS TO BE FIRST IN "include.h"
//
// These are minimal runtime definitions for MulleObjC.
// So that MulleObjC doesn't have to expose the complete API
// and pollute the completion space. Not quite there yet...
// They are therefore safe to be included by .c files without the need to
// define __MULLE_TPS__ and friends.
//
#include <mulle-c11/mulle-c11.h>
#include <mulle-c11/mulle-c11-bool.h>     // BOOL
#include <mulle-c11/mulle-c11-integer.h>  // NSInteger, NSUInteger

// keep after mulle-c11, we want to hit system headers as late as possible
#include <stddef.h>
#include <stdint.h>

typedef uint32_t   mulle_objc_classid_t;
typedef uint32_t   mulle_objc_methodid_t;
typedef uint32_t   mulle_objc_protocolid_t;
typedef uint32_t   mulle_objc_universeid_t;

struct _mulle_objc_infraclass;
struct _mulle_objc_ivar;
struct _mulle_objc_method;
struct _mulle_objc_universe;

#endif
