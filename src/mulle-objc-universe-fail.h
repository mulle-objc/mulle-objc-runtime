//
//  mulle-objc-universe-fail.h
//  mulle-objc-runtime
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014-2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014-2018 Codeon GmbH.
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

#ifndef mulle_objc_universe_fail_h__
#define mulle_objc_universe_fail_h__


#include "include.h"

#include "mulle-objc-universe-struct.h"

#include <errno.h>
#include <stdarg.h>


#pragma mark - fails, universe can be null

MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_code( struct _mulle_objc_universe *universe,
                                  int errnocode);

static inline MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_errno( struct _mulle_objc_universe *universe)
{
   mulle_objc_universe_fail_code( universe, errno);
}


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_perror( struct _mulle_objc_universe *universe, char *s);

MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_generic( struct _mulle_objc_universe *universe,
                                     char *format, ...);
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_failv_generic( struct _mulle_objc_universe *universe,
                                      char *format,
                                      va_list args);
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_inconsistency( struct _mulle_objc_universe *universe,
                                           char *format, ...);
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_failv_inconsistency( struct _mulle_objc_universe *universe,
                                            char *format,
                                            va_list args);

#pragma mark - fails

MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_classnotfound( struct _mulle_objc_universe *universe,
                                           mulle_objc_classid_t classid);
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_supernotfound( struct _mulle_objc_universe *universe,
                                           mulle_objc_superid_t superid);
MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   mulle_objc_universe_fail_methodnotfound( struct _mulle_objc_universe *universe,
                                            struct _mulle_objc_class *class,
                                            mulle_objc_methodid_t methodid);
MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_universe_init_fail( struct _mulle_objc_universe  *universe);

#endif
