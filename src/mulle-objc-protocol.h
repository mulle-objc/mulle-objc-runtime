//
//  mulle_objc_protocol.h
//  mulle-objc-runtime
//
//  Created by Nat! on 30.05.17
//  Copyright (c) 2017 Nat! - Mulle kybernetiK.
//  Copyright (c) 2017 Codeon GmbH.
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
#ifndef mulle_objc_protocol_h__
#define mulle_objc_protocol_h__

#include "include.h"

#include "mulle-objc-uniqueid.h"



//
// this structure is emitted by the compiler and saved in the universe
// the protocolname is there to catch possible protocolid collisions
// otherwise struct _mulle_objc_protocol is unused
struct _mulle_objc_protocol
{
   mulle_objc_protocolid_t    protocolid;
   char                       *name;
};


# pragma mark - method petty accessors

static inline char   *_mulle_objc_protocol_get_name( struct _mulle_objc_protocol *protocol)
{
   return( protocol->name);
}


static inline mulle_objc_protocolid_t  _mulle_objc_protocol_get_protocolid( struct _mulle_objc_protocol *protocol)
{
   return( protocol->protocolid);
}


#pragma mark - bsearch

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_protocol   *_mulle_objc_protocol_bsearch( struct _mulle_objc_protocol *buf,
                                                             unsigned int n,
                                                             mulle_objc_protocolid_t search);

#pragma mark - qsort

MULLE_OBJC_RUNTIME_GLOBAL
int  _mulle_objc_protocol_compare( struct _mulle_objc_protocol *a, struct _mulle_objc_protocol *b);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_protocol_sort( struct _mulle_objc_protocol *protocols,
                                 unsigned int n);

MULLE_OBJC_RUNTIME_GLOBAL
int  mulle_objc_protocol_is_sane( struct _mulle_objc_protocol *p);

#endif
