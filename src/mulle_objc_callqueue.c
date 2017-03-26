//
//  mulle_objc_callqueue.c
//  mulle-objc
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
#include "mulle_objc_callqueue.h"

#include "mulle_objc_runtime.h"

#include <mulle_allocator/mulle_allocator.h>
#include <stdlib.h>
#include <errno.h>


struct _mulle_objc_object;

struct _queue_entry
{
   mulle_atomic_pointer_t              obj;
   mulle_objc_methodid_t               uniqueid;
   mulle_objc_methodimplementation_t   imp;
};


static inline void   queue_entry_set( struct _queue_entry *q,
                                      struct _mulle_objc_object *obj,
                                      mulle_objc_methodid_t methodid,
                                      mulle_objc_methodimplementation_t imp)
{
   _mulle_atomic_pointer_nonatomic_write( &q->obj, obj);
   q->uniqueid = methodid;
   q->imp      = imp;
}


// ensure each entry is only executed once
static inline void   queue_entry_execute( struct _queue_entry *q)
{
   struct _mulle_objc_object *obj;

   obj = _mulle_atomic_pointer_read( &q->obj);
   if( obj == MULLE_CONCURRENT_INVALID_POINTER)
      return;
   if( ! _mulle_atomic_pointer_compare_and_swap( &q->obj, MULLE_CONCURRENT_INVALID_POINTER, obj))
      return;

   (*q->imp)( obj, q->uniqueid, q->imp);
}


#pragma mark - mulle_objc_callqueue


int   mulle_objc_callqueue_init( struct _mulle_objc_callqueue *queue, struct mulle_allocator *allocator)
{
   if( ! queue || ! allocator)
   {
      errno = EINVAL;
      return( -1);
   }

   _mulle_concurrent_pointerarray_init( &queue->list, 32, allocator);
   return( 0);
}


int   mulle_objc_callqueue_add( struct _mulle_objc_callqueue *queue,
                                struct _mulle_objc_object *obj,
                                mulle_objc_methodid_t methodid,
                                mulle_objc_methodimplementation_t imp)
{
   struct _queue_entry   *entry;

   if( ! queue || ! obj || ! imp || methodid == MULLE_OBJC_NO_METHODID || methodid == MULLE_OBJC_INVALID_METHODID)
   {
      errno = EINVAL;
      return( -1);
   }
   entry = _mulle_allocator_calloc( queue->list.allocator, 1, sizeof( struct _queue_entry));

   queue_entry_set( entry, obj, methodid, imp);
   _mulle_concurrent_pointerarray_add( &queue->list, entry);
   return( 0);
}


void   mulle_objc_callqueue_execute( struct _mulle_objc_callqueue *queue)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _queue_entry                              *entry;

   rover = mulle_concurrent_pointerarray_enumerate( &queue->list);
   while( entry = _mulle_concurrent_pointerarrayenumerator_next( &rover))
      queue_entry_execute( entry);
   mulle_concurrent_pointerarrayenumerator_done( &rover);
}


void   mulle_objc_callqueue_done( struct _mulle_objc_callqueue *queue)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _queue_entry                              *entry;

   rover = mulle_concurrent_pointerarray_enumerate( &queue->list);
   while( entry = _mulle_concurrent_pointerarrayenumerator_next( &rover))
      _mulle_allocator_abafree( queue->list.allocator, entry);
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   _mulle_concurrent_pointerarray_done( &queue->list);
}

