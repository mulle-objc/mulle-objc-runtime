//
//  mulle-objc-infraclass-reuse.c
//  mulle-objc-runtime
//
//  Copyright (c) 2023 Nat! - Mulle kybernetiK.
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
#include "mulle-objc-infraclass-reuse.h"

#include "include-private.h"



void   *_mulle_objc_infraclass_keep_alloc( struct _mulle_objc_infraclass *infra,
                                           void *alloc,
                                           struct mulle_allocator *allocator)
{
   struct _mulle_objc_reusealloc   *entry = alloc;
   struct _mulle_objc_universe     *universe;
   intptr_t                        instancereuse;
   size_t                          size;

   MULLE_C_ASSERT( sizeof( struct _mulle_objc_reusealloc) <= sizeof( struct _mulle_objc_objectheader));

   if( MULLE_C_UNLIKELY( infra->allocator != allocator))
      return( alloc);

   // check if instancereuse is disabled
   universe      = _mulle_objc_infraclass_get_universe( infra);
   instancereuse = (intptr_t) _mulle_atomic_pointer_read( &universe->instancereuse);
   if( MULLE_C_UNLIKELY( instancereuse == -1))
      return( alloc);

   // though slightly superflous if instance won't get reclaimed, wasting time
   // during free is probably preferable to make reuse faster
   //
   size = _mulle_objc_infraclass_get_allocationsize( infra);
   memset( alloc, 0, size);

   _mulle_concurrent_linkedlist_add( &infra->reuseallocs, &entry->_link);
   return( NULL);
}


//
// MEMO: as instances have been dealloced they are single threaded
//
void   _mulle_objc_infraclass_free_reuseallocs( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_concurrent_linkedlistentry  *next, *entry;

   next = _mulle_concurrent_linkedlist_remove_all( &infra->reuseallocs);
   while( (entry = next))
   {
      next = entry->_next;
      mulle_allocator_free( infra->allocator, entry);
   }
}


void   *_mulle_objc_infraclass_keep_alloc_thread( struct _mulle_objc_infraclass *infra,
                                                  void *alloc,
                                                  struct mulle_allocator *allocator)
{
   struct _mulle_linkedlistentry   *entry = alloc;
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_threadinfo   *threadinfo;
   struct _mulle_linkedlistentry   **head;
   unsigned int                    index;
   size_t                          size;

   MULLE_C_ASSERT( sizeof( struct _mulle_linkedlistentry) <= sizeof( struct _mulle_objc_objectheader));

   universe   = _mulle_objc_infraclass_get_universe( infra);
   threadinfo = _mulle_objc_thread_get_threadinfo( universe);
   if( MULLE_C_UNLIKELY( threadinfo->allocator != allocator))
      return( alloc);

   size = _mulle_objc_infraclass_get_instancesize( infra);
   memset( alloc, 0, size);

   index = _mulle_objc_infraclass_get_classindex( infra);
   head  = (struct _mulle_linkedlistentry **)
            _mulle__pointerarray_get_zeroing_address( &threadinfo->reuseallocsperclassindex,
                                                      index,
                                                      threadinfo->allocator);
   _mulle_linkedlistentry_chain( head, entry);
   return( NULL);
}


//
// MEMO: as instances have been dealloced they are single threaded
//
void   _mulle_objc_infraclass_free_reuseallocs_thread( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_threadinfo   *threadinfo;
   void                            *linkedlist;
   unsigned int                    index;

   universe   = _mulle_objc_infraclass_get_universe( infra);
   threadinfo = _mulle_objc_thread_get_threadinfo( universe);

   index      = _mulle_objc_infraclass_get_classindex( infra);
   if( index < _mulle__pointerarray_get_count( &threadinfo->reuseallocsperclassindex))
   {
      linkedlist = _mulle__pointerarray_get_zeroing_address( &threadinfo->reuseallocsperclassindex,
                                                            index,
                                                            threadinfo->allocator);

      mulle_linkedlistentry_walk( linkedlist,
                                  (mulle_linkedlistentry_walk_callback_t *) mulle_allocator_free,
                                  threadinfo->allocator);
      _mulle__pointerarray_set( &threadinfo->reuseallocsperclassindex, index, NULL);
   }
}

