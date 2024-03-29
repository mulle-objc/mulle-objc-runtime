//
//  mulle-objc-try-catch-finally.h
//  mulle-objc-runtime
//
//  Created by Nat! on 22/01/16.
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
#ifndef mulle_objc_try_catch_finally_h__
#define mulle_objc_try_catch_finally_h__

#include "include.h"  // for alignment mulle_objc_vararg.hcode

#include "mulle-objc-class.h"
#include "mulle-objc-uniqueid.h"
#include <setjmp.h>

//
// the naming is pretty much consistent with those of Apple
// for better or worse
// these methods are known as builtins by the compiler
//
MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_exception_throw( void *exception,
                                   mulle_objc_universeid_t universe);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_exception_tryenter( void *localExceptionData,
                                      mulle_objc_universeid_t universe);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_exception_tryexit( void *localExceptionData,
                                     mulle_objc_universeid_t universe);

MULLE_OBJC_RUNTIME_GLOBAL
void   *mulle_objc_exception_extract( void *localExceptionData,
                                      mulle_objc_universeid_t universe);

MULLE_OBJC_RUNTIME_GLOBAL
int    _mulle_objc_exception_match( void *exception,
                                    mulle_objc_universeid_t universe,
                                    mulle_objc_classid_t classid);


static inline int  mulle_objc_exception_match( void *exception,
                                               mulle_objc_universeid_t universeid,
                                               mulle_objc_classid_t classid)
{
   // short circuit, if classid is NSException
   if( classid == MULLE_OBJC_CLASSID( 0xa41284db))
      return( 1);
   return( _mulle_objc_exception_match( exception, universeid, classid));
}

#endif
