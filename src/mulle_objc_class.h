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
//

#ifndef mulle_objc_class_h__
#define mulle_objc_class_h__

#include "mulle_objc_objectheader.h"
#include "mulle_objc_class_struct.h"
#include "mulle_objc_universe_struct.h"
#include "mulle_objc_walktypes.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>



# pragma mark - exceptions

void   _mulle_objc_class_raise_null_exception( void)   MULLE_C_NO_RETURN;


# pragma mark - method alloc / free

void  _mulle_objc_class_init( struct _mulle_objc_class *cls,
                              char *name,
                              size_t  instancesize,
                              mulle_objc_classid_t classid,
                              struct _mulle_objc_class *superclass,
                              struct _mulle_objc_universe *universe);

void   _mulle_objc_class_done( struct _mulle_objc_class *class,
                               struct mulle_allocator *allocator);

void   _mulle_objc_class_setup_pointerarrays( struct _mulle_objc_class *cls,
                                              struct _mulle_objc_universe *universe);


# pragma mark - sanity check

// fast enough for a quick assert
int   __mulle_objc_class_is_sane( struct _mulle_objc_class *cls);

int   _mulle_objc_class_is_sane( struct _mulle_objc_class *cls);


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
   struct _mulle_objc_universe   *universe;

   universe = _mulle_objc_class_get_universe( cls);
   return( &universe->memory.allocator);
}


static inline struct _mulle_objc_kvccache   *
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


# pragma mark - methods

// this function doesn't invalidate caches!
int   _mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                        struct _mulle_objc_methodlist *list);

int   mulle_objc_class_add_methodlist( struct _mulle_objc_class *cls,
                                       struct _mulle_objc_methodlist *list);

void   mulle_objc_class_did_add_methodlist( struct _mulle_objc_class *cls,
                                           struct _mulle_objc_methodlist *list);


void   mulle_objc_class_unfailing_add_methodlist( struct _mulle_objc_class *cls,
                                                  struct _mulle_objc_methodlist *list);


// the way to find a category
struct _mulle_objc_methodlist   *mulle_objc_class_find_methodlist( struct _mulle_objc_class *cls,
                                                                   mulle_objc_categoryid_t categoryid);



unsigned int   _mulle_objc_class_count_preloadmethods( struct _mulle_objc_class *cls);

//
// to find -methods ask isa of object, to find +methods, ask the metaclass of isa
// these methods are uncached and authorative
//
struct _mulle_objc_method  *mulle_objc_class_search_method( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);

//
// cls: class to start search
// methodid: method to search for
// previous: the method that overrides the searched method (usually NULL)
// owner: search for a specific owner of the methodlist (usually MULLE_OBJC_ANY_OWNER)
// inheritance: the inheritance scheme to use (use cls->inheritance)
// result: returned if a method was found (otherwise unchanged!), can be NULL
//
struct _mulle_objc_searchresult
{
   struct _mulle_objc_class       *class;    // where method was found
   struct _mulle_objc_methodlist  *list;     // list containing method
   struct _mulle_objc_method      *method;   // method
};


struct _mulle_objc_method   *
   _mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                    mulle_objc_methodid_t methodid,
                                    struct _mulle_objc_method *previous,
                                    void *owner,
                                    unsigned int inheritance,
                                    struct _mulle_objc_searchresult *result);


# pragma mark - forwarding

static inline int   _mulle_objc_class_is_forwardmethodimplementation( struct _mulle_objc_class *cls, mulle_objc_methodimplementation_t  imp)
{
   if( ! cls->forwardmethod)
      return( 0);
   return( _mulle_objc_method_get_implementation( cls->forwardmethod) == imp);
}

//
// this should be used by the debugger, but the debugger is really fixed to
// use __forward_mulle_objc_object_call
//
mulle_objc_methodimplementation_t
   mulle_objc_class_get_forwardmethodimplementation( struct _mulle_objc_class *cls);


//
// this caches the forward method as a side effect
// the user_method is just there to create a nicer crash log
//
struct _mulle_objc_method    *_mulle_objc_class_getorsearch_forwardmethod( struct _mulle_objc_class *cls);


struct _mulle_objc_method    *_mulle_objc_class_unfailing_getorsearch_forwardmethod(
                                 struct _mulle_objc_class *cls,
                                 mulle_objc_methodid_t missing_method);

struct _mulle_objc_method  *mulle_objc_class_search_non_inherited_method( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);


MULLE_C_NON_NULL_RETURN
static inline struct _mulle_objc_method   *_mulle_objc_class_unfailing_search_method( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method   *method;

   method = _mulle_objc_class_search_method( cls, methodid,
                                             NULL, MULLE_OBJC_ANY_OWNER,
                                            _mulle_objc_class_get_inheritance( cls),
                                            NULL);
   assert( ! method || method->descriptor.methodid == methodid);
   if( ! method)
      method = _mulle_objc_class_unfailing_getorsearch_forwardmethod( cls, methodid);
   return( method);
}



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



# pragma mark - API class


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


static inline struct _mulle_objc_universe   *mulle_objc_class_get_universe( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_universe( cls) : NULL);
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
// if we are a infraclass, the result is NULL!
static inline struct _mulle_objc_infraclass   *mulle_objc_class_get_infraclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_infraclass( cls) : NULL);
}


static inline struct _mulle_objc_metaclass   *mulle_objc_class_get_metaclass( struct _mulle_objc_class *cls)
{
   return( cls ? _mulle_objc_class_get_metaclass( cls) : 0);
}



mulle_objc_walkcommand_t
   mulle_objc_class_walk( struct _mulle_objc_class   *cls,
                          enum mulle_objc_walkpointertype_t  type,
                          mulle_objc_walkcallback_t   callback,
                          void *parent,
                          void *userinfo);

#endif


/*
 * #1#
 *  void  *reallocbuf = obj[ class->extensionoffset];
 *  use a pthread_key similiar scheme to register space for ivars, so
 *  the key is really just an offset into reallocbuf
 *  can only work, if no instances have been created yet
 */
