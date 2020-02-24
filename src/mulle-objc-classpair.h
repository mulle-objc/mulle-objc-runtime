//
//  mulle_objc_classpair.h
//  mulle-objc-runtime
//
//  Created by Nat! on 17/04/08.
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
#ifndef mulle_objc_classpair_h__
#define mulle_objc_classpair_h__

#include "include.h"

#include "mulle-objc-atomicpointer.h"
#include "mulle-objc-objectheader.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-load.h"
#include "mulle-objc-uniqueid.h"
#include "mulle-objc-uniqueidarray.h"
#include "mulle-objc-walktypes.h"
#include "include.h"

struct _mulle_objc_protocollist;
struct _mulle_objc_loadclass;


#define _MULLE_OBJC_CLASSPAIR_PADDING     (0x10 - ((sizeof( struct _mulle_objc_infraclass) + sizeof( struct _mulle_objc_objectheader)) & 0xF))

//
// Put all the common information like
// protocolids, categoryids and the like into the classpair
// structure.
//
struct _mulle_objc_classpair
{
   // >>> dont' add anything >>>
   struct _mulle_objc_objectheader           infraclassheader;
   struct _mulle_objc_infraclass             infraclass;
   unsigned char                             _padding[ _MULLE_OBJC_CLASSPAIR_PADDING];
   struct _mulle_objc_objectheader           metaclassheader;
   struct _mulle_objc_metaclass              metaclass;
   // <<< dont' add anything <<<

   mulle_thread_mutex_t                      lock;   // used for initialize
   mulle_thread_t                            thread; // used for initialize
   struct _mulle_objc_loadclass              *loadclass;

   // common stuff
   struct mulle_concurrent_pointerarray      protocolclasses;
   union _mulle_objc_uniqueidarraypointer_t  p_protocolids;
   union _mulle_objc_uniqueidarraypointer_t  p_categoryids;

   uint32_t                                  classindex;       // set when added
   double                                    *_classextra[ 1]; // will not exist if classextra is 0
};


# pragma mark - init and done

void    _mulle_objc_classpair_plusinit( struct _mulle_objc_classpair *pair,
                                        struct mulle_allocator *allocator);

void    _mulle_objc_classpair_plusdone( struct _mulle_objc_classpair *pair,
                                        struct mulle_allocator *allocator);

void    _mulle_objc_classpair_free( struct _mulle_objc_classpair *pair,
                                    struct mulle_allocator *allocator);


void   _mulle_objc_classpair_call_class_finalize( struct _mulle_objc_classpair *pair);
void   mulle_objc_classpair_free( struct _mulle_objc_classpair *pair,
                                  struct mulle_allocator *allocator);


static inline size_t   mulle_objc_classpair_size( size_t extrasize)
{
   return( sizeof( struct _mulle_objc_classpair) - sizeof( double) + extrasize);
}


# pragma mark - petty accessors

static inline struct _mulle_objc_infraclass   *
   _mulle_objc_classpair_get_infraclass( struct _mulle_objc_classpair *pair)
{
   return( &pair->infraclass);
}


static inline struct _mulle_objc_metaclass   *
   _mulle_objc_classpair_get_metaclass( struct _mulle_objc_classpair *pair)
{
   return( &pair->metaclass);
}


static inline struct _mulle_objc_infraclass   *
   mulle_objc_classpair_get_infraclass( struct _mulle_objc_classpair *pair)
{
   return( pair ? _mulle_objc_classpair_get_infraclass( pair) : NULL);
}

static inline struct _mulle_objc_metaclass   *
   mulle_objc_classpair_get_metaclass( struct _mulle_objc_classpair *pair)
{
   return( pair ? _mulle_objc_classpair_get_metaclass( pair) : NULL);
}


static inline mulle_thread_mutex_t *
   _mulle_objc_classpair_get_lock( struct _mulle_objc_classpair *pair)
{
   return( &pair->lock);
}


static inline struct _mulle_objc_loadclass  *
   _mulle_objc_classpair_get_loadclass( struct _mulle_objc_classpair *pair)
{
   return( pair->loadclass);
}


static inline void
   _mulle_objc_classpair_set_loadclass( struct _mulle_objc_classpair *pair,
                                        struct _mulle_objc_loadclass *loadclass)
{
   pair->loadclass = loadclass;
}


static inline char   *
   _mulle_objc_classpair_get_origin( struct _mulle_objc_classpair *pair)
{
   return( pair->loadclass ? pair->loadclass->origin : 0);
}


static inline void   *
   _mulle_objc_classpair_get_classextra( struct _mulle_objc_classpair *pair)
{
   return( pair->_classextra);
}


static inline void
   _mulle_objc_classpair_set_classindex( struct _mulle_objc_classpair *pair,
                                         uint32_t classindex)
{
   pair->classindex = classindex;
}


static inline uint32_t
   _mulle_objc_classpair_get_classindex( struct _mulle_objc_classpair *pair)
{
   return( pair->classindex);
}


# pragma mark - conveniences

static inline struct _mulle_objc_universe   *
   _mulle_objc_classpair_get_universe( struct _mulle_objc_classpair *pair)
{
   return( _mulle_objc_infraclass_get_universe( _mulle_objc_classpair_get_infraclass( pair)));
}


static inline struct _mulle_objc_universe   *
   mulle_objc_classpair_get_universe( struct _mulle_objc_classpair *pair)
{
   return( pair ? _mulle_objc_classpair_get_universe( pair) : NULL);
}


static inline char   *
   _mulle_objc_classpair_get_name( struct _mulle_objc_classpair *pair)
{
   return( _mulle_objc_infraclass_get_name( _mulle_objc_classpair_get_infraclass( pair)));
}


static inline mulle_objc_classid_t
   _mulle_objc_classpair_get_classid( struct _mulle_objc_classpair *pair)
{
   return( _mulle_objc_infraclass_get_classid( _mulle_objc_classpair_get_infraclass( pair)));
}


# pragma mark - infra/metaclass reverse calculations

static inline struct _mulle_objc_classpair   *
   _mulle_objc_infraclass_get_classpair( struct _mulle_objc_infraclass *infra)
{
   assert( infra->base.infraclass == NULL);
   return( (struct _mulle_objc_classpair *) _mulle_objc_object_get_objectheader( infra));
}


//
// this is faster than doing _mulle_objc_class_get_classpair_from_infraclass( cls->infraclass)
// because the result is just an address offset, not a dereference
//
static inline struct _mulle_objc_classpair   *
   _mulle_objc_metaclass_get_classpair( struct _mulle_objc_metaclass *meta)
{
   int   offset;

   assert( meta->base.infraclass != NULL);

   offset = - (int) offsetof( struct _mulle_objc_classpair, metaclass);
   return( (struct _mulle_objc_classpair *) &((char *) meta)[ offset]);
}


# pragma mark - infra/metaclass conveniences

// code is here, because infraclass doesn't know about pair
static inline struct _mulle_objc_metaclass   *
   _mulle_objc_infraclass_get_metaclass( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_infraclass_get_classpair( infra);
   return( _mulle_objc_classpair_get_metaclass( pair));
}


// code is here, because metaclass doesn't know about pair
static inline struct _mulle_objc_infraclass   *
   _mulle_objc_metaclass_get_infraclass( struct _mulle_objc_metaclass *cls)
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_metaclass_get_classpair( cls);
   return( _mulle_objc_classpair_get_infraclass( pair));
}


# pragma mark - class reverse

static inline struct _mulle_objc_classpair   *
   _mulle_objc_class_get_classpair( struct _mulle_objc_class *cls)
{
   if( ! cls->infraclass)
      return( _mulle_objc_infraclass_get_classpair( (struct _mulle_objc_infraclass *) cls));
   return( _mulle_objc_metaclass_get_classpair( (struct _mulle_objc_metaclass *) cls));
}


static inline struct _mulle_objc_classpair   *
   mulle_objc_class_get_classpair( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_classpair( cls) : NULL);
}


#pragma mark - categories

static inline int
   _mulle_objc_classpair_has_categoryid( struct _mulle_objc_classpair *pair,
                                         mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_uniqueidarray   *array;

   array = _mulle_atomic_pointer_read( &pair->p_categoryids.pointer);
   return( _mulle_objc_uniqueidarray_search( array, categoryid));
}

void   _mulle_objc_classpair_add_categoryid( struct _mulle_objc_classpair *pair,
                                             mulle_objc_categoryid_t categoryid);

void   _mulle_objc_classpair_add_categoryid( struct _mulle_objc_classpair *pair,
                                             mulle_objc_categoryid_t categoryid);


static inline void
   _mulle_objc_classpair_add_categoryids( struct _mulle_objc_classpair *pair,
                                          unsigned int n,
                                          mulle_objc_protocolid_t *categoryids)
{
   void  _mulle_objc_classpair_add_uniqueidarray_ids( struct _mulle_objc_classpair *pair,
                                                      mulle_atomic_pointer_t *pointer,
                                                      unsigned int n,
                                                      mulle_objc_uniqueid_t *uniqueids);

   _mulle_objc_classpair_add_uniqueidarray_ids( pair, &pair->p_categoryids.pointer, n, categoryids);
}

typedef mulle_objc_walkcommand_t
   mulle_objc_walkcategoryidscallback( mulle_objc_categoryid_t,
                                       struct _mulle_objc_classpair *,
                                       void *);


mulle_objc_walkcommand_t
	_mulle_objc_classpair_walk_categoryids( struct _mulle_objc_classpair *pair,
                                           unsigned int inheritance,
                                           mulle_objc_walkcategoryidscallback *f,
                                           void *userinfo);


void   mulle_objc_classpair_add_categoryid_nofail( struct _mulle_objc_classpair *pair,
                                                   mulle_objc_categoryid_t categoryid);


#pragma mark - protocolclasses


static inline int
   _mulle_objc_classpair_has_protocolclass( struct _mulle_objc_classpair *pair,
                                            struct _mulle_objc_infraclass *proto_cls)
{
   return( _mulle_concurrent_pointerarray_find( &pair->protocolclasses, proto_cls));
}


static inline unsigned int   _mulle_objc_classpair_get_protocolclasscount( struct _mulle_objc_classpair *pair)
{
   return( _mulle_concurrent_pointerarray_get_count( &pair->protocolclasses));
}


typedef mulle_objc_walkcommand_t
   mulle_objc_walkprotocolclassescallback( struct _mulle_objc_infraclass *,
                                           struct _mulle_objc_classpair *,
                                           void *);


mulle_objc_walkcommand_t
	_mulle_objc_classpair_walk_protocolclasses( struct _mulle_objc_classpair *pair,
                                               unsigned int inheritance,
                                               mulle_objc_walkprotocolclassescallback *f,
                                               void *userinfo);


void   _mulle_objc_classpair_add_protocolclass( struct _mulle_objc_classpair *pair,
                                                struct _mulle_objc_infraclass *proto_cls);

void   mulle_objc_classpair_add_protocolclassids_nofail( struct _mulle_objc_classpair *pair,
                                                         mulle_objc_protocolid_t *protocolids);

#pragma mark - protocols

// does not search deep!
static inline int
   _mulle_objc_classpair_has_protocolid( struct _mulle_objc_classpair *pair,
                                         mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_uniqueidarray   *array;

   array = _mulle_atomic_pointer_read( &pair->p_protocolids.pointer);
   return( _mulle_objc_uniqueidarray_search( array, protocolid));
}


typedef mulle_objc_walkcommand_t
   mulle_objc_walkprotocolidscallback( mulle_objc_protocolid_t,
                                       struct _mulle_objc_classpair *,
                                       void *);


mulle_objc_walkcommand_t
	_mulle_objc_classpair_walk_protocolids( struct _mulle_objc_classpair *pair,
                                          unsigned int inheritance,
                                          mulle_objc_walkprotocolidscallback *f,
                                          void *userinfo);

int
   __mulle_objc_classpair_conformsto_protocolid( struct _mulle_objc_classpair *pair,
                                                 unsigned int inheritance,
                                                 mulle_objc_protocolid_t protocolid);

static inline int
   _mulle_objc_classpair_conformsto_protocolid( struct _mulle_objc_classpair *pair,
                                                mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_infraclass   *infra;
   unsigned int                    inheritance;

   infra       = _mulle_objc_classpair_get_infraclass( pair);
   inheritance = _mulle_objc_infraclass_get_inheritance( infra);
   return( __mulle_objc_classpair_conformsto_protocolid( pair,
                                                         inheritance,
                                                         protocolid));
}


static inline int
   mulle_objc_classpair_conformsto_protocolid( struct _mulle_objc_classpair *pair,
                                               mulle_objc_protocolid_t protocolid)
{
   return( pair ? _mulle_objc_classpair_conformsto_protocolid( pair, protocolid) : 0);
}


static inline void
   _mulle_objc_classpair_add_protocolids( struct _mulle_objc_classpair *pair,
                                          unsigned int n,
                                          mulle_objc_protocolid_t *protocolids)
{
   void  _mulle_objc_classpair_add_uniqueidarray_ids( struct _mulle_objc_classpair *pair,
                                                      mulle_atomic_pointer_t *pointer,
                                                      unsigned int n,
                                                      mulle_objc_uniqueid_t *uniqueids);

   _mulle_objc_classpair_add_uniqueidarray_ids( pair, &pair->p_protocolids.pointer, n, protocolids);
}


void   mulle_objc_classpair_add_protocollist_nofail( struct _mulle_objc_classpair *pair,
                                                     struct _mulle_objc_protocollist *protocols);


# pragma mark - protocol class enumerator

struct _mulle_objc_protocolclassenumerator
{
   struct mulle_concurrent_pointerarrayenumerator   list_rover;
   struct _mulle_objc_infraclass                    *infra;
};


static inline struct _mulle_objc_protocolclassenumerator
   _mulle_objc_classpair_enumerate_protocolclasses( struct _mulle_objc_classpair *pair)
{
   struct _mulle_objc_protocolclassenumerator  rover;

   rover.list_rover = mulle_concurrent_pointerarray_enumerate( &pair->protocolclasses);
   rover.infra      = _mulle_objc_classpair_get_infraclass( pair);
   return( rover);
}


static inline void
   _mulle_objc_protocolclassenumerator_done( struct _mulle_objc_protocolclassenumerator *rover)
{
   mulle_concurrent_pointerarrayenumerator_done( &rover->list_rover);
}


static inline struct _mulle_objc_infraclass  *
   _mulle_objc_protocolclassenumerator_get_infraclass( struct _mulle_objc_protocolclassenumerator *rover)
{
   return( rover->infra);
}


struct _mulle_objc_infraclass  *
   _mulle_objc_protocolclassenumerator_next( struct _mulle_objc_protocolclassenumerator *rover);


# pragma mark - protocol class reverse enumerator

struct _mulle_objc_protocolclassreverseenumerator
{
   struct mulle_concurrent_pointerarrayreverseenumerator   list_rover;
   struct _mulle_objc_infraclass                           *infra;
};


static inline struct _mulle_objc_protocolclassreverseenumerator
   _mulle_objc_classpair_reverseenumerate_protocolclasses( struct _mulle_objc_classpair *pair)
{
   struct _mulle_objc_protocolclassreverseenumerator  rover;
   unsigned int    n;

   n = mulle_concurrent_pointerarray_get_count( &pair->protocolclasses);
   rover.list_rover = mulle_concurrent_pointerarray_reverseenumerate( &pair->protocolclasses, n);
   rover.infra      = _mulle_objc_classpair_get_infraclass( pair);
   return( rover);
}


static inline void  _mulle_objc_protocolclassreverseenumerator_done( struct _mulle_objc_protocolclassreverseenumerator *rover)
{
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover->list_rover);
}


static inline struct _mulle_objc_infraclass  *
   _mulle_objc_protocolclassreverseenumerator_get_infraclass( struct _mulle_objc_protocolclassreverseenumerator *rover)
{
   return( rover->infra);
}


struct _mulle_objc_infraclass  *
   _mulle_objc_protocolclassreverseenumerator_next( struct _mulle_objc_protocolclassreverseenumerator *rover);


#pragma mark - debug support

mulle_objc_walkcommand_t
   mulle_objc_classpair_walk( struct _mulle_objc_classpair *pair,
                              mulle_objc_walkcallback_t callback,
                              void *userinfo);



#pragma mark - conveniences for infraclass

static inline int
   _mulle_objc_infraclass_conformsto_protocolid( struct _mulle_objc_infraclass *infra,
                                                 mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_infraclass_get_classpair( infra);
   return( _mulle_objc_classpair_conformsto_protocolid( pair, protocolid));
}


#if 0

#pragma mark - conveniences for class
static inline int   _mulle_objc_class_has_categoryid( struct _mulle_objc_class *cls,
                                                      mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_class_get_classpair( cls);
   return( _mulle_objc_classpair_has_categoryid( pair, categoryid));
}

static inline int   _mulle_objc_class_has_protocolid( struct _mulle_objc_class *cls,
                                                     mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_class_get_classpair( cls);
   return( _mulle_objc_classpair_has_protocolid( pair, protocolid));
}


static inline int   mulle_objc_class_conforms_to_protocol( struct _mulle_objc_class *cls, mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_classpair   *pair;

   pair = _mulle_objc_class_get_classpair( cls);
   return( mulle_objc_classpair_conformsto_protocolid( pair, protocolid));
}


#endif

#pragma mark - conveniences for compatibility layer

// not recommended to use

void  _mulle_objc_classpair_set_uniqueidarray( struct _mulle_objc_classpair *pair,
                                               mulle_atomic_pointer_t *pointer,
                                               struct _mulle_objc_uniqueidarray *array);

static inline struct _mulle_objc_uniqueidarray *
   _mulle_objc_classpair_get_protocolidarray( struct _mulle_objc_classpair *pair)
{
   return( _mulle_atomic_pointer_read( &pair->p_protocolids.pointer));
}



static inline  struct _mulle_objc_uniqueidarray *
   _mulle_objc_classpair_get_categoryidarray( struct _mulle_objc_classpair *pair)
{
   return( _mulle_atomic_pointer_read( &pair->p_categoryids.pointer));
}


static inline void
   _mulle_objc_classpair_set_protocolidarray( struct _mulle_objc_classpair *pair,
                                              struct _mulle_objc_uniqueidarray *array)
{
   return( _mulle_objc_classpair_set_uniqueidarray( pair,
                                                    &pair->p_protocolids.pointer,
                                                    array));
}


static inline void
   _mulle_objc_classpair_set_categoryidarray( struct _mulle_objc_classpair *pair,
                                              struct _mulle_objc_uniqueidarray *array)
{
   return( _mulle_objc_classpair_set_uniqueidarray( pair,
                                                    &pair->p_categoryids.pointer,
                                                    array));
}


#endif /* mulle_objc_classpair_h */
