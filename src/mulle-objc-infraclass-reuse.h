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
#ifndef mulle_objc_infraclass_reuse_h__
#define mulle_objc_infraclass_reuse_h__

#include "mulle-objc-infraclass.h"
#include "mulle-objc-classpair.h"

#include "include.h"

#include "mulle-objc-universe.h"


// turn off with zero. code will exist but won't be used by instance
// allocator
#define MULLE_OBJC_CLASS_REUSE_ALLOC_NONE    0
#define MULLE_OBJC_CLASS_REUSE_ALLOC_THREAD  1
#define MULLE_OBJC_CLASS_REUSE_ALLOC_GLOBAL  2


#ifndef MULLE_OBJC_CLASS_REUSE_ALLOC
# define MULLE_OBJC_CLASS_REUSE_ALLOC   MULLE_OBJC_CLASS_REUSE_ALLOC_THREAD
#endif



struct _mulle_objc_reusealloc
{
   struct _mulle_concurrent_linkedlistentry   _link;
   // struct mulle_allocator                     *allocator;
};


//
// reuse instances, previously freed
//
static inline void   *_mulle_objc_infraclass_reuse_alloc( struct _mulle_objc_infraclass *infra,
                                                          size_t size,
                                                          struct mulle_allocator *allocator)
{
   struct _mulle_objc_reusealloc   *entry;

   if( MULLE_C_UNLIKELY( infra->allocator != allocator))
      return( NULL);
   if( MULLE_C_UNLIKELY( size != _mulle_objc_infraclass_get_allocationsize( infra)))
      return( NULL);

   entry = (struct _mulle_objc_reusealloc *) _mulle_concurrent_linkedlist_remove_one( &infra->reuseallocs);
   return( entry);
}


MULLE_OBJC_RUNTIME_GLOBAL
void   *_mulle_objc_infraclass_keep_alloc( struct _mulle_objc_infraclass *infra,
                                           void *alloc,
                                           struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_infraclass_free_reuseallocs( struct _mulle_objc_infraclass *infra);



static inline void  *_mulle_objc_infraclass_reuse_alloc_thread( struct _mulle_objc_infraclass *infra,
                                                                size_t size,
                                                                struct mulle_allocator *allocator)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_threadinfo   *threadinfo;
   struct _mulle_linkedlistentry   **head;
   unsigned int                    index;
   void                            *alloc;

   if( MULLE_C_UNLIKELY( size != _mulle_objc_infraclass_get_allocationsize( infra)))
      return( NULL);

   universe   = _mulle_objc_infraclass_get_universe( infra);
   threadinfo = _mulle_objc_thread_get_threadinfo( universe);
   if( MULLE_C_UNLIKELY( threadinfo->allocator != allocator))
      return( NULL);

   assert( sizeof( void *) >= sizeof( struct _mulle_linkedlistentry *));
   assert( sizeof( void **) >= sizeof( struct _mulle_linkedlistentry **));

   index = _mulle_objc_infraclass_get_classindex( infra);
   head  = (struct _mulle_linkedlistentry **)
               _mulle__pointerarray_get_zeroing_address( &threadinfo->reuseallocsperclassindex,
                                                         index,
                                                         threadinfo->allocator);
   alloc = _mulle_linkedlistentry_unchain( head);
   return( alloc);
}


MULLE_OBJC_RUNTIME_GLOBAL
void   *_mulle_objc_infraclass_keep_alloc_thread( struct _mulle_objc_infraclass *infra,
                                                  void *alloc,
                                                  struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_infraclass_free_reuseallocs_thread( struct _mulle_objc_infraclass *infra);


#endif
