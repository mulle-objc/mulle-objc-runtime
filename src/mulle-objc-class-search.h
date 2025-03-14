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

#include "include.h"

#include "mulle-objc-uniqueid.h"
#include "mulle-objc-class.h"
#include "mulle-objc-method.h"
#include "mulle-objc-methodlist.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

struct _mulle_objc_class;


// Searches walk the class / methodlist hierarchy. This isn't fast. That's
// why there are caches. Use "lookup" for checking the cache, before hitting
// the cache

//
// to find -methods ask isa of object, to find +methods, ask the metaclass of isa
// these methods are uncached and authorative
//
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_method  *
   _mulle_objc_class_defaultsearch_method( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t methodid,
                                           int *error);
static inline struct _mulle_objc_method  *
   mulle_objc_class_defaultsearch_method( struct _mulle_objc_class *cls,
                                          mulle_objc_methodid_t methodid)
{
   int   error;

   if( ! cls)
      return( NULL);
   return( _mulle_objc_class_defaultsearch_method( cls, methodid, &error));
}


enum
{
   MULLE_OBJC_SEARCH_INVALID               = -1,   // used by cache to build entry
   MULLE_OBJC_SEARCH_DEFAULT               = 0,    // not cacheable
   MULLE_OBJC_SEARCH_IMP                   = 1,    // not cacheable
   MULLE_OBJC_SEARCH_SUPER_METHOD          = 2,    // like [super call]
   MULLE_OBJC_SEARCH_OVERRIDDEN_METHOD     = 3,    // find the method overridden by class,category
   MULLE_OBJC_SEARCH_SPECIFIC_METHOD       = 4,    // find method implemented in class,category
   MULLE_OBJC_SEARCH_PREVIOUS_METHOD       = 5,    // not cacheable
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
         mulle_objc_uniqueid_t         mode;
         mulle_objc_methodid_t         methodid;
         mulle_objc_classid_t          classid;
         mulle_objc_classid_t          categoryid;
      };
      mulle_atomic_functionpointer_t   pointer;
   };
};


//
// ***NOT USED ATM***
//
// this must be as fast as possible, while still
// maintaining decent diffusion, as this affects call speed
// This is something the compiler can do for us at compile time!
//
static inline uintptr_t
   _mulle_objc_searchargumentscachable_hash( struct _mulle_objc_searchargumentscachable *args)
{
   uintptr_t   hash;

   hash  = args->classid;
   hash ^= args->categoryid;  // important that it's zero if unused!
   hash ^= args->methodid;
   hash ^= args->mode;

   return( hash);
}


static inline int
   _mulle_objc_searchargumentscachable_equals( struct _mulle_objc_searchargumentscachable *a,
                                               struct _mulle_objc_searchargumentscachable *b)
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

   assert( (p->mode == MULLE_OBJC_SEARCH_IMP) ||
          (p->methodid != MULLE_OBJC_NO_METHODID && p->methodid != MULLE_OBJC_INVALID_METHODID));

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
// MEMO: used to use "init" functions to initialize the struct, but this
//       should be none worse, and we get zero init of unused fields for free
//       which seems to be a good thing for valgrind. (Also we do some
//       sanity asserts, but searching should not be super time criticial,
//       since the result oughta be cached)
//
static inline struct _mulle_objc_searchargumentscachable
  mulle_objc_searchargumentscacheable_make_super( mulle_objc_methodid_t methodid,
                                                  mulle_objc_classid_t classid)
{
   return( (struct _mulle_objc_searchargumentscachable)
   {
      .mode       = MULLE_OBJC_SEARCH_SUPER_METHOD,
      .methodid   = methodid,
      .classid    = classid,
      .categoryid = MULLE_OBJC_INVALID_CATEGORYID  // 4 cache and asserts
   });
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
static inline struct _mulle_objc_searchargumentscachable
   mulle_objc_searchargumentscacheable_make_overridden( mulle_objc_methodid_t methodid,
                                                        mulle_objc_classid_t classid,
                                                        mulle_objc_categoryid_t categoryid)
{
   return( (struct _mulle_objc_searchargumentscachable)
   {
      .mode       = MULLE_OBJC_SEARCH_OVERRIDDEN_METHOD,
      .methodid   = methodid,
      .classid    = classid,
      .categoryid = categoryid
   });
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
static inline struct _mulle_objc_searchargumentscachable
   mulle_objc_searchargumentscacheable_make_specific( mulle_objc_methodid_t methodid,
                                                      mulle_objc_classid_t classid,
                                                      mulle_objc_categoryid_t categoryid)
{
   return( (struct _mulle_objc_searchargumentscachable)
   {
      .mode       = MULLE_OBJC_SEARCH_SPECIFIC_METHOD,
      .methodid   = methodid,
      .classid    = classid,
      .categoryid = categoryid
   });
}


#pragma mark - _mulle_objc_searcharguments

struct _mulle_objc_searcharguments;
struct _mulle_objc_searchresult;


// userinfo in args
typedef int   mulle_objc_search_callback_t( struct _mulle_objc_class *cls,
                                            struct _mulle_objc_searcharguments *search,
                                            unsigned int inheritance,
                                            struct _mulle_objc_searchresult *result);



struct _mulle_objc_searcharguments
{
   struct _mulle_objc_searchargumentscachable   args;
   mulle_objc_implementation_t                  imp;
   struct _mulle_objc_method                    *previous_method;
   mulle_objc_classid_t                         stop_classid;
   mulle_objc_search_callback_t                 *callback;
   void                                         *userinfo;
   int                                          initialize;
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
   assert( (p->args.mode == MULLE_OBJC_SEARCH_PREVIOUS_METHOD && p->previous_method != 0) ||
           (p->args.mode != MULLE_OBJC_SEARCH_PREVIOUS_METHOD && p->previous_method == 0));
}


static inline struct _mulle_objc_searcharguments
   mulle_objc_searcharguments_make_default( mulle_objc_methodid_t methodid)
{
   return( (struct _mulle_objc_searcharguments)
   {
      .args =
      {
         .mode     = MULLE_OBJC_SEARCH_DEFAULT,
         .methodid = methodid
      },
      .initialize = 1
   });
}



static inline struct _mulle_objc_searcharguments
   mulle_objc_searcharguments_make_imp( mulle_objc_implementation_t imp)
{
   return( (struct _mulle_objc_searcharguments)
   {
      .args =
      {
         .mode  = MULLE_OBJC_SEARCH_IMP,
      },
      .imp = imp,
      .initialize = 1
   });
}


static inline struct _mulle_objc_searcharguments
   mulle_objc_searcharguments_make_imp_no_initialize( mulle_objc_implementation_t imp)
{
   return( (struct _mulle_objc_searcharguments)
   {
      .args =
      {
         .mode    = MULLE_OBJC_SEARCH_IMP,
      },
      .imp        = imp,
      .initialize = 0
   });
}


static inline struct _mulle_objc_searcharguments
   mulle_objc_searcharguments_make_previous( struct _mulle_objc_method *method)
{
   return( (struct _mulle_objc_searcharguments)
   {
      .args =
      {
         .mode       = MULLE_OBJC_SEARCH_PREVIOUS_METHOD,
         .methodid   = _mulle_objc_method_get_methodid( method)
      },
      .previous_method = method,
      .initialize = 1
   });
}


static inline struct _mulle_objc_searcharguments
   mulle_objc_searcharguments_make_super( mulle_objc_methodid_t methodid,
                                           mulle_objc_classid_t classid)
{
   return( (struct _mulle_objc_searcharguments)
   {
      .args       = mulle_objc_searchargumentscacheable_make_super( methodid, classid),
      .initialize = 1
   });
}


static inline struct _mulle_objc_searcharguments
   mulle_objc_searcharguments_make_overridden( mulle_objc_methodid_t methodid,
                                                mulle_objc_classid_t classid,
                                                mulle_objc_classid_t category)
{
   return( (struct _mulle_objc_searcharguments)
   {
      .args       = mulle_objc_searchargumentscacheable_make_overridden( methodid, classid, category),
      .initialize = 1
   });
}


static inline struct _mulle_objc_searcharguments
   mulle_objc_searcharguments_make_specific( mulle_objc_methodid_t methodid,
                                              mulle_objc_classid_t classid,
                                              mulle_objc_classid_t category)
{
   return( (struct _mulle_objc_searcharguments)
   {
      .args       = mulle_objc_searchargumentscacheable_make_specific( methodid, classid, category),
      .initialize = 1
   });
}


static inline void
   _mulle_objc_searcharguments_set_initialize( struct _mulle_objc_searcharguments *p,
                                               int flag)
{
   p->initialize = flag;
}



static inline void
   _mulle_objc_searcharguments_set_stop_classid( struct _mulle_objc_searcharguments *p,
                                                 mulle_objc_classid_t classid)
{
   p->stop_classid = classid;
}


static inline void
   _mulle_objc_searcharguments_set_callback( struct _mulle_objc_searcharguments *p,
                                             mulle_objc_search_callback_t f)
{
   p->callback = f;
}


static inline void
   _mulle_objc_searcharguments_set_userinfo( struct _mulle_objc_searcharguments *p,
                                             void *userinfo)
{
   p->userinfo = userinfo;
}


static inline void *
   _mulle_objc_searcharguments_get_userinfo( struct _mulle_objc_searcharguments *p)
{
   return( p->userinfo);
}



struct _mulle_objc_searchresult
{
   struct _mulle_objc_class       *class;    // where method was found
   struct _mulle_objc_methodlist  *list;     // list containing method
   struct _mulle_objc_method      *method;   // method
   int                            error;
};


static inline mulle_objc_categoryid_t
   mulle_objc_searchresult_get_categoryid( struct _mulle_objc_searchresult *p)
{
   return( p ? mulle_objc_methodlist_get_categoryid( p->list) : MULLE_OBJC_NO_CATEGORYID);
}


static inline mulle_objc_classid_t
   mulle_objc_searchresult_get_classid( struct _mulle_objc_searchresult *p)
{
   return( p ? mulle_objc_class_get_classid( p->class) : MULLE_OBJC_NO_CLASSID);
}


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


# pragma mark - failing

MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   _mulle_objc_class_fail_methodnotfound( struct _mulle_objc_class *cls,
                                           mulle_objc_methodid_t missing_method);

MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NO_RETURN void
   _mulle_objc_class_fail_forwardmethodnotfound( struct _mulle_objc_class *cls,
                                                mulle_objc_methodid_t missing_method,
                                                int error);

# pragma mark - forwarding

//
// this should be used by the debugger, but the debugger is really fixed to
// use __forward_mulle_objc_object_call
//
MULLE_OBJC_RUNTIME_GLOBAL
mulle_objc_implementation_t
   mulle_objc_class_get_forwardimplementation( struct _mulle_objc_class *cls);


//
// this caches the forward method as a side effect
// the user_method is just there to create a nicer crash log
//
struct _mulle_objc_method   *
   _mulle_objc_class_lazyget_forwardmethod( struct _mulle_objc_class *cls,
                                            int *error);


MULLE_OBJC_RUNTIME_GLOBAL
MULLE_C_NONNULL_RETURN
struct _mulle_objc_method *
   _mulle_objc_class_get_forwardmethod_lazy_nofail(
                              struct _mulle_objc_class *cls,
                              mulle_objc_methodid_t missing_method);

// error contains a errno constant, if return value is NULL
MULLE_OBJC_RUNTIME_GLOBAL
struct _mulle_objc_method  *
   mulle_objc_class_search_non_inherited_method( struct _mulle_objc_class *cls,
                                                 mulle_objc_methodid_t methodid,
                                                 int *error);


# pragma mark - regular search

MULLE_C_NONNULL_RETURN MULLE_C_CONST_RETURN
static inline struct _mulle_objc_method *
   mulle_objc_class_search_method_nofail( struct _mulle_objc_class *cls,
                                          mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method   *method;

   method = mulle_objc_class_defaultsearch_method( cls, methodid);
   assert( ! method || method->descriptor.methodid == methodid);
   if( ! method)
      method = _mulle_objc_class_get_forwardmethod_lazy_nofail( cls, methodid);
   return( method);
}


MULLE_C_NONNULL_RETURN MULLE_C_CONST_RETURN
static inline mulle_objc_implementation_t
   mulle_objc_class_search_implementation_nofail( struct _mulle_objc_class *cls,
                                                 mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_method *method;

   method = mulle_objc_class_search_method_nofail( cls, methodid);
   return( _mulle_objc_method_get_implementation( method));
}


# pragma mark - super search

struct _mulle_objc_method *
   _mulle_objc_class_supersearch_method( struct _mulle_objc_class *cls,
                                         mulle_objc_superid_t superid);


MULLE_C_NONNULL_RETURN
struct _mulle_objc_method *
   _mulle_objc_class_supersearch_method_nofail( struct _mulle_objc_class *cls,
                                                mulle_objc_superid_t superid);


MULLE_C_NONNULL_RETURN MULLE_C_CONST_RETURN
static inline mulle_objc_implementation_t
   _mulle_objc_class_supersearch_implementation_nofail( struct _mulle_objc_class *cls,
                                                        mulle_objc_methodid_t superid)
{
   struct _mulle_objc_method *method;

   method = _mulle_objc_class_supersearch_method_nofail( cls, superid);
   return( _mulle_objc_method_get_implementation( method));
}



// Conveniences for class
// cachepivot can be NULL, in which case the impcachepivot of class is
// used
MULLE_OBJC_RUNTIME_GLOBAL
void
   _mulle_objc_class_fill_impcache_method( struct _mulle_objc_class *cls,
                                           struct _mulle_objc_method *method,
                                           mulle_objc_uniqueid_t uniqueid);



struct mulle_objc_clobberchainenumerator
{
   struct _mulle_objc_class            *cls;
   struct _mulle_objc_searcharguments  args;
   struct _mulle_objc_searchresult     result;
};


static inline struct mulle_objc_clobberchainenumerator
   mulle_objc_class_clobberchain_enumerate( struct _mulle_objc_class *cls,
                                            mulle_objc_methodid_t methodid)
{
   return( (struct mulle_objc_clobberchainenumerator)
   {
      .cls  = cls,
      .args = mulle_objc_searcharguments_make_default( methodid)
   });
}


static inline int
   _mulle_objc_clobberchainenumerator_next( struct mulle_objc_clobberchainenumerator *rover,
                                            mulle_objc_implementation_t *imp)
{
   unsigned int                inheritance;
   struct _mulle_objc_method   *method;

   if( ! rover->cls)
      return( 0);

   inheritance = _mulle_objc_class_get_inheritance( rover->cls)
                 | MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS
                 | MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS
//                 | MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META
                 | MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES;

   method = mulle_objc_class_search_method( rover->cls,
                                            &rover->args,
                                            inheritance,
                                            &rover->result);
   if( ! method)
      return( 0);

   rover->args = mulle_objc_searcharguments_make_overridden( _mulle_objc_method_get_methodid( method),
                                                             mulle_objc_searchresult_get_classid( &rover->result),
                                                             mulle_objc_searchresult_get_categoryid( &rover->result));
   *imp = _mulle_objc_method_get_implementation( method);
   return( 1);
}


static inline void
   mulle_objc_clobberchainenumerator_done( struct mulle_objc_clobberchainenumerator *rover)
{
}


#define mulle_objc_class_clobberchain_for( cls, sel, item)                                    \
   assert( sizeof( item) == sizeof( mulle_objc_implementation_t));                            \
   for( struct mulle_objc_clobberchainenumerator                                              \
           rover__ ## item = mulle_objc_class_clobberchain_enumerate( (cls), (sel)),          \
           *rover__  ## item ## __i = (void *) 0;                                             \
        ! rover__  ## item ## __i;                                                            \
        rover__ ## item ## __i = (mulle_objc_clobberchainenumerator_done( &rover__ ## item),  \
                                   (void *) 1))                                               \
      while( _mulle_objc_clobberchainenumerator_next( &rover__ ## item,                       \
                                                     (mulle_objc_implementation_t *) &item))


#endif /* mulle_objc_class_search_h */
