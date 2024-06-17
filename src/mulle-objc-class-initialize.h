//
//  mulle-objc-class-initialize.c
//  mulle-objc-runtime
//
//  Copyright (c) 2021 Nat! - Mulle kybernetiK.
//  Copyright (c) 2021 Codeon GmbH.
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
#ifndef mulle_objc_class_initialize_h__
#define mulle_objc_class_initialize_h__

#include "include.h"


struct _mulle_objc_class;
struct _mulle_objc_impcache;
struct _mulle_objc_infraclass;
struct _mulle_objc_impcache_callback;

//
// You are likely better off using [self self] to force unlazing a
// class as its more portable and readable
//
MULLE_OBJC_RUNTIME_GLOBAL
int  _mulle_objc_class_setup( struct _mulle_objc_class *cls);


MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_class_warn_recursive_initialize( struct _mulle_objc_class *cls);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_infraclass_call_deinitialize( struct _mulle_objc_infraclass *infra);

// returns 0 if cache was already set up by someone else
MULLE_OBJC_RUNTIME_GLOBAL
int    _mulle_objc_class_setup_initial_cache_if_needed( struct _mulle_objc_class *cls,
                                                        struct _mulle_objc_impcache_callback *callback);

#endif
