//
//  mulle_objc_class.h
//  mulle-objc
//
//  Created by Nat! on 16/11/14.
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


#ifndef mulle_objc_class_h__
#define mulle_objc_class_h__

#include "mulle_objc_atomicpointer.h"
#include "mulle_objc_cache.h"
#include "mulle_objc_kvccache.h"
#include "mulle_objc_fastmethodtable.h"
#include "mulle_objc_objectheader.h"
#include "mulle_objc_uniqueid.h"
#include "mulle_objc_runtime.h"

#include <mulle_concurrent/mulle_concurrent.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>


struct _mulle_objc_class;
struct _mulle_objc_callqueue;
struct _mulle_objc_ivarlist;
struct _mulle_objc_methodlist;
struct _mulle_objc_object;
struct _mulle_objc_propertylist;
struct _mulle_objc_runtime;


//
// the order of these structure elements is architecture dependent
// the trick is, that the mask is "behind" the buckets in malloced
// space. the vtab is in there, because runtime and class share this
// (why not make the runtime a first class object, because then
// the access to classes slows. We could put the runtime into another object
//

struct _mulle_objc_methodcachepivot
{
   struct _mulle_objc_cachepivot       pivot; // for atomic XCHG with pointer indirection
   mulle_objc_methodimplementation_t   call2;
};


#define MULLE_OBJC_ANY_OWNER  ((void *) -1)

enum
{
   MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS          = 0x01,
   MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES          = 0x02,
   MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS           = 0x04,
   MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES = 0x08
};

#define MULLE_OBJC_CLASS_S_FAST_METHODS  16


// 0x0001 - 0x0080        - mulle-objc
// 0x0100 - 0x8000        - Foundation
// 0x00010000 - 0x80000   - User

enum
{
   MULLE_OBJC_LOAD_SCHEDULED     = 0x1, // only if a class +load existed
   MULLE_OBJC_INITIALIZE_DONE    = 0x2,
   MULLE_OBJC_CACHE_INITIALIZED  = 0x4,
   MULLE_OBJC_ALWAYS_EMPTY_CACHE = 0x8,
   MULLE_OBJC_WARN_PROTOCOL      = 0x10,

   MULLE_OBJC_FOUNDATION_BIT0    = 0x00100,
   MULLE_OBJC_FOUNDATION_BIT15   = 0x08000,

   MULLE_OBJC_USER_BIT0          = 0x000010000
//   MULLE_OBJC_USER_BIT15         = 0x800000000
};


//
// this is a fairly hefty struct, and the runtime needs two for each class
// assume that each new class costs you 1K
//
struct _mulle_objc_class
{
   struct _mulle_objc_methodcachepivot   cachepivot;  // DON'T MOVE

   void                                  *(*call)( void *, mulle_objc_methodid_t, void *, struct _mulle_objc_class *);

   /* ^^^ keep above like this, or change mulle_objc_fastmethodtable fault */

   char                              *name;         // offset (void **)[ 3]
   struct _mulle_objc_runtime        *runtime;

   struct _mulle_objc_class          *superclass;
   struct _mulle_objc_class          *infraclass;   // meta calls (+) use this

   uintptr_t                         allocationsize;     // instancesize + header
   uintptr_t                         extensionoffset;    // 4 later #1#
   mulle_objc_classid_t              classid;
   mulle_objc_classid_t              superclassid;
   mulle_objc_hash_t                 ivarhash;

   struct _mulle_objc_method         *forwardmethod;
   union _mulle_objc_atomicobjectpointer_t  placeholder;  // metaclass uses this for +instantiate

   mulle_atomic_pointer_t            thread;
   mulle_atomic_pointer_t            state;
   mulle_atomic_pointer_t            coderversion; // for NSCoder
   mulle_atomic_pointer_t            taggedpointerindex;

   // general storage mechanism for class variable, only in the infraclass
   struct mulle_concurrent_hashmap     cvars;
   struct _mulle_objc_kvccachepivot    kvc;

   uint16_t                                inheritance;
   uint16_t                                preloads;

   struct mulle_concurrent_pointerarray    ivarlists;
   struct mulle_concurrent_pointerarray    methodlists;
   struct mulle_concurrent_pointerarray    propertylists;
   struct mulle_concurrent_pointerarray    protocolids;
   struct mulle_concurrent_pointerarray    categoryids;

   struct _mulle_objc_fastmethodtable      vtab;  // dont' move it up, debugs nicer here
};


#define _MULLE_OBJC_CLASSPAIR_PADDING     (0x10 - ((sizeof( struct _mulle_objc_class) + sizeof( struct _mulle_objc_objectheader)) & 0xF))

// TODO:
// I should put all the common information like name, runtime.
// protocolids, categoryids and the like into the classpair
// structure. I should also have a separate space for infraclassinfo,
// that just pertains to the infraclass.
//
struct _mulle_objc_classpair
{
   struct _mulle_objc_objectheader    infraclassheader;
   struct _mulle_objc_class           infraclass;
   unsigned char                      _padding[ _MULLE_OBJC_CLASSPAIR_PADDING];
   struct _mulle_objc_objectheader    metaclassheader;
   struct _mulle_objc_class           metaclass;
   
   char                               *origin;  // a start of shared info
};


static inline struct _mulle_objc_class   *_mulle_objc_classpair_get_infraclass( struct _mulle_objc_classpair *pair)
{
   return( &pair->infraclass);
}


static inline struct _mulle_objc_class   *_mulle_objc_classpair_get_metaclass( struct _mulle_objc_classpair *pair)
{
   return( &pair->metaclass);
}


static inline struct _mulle_objc_classpair   *_mulle_objc_class_get_classpair( struct _mulle_objc_class *cls)
{
   assert( cls->infraclass == NULL);
   return( (struct _mulle_objc_classpair *) _mulle_objc_object_get_objectheader( cls));
}


static inline void   _mulle_objc_classpair_set_origin( struct _mulle_objc_classpair *pair,
                                                char *name)
{
   pair->origin = name;  // not copied gotta be a constant string
}


static inline char   *_mulle_objc_classpair_get_origin( struct _mulle_objc_classpair *pair)
{
   return( pair->origin);
}



void   _mulle_objc_classpair_call_class_finalize( struct _mulle_objc_classpair *pair);
void   _mulle_objc_classpair_destroy( struct _mulle_objc_classpair *pair, struct mulle_allocator *allocator);
void   mulle_objc_classpair_free( struct _mulle_objc_classpair *pair, struct mulle_allocator *allocator);


# pragma mark - API


static inline struct _mulle_objc_class   *mulle_objc_classpair_get_infraclass( struct _mulle_objc_classpair *pair)
{
   return( pair ? _mulle_objc_classpair_get_infraclass( pair) : NULL);
}



static inline struct _mulle_objc_class   *mulle_objc_classpair_get_metaclass( struct _mulle_objc_classpair *pair)
{
   return( pair ? _mulle_objc_classpair_get_metaclass( pair) : NULL);
}


# pragma mark - exceptions

void   _mulle_objc_class_raise_null_exception( void)   MULLE_C_NO_RETURN;


# pragma mark - method alloc / free

void  _mulle_objc_class_init( struct _mulle_objc_class *cls,
                              char *name,
                              size_t  instancesize,
                              mulle_objc_classid_t classid,
                              struct _mulle_objc_class *superclass,
                              struct _mulle_objc_runtime *runtime);

void   _mulle_objc_class_setup_pointerarrays( struct _mulle_objc_class *cls,
                                              struct _mulle_objc_runtime *runtime);


# pragma mark - petty accessors


static inline struct _mulle_objc_class   *_mulle_objc_class_get_superclass( struct _mulle_objc_class *cls)
{
   return( cls->superclass);
}


static inline mulle_objc_classid_t   _mulle_objc_class_get_classid( struct _mulle_objc_class *cls)
{
   return( cls->classid);
}


static inline char   *_mulle_objc_class_get_name( struct _mulle_objc_class *cls)
{
   return( cls->name);
}


static inline struct _mulle_objc_runtime   *_mulle_objc_class_get_runtime( struct _mulle_objc_class *cls)
{
   return( cls->runtime);
}


static inline size_t   _mulle_objc_class_get_allocationsize( struct _mulle_objc_class *cls)
{
   return( cls->allocationsize);
}


static inline size_t   _mulle_objc_class_get_instancesize( struct _mulle_objc_class *cls)
{
   return( cls->allocationsize - sizeof( struct _mulle_objc_objectheader));
}


// deprecated
static inline size_t   _mulle_objc_class_get_instance_size( struct _mulle_objc_class *cls)
{
   return( _mulle_objc_class_get_instancesize( cls));
}


static inline unsigned int   _mulle_objc_class_get_inheritance( struct _mulle_objc_class *cls)
{
   return( cls->inheritance);
}

static inline void   _mulle_objc_class_set_inheritance( struct _mulle_objc_class *cls, unsigned int inheritance)
{
   assert( (unsigned short) inheritance == inheritance);

   cls->inheritance = (unsigned short) inheritance;
}


static inline mulle_objc_hash_t   _mulle_objc_class_get_ivarhash( struct _mulle_objc_class *cls)
{
   return( cls->ivarhash);
}


static inline void   _mulle_objc_class_set_ivarhash( struct _mulle_objc_class *cls, mulle_objc_hash_t ivarhash)
{
   cls->ivarhash = ivarhash;
}



static inline struct _mulle_objc_method   *_mulle_objc_class_get_forwardmethod( struct _mulle_objc_class *cls)
{
   return( cls->forwardmethod);
}


static inline void   _mulle_objc_class_set_forwardmethod( struct _mulle_objc_class *cls, struct _mulle_objc_method *method)
{
   assert( ! method || (method->descriptor.methodid != MULLE_OBJC_NO_METHODID &&
                        method->descriptor.methodid != MULLE_OBJC_INVALID_METHODID));

   cls->forwardmethod = method;
}

//
// if we are a metaclass, the infraclass is the "actual" ''infraclass''
//
static inline struct _mulle_objc_class   *_mulle_objc_class_get_infraclass( struct _mulle_objc_class *cls)
{
   return( cls->infraclass);
}

// only used during new_classpair
static inline void   _mulle_objc_class_set_infraclass( struct _mulle_objc_class *cls,  struct _mulle_objc_class *infra)
{
   cls->infraclass = infra;
}


MULLE_C_ALWAYS_INLINE_NON_NULL_CONST_RETURN
static inline struct _mulle_objc_class   *_mulle_objc_class_unfailing_get_infraclass( struct _mulle_objc_class *cls)
{
   return( cls->infraclass);
}


static inline int   _mulle_objc_class_is_infraclass( struct _mulle_objc_class *cls)
{
   return( cls->infraclass == NULL);
}


static inline int   _mulle_objc_class_is_metaclass( struct _mulle_objc_class *cls)
{
   return( cls->infraclass != NULL);
}


static inline struct _mulle_objc_class   *_mulle_objc_class_get_metaclass( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_objectheader   *header;

   header = _mulle_objc_object_get_objectheader( (struct _mulle_objc_object *) cls);
   return( _mulle_objc_objectheader_get_isa( header));
}


static inline char   *_mulle_objc_class_get_classtypename( struct _mulle_objc_class *cls)
{
   return( _mulle_objc_class_is_metaclass( cls) ? "metaclass" : "infraclass");
}


int   _mulle_objc_class_set_state_bit( struct _mulle_objc_class *cls, unsigned int bit);

static inline unsigned int   _mulle_objc_class_get_state_bit( struct _mulle_objc_class *cls,
                                                              unsigned int bit)
{
   void   *old;

   old = _mulle_atomic_pointer_read( &cls->state);
   return( (unsigned int) (uintptr_t) old & bit);
}


//
// the placeholder is used for +instantiate in foundation
//
static inline struct _mulle_objc_object *   _mulle_objc_class_get_placeholder( struct _mulle_objc_class *cls)
{
   assert( _mulle_objc_class_is_infraclass( cls));
   return( _mulle_atomic_pointer_read( &cls->placeholder.pointer));
}


// 1: it has worked, 0: someone else was faster
int   _mulle_objc_class_set_placeholder( struct _mulle_objc_class *cls,
                                         struct _mulle_objc_object *obj);


//
// the "aux" placeholder uses the storage in the infraclass, which is otherwise
// unused. class clusters can use this to store an 'alloc' placeholder
//
static inline struct _mulle_objc_object *   _mulle_objc_class_get_auxplaceholder( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class   *meta;

   assert( _mulle_objc_class_is_infraclass( cls));
   meta = _mulle_objc_class_get_metaclass( cls);
   return( _mulle_atomic_pointer_read( &meta->placeholder.pointer));
}


// 1: it has worked, 0: someone else was faster
int   _mulle_objc_class_set_auxplaceholder( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_object *obj);



//
// version is kept in the infraclass
//
static inline uintptr_t   _mulle_objc_class_get_coderversion( struct _mulle_objc_class *cls)
{
   assert( _mulle_objc_class_is_infraclass( cls));
   return( (uintptr_t) _mulle_atomic_pointer_read( &cls->coderversion));
}

// 1: it has worked, 0: someone else was faster, can only be set once
int   _mulle_objc_class_set_coderversion( struct _mulle_objc_class *cls,
                                          uintptr_t value);


//
// version is kept in the infraclass
//
static inline unsigned int   _mulle_objc_class_get_taggedpointerindex( struct _mulle_objc_class *cls)
{
   assert( _mulle_objc_class_is_infraclass( cls));
   return( (unsigned int) (uintptr_t) _mulle_atomic_pointer_read( &cls->taggedpointerindex));
}

// 1: it has worked, 0: someone else was faster, can only be set once
int   _mulle_objc_class_set_taggedpointerindex( struct _mulle_objc_class *cls,
                                         unsigned int value);



static inline int   _mulle_objc_class_is_taggedpointerclass( struct _mulle_objc_class *cls)
{
   if( ! _mulle_objc_class_is_infraclass( cls))
      cls = _mulle_objc_class_get_infraclass( cls);
   return( _mulle_objc_class_get_taggedpointerindex( cls) != 0);
}

# pragma mark - convenience accessors

static inline unsigned int   _mulle_objc_class_count_depth( struct _mulle_objc_class *cls)
{
   unsigned int   depth;

   depth = (unsigned int) -1;
   do
   {
      ++depth;
      cls = _mulle_objc_class_get_superclass( cls);
   }
   while( cls);

   return( depth);
}


# pragma mark - method cache

static inline struct _mulle_objc_cache   *_mulle_objc_class_get_methodcache( struct _mulle_objc_class *cls)
{
   return( _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot));
}


# pragma mark - kvc caches

static inline struct _mulle_objc_kvccachepivot   *_mulle_objc_class_get_kvccachepivot( struct _mulle_objc_class *cls)
{
   return( &cls->kvc);
}


static inline struct mulle_allocator   *
   _mulle_objc_class_get_kvcinfo_allocator( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = _mulle_objc_class_get_runtime( cls);
   return( &runtime->memory.allocator);
}


static inline struct _mulle_objc_kvccache   *
_mulle_objc_class_get_empty_kvccache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = _mulle_objc_class_get_runtime( cls);
   return( (struct _mulle_objc_kvccache *) &runtime->empty_cache);
}


static inline int    _mulle_objc_class_set_kvcinfo( struct _mulle_objc_class *cls,
                                                   struct _mulle_objc_kvcinfo *info)
{
   struct _mulle_objc_kvccache        *empty_cache;
   struct _mulle_objc_kvccachepivot   *pivot;
   struct mulle_allocator             *allocator;

   pivot       = _mulle_objc_class_get_kvccachepivot( cls);
   allocator   = _mulle_objc_class_get_kvcinfo_allocator( cls);
   empty_cache = _mulle_objc_class_get_empty_kvccache( cls);

   return( _mulle_objc_kvccachepivot_set_kvcinfo( pivot, info, empty_cache, allocator));
}


static inline struct _mulle_objc_kvcinfo  *
_mulle_objc_class_lookup_kvcinfo( struct _mulle_objc_class *cls,
                                 char  *key)
{
   struct _mulle_objc_kvccachepivot   *pivot;
   struct _mulle_objc_kvccache        *cache;

   pivot = _mulle_objc_class_get_kvccachepivot( cls);
   cache = _mulle_objc_kvccachepivot_atomic_get_cache( pivot);
   return( _mulle_objc_kvccache_lookup_kvcinfo( cache, key));
}


static inline void
   _mulle_objc_class_invalidate_all_kvcinfos( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_kvccache        *empty_cache;
   struct _mulle_objc_kvccachepivot   *pivot;
   struct mulle_allocator             *allocator;

   pivot       = _mulle_objc_class_get_kvccachepivot( cls);
   allocator   = _mulle_objc_class_get_kvcinfo_allocator( cls);
   empty_cache = _mulle_objc_class_get_empty_kvccache( cls);

   _mulle_objc_kvccachepivot_invalidate( pivot, empty_cache, allocator);
}


# pragma mark - ivar lists

int   mulle_objc_class_add_ivarlist( struct _mulle_objc_class *cls,
                                      struct _mulle_objc_ivarlist *list);

void   mulle_objc_class_unfailing_add_ivarlist( struct _mulle_objc_class *cls,
                                                 struct _mulle_objc_ivarlist *list);


# pragma mark - class variables

static inline void   *_mulle_objc_class_get_cvar( struct _mulle_objc_class *cls, void *key)
{
   return( _mulle_concurrent_hashmap_lookup( &cls->cvars, (intptr_t) key));
}


static inline int   _mulle_objc_class_set_cvar( struct _mulle_objc_class *cls, void *key, void *value)
{
   return( _mulle_concurrent_hashmap_insert( &cls->cvars, (intptr_t) key, value));
}


static inline int   _mulle_objc_class_remove_cvar( struct _mulle_objc_class *cls, void *key, void *value)
{
   return( _mulle_concurrent_hashmap_remove( &cls->cvars, (intptr_t) key, value));
}


static inline struct mulle_concurrent_hashmapenumerator    _mulle_objc_class_enumerate_cvars( struct _mulle_objc_class *cls)
{
   return( mulle_concurrent_hashmap_enumerate( &cls->cvars));
}


# pragma mark - ivars

struct _mulle_objc_ivar   *_mulle_objc_class_search_ivar( struct _mulle_objc_class *cls,
                                                          mulle_objc_ivarid_t ivarid);


# pragma mark - properties

int   mulle_objc_class_add_propertylist( struct _mulle_objc_class *cls,
                                         struct _mulle_objc_propertylist *list);

void   mulle_objc_class_unfailing_add_propertylist( struct _mulle_objc_class *cls,
                                                    struct _mulle_objc_propertylist *list);

struct _mulle_objc_property   *_mulle_objc_class_search_property( struct _mulle_objc_class *cls,
                                                                  mulle_objc_propertyid_t propertyid);

struct _mulle_objc_property  *mulle_objc_class_search_property( struct _mulle_objc_class *cls,
                                                                mulle_objc_propertyid_t propertyid);

# pragma mark - methods

// this function doesn't invalidate caches!
int   _mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                        struct _mulle_objc_methodlist *list);

int   mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                       struct _mulle_objc_methodlist *list);

void   mulle_objc_class_did_add_methodlist( struct _mulle_objc_class *cls,
                                           struct _mulle_objc_methodlist *list);


static inline int   mulle_objc_class_add_instancemethodlist( struct _mulle_objc_class *cls,
                                                             struct _mulle_objc_methodlist *list)
{
   return( mulle_objc_class_add_methodlist( cls, list));
}


static inline int   mulle_objc_class_add_classmethodlist( struct _mulle_objc_class *cls,
                                                          struct _mulle_objc_methodlist *list)
{
   return( mulle_objc_class_add_methodlist( cls ? _mulle_objc_class_get_metaclass( cls) : cls, list));
}


void   mulle_objc_class_unfailing_add_methodlist( struct _mulle_objc_class *cls,
                                                  struct _mulle_objc_methodlist *list);

static inline void
mulle_objc_class_unfailing_add_instancemethodlist( struct _mulle_objc_class *cls,
                                                   struct _mulle_objc_methodlist *list)
{
   mulle_objc_class_unfailing_add_methodlist( cls, list);
}


static inline void
mulle_objc_class_unfailing_add_classmethodlist( struct _mulle_objc_class *cls,
                                               struct _mulle_objc_methodlist *list)
{
   mulle_objc_class_unfailing_add_methodlist( cls ? _mulle_objc_class_get_metaclass( cls) : cls, list);
}


static inline int   mulle_objc_class_find_instancemethodlist( struct _mulle_objc_class *cls,
                                                              struct _mulle_objc_methodlist *list)
{
   return( mulle_objc_class_add_methodlist( cls, list));
}


static inline int   mulle_objc_class_find_classmethodlist( struct _mulle_objc_class *cls,
                                                           struct _mulle_objc_methodlist *list)
{
   return( mulle_objc_class_add_methodlist( cls ? _mulle_objc_class_get_metaclass( cls) : cls, list));
}


// the way to find a category
struct _mulle_objc_methodlist   *mulle_objc_class_find_methodlist( struct _mulle_objc_class *cls,
                                                                   mulle_objc_categoryid_t categoryid);



unsigned int   _mulle_objc_class_count_preload_methods( struct _mulle_objc_class *cls);

//
// to find -methods ask isa of object, to find +methods, ask the metaclass of isa
// these methods are uncached and authorative
//
struct _mulle_objc_method  *mulle_objc_class_search_method( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);

struct _mulle_objc_method   *_mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                                              mulle_objc_methodid_t methodid,
                                                              struct _mulle_objc_method *previous,
                                                              void *owner,
                                                              unsigned int inheritance);

# pragma mark - categories


static inline int   _mulle_objc_class_has_category( struct _mulle_objc_class *cls, mulle_objc_categoryid_t categoryid)
{
   return( _mulle_concurrent_pointerarray_find( &cls->categoryids, (void *) (uintptr_t) categoryid));
}


static inline void   _mulle_objc_class_add_category( struct _mulle_objc_class *cls,
                                                     mulle_objc_categoryid_t categoryid)
{
   assert( cls);
   assert( categoryid != MULLE_OBJC_NO_CATEGORYID);
   assert( categoryid != MULLE_OBJC_INVALID_CATEGORYID);
   assert( ! _mulle_objc_class_has_category( cls, categoryid));
   
   _mulle_concurrent_pointerarray_add( &cls->categoryids, (void *) (uintptr_t) categoryid);
}


void   mulle_objc_class_unfailing_add_category( struct _mulle_objc_class *cls,
                                                mulle_objc_categoryid_t categoryid);


int   _mulle_objc_class_walk_categoryids( struct _mulle_objc_class *cls,
                                          int (*f)( mulle_objc_categoryid_t, struct  _mulle_objc_class *, void *),
                                          void *userinfo);

# pragma mark - protocols

void   _mulle_objc_class_add_protocol( struct _mulle_objc_class *cls,
                                       mulle_objc_protocolid_t uniqueid);
void   mulle_objc_class_unfailing_add_protocols( struct _mulle_objc_class *cls,
                                                 mulle_objc_protocolid_t *protocolids);

static inline int   _mulle_objc_class_has_protocol( struct _mulle_objc_class *cls,
                                                    mulle_objc_protocolid_t protocolid)
{
   return( _mulle_concurrent_pointerarray_find( &cls->protocolids, (void *) (uintptr_t) protocolid));
}

// searches hierarchy
int   _mulle_objc_class_conformsto_protocol( struct _mulle_objc_class *cls,
                                             mulle_objc_protocolid_t protocolid);


# pragma mark - protocol conveniences

struct _mulle_objc_protocolclassenumerator
{
   struct mulle_concurrent_pointerarrayenumerator   list_rover;
   struct _mulle_objc_class                         *cls;
};


static inline  struct _mulle_objc_protocolclassenumerator  _mulle_objc_class_enumerate_protocolclasses( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_protocolclassenumerator  rover;

   rover.list_rover = mulle_concurrent_pointerarray_enumerate( &cls->protocolids);
   rover.cls        = cls;
   return( rover);
}


static inline void  _mulle_objc_protocolclassenumerator_done( struct _mulle_objc_protocolclassenumerator *rover)
{
   mulle_concurrent_pointerarrayenumerator_done( &rover->list_rover);
}


static inline struct _mulle_objc_class  *_mulle_objc_protocolclassenumerator_get_class( struct _mulle_objc_protocolclassenumerator *rover)
{
   return( rover->cls);
}


struct _mulle_objc_class  *_mulle_objc_protocolclassenumerator_next( struct _mulle_objc_protocolclassenumerator *rover);


# pragma mark - forwarding

static inline int   _mulle_objc_class_is_forwardmethodimplementation( struct _mulle_objc_class *cls, mulle_objc_methodimplementation_t  imp)
{
   if( ! cls->forwardmethod)
      return( 0);
   return( _mulle_objc_method_get_implementation( cls->forwardmethod) == imp);
}


//
// this caches the forward method as a side effect
// the user_method is just there to create a nicer crash log
//
struct _mulle_objc_method    *_mulle_objc_class_get_or_search_forwardmethod( struct _mulle_objc_class *cls);


struct _mulle_objc_method    *_mulle_objc_class_unfailing_get_or_search_forwardmethod(
                                 struct _mulle_objc_class *cls,
                                 mulle_objc_methodid_t missing_method);

struct _mulle_objc_method  *mulle_objc_class_search_non_inherited_method( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);


MULLE_C_NON_NULL_RETURN
static inline struct _mulle_objc_method   *_mulle_objc_class_unfailing_search_method( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method   *method;

   method = _mulle_objc_class_search_method( cls, methodid, NULL, MULLE_OBJC_ANY_OWNER, _mulle_objc_class_get_inheritance( cls));
   assert( ! method || method->descriptor.methodid == methodid);
   if( ! method)
      method = _mulle_objc_class_unfailing_get_or_search_forwardmethod( cls, methodid);
   return( method);
}


# pragma mark - consistency

int   mulle_objc_class_is_sane( struct _mulle_objc_class *cls);

#pragma mark - walking

// use it like this, for finding something by name (though this will be slow)
//
// static int  find( struct _mulle_objc_method *method,
//                   struct _mulle_objc_class *cls,
//                   struct search_method_by_name *info)
// {
//    if( ! strcmp( _mulle_objc_class_get_name( method), info->name))
//    {
//       info->found = method;
//       return( 1);
//    }
//    return( 0);
// }
//
// struct search_method_by_name
// {
//    char                        *name;
//    struct _mulle_objc_method   *found;
// } info = { "whatever", 0 };
//
// if( _mulle_objc_class_walk_methods( cls, -1, (void *) find, &info))
//    return( info.found);
//
int   _mulle_objc_class_walk_methods( struct _mulle_objc_class *cls, unsigned int inheritance , int (*f)( struct _mulle_objc_method *, struct _mulle_objc_class *, void *), void *userinfo);
int   _mulle_objc_class_walk_ivars( struct _mulle_objc_class *cls, unsigned int inheritance , int (*f)( struct _mulle_objc_ivar *, struct _mulle_objc_class *, void *), void *userinfo);
int   _mulle_objc_class_walk_properties( struct _mulle_objc_class *cls, unsigned int inheritance , int (*f)( struct _mulle_objc_property *, struct _mulle_objc_class *, void *), void *userinfo);


// this function doesn't "inherit" it's just local protocol ids
int   _mulle_objc_class_walk_protocolids( struct _mulle_objc_class *cls,
                                          int (*f)( mulle_objc_protocolid_t, struct _mulle_objc_class *, void *),
                                          void *userinfo);


# pragma mark - API class

int  mulle_objc_class_is_protocol_class( struct _mulle_objc_class *cls);


static inline struct _mulle_objc_class   *mulle_objc_class_get_superclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_superclass( cls) : NULL);
}


static inline mulle_objc_classid_t   mulle_objc_class_get_classid( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_classid( cls) : MULLE_OBJC_NO_CLASSID);
}


static inline char   *mulle_objc_class_get_name( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_name( cls) : NULL);
}


static inline struct _mulle_objc_runtime   *mulle_objc_class_get_runtime( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_runtime( cls) : NULL);
}


static inline size_t   mulle_objc_class_get_instancesize( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_instancesize( cls) : (size_t) -1);
}

// deprecated
static inline size_t   mulle_objc_class_get_instance_size( struct _mulle_objc_class *cls)
{
   return( mulle_objc_class_get_instancesize( cls));
}


static inline unsigned int   mulle_objc_class_get_inheritance( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_inheritance( cls) : ~0);
}


static inline struct _mulle_objc_method   *mulle_objc_class_get_forwardmethod( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_forwardmethod( cls) : NULL);
}


static inline int   mulle_objc_class_is_infraclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_is_infraclass( cls) : 0);
}


static inline int   mulle_objc_class_is_metaclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_is_metaclass( cls) : 0);
}

//
// if we are a metaclass, the infraclass is the "actual" ''infraclass''
//
static inline struct _mulle_objc_class   *mulle_objc_class_get_infraclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_infraclass( cls) : NULL);
}


static inline struct _mulle_objc_class   *mulle_objc_class_get_metaclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_metaclass( cls) : 0);
}


# pragma mark - API protocols


static inline int   mulle_objc_class_conformsto_protocol( struct _mulle_objc_class *cls, mulle_objc_protocolid_t protocolid)
{
   if( ! cls)
      return( 0);
   return( _mulle_objc_class_conformsto_protocol( cls, protocolid));
}

// deprecated
static inline int   mulle_objc_class_conforms_to_protocol( struct _mulle_objc_class *cls, mulle_objc_protocolid_t protocolid)
{
   return( mulle_objc_class_conformsto_protocol( cls, protocolid));
}

void   mulle_objc_class_unfailing_add_protocols( struct _mulle_objc_class *cls,
                                                 mulle_objc_protocolid_t *protocolids);


// deprecated
static inline void   mulle_objc_class_add_protocol( struct _mulle_objc_class *cls,
                                                    mulle_objc_protocolid_t protocolid)
{
   auto mulle_objc_protocolid_t  ids[ 2] = { protocolid, 0 };

   mulle_objc_class_unfailing_add_protocols( cls, ids);
}



# pragma mark - API properties

struct _mulle_objc_property  *mulle_objc_class_search_property( struct _mulle_objc_class *cls,
                                                                mulle_objc_propertyid_t propertyid);


# pragma mark - API ivars

struct _mulle_objc_ivar  *mulle_objc_class_search_ivar( struct _mulle_objc_class *cls,
                                                        mulle_objc_ivarid_t ivarid);


#pragma mark - debug support

mulle_objc_runtime_walkcommand_t
   mulle_objc_classpair_walk( struct _mulle_objc_classpair *pair,
                              mulle_objc_runtime_walkcallback callback,
                              void *userinfo);

#endif


/*
 * #1#
 *  void  *reallocbuf = obj[ class->extensionoffset];
 *  use a pthread_key similiar scheme to register space for ivars, so
 *  the key is really just an offset into reallocbuf
 *  can only work, if no instances have been created yet
 */
