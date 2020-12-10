//
//  mulle_objc_try-catch-finally.c
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
#include "mulle-objc-try-catch-finally.h"

#include "mulle-objc-universe.h"

//
// these functions vector throught the universe,
// usually into a Foundation
//
static void   objc_exception_throw( void *exception, struct _mulle_objc_universe *universe)  // familar name
{
   universe->exceptionvectors.throw( universe, exception);
}


void   mulle_objc_exception_throw( void *exception, mulle_objc_classid_t universeid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( universeid);
   objc_exception_throw( exception, universe);
}


void   mulle_objc_exception_tryenter( void *localExceptionData, mulle_objc_classid_t universeid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( universeid);
   universe->exceptionvectors.try_enter( universe, localExceptionData);
}


void   mulle_objc_exception_tryexit( void *localExceptionData, mulle_objc_classid_t universeid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( universeid);
   universe->exceptionvectors.try_exit( universe, localExceptionData);
}


void   *mulle_objc_exception_extract( void *localExceptionData, mulle_objc_classid_t universeid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( universeid);
   return( universe->exceptionvectors.extract( universe, localExceptionData));
}


int   _mulle_objc_exception_match( void *exception, mulle_objc_classid_t universeid, mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe_inline( universeid);
   return( universe->exceptionvectors.match( universe, classid, exception));
}

