//
//  mulle_objc_lldb.h
//  mulle-objc-runtime-universe
//
//  Created by Nat! on 16.05.17.
//  Copyright © 2017 Mulle kybernetiK. All rights reserved.
//  Copyright © 2017 Codeon GmbH. All rights reserved.
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
#ifndef mulle_objc_lldb_h__
#define mulle_objc_lldb_h__

#include "mulle-objc-uniqueid.h"
#include "mulle-objc-method.h"


void   $__lldb_objc_object_check( void *obj, mulle_objc_methodid_t methodid);
void   mulle_objc_lldb_check_object( void *obj, mulle_objc_methodid_t methodid);

struct _mulle_objc_descriptor  *
   mulle_objc_lldb_lookup_descriptor_by_name( char *name);

struct mulle_objc_lldb_lookup_implementation_args
{
   void   *class_or_superid;
   int    calltype;
   int    debug;
};

mulle_objc_implementation_t
   mulle_objc_lldb_lookup_implementation( void *object,
                                          mulle_objc_methodid_t sel,
                                          struct mulle_objc_lldb_lookup_implementation_args *args);

struct _mulle_objc_class *
   mulle_objc_lldb_lookup_isa( void *obj, int debug);

void   *mulle_objc_lldb_get_dangerous_classstorage_pointer( void);


void   *mulle_objc_lldb_create_staticstring( void *cfalloc,
                                             uint8_t *bytes,
                                             intptr_t numBytes,
                                             int encoding,
                                             char isExternalRepresentation);
#endif /* mulle_objc_lldb_h */


