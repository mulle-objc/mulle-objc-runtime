//
//  mulle_objc_infraclass.h
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
#ifndef mulle_objc_infraclass_h__
#define mulle_objc_infraclass_h__

#include "mulle-objc-atomicpointer.h"
#include "mulle-objc-class-struct.h"
#include "mulle-objc-walktypes.h"

#include "include.h"
#include <assert.h>


struct _mulle_objc_ivar;
struct _mulle_objc_ivarlist;
struct _mulle_objc_property;
struct _mulle_objc_method;


enum _mulle_objc_infraclass_state
{
   MULLE_OBJC_INFRACLASS_HAS_CLEARABLE_PROPERTY = _MULLE_OBJC_CLASS_HAS_CLEARABLE_PROPERTY,
   MULLE_OBJC_INFRACLASS_WARN_PROTOCOL          = _MULLE_OBJC_CLASS_WARN_PROTOCOL,
   MULLE_OBJC_INFRACLASS_IS_PROTOCOLCLASS       = _MULLE_OBJC_CLASS_IS_PROTOCOLCLASS,
   MULLE_OBJC_INFRACLASS_INITIALIZING           = MULLE_OBJC_CLASS_INITIALIZING,
   MULLE_OBJC_INFRACLASS_INITIALIZE_DONE        = MULLE_OBJC_CLASS_INITIALIZE_DONE,
   MULLE_OBJC_INFRACLASS_FINALIZE_DONE          = MULLE_OBJC_CLASS_FINALIZE_DONE
};


enum _mulle_objc_infraclass_placeholder_index
{
   MULLE_OBJC_INFRACLASS_CLASSCLUSTER_INDEX = 0,
   MULLE_OBJC_INFRACLASS_INSTANTIATE_INDEX  = 1,
   MULLE_OBJC_INFRACLASS_SINGLETON_INDEX    = 2
};


struct _mulle_objc_infraclass
{
   struct _mulle_objc_class                  base;

   mulle_objc_hash_t                         ivarhash;
   mulle_atomic_pointer_t                    taggedpointerindex;
   mulle_atomic_pointer_t                    coderversion; // for NSCoder

   struct mulle_concurrent_pointerarray      ivarlists;
   struct mulle_concurrent_pointerarray      propertylists;

#if 0
   struct mulle_concurrent_hashmap           cvars;
#endif
   // various placeholders
   union _mulle_objc_atomicobjectpointer_t   placeholders[ 3];

   struct mulle_allocator                    *allocator;  // must not be NULL
   mulle_atomic_pointer_t                    allocatedInstances;
};


void   _mulle_objc_infraclass_plusinit( struct _mulle_objc_infraclass *infra,
                                        struct mulle_allocator *allocator);
void   _mulle_objc_infraclass_plusdone( struct _mulle_objc_infraclass *infra);



static inline struct _mulle_objc_class   *
   _mulle_objc_infraclass_as_class( struct _mulle_objc_infraclass *infra)
{
   return( &infra->base);
}


static inline struct _mulle_objc_infraclass   *
   _mulle_objc_class_as_infraclass( struct _mulle_objc_class *cls)
{
   assert( _mulle_objc_class_is_infraclass( cls));

   return( (struct _mulle_objc_infraclass *) cls);
}


# pragma mark - conveniences

static inline struct _mulle_objc_universe   *
   _mulle_objc_infraclass_get_universe( struct _mulle_objc_infraclass *infra)
{
   return( infra->base.universe);
}


static inline mulle_objc_classid_t
   _mulle_objc_infraclass_get_classid( struct _mulle_objc_infraclass *infra)
{
   return( infra->base.classid);
}


static inline char   *
   _mulle_objc_infraclass_get_name( struct _mulle_objc_infraclass *infra)
{
   return( infra->base.name);
}


static inline struct _mulle_objc_infraclass   *
   _mulle_objc_infraclass_get_superclass( struct _mulle_objc_infraclass *infra)
{
   return( (struct _mulle_objc_infraclass *) infra->base.superclass);
}


static inline struct _mulle_objc_infraclass   *
   mulle_objc_infraclass_get_superclass( struct _mulle_objc_infraclass *infra)
{
   return( infra ?  (struct _mulle_objc_infraclass *) infra->base.superclass : NULL);
}


static inline unsigned int
   _mulle_objc_infraclass_get_inheritance( struct _mulle_objc_infraclass *infra)
{
   return( infra->base.inheritance);
}


static inline void
   _mulle_objc_infraclass_set_inheritance( struct _mulle_objc_infraclass *infra,
                                           unsigned int inheritance)
{
   assert( (unsigned short) inheritance == inheritance);

   infra->base.inheritance = (unsigned short) inheritance;
}


static inline int
   _mulle_objc_infraclass_set_state_bit( struct _mulle_objc_infraclass *infra,
                                         enum _mulle_objc_infraclass_state bit)
{
   return( _mulle_objc_class_set_state_bit( &infra->base, bit));
}


static inline unsigned int
   _mulle_objc_infraclass_get_state_bit( struct _mulle_objc_infraclass *infra,
                                         enum _mulle_objc_infraclass_state bit)
{
   return( _mulle_objc_class_get_state_bit( &infra->base, bit));
}

static inline  int
   _mulle_objc_infraclass_is_initialized( struct _mulle_objc_infraclass *infra)
{
   return( _mulle_objc_class_get_state_bit( &infra->base,
                                            MULLE_OBJC_INFRACLASS_INITIALIZE_DONE));
}


static inline void
   mulle_objc_infraclass_add_methodlist_nofail( struct _mulle_objc_infraclass *infra,
                                                struct _mulle_objc_methodlist *list)
{
   extern void   mulle_objc_class_add_methodlist_nofail( struct _mulle_objc_class *cls,
                                                           struct _mulle_objc_methodlist *list);

   mulle_objc_class_add_methodlist_nofail( &infra->base, list);
}


static inline size_t
   _mulle_objc_infraclass_get_allocationsize( struct _mulle_objc_infraclass *infra)
{
   return( _mulle_objc_class_get_allocationsize( &infra->base));
}


static inline size_t
   _mulle_objc_infraclass_get_instancesize( struct _mulle_objc_infraclass *infra)
{
   return( _mulle_objc_class_get_instancesize( &infra->base));
}


static inline struct _mulle_objc_method  *
    mulle_objc_infraclass_defaultsearch_method( struct _mulle_objc_infraclass *infra,
                                                mulle_objc_methodid_t methodid)
{
   extern struct _mulle_objc_method   *
      _mulle_objc_class_defaultsearch_method( struct _mulle_objc_class *cls,
                                              mulle_objc_methodid_t methodid,
                                              int *error);
   int   error;

   return( _mulle_objc_class_defaultsearch_method( &infra->base, methodid, &error));
}


struct _mulle_objc_searchargumentscachable;

static inline mulle_objc_implementation_t
   _mulle_objc_infraclass_lookup_superimplementation( struct _mulle_objc_infraclass *infra,
                                                      mulle_objc_superid_t superid)
{
   extern mulle_objc_implementation_t
      _mulle_objc_class_superlookup_implementation_nofail( struct _mulle_objc_class *cls,
                                                    mulle_objc_superid_t superid);

   return( _mulle_objc_class_superlookup_implementation_nofail( &infra->base, superid));
}


#pragma mark - petty accessors


static inline int
   _mulle_objc_infraclass_set_placeholder( struct _mulle_objc_infraclass *infra,
                                           enum _mulle_objc_infraclass_placeholder_index index,
                                           struct _mulle_objc_object *obj)
{
   assert( obj);
   return( _mulle_atomic_pointer_cas( &infra->placeholders[ index].pointer,
                                      obj,
                                      NULL));
}


static inline struct _mulle_objc_object *
   _mulle_objc_infraclass_get_placeholder( struct _mulle_objc_infraclass *infra,
                                           enum _mulle_objc_infraclass_placeholder_index index)
{
   return( _mulle_atomic_pointer_read( &infra->placeholders[ index].pointer));
}


static inline struct _mulle_objc_object *
   _mulle_objc_infraclass_get_instantiate( struct _mulle_objc_infraclass *infra)
{
   return( _mulle_objc_infraclass_get_placeholder( infra,
                                                   MULLE_OBJC_INFRACLASS_INSTANTIATE_INDEX));
}


// 1: it has worked, 0: someone else was faster
static inline int
   _mulle_objc_infraclass_set_instantiate( struct _mulle_objc_infraclass *infra,
                                            struct _mulle_objc_object *obj)
{
   return( _mulle_objc_infraclass_set_placeholder( infra,
                                                   MULLE_OBJC_INFRACLASS_INSTANTIATE_INDEX,
                                                   obj));
}



static inline struct _mulle_objc_object *
   _mulle_objc_infraclass_get_singleton( struct _mulle_objc_infraclass *infra)
{
   return( _mulle_objc_infraclass_get_placeholder( infra,
                                                   MULLE_OBJC_INFRACLASS_SINGLETON_INDEX));
}


// 1: it has worked, 0: someone else was faster
static inline int
   _mulle_objc_infraclass_set_singleton( struct _mulle_objc_infraclass *infra,
                                         struct _mulle_objc_object *obj)
{
   return( _mulle_objc_infraclass_set_placeholder( infra,
                                                   MULLE_OBJC_INFRACLASS_SINGLETON_INDEX,
                                                   obj));
}


static inline struct _mulle_objc_object *
   _mulle_objc_infraclass_get_classcluster( struct _mulle_objc_infraclass *infra)
{
   return( _mulle_objc_infraclass_get_placeholder( infra,
                                                   MULLE_OBJC_INFRACLASS_CLASSCLUSTER_INDEX));
}


// 1: it has worked, 0: someone else was faster
static inline int
   _mulle_objc_infraclass_set_classcluster( struct _mulle_objc_infraclass *infra,
                                            struct _mulle_objc_object *obj)
{
   return( _mulle_objc_infraclass_set_placeholder( infra,
                                                   MULLE_OBJC_INFRACLASS_CLASSCLUSTER_INDEX,
                                                   obj));
}


//
// version is kept in the infraclass
//
static inline uintptr_t
   _mulle_objc_infraclass_get_coderversion( struct _mulle_objc_infraclass *infra)
{
   return( (uintptr_t) _mulle_atomic_pointer_read( &infra->coderversion));
}


MULLE_C_NONNULL_RETURN
static inline struct mulle_allocator   *
   _mulle_objc_infraclass_get_allocator( struct _mulle_objc_infraclass *infra)
{
   return( infra->allocator);
}


static inline void
   _mulle_objc_infraclass_set_allocator( struct _mulle_objc_infraclass *infra,
                                         struct mulle_allocator  *allocator)
{
   assert( allocator);
   infra->allocator = allocator;
}


//
// version is kept in the infraclass
//
static inline unsigned int
   _mulle_objc_infraclass_get_taggedpointerindex( struct _mulle_objc_infraclass *infra)
{
   return( (unsigned int) (uintptr_t) _mulle_atomic_pointer_read( &infra->taggedpointerindex));
}


// 1: it has worked, 0: someone else was faster, can only be set once
static inline int
   _mulle_objc_infraclass_set_coderversion( struct _mulle_objc_infraclass *infra,
                                            uintptr_t value)
{
   if( ! value)
      return( 1);
   return( _mulle_atomic_pointer_cas( &infra->coderversion,
                                      (void *) value,
                                      NULL));
}


// 1: it has worked, 0: someone else was faster
static inline int
   _mulle_objc_infraclass_set_taggedpointerindex( struct _mulle_objc_infraclass *infra,
                                                  unsigned int value)
{
   if( ! value)
      return( 1);
   assert( value <= 0x7);
   return( _mulle_atomic_pointer_cas( &infra->taggedpointerindex,
                                      (void *) (uintptr_t) value,
                                      NULL));
}


static inline mulle_objc_hash_t
   _mulle_objc_infraclass_get_ivarhash( struct _mulle_objc_infraclass *infra)
{
   return( infra->ivarhash);
}


static inline void
   _mulle_objc_infraclass_set_ivarhash( struct _mulle_objc_infraclass *infra,
                                        mulle_objc_hash_t ivarhash)
{
   infra->ivarhash = ivarhash;
}


static inline int
   _mulle_objc_infraclass_is_taggedpointerclass( struct _mulle_objc_infraclass *infra)
{
   intptr_t   index;

   index = (intptr_t) _mulle_atomic_pointer_read( &infra->taggedpointerindex);
   return( index != 0);
}


# pragma mark - sanity check

int   mulle_objc_infraclass_is_sane( struct _mulle_objc_infraclass *infra);



#if 0

# pragma mark - class variables

static inline void   *_mulle_objc_infraclass_get_cvar( struct _mulle_objc_infraclass *infra,
                                                       struct _mulle_objc_object *key)
{
   return( _mulle_concurrent_hashmap_lookup( &infra->cvars, (intptr_t) key));
}


static inline int   _mulle_objc_infraclass_set_cvar( struct _mulle_objc_infraclass *infra,
                                                     struct _mulle_objc_object *key,
                                                     void *value)
{
   return( _mulle_concurrent_hashmap_insert( &infra->cvars, (intptr_t) key, value));
}


static inline int   _mulle_objc_infraclass_remove_cvar( struct _mulle_objc_infraclass *infra,
                                                        struct _mulle_objc_object *key,
                                                        void *value)
{
   return( _mulle_concurrent_hashmap_remove( &infra->cvars, (intptr_t) key, value));
}


static inline struct mulle_concurrent_hashmapenumerator
    _mulle_objc_infraclass_enumerate_cvars( struct _mulle_objc_infraclass *infra)
{
   return( mulle_concurrent_hashmap_enumerate( &infra->cvars));
}

#endif


# pragma mark - properties

int   mulle_objc_infraclass_add_propertylist( struct _mulle_objc_infraclass *infra,
                                              struct _mulle_objc_propertylist *list);

void   mulle_objc_infraclass_add_propertylist_nofail( struct _mulle_objc_infraclass *infra,
                                                        struct _mulle_objc_propertylist *list);

struct _mulle_objc_property   *
   _mulle_objc_infraclass_search_property( struct _mulle_objc_infraclass *infra,
                                           mulle_objc_propertyid_t propertyid);

struct _mulle_objc_property  *
   mulle_objc_infraclass_search_property( struct _mulle_objc_infraclass *infra,
                                          mulle_objc_propertyid_t propertyid);


# pragma mark - ivar lists

int   mulle_objc_infraclass_add_ivarlist( struct _mulle_objc_infraclass *infra,
                                          struct _mulle_objc_ivarlist *list);

void   mulle_objc_infraclass_add_ivarlist_nofail( struct _mulle_objc_infraclass *infra,
                                                  struct _mulle_objc_ivarlist *list);


# pragma mark - ivars

struct _mulle_objc_ivar   *_mulle_objc_infraclass_search_ivar( struct _mulle_objc_infraclass *infra,
                                                               mulle_objc_ivarid_t ivarid);

struct _mulle_objc_ivar  *mulle_objc_infraclass_search_ivar( struct _mulle_objc_infraclass *infra,
                                                             mulle_objc_ivarid_t ivarid);


#pragma mark - walkers

typedef mulle_objc_walkcommand_t
   mulle_objc_walkpropertiescallback( struct _mulle_objc_property *,
                                      struct _mulle_objc_infraclass *,
                                      void *);
typedef mulle_objc_walkcommand_t
   mulle_objc_walkivarscallback( struct _mulle_objc_ivar *,
                                 struct _mulle_objc_infraclass *,
                                 void *);


mulle_objc_walkcommand_t
	_mulle_objc_infraclass_walk_ivars( struct _mulle_objc_infraclass *cls,
                                      unsigned int inheritance,
                                      mulle_objc_walkivarscallback *f,
                                      void *userinfo);
mulle_objc_walkcommand_t
	_mulle_objc_infraclass_walk_properties( struct _mulle_objc_infraclass *infra,
                                           unsigned int inheritance,
                                           mulle_objc_walkpropertiescallback *f,
                                           void *userinfo);

mulle_objc_walkcommand_t
	mulle_objc_infraclass_walk( struct _mulle_objc_infraclass   *infra,
                              enum mulle_objc_walkpointertype_t  type,
                              mulle_objc_walkcallback_t   callback,
                              void *parent,
                              void *userinfo);

#pragma mark - some other methods, that are only defined on infraclass

int    mulle_objc_infraclass_is_protocolclass( struct _mulle_objc_infraclass *infra);

// check is same, but also emits warnings
int    mulle_objc_infraclass_check_protocolclass( struct _mulle_objc_infraclass *infra);


// hairy code for the compat layer
static inline void
  __mulle_objc_infraclass_set_instancesize( struct _mulle_objc_infraclass *infra,
                                            size_t size)
{
   infra->base.allocationsize = sizeof( struct _mulle_objc_objectheader) + size
   + infra->base.headerextrasize;
}


void   _mulle_objc_infraclass_call_categories_unload( struct _mulle_objc_infraclass *infra);
void   _mulle_objc_infraclass_call_deinitialize( struct _mulle_objc_infraclass *infra);
void   _mulle_objc_infraclass_call_finalize( struct _mulle_objc_infraclass *infra);
void   _mulle_objc_infraclass_call_unload( struct _mulle_objc_infraclass *infra);
void   _mulle_objc_infraclass_call_willfinalize( struct _mulle_objc_infraclass *infra);


// move where ?
static inline void
   _mulle_objc_infraclass_setup_if_needed( struct _mulle_objc_infraclass *infra)
{
   int  _mulle_objc_class_setup( struct _mulle_objc_class *cls);

   if( ! _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE))
      _mulle_objc_class_setup( _mulle_objc_infraclass_as_class( infra));
}


#endif /* mulle_objc_infraclass_h */
