//
//  mulle_objc_callqueue.h
//  mulle-objc-runtime
//
//  Created by Nat! on 20/11/14.
//  Copyright (c) 2014 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014 Codeon GmbH.
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
#ifndef mulle_objc_callqueue_h__
#define mulle_objc_callqueue_h__

#include "mulle-objc-method.h"

#include "dependencies.h"


struct _mulle_objc_object;

// this is a  growing storage mechanism
// to store imps with objc and a method id (no parameters)
//
// ok, ok basically just +load calls ;)
//
// Should probably be rewritten to not use concurrent,
// now that +load is locked anyway, but that's what we have here
// in the universe.
//
struct _mulle_objc_callqueue
{
   struct mulle_concurrent_pointerarray   list;
};


# pragma mark  -
# pragma mark init/destroy


int   mulle_objc_callqueue_init( struct _mulle_objc_callqueue *p, struct mulle_allocator *allocator);
void   mulle_objc_callqueue_done( struct _mulle_objc_callqueue *p);


# pragma mark  -
# pragma mark functionality

int   mulle_objc_callqueue_add( struct _mulle_objc_callqueue *p,
                                struct _mulle_objc_object *obj,
                                mulle_objc_methodid_t methodid,
                                mulle_objc_implementation_t imp);


void   mulle_objc_callqueue_execute( struct _mulle_objc_callqueue *p);

void   mulle_objc_callqueue_walk( struct _mulle_objc_callqueue *queue,
                                  void (*callback)( struct _mulle_objc_object *obj,
                                                    mulle_objc_methodid_t methodid,
                                                    mulle_objc_implementation_t imp,
                                                    void *userinfo),
                                  void *userinfo);


#endif
