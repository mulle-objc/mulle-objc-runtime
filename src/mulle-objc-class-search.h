//
//  mulle_objc_class_search.h
//  mulle-objc-runtime
//
//  Created by Nat! on 17/08/03.
//  Copyright (c) 2014-2017 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014-2017 Codeon GmbH.
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
#ifndef mulle_objc_class_search_h__
#define mulle_objc_class_search_h__

#include "mulle-objc-uniqueid.h"
#include "mulle-objc-method.h"

#include "include.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

struct _mulle_objc_class;

//
// to find -methods ask isa of object, to find +methods, ask the metaclass of isa
// these methods are uncached and authorative
//
struct _mulle_objc_method  *mulle_objc_class_defaultsearch_method( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid);

enum
{
   MULLE_OBJC_SEARCH_INVALID           = -1,   // used by cache to build entry
   MULLE_OBJC_SEARCH_DEFAULT           = 0,    // not cacheable
   MULLE_OBJC_SEARCH_SUPER_METHOD      = 1,    // like [super call]
   MULLE_OBJC_SEARCH_OVERRIDDEN_METHOD = 2,    // find the method overridden by class,category
   MULLE_OBJC_SEARCH_SPECIFIC_METHOD   = 3,    // find method implemented in class,category
   MULLE_OBJC_SEARCH_PREVIOUS_METHOD   = 4     // not cacheable
};


//
// assume super is called most often
// then overridden then specific
//
#pragma mark - _mulle_objc_searchargumentscachable

struct _mulle_objc_searchargumentscachable
{
   union
   {
      // should be 16 bytes on 32bit and 32 bytes on 64 bit
      struct
      {
         mulle_objc_uniqueid_t  mode;
         mulle_objc_methodid_t  methodid;
         mulle_objc_classid_t   classid;
         mulle_objc_classid_t   categoryid;
      };
      mulle_atomic_functionpointer_t    pointer;
   };
};


//
// this must be as fast as possible, while still
// maintaing decent diffusion, as this affects call speed
// This is something the compiler can do for us at compile time!
//
static inline uintptr_t
   _mulle_objc_searchargumentscachable_hash( struct _mulle_objc_searchargumentscachable
                                          *args)
{
   uintptr_t   hash;

   hash  = args->classid;
   hash ^= args->categoryid;  // important that it's zero if unused!
   hash ^= args->methodid;
   hash ^= args->mode;

   return( hash);
}


static inline int
   _mulle_objc_searchargumentscachable_equals( struct _mulle_objc_searchargumentscachable
                                          *a,
                                               struct _mulle_objc_searchargumentscachable
                                                 *b)
{
   // in estimated order of likeliness of difference
   return( a->methodid   == b->methodid &&
           a->classid    == b->classid &&
           a->mode       == b->mode &&
           a->categoryid == b->categoryid);
}


static inline void
   _mulle_objc_searchargumentscacheable_assert( struct _mulle_objc_searchargumentscachable *p)
{
   assert( p);
   assert( p->methodid != MULLE_OBJC_NO_METHODID && p->methodid != MULLE_OBJC_INVALID_METHODID);
   assert( p->mode != MULLE_OBJC_SEARCH_SUPER_METHOD ||
          (p->classid != 0 && p->categoryid == MULLE_OBJC_INVALID_CATEGORYID));
   assert( p->mode != MULLE_OBJC_SEARCH_OVERRIDDEN_METHOD ||
          (p->classid != 0 && p->categoryid != MULLE_OBJC_INVALID_CATEGORYID));
   assert( p->mode != MULLE_OBJC_SEARCH_SPECIFIC_METHOD ||
          (p->classid != 0 && p->categoryid != MULLE_OBJC_INVALID_CATEGORYID));
}


// find the overridden method like [super foo] would:
// ```
// @implementation Y
// - foo {}
// @end
// @implementation X : Y
// - foo {}
// @end
// @implementation A : X
// - foo {}
// @end
// ```
// foo of 'Y' will be found with:
//   _mulle_objc_searcharguments_init_super( &search, @selector( foo), @selector( X))
//   mulle_objc_class_search_method( [A class], &search,....)
// foo of 'X' will be found with:
//   _mulle_objc_searcharguments_init_supreme( &search, @selector( foo), @selector( A))
//   mulle_objc_class_search_method( [A class], &search,....)
//
// Classid is the id of the implementation class, that does the call
//
static inline void   _mulle_objc_searchargumentscacheable_superinit( struct _mulle_objc_searchargumentscachable *p,
                                                             mulle_objc_methodid_t methodid,

                                                             mulle_objc_classid_t classid)
{
   p->mode       = MULLE_OBJC_SEARCH_SUPER_METHOD;
   p->methodid   = methodid;
   p->classid    = classid;
   p->categoryid = MULLE_OBJC_INVALID_CATEGORYID;  // 4 cache and asserts
}


// find the directly overridden method:
// ```
// @implementation A
// - foo {}
// @end
// @implementation A(B)
// - foo {}
// @end
// ```
// foo of 'A' will be found with:
//   _mulle_objc_searcharguments_init_overridden( &search, @selector( A), @selector( B))
//   mulle_objc_class_search_method( [A class], @selector( foo), &search,....)
//
// Classid, categoryid is the id of the implementation class/category, that does the call
//
static inline void   _mulle_objc_searchargumentscacheable_overriddeninit( struct _mulle_objc_searchargumentscachable *p,
                                                                  mulle_objc_methodid_t methodid,
                                                                  mulle_objc_classid_t classid,
                                                                  mulle_objc_categoryid_t categoryid)
{
   p->mode       = MULLE_OBJC_SEARCH_OVERRIDDEN_METHOD;
   p->methodid   = methodid;
   p->classid    = classid;
   p->categoryid = categoryid;
}


// find implementation of a category exactly:
// ```
// @implementation A(B)
// - foo {}
// @end
// ```
// will be found with:
//   _mulle_objc_searcharguments_init_specific( &search, @selector( A), @selector( B))
//   mulle_objc_class_search_method( [A class], @selector( foo), &search,....)
//
static inline void   _mulle_objc_searchargumentscacheable_specificinit( struct _mulle_objc_searchargumentscachable *p,
                                                                mulle_objc_methodid_t methodid,
                                                                mulle_objc_classid_t classid,
                                                                mulle_objc_categoryid_t categoryid)
{
   p->mode       = MULLE_OBJC_SEARCH_SPECIFIC_METHOD;
   p->methodid   = methodid;
   p->classid    = classid;
   p->categoryid = categoryid;
}

#pragma mark - _mulle_objc_searcharguments

struct _mulle_objc_searcharguments
{
   struct _mulle_objc_searchargumentscachable    args;
   struct _mulle_objc_method                     *previous_method;    // default 0
};


static inline int  _mulle_objc_is_cacheablesearchmode( intptr_t mode)
{
   return( mode >= MULLE_OBJC_SEARCH_SUPER_METHOD &&
           mode <= MULLE_OBJC_SEARCH_SPECIFIC_METHOD);
}


static inline void
   _mulle_objc_searcharguments_assert( struct _mulle_objc_searcharguments *p)
{
   _mulle_objc_searchargumentscacheable_assert( &p->args);

   assert( p->args.mode >= MULLE_OBJC_SEARCH_DEFAULT && p->args.mode <= MULLE_OBJC_SEARCH_PREVIOUS_METHOD);
   assert( p->args.mode != MULLE_OBJC_SEARCH_PREVIOUS_METHOD ||
          (p->previous_method != 0));
}


static inline void
   _mulle_objc_searcharguments_previousmethodinit( struct _mulle_objc_searcharguments *p,
                                                   struct _mulle_objc_method *method)
{
   p->args.mode       = MULLE_OBJC_SEARCH_PREVIOUS_METHOD;
   p->args.methodid   = _mulle_objc_method_get_methodid( method);
   p->previous_method = method;
}



static inline void
   _mulle_objc_searcharguments_defaultinit( struct _mulle_objc_searcharguments *p,
                                            mulle_objc_methodid_t methodid)
{
   p->args.mode     = MULLE_OBJC_SEARCH_DEFAULT;
   p->args.methodid = methodid;
}


static inline void   _mulle_objc_searcharguments_superinit( struct _mulle_objc_searcharguments *p,
                                                             mulle_objc_methodid_t methodid,

                                                             mulle_objc_classid_t classid)
{
   _mulle_objc_searchargumentscacheable_superinit( &p->args, methodid, classid);
}


static inline void   _mulle_objc_searcharguments_overriddeninit( struct _mulle_objc_searcharguments *p,
                                                             mulle_objc_methodid_t methodid,

                                                             mulle_objc_classid_t classid,
                                                             mulle_objc_classid_t category)
{
   _mulle_objc_searchargumentscacheable_overriddeninit( &p->args, methodid, classid, category);
}


static inline void   _mulle_objc_searcharguments_specificinit( struct _mulle_objc_searcharguments *p,
                                                             mulle_objc_methodid_t methodid,

                                                             mulle_objc_classid_t classid,
                                                             mulle_objc_classid_t category)
{
   _mulle_objc_searchargumentscacheable_specificinit( &p->args, methodid, classid, category);
}



struct _mulle_objc_searchresult
{
   struct _mulle_objc_class       *class;    // where method was found
   struct _mulle_objc_methodlist  *list;     // list containing method
   struct _mulle_objc_method      *method;   // method
};


//
// cls: class to start search
// methodid: method to search for
// search: modify search to not start at the default last methodlist (usually NULL)
// inheritance: the inheritance scheme to use (use cls->inheritance)
// result: returned if a method was found (otherwise unchanged!), can be NULL
//
struct _mulle_objc_method   *
   mulle_objc_class_search_method( struct _mulle_objc_class *cls,
                                   struct _mulle_objc_searcharguments *search,
                                   unsigned int inheritance,
                                   struct _mulle_objc_searchresult *result);


# pragma mark - forwarding

//
// this should be used by the debugger, but the debugger is really fixed to
// use __forward_mulle_objc_object_call
//
mulle_objc_implementation_t
   mulle_objc_class_get_forwardimplementation( struct _mulle_objc_class *cls);


//
// this caches the forward method as a side effect
// the user_method is just there to create a nicer crash log
//
struct _mulle_objc_method   *
   _mulle_objc_class_lazyget_forwardmethod( struct _mulle_objc_class *cls);


MULLE_C_NON_NULL_RETURN struct _mulle_objc_method *
   _mulle_objc_class_unfailinglazyget_forwardmethod(
                              struct _mulle_objc_class *cls,
                              mulle_objc_methodid_t missing_method);

struct _mulle_objc_method  *
   mulle_objc_class_search_non_inherited_method( struct _mulle_objc_class *cls,
                                                 mulle_objc_methodid_t methodid);


MULLE_C_NON_NULL_RETURN static inline struct _mulle_objc_method *
   mulle_objc_class_unfailingsearch_method( struct _mulle_objc_class *cls,
                                            mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method   *method;

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   assert( ! method || method->descriptor.methodid == methodid);
   if( ! method)
      method = _mulle_objc_class_unfailinglazyget_forwardmethod( cls, methodid);
   return( method);
}


#pragma mark - cached searches

// will return NULL when not found and not forward!
mulle_objc_implementation_t
   _mulle_objc_class_lookup_superimplementation( struct _mulle_objc_class *cls,
                                                       mulle_objc_superid_t superid);

#endif /* mulle_objc_class_search_h */
