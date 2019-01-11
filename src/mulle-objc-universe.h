//
//  mulle_objc_universe.h
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

#ifndef mulle_objc_universe_h__
#define mulle_objc_universe_h__

#include "mulle-objc-cache.h"
#include "mulle-objc-fastmethodtable.h"
#include "mulle-objc-fastclasstable.h"
#include "mulle-objc-ivarlist.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-protocollist.h"
#include "mulle-objc-propertylist.h"
#include "mulle-objc-load.h"
#include "mulle-objc-walktypes.h"

#include "include.h"

#include "mulle-objc-universe-struct.h"
#include "mulle-objc-universe-global.h"
#include "mulle-objc-universe-fail.h"

/*
 * forward declaration:
 * this will be declared by the Foundation (or the test program)
 *
 */

// this is defined by the user
extern MULLE_C_CONST_RETURN struct _mulle_objc_universe  *
   __register_mulle_objc_universe( mulle_objc_universeid_t universe,
                                   char *universename);

#pragma mark - stuff to be used in _get_or_create

//
// most of the interesting stuff in _get_or_create
// you can pass in NULL for all the peramaters execpt universe
// the runtime will then use it's default values
//
void   _mulle_objc_universe_bang( struct _mulle_objc_universe  *universe,
                                  void (*bang)( struct _mulle_objc_universe  *universe,
                                                struct mulle_allocator *allocator,
                                                void *userinfo),
                                  struct mulle_allocator *allocator,
                                  void *userinfo);

void   _mulle_objc_universe_crunch( struct _mulle_objc_universe  *universe,
                                    void (*crunch)( struct _mulle_objc_universe  *universe));

void   _mulle_objc_universe_defaultbang( struct _mulle_objc_universe  *universe,
                                         struct mulle_allocator *allocator,
                                         void *userinfo);

//
// if it returns 1, put a teardown function (mulle_objc_global_release_defaultuniverse) into
// atexit (pedantic exit)
//
void   _mulle_objc_universe_init( struct _mulle_objc_universe *universe,
                                  struct mulle_allocator *allocator);
void   _mulle_objc_universe_assert_runtimeversion( struct _mulle_objc_universe  *universe,
                                                   struct mulle_objc_loadversion *version);


struct _mulle_objc_universe  *
   mulle_objc_alloc_universe( mulle_objc_universeid_t universeid,
                              char *universename);

void  _mulle_objc_universe_done( struct _mulle_objc_universe *universe);


// use
static inline void  _mulle_objc_universe_retain( struct _mulle_objc_universe *universe)
{
   _mulle_atomic_pointer_increment( &universe->retaincount_1);
}


static inline void  _mulle_objc_universe_release( struct _mulle_objc_universe *universe)
{
   if( _mulle_atomic_pointer_decrement( &universe->retaincount_1) == 0)
      _mulle_objc_universe_crunch( universe, _mulle_objc_universe_done);
}



#pragma mark - globals / tables

// keep it to 512 bytes
#define S_MULLE_OBJC_THREADCONFIG_FOUNDATION_SPACE  (256 - sizeof( struct _mulle_objc_universe *))
#define S_MULLE_OBJC_THREADCONFIG_USER_SPACE        (256 - sizeof( struct _mulle_objc_exceptionstackentry *))


struct _mulle_objc_exceptionstackentry;

struct _mulle_objc_threadinfo
{
   struct mulle_allocator                   *allocator;
   struct _mulle_objc_exceptionstackentry   *exception_stack;

   intptr_t   foundationspace[ S_MULLE_OBJC_THREADCONFIG_FOUNDATION_SPACE / sizeof( intptr_t)];
   intptr_t   userspace[ S_MULLE_OBJC_THREADCONFIG_USER_SPACE / sizeof( intptr_t)];
};

void   mulle_objc_thread_setup_threadinfo( struct _mulle_objc_universe *universe);
void   mulle_objc_thread_unset_threadinfo( struct _mulle_objc_universe *universe);


static inline void *
   _mulle_objc_threadinfo_get_foundationspace( struct _mulle_objc_threadinfo  *config)
{
   return( &config->foundationspace[ 0]);
}


static inline void   *
   _mulle_objc_threadinfo_get_userspace( struct _mulle_objc_threadinfo  *config)
{
   return( &config->userspace[ 0]);
}


#pragma mark - loadbits and tagged pointer support

static inline uintptr_t
   _mulle_objc_universe_get_loadbits( struct _mulle_objc_universe *universe)
{
   return( (uintptr_t) _mulle_atomic_pointer_read( &universe->loadbits));
}

void   _mulle_objc_universe_set_loadbit( struct _mulle_objc_universe *universe,
                                         uintptr_t value);


//
// returns 1 if index is out of bounds for this cpu
//
int  _mulle_objc_universe_set_taggedpointerclass_at_index( struct _mulle_objc_universe  *universe,
                                                           struct _mulle_objc_infraclass *infra,
                                                           unsigned int index);

//
// this method is used by API consumers. It is generally assumed that the
// foundation sets up each thread properly with a universe pointer.
//
// It is not possible to have multiple universes in the same thread.
//
// This call assumes that a universe has been setup, it may crash if not.
// If it returns NULL, then you didn't setup a universe for either your
// current thread or globally. (Hint: thread pools are tough, because you can't
// initialize them easily)
//

MULLE_C_CONST_NON_NULL_RETURN struct _mulle_objc_universe *
   mulle_objc_global_get_universe( mulle_objc_universeid_t universeid);

// always returns same value (in same thread)
MULLE_C_CONST_NON_NULL_RETURN  static inline struct _mulle_objc_universe *
   mulle_objc_global_inlineget_universe( mulle_objc_universeid_t universeid)
{
   if( universeid == MULLE_OBJC_DEFAULTUNIVERSEID)
      return( mulle_objc_global_get_defaultuniverse());
   return( mulle_objc_global_lookup_universe( universeid));
}


// use this only during mulle_objc_global_register_universe
// always returns same value (in same thread)
MULLE_C_CONST_NON_NULL_RETURN struct _mulle_objc_universe   *
   __mulle_objc_global_get_universe(  mulle_objc_universeid_t universeid,
                                      char *universename);

//
// This call creates a global universe, if there is no thread or global universe
// present yet.
//
// This method is used in the universe for loading and compositing classes
// manually. Others preferably use mulle_objc_global_inlineget_universe.
//
// It is not CONST, so you can reinitialize a universe with this function.
//
MULLE_C_NON_NULL_RETURN struct _mulle_objc_universe   *
   mulle_objc_global_register_universe( mulle_objc_universeid_t universeid,
                                        char *universename);

//
// "convenience" use this in test cases to tear down all objects
// and check for leaks
//
void  mulle_objc_global_release_defaultuniverse( void);


#pragma mark - string tables

struct mulle_concurrent_hashmap  *
   _mulle_objc_universe_get_hashnames( struct _mulle_objc_universe *universe);

struct _mulle_concurrent_pointerarray  *
   _mulle_objc_universe_get_staticstrings( struct _mulle_objc_universe *universe);


#pragma mark - non concurrent memory allocation

//
// used as default value for the infraclass allocator, which creates
// objects
//
MULLE_C_NON_NULL_RETURN static inline struct mulle_allocator   *
   _mulle_objc_foundation_get_allocator( struct _mulle_objc_foundation *foundation)
{
   return( &foundation->allocator);
}


MULLE_C_NON_NULL_RETURN static inline struct mulle_allocator   *
   _mulle_objc_universe_get_foundationallocator( struct _mulle_objc_universe *universe)
{
   return( _mulle_objc_foundation_get_allocator( &universe->foundation));
}



//
// a gift must have been allocated with the universe->memory.allocator
//
# pragma mark - gifts (externally allocated memory)

static inline void
  _mulle_objc_universe_add_gift( struct _mulle_objc_universe *universe,
                                 void *gift)
{
   _mulle_concurrent_pointerarray_add( &universe->gifts, gift);
}


static inline void
  mulle_objc_universe_add_gift_nofail( struct _mulle_objc_universe *universe,
                                       void *gift)
{
   if( ! universe || ! gift)
      return;

   _mulle_objc_universe_add_gift( universe, gift);
}


#pragma mark - memory allocation with "gifting"


MULLE_C_NON_NULL_RETURN
static inline void   *_mulle_objc_universe_strdup( struct _mulle_objc_universe *universe,
                                                   char  *s)
{
   struct mulle_allocator   *allocator;
   void                     *p;

   allocator = _mulle_objc_universe_get_allocator( universe);
   p         = _mulle_allocator_strdup( allocator, s);
   _mulle_objc_universe_add_gift( universe, p);
   return( p);
}


MULLE_C_NON_NULL_RETURN static inline void *
   _mulle_objc_universe_calloc( struct _mulle_objc_universe *universe,
                                size_t n,
                                size_t size)
{
   struct mulle_allocator   *allocator;
   void                     *p;

   allocator = _mulle_objc_universe_get_allocator( universe);
   p         = _mulle_allocator_calloc( allocator, n, size);
   _mulle_objc_universe_add_gift( universe, p);
   return( p);
}


MULLE_C_NON_NULL_RETURN static inline void   *
   mulle_objc_universe_strdup( struct _mulle_objc_universe *universe,
                               char *s)
{
   if( ! universe || ! s)
   {
      errno = EINVAL;
      mulle_objc_universe_fail_errno( NULL);
   }
   return( _mulle_objc_universe_strdup( universe, s));
}


static inline void   *
   mulle_objc_universe_calloc( struct _mulle_objc_universe *universe,
                               size_t n,
                               size_t size)
{
   if( ! universe)
   {
      errno = EINVAL;
      mulle_objc_universe_fail_errno( NULL);
   }
   return( _mulle_objc_universe_calloc( universe, n, size));
}


#pragma mark - debug support for fuundation (with gifting)

static inline void   mulle_objc_universe_set_path( struct _mulle_objc_universe *universe,
                                                   char *s)
{
   if( ! universe || ! s)
   {
      errno = EINVAL;
      mulle_objc_universe_fail_errno( NULL);
   }

   // can only set once
   if( universe->path)
      mulle_objc_universe_fail_inconsistency( universe, "path already set");

   universe->path = mulle_objc_universe_strdup( universe, s);
}


#pragma mark - load lock

static inline int   _mulle_objc_universe_lock_waitqueues( struct _mulle_objc_universe  *universe)
{
   return( mulle_thread_mutex_lock( &universe->waitqueues.lock));
}


static inline int   _mulle_objc_universe_trylock_waitqueues( struct _mulle_objc_universe  *universe)
{
   return( mulle_thread_mutex_trylock( &universe->waitqueues.lock));
}


static inline int   _mulle_objc_universe_unlock_waitqueues( struct _mulle_objc_universe  *universe)
{
   return( mulle_thread_mutex_unlock( &universe->waitqueues.lock));
}


#pragma mark - user lock support

//
// If your program wants to make some other changes to the universe,
// rather than adding classes or methods, you should lock the universe
// to cooperate with others.
//
static inline int   _mulle_objc_universe_lock( struct _mulle_objc_universe  *universe)
{
   return( mulle_thread_mutex_lock( &universe->lock));
}


static inline int   _mulle_objc_universe_trylock( struct _mulle_objc_universe  *universe)
{
   return( mulle_thread_mutex_trylock( &universe->lock));
}


static inline int   _mulle_objc_universe_unlock( struct _mulle_objc_universe  *universe)
{
   return( mulle_thread_mutex_unlock( &universe->lock));
}


#pragma mark - memory garbage collection

// don't use this too much

static inline struct _mulle_objc_garbagecollection  *
   _mulle_objc_universe_get_gc( struct _mulle_objc_universe *universe)
{
   assert( _mulle_aba_is_setup( &universe->garbage.aba));
   return( &universe->garbage);
}

//
// Manages the current thread with ABA gc
// the "foundation" autoreleasepool should do a periodic checkin
//
int    _mulle_objc_thread_isregistered_universe_gc( struct _mulle_objc_universe *universe);
void   _mulle_objc_thread_checkin_universe_gc( struct _mulle_objc_universe *universe);
void   _mulle_objc_thread_register_universe_gc_if_needed( struct _mulle_objc_universe *universe);
void   _mulle_objc_thread_register_universe_gc( struct _mulle_objc_universe *universe);
void   _mulle_objc_thread_remove_universe_gc( struct _mulle_objc_universe *universe);


#pragma mark - ObjC thread support

#ifndef MULLE_OBJC_NO_CONVENIENCES

// if you are using threads with ObjC, you should create your threads with
// mulle_objc_create_thread or register them with mulle_objc_thread__register.

int   mulle_objc_create_thread( mulle_thread_rval_t (*f)(void *),
                                void *arg,
                                mulle_thread_t *thread);

#endif


#pragma mark - friends

static inline void  _mulle_objc_universe_set_userinfo( struct _mulle_objc_universe *universe,
                                                       struct _mulle_objc_universefriend *pfriend)
{
   universe->userinfo = *pfriend;
}


static inline void  _mulle_objc_universe_set_foundation( struct _mulle_objc_universe *universe,
                                                         struct _mulle_objc_foundation *pfriend)
{
   universe->foundation = *pfriend;
}


static inline void  *_mulle_objc_universe_get_userinfodata( struct _mulle_objc_universe *universe)
{
   return( universe->userinfo.data);
}


static inline struct _mulle_objc_foundation *
   _mulle_objc_universe_get_foundation( struct _mulle_objc_universe *universe)
{
   return( &universe->foundation);
}


static inline void  *
   _mulle_objc_universe_get_foundationdata( struct _mulle_objc_universe *universe)
{
   return( universe->foundation.universefriend.data);
}


static inline void
   _mulle_objc_universe_get_foundationspace( struct _mulle_objc_universe *universe,
                                             void **dst,
                                             size_t *size)
{
   if( dst)
      *dst  = universe->foundationspace;
   if( size)
      *size = sizeof( universe->foundationspace);
}


#pragma mark - classes

struct _mulle_objc_classpair   *
   mulle_objc_universe_new_classpair( struct _mulle_objc_universe *universe,
                                      mulle_objc_classid_t  classid,
                                      char *name,
                                      size_t instancesize,
                                      size_t extrasize,
                                      struct _mulle_objc_infraclass *superclass);

//    -1 on error, 0 success
// errno:
//    EFAULT= superclass not present
//    EINVAL= parameter or property NULL
//    ECHILD= missing methodlist, property etc.
//
int   mulle_objc_universe_add_infraclass( struct _mulle_objc_universe *universe,
                                          struct _mulle_objc_infraclass *infra);


void   mulle_objc_universe_add_infraclass_nofail( struct _mulle_objc_universe *universe,
                                                  struct _mulle_objc_infraclass *infra);


// do not use, it's just there for compat
// doesn't clean caches
int   mulle_objc_universe_remove_infraclass( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_infraclass *infra);


void  _mulle_objc_universe_set_fastclass( struct _mulle_objc_universe *universe,
                                          struct _mulle_objc_infraclass *infra,
                                          unsigned int index);

#pragma mark - method cache

static inline unsigned int
   _mulle_objc_universe_get_numberofpreloadmethods( struct _mulle_objc_universe *universe)
{
   return( universe->methodidstopreload.n);
}


int  _mulle_objc_universe_should_grow_cache( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_cache *cache);


#pragma mark - methods

int    _mulle_objc_universe_add_descriptor( struct _mulle_objc_universe *universe,
                                            struct _mulle_objc_descriptor *p);

static inline int   mulle_objc_universe_add_descriptor( struct _mulle_objc_universe *universe,
                                                        struct _mulle_objc_descriptor *p);

struct _mulle_objc_descriptor *
    _mulle_objc_universe_register_descriptor_nofail( struct _mulle_objc_universe *universe,
                                                       struct _mulle_objc_descriptor *p);

struct _mulle_objc_descriptor *
   mulle_objc_universe_register_descriptor_nofail( struct _mulle_objc_universe *universe,
                                                     struct _mulle_objc_descriptor *p);

// get name from methodid for example
struct _mulle_objc_descriptor   *
   _mulle_objc_universe_lookup_descriptor( struct _mulle_objc_universe *universe,
                                           mulle_objc_methodid_t methodid);


// this function automatically gifts the methodlist to the
// universe, do not free it yourself!

static inline struct _mulle_objc_method  *
   mulle_objc_universe_alloc_array_of_methods( struct _mulle_objc_universe *universe,
                                               unsigned int count_methods)
{
   return( _mulle_objc_universe_calloc( universe,
                                        count_methods,
                                        sizeof( struct _mulle_objc_method)));
}


char   *mulle_objc_universe_lookup_methodname( struct _mulle_objc_universe *universe,
                                               mulle_objc_methodid_t methodid);

char   *mulle_objc_global_lookup_methodname( mulle_objc_universeid_t universeid,
                                             mulle_objc_methodid_t methodid);

# pragma mark - superstrings

# pragma mark - super map

struct _mulle_objc_super   *
   _mulle_objc_universe_lookup_super( struct _mulle_objc_universe *universe,
                                      mulle_objc_superid_t superid);

struct _mulle_objc_super   *
   _mulle_objc_universe_lookup_super_nofail( struct _mulle_objc_universe *universe,
                                               mulle_objc_superid_t superid);

int   _mulle_objc_universe_add_super( struct _mulle_objc_universe *universe,
                                      struct _mulle_objc_super *p);
void    mulle_objc_universe_add_super_nofail( struct _mulle_objc_universe *universe,
                                                struct _mulle_objc_super *p);


# pragma mark - protocols

struct _mulle_objc_protocol  *
   _mulle_objc_universe_lookup_protocol( struct _mulle_objc_universe *universe,
                                         mulle_objc_protocolid_t protocolid);

int   _mulle_objc_universe_add_protocol( struct _mulle_objc_universe *universe,
                                         struct _mulle_objc_protocol *protocol);

static inline int   mulle_objc_universe_add_protocol( struct _mulle_objc_universe *universe,
                                                      struct _mulle_objc_protocol *protocol)
{
   if( ! universe)
   {
      errno = EINVAL;
      return( -1);
   }
   return( _mulle_objc_universe_add_protocol( universe, protocol));
}


void    mulle_objc_universe_add_protocol_nofail( struct _mulle_objc_universe *universe,
                                                 struct _mulle_objc_protocol *protocol);

struct _mulle_objc_protocol  *mulle_objc_lookup_protocol( mulle_objc_protocolid_t protocolid);


typedef mulle_objc_walkcommand_t
   (*mulle_objc_walk_protocols_callback)( struct _mulle_objc_universe *,
                                          struct _mulle_objc_protocol *,
                                          void *);

mulle_objc_walkcommand_t
   _mulle_objc_universe_walk_protocols( struct _mulle_objc_universe  *universe,
                                        mulle_objc_walk_protocols_callback callback,
                                        void *userinfo);


size_t   _mulle_objc_universe_count_protocols( struct _mulle_objc_universe  *universe);

#pragma mark - protocol lists

//
// these methods are here, because they use the universe allocer
//
// Do not free the returned protocollist, it has been gifted to the
// universe already
//
static inline struct _mulle_objc_protocollist  *
   mulle_objc_universe_alloc_protocollist( struct _mulle_objc_universe *universe,
                                           unsigned int n)
{
   return( _mulle_objc_universe_calloc( universe, 1, mulle_objc_sizeof_protocollist( n)));
}


# pragma mark - categories

char   *_mulle_objc_universe_lookup_category( struct _mulle_objc_universe *universe,
                                              mulle_objc_categoryid_t categoryid);

int   _mulle_objc_universe_add_category( struct _mulle_objc_universe *universe,
                                         mulle_objc_categoryid_t categoryid,
                                         char *name);

void    mulle_objc_universe_add_category_nofail( struct _mulle_objc_universe *universe,
                                                 mulle_objc_categoryid_t categoryid,
                                                 char *name);

char   *mulle_objc_lookup_category( mulle_objc_categoryid_t categoryid);


#pragma mark - method, property, ivar lists

//
// method lists
// these methods are here, because they use the universe allocer
//
// Do not free the returned methodlist, it has been gifted to the
// universe already
//
static inline struct _mulle_objc_methodlist  *
   mulle_objc_universe_alloc_methodlist( struct _mulle_objc_universe *universe,
                                         unsigned int n)
{
   return( _mulle_objc_universe_calloc( universe, 1, mulle_objc_sizeof_methodlist( n)));
}


static inline struct _mulle_objc_propertylist  *
   mulle_objc_universe_alloc_propertylist( struct _mulle_objc_universe *universe,
                                          unsigned int n)
{
   return( _mulle_objc_universe_calloc( universe, 1, mulle_objc_sizeof_propertylist( n)));
}


static inline struct _mulle_objc_ivarlist  *
   mulle_objc_universe_alloc_ivarlist( struct _mulle_objc_universe *universe,
                                       unsigned int n)
{
   return( _mulle_objc_universe_calloc( universe, 1, mulle_objc_sizeof_ivarlist( n)));
}


#pragma mark - static strings


static inline mulle_objc_classid_t
   _mulle_objc_universe_get_rootclassid( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_foundation  *foundation;

   foundation = _mulle_objc_universe_get_foundation( universe);
   return( foundation ->rootclassid);
}

#pragma mark - static strings

struct _mulle_objc_staticstring  // really an object
{
   char            *_s;
   unsigned int    _len;
};

void   _mulle_objc_universe_add_staticstring( struct _mulle_objc_universe *universe,
                                              struct _mulle_objc_object *string);

// this changes the class of ALL (!) static strings to -> foundation.staticstringclass
void   _mulle_objc_universe_didchange_staticstringclass( struct _mulle_objc_universe *universe);

void  _mulle_objc_universe_set_staticstringclass( struct _mulle_objc_universe *universe,
                                                  struct _mulle_objc_infraclass *infra);

static inline struct _mulle_objc_infraclass  *_mulle_objc_universe_get_staticstringclass( struct _mulle_objc_universe *universe)
{
  return( universe->foundation.staticstringclass);
}


#pragma mark - hashnames (debug output only)

void   _mulle_objc_universe_add_loadhashedstringlist( struct _mulle_objc_universe *universe,
                                                      struct _mulle_objc_loadhashedstringlist *hashnames);

char   *_mulle_objc_universe_search_debughashname( struct _mulle_objc_universe *universe,
                                                   mulle_objc_uniqueid_t hash);

#pragma mark - uniqueid to string conversions

MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_describe_uniqueid( struct _mulle_objc_universe *universe,
                                                mulle_objc_uniqueid_t uniqueid);
MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_describe_classid( struct _mulle_objc_universe *universe,
                                                mulle_objc_classid_t classid);
MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_describe_methodid( struct _mulle_objc_universe *universe,
                                                mulle_objc_methodid_t methodid);
MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_describe_protocolid( struct _mulle_objc_universe *universe,
                                                  mulle_objc_protocolid_t protocolid);
MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_describe_categoryid( struct _mulle_objc_universe *universe,
                                                  mulle_objc_categoryid_t categoryid);
MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_describe_superid( struct _mulle_objc_universe *universe,
                                               mulle_objc_superid_t classid);

# pragma mark - debug waitqueues

enum mulle_objc_universe_status
{
   mulle_objc_universe_is_ok            = 0,
   mulle_objc_universe_is_missing       = -1,
   mulle_objc_universe_is_incomplete    = -2,
   mulle_objc_universe_is_locked        = -3,
   mulle_objc_universe_is_wrong_version = -4,
};


enum mulle_objc_universe_status
   _mulle_objc_universe_check_waitqueues( struct _mulle_objc_universe *universe);

// as above just a bit more convenient and does a version check!
enum mulle_objc_universe_status
   __mulle_objc_global_check_universe( char *name, uint32_t version);

enum mulle_objc_universe_status
   __mulle_objc_universe_check( struct _mulle_objc_universe *universe,
                                uint32_t version);

//
// the universe should be compiled into the user program
// universe mismatches should be caught by the shared library version in
// theory, but while developing that's often not the case though.
// this is cheap...
//
static inline enum mulle_objc_universe_status
   mulle_objc_global_check_universe( char *name)
{
   return( __mulle_objc_global_check_universe( name, MULLE_OBJC_RUNTIME_VERSION));
}


/* debug support */

void  mulle_objc_universe_trace( struct _mulle_objc_universe *universe,
                                 char *format,
                                 ...);

void  mulle_objc_universe_trace_nolf( struct _mulle_objc_universe *universe,
                                      char *format,
                                      ...);

// this walks recursively through the whole universe
mulle_objc_walkcommand_t
   mulle_objc_universe_walk( struct _mulle_objc_universe *universe,
                             mulle_objc_walkcallback_t   callback,
                             void *userinfo);

// this just walks over the classes
mulle_objc_walkcommand_t
   _mulle_objc_universe_walk_classes( struct _mulle_objc_universe  *universe,
                                      int with_meta,
                                      mulle_objc_walkcallback_t callback,
                                      void *userinfo);

// compatible
static inline mulle_objc_walkcommand_t
   mulle_objc_universe_walk_classes( struct _mulle_objc_universe  *universe,
                                    mulle_objc_walkcallback_t callback,
                                    void *userinfo)
{
   return( _mulle_objc_universe_walk_classes( universe, 1, callback, userinfo));
}


static inline mulle_objc_walkcommand_t
   mulle_objc_universe_walk_infraclasses( struct _mulle_objc_universe  *universe,
                                          mulle_objc_walkcallback_t callback,
                                          void *userinfo)
{
   return( _mulle_objc_universe_walk_classes( universe, 0, callback, userinfo));
}


static inline mulle_objc_walkcommand_t
   mulle_objc_walk_classes( mulle_objc_walkcallback_t callback,
                            void *userinfo,
                            mulle_objc_universeid_t universeid)
{
   struct _mulle_objc_universe  *universe;

   universe = mulle_objc_global_get_universe( universeid);
   return( _mulle_objc_universe_walk_classes( universe, 1, callback, userinfo));
}


# pragma mark - cache control

void  _mulle_objc_universe_invalidate_classcaches( struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_infraclass *kindofcls);


static inline void   mulle_objc_invalidate_classcaches( mulle_objc_universeid_t universeid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_get_universe( universeid);
   _mulle_objc_universe_invalidate_classcaches( universe, NULL);
}


# pragma mark - garbage collection

static inline void   mulle_objc_register_current_thread( mulle_objc_universeid_t universeid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_inlineget_universe( universeid);
   _mulle_objc_thread_register_universe_gc( universe);
}


static inline void   mulle_objc_unregister_current_thread( mulle_objc_universeid_t universeid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_inlineget_universe( universeid);
   _mulle_objc_thread_remove_universe_gc( universe);
}


static inline void   mulle_objc_checkin_current_thread( mulle_objc_universeid_t universeid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_global_inlineget_universe( universeid);
   _mulle_objc_thread_checkin_universe_gc( universe);
}


static inline struct _mulle_objc_threadinfo  *
   __mulle_objc_thread_get_threadinfo( struct _mulle_objc_universe *universe)
{
   mulle_thread_tss_t              threadkey;
   struct _mulle_objc_threadinfo   *config;

   threadkey = _mulle_objc_universe_get_threadkey( universe);
   config    = mulle_thread_tss_get( threadkey);
   return( config);
}


// always returns same value (in same thread)
MULLE_C_CONST_NON_NULL_RETURN static inline struct _mulle_objc_threadinfo *
   _mulle_objc_thread_get_threadinfo( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_threadinfo   *config;

   /* if you crash here [^1] */
   config = __mulle_objc_thread_get_threadinfo( universe);
   assert( config);

   return( config);
}


#pragma mark - getenv

int  mulle_objc_environment_get_yes_no_default( char *name, int default_value);
int  mulle_objc_environment_get_yes_no( char *name);


#endif /* mulle_objc_universe_h__*/

/* [^1] When you crash here, you have called this function before the universe
has been setup correctly. Or maybe it is gone already ? Solution a) don't call
this function then. b) Setup the universe before calling this */

