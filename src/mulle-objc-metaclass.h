//
//  mulle_objc_metaclass.h
//  mulle-objc-runtime
//
//  Created by Nat! on 17/04/07
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
#ifndef mulle_objc_metaclass_h__
#define mulle_objc_metaclass_h__

#include "include.h"

#include "mulle-objc-class-struct.h"
#include "mulle-objc-atomicpointer.h"
#include "mulle-objc-walktypes.h"
#include <mulle-thread/mulle-thread.h>
#include <mulle-concurrent/mulle-concurrent.h>
#include <assert.h>


enum _mulle_objc_metaclass_state
{
   MULLE_OBJC_METACLASS_LOAD_SCHEDULED   = _MULLE_OBJC_CLASS_LOAD_SCHEDULED,
   MULLE_OBJC_METACLASS_IS_MIXIN = MULLE_OBJC_CLASS_IS_MIXIN
};


struct _mulle_objc_metaclass
{
   struct _mulle_objc_class              base;
   mulle_thread_recursive_mutex_t        classpropertylock;  // dormant until initializeSelf
   struct mulle_concurrent_pointerarray  propertylists;
   struct mulle_concurrent_pointerarray  ivarlists;
};


static inline struct _mulle_objc_class   *_mulle_objc_metaclass_as_class( struct _mulle_objc_metaclass *meta)
{
   return( &meta->base);
}


static inline struct _mulle_objc_metaclass   *_mulle_objc_class_as_metaclass( struct _mulle_objc_class *cls)
{
   assert( _mulle_objc_class_is_metaclass( cls));

   return( (struct _mulle_objc_metaclass *) cls);
}


#pragma mark - init


static inline void   _mulle_objc_metaclass_plusinit( struct _mulle_objc_metaclass *meta,
                                                     struct mulle_allocator *allocator)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_objc_metaclass_as_class( meta);
   _mulle_objc_class_set_state_bit( cls, MULLE_OBJC_CLASS_IS_NOT_THREAD_AFFINE);

   // _depth = -1 sentinel: lock is dormant until initializeSelf is called
   _mulle_atomic_pointer_nonatomic_write( &meta->classpropertylock._depth, (void *) -1);

   _mulle_concurrent_pointerarray_init( &meta->propertylists, 0, allocator);
   _mulle_concurrent_pointerarray_init( &meta->ivarlists, 0, allocator);
}


static inline void   _mulle_objc_metaclass_plusdone( struct _mulle_objc_metaclass *meta)
{
   // only destroy lock if it was initialized (depth != -1)
   if( _mulle_atomic_pointer_read( &meta->classpropertylock._depth) != (void *) -1)
      mulle_thread_recursive_mutex_done( &meta->classpropertylock);

   _mulle_concurrent_pointerarray_done( &meta->propertylists);
   _mulle_concurrent_pointerarray_done( &meta->ivarlists);
}


#pragma mark - class property lock

static inline void
   _mulle_objc_metaclass_lock_classproperty( struct _mulle_objc_metaclass *meta)
{
   mulle_thread_recursive_mutex_lock( &meta->classpropertylock);
}

static inline void
   _mulle_objc_metaclass_unlock_classproperty( struct _mulle_objc_metaclass *meta)
{
   mulle_thread_recursive_mutex_unlock( &meta->classpropertylock);
}

static inline int
   _mulle_objc_metaclass_trylock_classproperty( struct _mulle_objc_metaclass *meta)
{
   return( mulle_thread_recursive_mutex_trylock( &meta->classpropertylock));
}


#pragma mark - sanity check

MULLE_OBJC_RUNTIME_GLOBAL
int   mulle_objc_metaclass_is_sane( struct _mulle_objc_metaclass *meta);



# pragma mark - conveniences

static inline struct _mulle_objc_universe   *
   _mulle_objc_metaclass_get_universe( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.universe);
}


static inline mulle_objc_classid_t
   _mulle_objc_metaclass_get_classid( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.classid);
}


static inline char *
   _mulle_objc_metaclass_get_name( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.name);
}

// this can be either an infraclass or a metaclass
static inline struct _mulle_objc_class *
   _mulle_objc_metaclass_get_superclass( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.superclass);
}


static inline unsigned int
   _mulle_objc_metaclass_get_inheritance( struct _mulle_objc_metaclass *meta)
{
   return( meta->base.inheritance);
}


static inline void
   _mulle_objc_metaclass_set_inheritance( struct _mulle_objc_metaclass *meta,
                                          unsigned int inheritance)
{
   assert( (unsigned short) inheritance == inheritance);

   meta->base.inheritance = (unsigned short) inheritance;
}


static inline int
   _mulle_objc_metaclass_set_state_bit( struct _mulle_objc_metaclass *meta,
                                        enum _mulle_objc_metaclass_state bit)
{
   return( _mulle_objc_class_set_state_bit( &meta->base, (unsigned int) bit));
}


static inline unsigned int
   _mulle_objc_metaclass_get_state_bit( struct _mulle_objc_metaclass *meta,
                                        enum _mulle_objc_metaclass_state bit)
{
   return( _mulle_objc_class_get_state_bit( &meta->base, (unsigned int) bit));
}

#pragma mark -

struct _mulle_objc_searchargumentscachable;

static inline mulle_objc_implementation_t
   _mulle_objc_metaclass_lookup_superimplementation( struct _mulle_objc_metaclass *meta,
                                                     mulle_objc_superid_t superid)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   mulle_objc_implementation_t
      _mulle_objc_class_lookup_superimplementation_nofail( struct _mulle_objc_class *cls,
                                                           mulle_objc_superid_t superid);

   return( _mulle_objc_class_lookup_superimplementation_nofail( &meta->base, superid));
}


static inline void
   mulle_objc_metaclass_add_methodlist_nofail( struct _mulle_objc_metaclass *meta,
                                               struct _mulle_objc_methodlist *list)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   void   mulle_objc_class_add_methodlist_nofail( struct _mulle_objc_class *cls,
                                                  struct _mulle_objc_methodlist *list);

   mulle_objc_class_add_methodlist_nofail( &meta->base, list);
}


static inline struct _mulle_objc_method  *
    mulle_objc_metaclass_defaultsearch_method( struct _mulle_objc_metaclass *meta,
                                               mulle_objc_methodid_t methodid)
{
   MULLE_OBJC_RUNTIME_GLOBAL
   struct _mulle_objc_method   *
      _mulle_objc_class_defaultsearch_method( struct _mulle_objc_class *cls,
                                              mulle_objc_methodid_t methodid,
                                              int *error);
   int   error;

   return( _mulle_objc_class_defaultsearch_method( &meta->base, methodid, &error));
}



MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_walkcommand_t
   mulle_objc_metaclass_walk( struct _mulle_objc_metaclass   *meta,
                              enum mulle_objc_walkpointertype_t  type,
                              mulle_objc_walkcallback_t   callback,
                              void *parent,
                              void *userinfo);


#pragma mark - class properties

MULLE_OBJC_RUNTIME_GLOBAL
int   mulle_objc_metaclass_add_propertylist( struct _mulle_objc_metaclass *meta,
                                             struct _mulle_objc_propertylist *list);

MULLE_OBJC_RUNTIME_GLOBAL
void  mulle_objc_metaclass_add_propertylist_nofail( struct _mulle_objc_metaclass *meta,
                                                    struct _mulle_objc_propertylist *list);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_property *
   mulle_objc_metaclass_search_property( struct _mulle_objc_metaclass *meta,
                                         mulle_objc_propertyid_t propertyid);


#pragma mark - class variables

MULLE_OBJC_RUNTIME_GLOBAL
int   mulle_objc_metaclass_add_ivarlist( struct _mulle_objc_metaclass *meta,
                                         struct _mulle_objc_ivarlist *list);

MULLE_OBJC_RUNTIME_GLOBAL
void  mulle_objc_metaclass_add_ivarlist_nofail( struct _mulle_objc_metaclass *meta,
                                                struct _mulle_objc_ivarlist *list);

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_ivar *
   mulle_objc_metaclass_search_ivar( struct _mulle_objc_metaclass *meta,
                                     mulle_objc_ivarid_t ivarid);


#endif /* mulle_objc_metaclass_h */
