//
//  mulle-objc-universe-dll-loader.c
//  mulle-objc-runtime
//
//  Copyright (c) 2026 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
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
#ifndef mulle_objc_universe_dll_loader_h__
#define mulle_objc_universe_dll_loader_h__

#include "include.h"

#include "mulle-objc-uniqueid.h"

struct _mulle_objc_universe;


//
// This is not really 'API' if you feel this is too limited or does the
// wrong thing, create a list of DLLs you _want_ to load and then do it
// yourself in 'main' before the first objc call.
//
#ifdef _WIN32

typedef int   mulle_objc_dll_filter_t( char *filename);

// returns -1, if nothing was loaded. 0 if something was loaded
// this only runs and does nothing for other invocations, also doesnt
// block
MULLE_OBJC_RUNTIME_GLOBAL
int  _mulle_objc_universe_dll_loader( struct _mulle_objc_universe *universe,
                                       char *dllpath);

// returns -1, if nothing was loaded. 0 if something was loaded
// this does not only "run" once
MULLE_OBJC_RUNTIME_GLOBAL
int  _mulle_objc_universe_dll_loader_filtered( struct _mulle_objc_universe *universe,
                                               char *dllpath,
                                               mulle_objc_dll_filter_t *filter);
#endif

#endif
