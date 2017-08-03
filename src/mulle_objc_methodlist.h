//
//  mulle_objc_methodlist.h
//  mulle-objc-runtime
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
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
#ifndef mulle_objc_methodlist_h__
#define mulle_objc_methodlist_h__

#include "mulle_objc_method.h"

#include <assert.h>


struct _mulle_objc_class;
struct _mulle_objc_callqueue;
struct _mulle_objc_metaclass;

//
// methods have to be sorted by methodid
// its better if the compiler can do this
//
struct _mulle_objc_methodlist
{
   unsigned int                n_methods; // must be #0 and same as struct _mulle_objc_ivarlist
   void                        *owner;    // it's a void because of compiler
   struct _mulle_objc_method   methods[ 1];
};


static inline size_t   mulle_objc_sizeof_methodlist( unsigned int n_methods)
{
   return( sizeof( struct _mulle_objc_methodlist) + (n_methods - 1) * sizeof( struct _mulle_objc_method));
}


void   mulle_objc_methodlist_sort( struct _mulle_objc_methodlist *list);

struct _mulle_objc_method  *_mulle_objc_methodlist_linear_search( struct _mulle_objc_methodlist *list,
                                                                  mulle_objc_methodid_t methodid);

static inline struct _mulle_objc_method  *_mulle_objc_methodlist_binary_search( struct _mulle_objc_methodlist *list,
                                                                   mulle_objc_methodid_t methodid)
{
   return( _mulle_objc_method_bsearch( list->methods, list->n_methods, methodid));
}


static inline struct _mulle_objc_method  *_mulle_objc_methodlist_search( struct _mulle_objc_methodlist *list,
                                                                                 mulle_objc_methodid_t methodid)
{
   if( list->n_methods >= 14) // 14 is a researched value (i7)
      return( _mulle_objc_methodlist_binary_search( list, methodid));
   return( _mulle_objc_methodlist_linear_search( list, methodid));
}


# pragma mark - +load

int   mulle_objc_methodlist_add_load_to_callqueue( struct _mulle_objc_methodlist *list,
                                                   struct _mulle_objc_metaclass *cls,
                                                   struct _mulle_objc_callqueue *loads);

void   mulle_objc_methodlist_unfailingadd_load_to_callqueue( struct _mulle_objc_methodlist *list,
                                                              struct _mulle_objc_metaclass *cls,
                                                              struct _mulle_objc_callqueue *loads);


# pragma mark - Enumerator

struct _mulle_objc_methodlistenumerator
{
   struct _mulle_objc_method   *sentinel;
   struct _mulle_objc_method   *method;
};


static inline struct  _mulle_objc_methodlistenumerator   _mulle_objc_methodlist_enumerate( struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_methodlistenumerator   rover;

   rover.method   = &list->methods[ 0];
   rover.sentinel = &list->methods[ list->n_methods];

   assert( rover.sentinel >= rover.method);

   return( rover);
}


static inline struct _mulle_objc_method   *_mulle_objc_methodlistenumerator_next( struct _mulle_objc_methodlistenumerator *rover)
{
   return( rover->method < rover->sentinel ? rover->method++ : 0);
}


static inline void  _mulle_objc_methodlistenumerator_done( struct _mulle_objc_methodlistenumerator *rover)
{
}


# pragma mark - Method Walker

//
// supply cls and userinfo for callback, the cls is kinda ugly,
// but it's easier this way (no need to reorganize userinfo)
//
int   _mulle_objc_methodlist_walk( struct _mulle_objc_methodlist *list,
                                   int (*f)( struct _mulle_objc_method *, struct _mulle_objc_class *, void *),
                                   struct _mulle_objc_class *cls,
                                   void *userinfo);

# pragma mark - methodlist API

static inline int   mulle_objc_methodlist_walk( struct _mulle_objc_methodlist *list,
                                                int (*f)( struct _mulle_objc_method *, struct _mulle_objc_class *, void *),
                                                struct _mulle_objc_class *cls,
                                                void *userinfo)
{
   if( ! list || ! f)
      return( -1);
   return( _mulle_objc_methodlist_walk( list, f, cls, userinfo));
}


static inline struct  _mulle_objc_methodlistenumerator   mulle_objc_methodlist_enumerate( struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_methodlistenumerator   rover;

   if( ! list)
   {
      rover.method   = NULL;
      rover.sentinel = NULL;
      return( rover);
   }
   return( _mulle_objc_methodlist_enumerate( list));
}


static inline struct _mulle_objc_method   *mulle_objc_methodlistenumerator_next( struct _mulle_objc_methodlistenumerator *rover)
{
   return( rover ? _mulle_objc_methodlistenumerator_next( rover) : NULL);
}


static inline void  mulle_objc_methodlistenumerator_done( struct _mulle_objc_methodlistenumerator *rover)
{
   if( rover)
      _mulle_objc_methodlistenumerator_done( rover);
}

#endif /* defined(__MULLE_OBJC__mulle_objc_methodlist__) */
