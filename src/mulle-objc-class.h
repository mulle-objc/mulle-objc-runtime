//
//  mulle_objc_class.h
//  mulle-objc-runtime
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
//
#ifndef mulle_objc_class_h__
#define mulle_objc_class_h__

#include "include.h"

#include "mulle-objc-objectheader.h"
#include "mulle-objc-class-struct.h"
#include "mulle-objc-universe-struct.h"
#include "mulle-objc-walktypes.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>



# pragma mark - exceptions

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_class_raise_null_exception( void)   _MULLE_C_NO_RETURN;


# pragma mark - class alloc / free

MULLE_OBJC_RUNTIME_GLOBAL
void  _mulle_objc_class_init( struct _mulle_objc_class *cls,
                              char *name,
                              size_t instancesize,
                              size_t headerextrasize,
                              mulle_objc_classid_t classid,
                              struct _mulle_objc_class *superclass,
                              struct _mulle_objc_universe *universe);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_class_done( struct _mulle_objc_class *class,
                               struct mulle_allocator *allocator);

MULLE_OBJC_RUNTIME_GLOBAL
void   _mulle_objc_class_setup_pointerarrays( struct _mulle_objc_class *cls,
                                              struct _mulle_objc_universe *universe);


# pragma mark - sanity check

// fast enough for a quick assert
MULLE_OBJC_RUNTIME_GLOBAL
int   __mulle_objc_class_is_sane( struct _mulle_objc_class *cls);

MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_class_is_sane( struct _mulle_objc_class *cls);



# pragma mark - stuff that can not be in struct

static inline size_t
   _mulle_objc_class_get_instancesize( struct _mulle_objc_class *cls)
{
   return( cls->allocationsize -
           sizeof( struct _mulle_objc_objectheader) -
           cls->headerextrasize);
}


// deprecated
static inline size_t
   _mulle_objc_class_get_instance_size( struct _mulle_objc_class *cls)
{
   return( _mulle_objc_class_get_instancesize( cls));
}


static inline struct _mulle_objc_metaclass   *
   _mulle_objc_class_get_metaclass( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_objectheader   *header;

   header = _mulle_objc_object_get_objectheader( (struct _mulle_objc_object *) cls);
   return( (struct _mulle_objc_metaclass *) _mulle_objc_objectheader_get_isa( header));
}



# pragma mark - convenience accessors

static inline unsigned int
   _mulle_objc_class_count_depth( struct _mulle_objc_class *cls)
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


# pragma mark - method caches

static inline struct _mulle_objc_impcachepivot *
   _mulle_objc_class_get_impcachepivot( struct _mulle_objc_class *cls)
{
   return( &cls->cachepivot);
}


static inline struct _mulle_objc_impcache *
   _mulle_objc_class_get_impcache( struct _mulle_objc_class *cls)
{
   return( _mulle_objc_impcachepivot_get_impcache( &cls->cachepivot));
}


static inline struct _mulle_objc_cache *
   _mulle_objc_class_get_impcache_cache( struct _mulle_objc_class *cls)
{
   return( _mulle_objc_impcachepivot_get_impcache_cache( &cls->cachepivot));
}


MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_class_invalidate_impcacheentry( struct _mulle_objc_class *cls,
                                                     mulle_objc_methodid_t methodid);

static inline void
   mulle_objc_class_invalidate_impcache( struct _mulle_objc_class *cls)
{
   if( ! cls)
      return;
   _mulle_objc_class_invalidate_impcacheentry( cls,
                                                  MULLE_OBJC_NO_METHODID);
}


//static inline struct _mulle_objc_cache   *_mulle_objc_class_get_supercache( struct _mulle_objc_class *cls)
//{
//   return( _mulle_objc_cachepivot_get_cache_atomic( &cls->supercachepivot));
//}

# pragma mark - kvc caches

static inline struct _mulle_objc_kvccachepivot *
   _mulle_objc_class_get_kvccachepivot( struct _mulle_objc_class *cls)
{
   return( &cls->kvc);
}


static inline struct mulle_allocator *
   _mulle_objc_class_get_kvcinfo_allocator( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   return( _mulle_objc_universe_get_allocator( universe));
}


static inline struct _mulle_objc_kvccache *
   _mulle_objc_class_get_empty_kvccache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   return( (struct _mulle_objc_kvccache *) &universe->empty_cache);
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
   cache = _mulle_objc_kvccachepivot_get_cache_atomic( pivot);
   return( _mulle_objc_kvccache_lookup_kvcinfo( cache, key));
}


static inline void
   _mulle_objc_class_invalidate_kvccache( struct _mulle_objc_class *cls)
{
   struct _mulle_objc_kvccache        *empty_cache;
   struct _mulle_objc_kvccachepivot   *pivot;
   struct mulle_allocator             *allocator;

   pivot       = _mulle_objc_class_get_kvccachepivot( cls);
   allocator   = _mulle_objc_class_get_kvcinfo_allocator( cls);
   empty_cache = _mulle_objc_class_get_empty_kvccache( cls);

   _mulle_objc_kvccachepivot_invalidate( pivot, empty_cache, allocator);
}


# pragma mark - methodlists

// this function doesn't invalidate caches!
MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                        struct _mulle_objc_methodlist *list);

MULLE_OBJC_RUNTIME_GLOBAL
int   mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                       struct _mulle_objc_methodlist *list);

MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_class_didadd_methodlist( struct _mulle_objc_class *cls,
                                           struct _mulle_objc_methodlist *list);


MULLE_OBJC_RUNTIME_GLOBAL
void   mulle_objc_class_add_methodlist_nofail( struct _mulle_objc_class *cls,
                                                 struct _mulle_objc_methodlist *list);


// the way to find a category
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_methodlist *
   mulle_objc_class_find_methodlist( struct _mulle_objc_class *cls,
                                     mulle_objc_categoryid_t categoryid);

MULLE_OBJC_RUNTIME_GLOBAL
unsigned int   _mulle_objc_class_count_preloadmethods( struct _mulle_objc_class *cls);


#pragma mark - walking

MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_method  *
   _mulle_objc_class_lookup_method( struct _mulle_objc_class *cls,
                                    mulle_objc_methodid_t methodid);


static inline struct _mulle_objc_method  *
   mulle_objc_class_lookup_method( struct _mulle_objc_class *cls,
                                   mulle_objc_methodid_t methodid)
{
   return( cls ? _mulle_objc_class_lookup_method( cls, methodid) : NULL);
}


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

MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_walkcommand_t
   _mulle_objc_class_walk_methods( struct _mulle_objc_class *cls,
                                   unsigned int inheritance,
                                   mulle_objc_method_walkcallback_t callback,
                                   void *userinfo);


static inline int
   _mulle_objc_class_is_forwardimplementation( struct _mulle_objc_class *cls,
                                               mulle_objc_implementation_t  imp)
{
   if( ! cls->forwardmethod)
      return( 0);
   return( _mulle_objc_method_get_implementation( cls->forwardmethod) == imp);
}


# pragma mark - API class


static inline struct _mulle_objc_class   *
   mulle_objc_class_get_superclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_superclass( cls) : NULL);
}


static inline mulle_objc_classid_t
   mulle_objc_class_get_classid( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_classid( cls) : MULLE_OBJC_NO_CLASSID);
}


static inline char   *mulle_objc_class_get_name( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_name( cls) : NULL);
}


static inline struct _mulle_objc_universe *
   mulle_objc_class_get_universe( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_universe( cls) : NULL);
}


static inline size_t
   mulle_objc_class_get_instancesize( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_instancesize( cls) : (size_t) -1);
}

// deprecated
static inline size_t
   mulle_objc_class_get_instance_size( struct _mulle_objc_class *cls)
{
   return( mulle_objc_class_get_instancesize( cls));
}


static inline unsigned int
   mulle_objc_class_get_inheritance( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_inheritance( cls) : ~0);
}


static inline struct _mulle_objc_method *
   mulle_objc_class_get_forwardmethod( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_forwardmethod( cls) : NULL);
}


static inline int
   mulle_objc_class_is_infraclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_is_infraclass( cls) : 0);
}


static inline int
   mulle_objc_class_is_metaclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_is_metaclass( cls) : 0);
}

//
// if we are a metaclass, the infraclass is the "actual" ''infraclass''
// if we are a infraclass, the result is NULL!
static inline struct _mulle_objc_infraclass   *
   mulle_objc_class_get_infraclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_infraclass( cls) : NULL);
}


static inline struct _mulle_objc_metaclass   *
   mulle_objc_class_get_metaclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_metaclass( cls) : 0);
}


MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_walkcommand_t
   mulle_objc_class_walk( struct _mulle_objc_class   *cls,
                          enum mulle_objc_walkpointertype_t  type,
                          mulle_objc_walkcallback_t   callback,
                          void *parent,
                          void *userinfo);

// checks if a is subclass of b or b is subclass of a, does not check for
// NULL. 1=YES 0=NO
MULLE_OBJC_RUNTIME_GLOBAL
int   _mulle_objc_class_has_direct_relation_to_class( struct _mulle_objc_class *a,
                                                      struct _mulle_objc_class *b);

#endif


/*
 * #1#
 *  void  *reallocbuf = obj[ class->extensionoffset];
 *  use a pthread_key similiar scheme to register space for ivars, so
 *  the key is really just an offset into reallocbuf
 *  can only work, if no instances have been created yet
 */
