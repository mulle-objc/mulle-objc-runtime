//
//  mulle_objc_universe.h
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
//

#ifndef mulle_objc_universe_h__
#define mulle_objc_universe_h__

#include "mulle_objc_cache.h"
#include "mulle_objc_fastmethodtable.h"
#include "mulle_objc_fastclasstable.h"
#include "mulle_objc_ivarlist.h"
#include "mulle_objc_methodlist.h"
#include "mulle_objc_protocollist.h"
#include "mulle_objc_propertylist.h"
#include "mulle_objc_load.h"
#include "mulle_objc_walktypes.h"

#include <mulle_allocator/mulle_allocator.h>
#include <mulle_concurrent/mulle_concurrent.h>
#include <mulle_c11/mulle_c11.h>

#include "mulle_objc_universe_struct.h"
#include "mulle_objc_universe_global.h"

/*
 * forward declaration:
 * this will be declared by the Foundation (or the test program)
 *
 */
mulle_thread_tss_t   mulle_objc_unfailing_get_or_create_threadkey( void);
void   mulle_objc_delete_threadkey( void);


#pragma mark - stuff to be used in _get_or_create


//
// most of the interesting stuff in _get_or_create
// you can pass in NULL for all the peramaters execpt universe
// the runtime will then use it's default values
//
void   _mulle_objc_universe_bang( struct _mulle_objc_universe  *universe,
                                  void (*bang)( struct _mulle_objc_universe  *universe,
                                                  void (*crunch)( void),
                                                  void *userinfo),
                                  void (*crunch)( void),
                                  void *userinfo);

void   _mulle_objc_universe_crunch( struct _mulle_objc_universe  *universe,
                                    void (*crunch)( struct _mulle_objc_universe  *universe));

void   _mulle_objc_universe_defaultbang( struct _mulle_objc_universe  *universe,
                                        void (*exitus)( void),
                                        void *userinfo);
void   _mulle_objc_universe_defaultexitus( void);

//
// if it returns 1, put a teardown function (mulle_objc_release_universe) into
// atexit (pedantic exit)
//
void   __mulle_objc_universe_setup( struct _mulle_objc_universe *universe,
                                  struct mulle_allocator *allocator);
void   _mulle_objc_universe_assert_version( struct _mulle_objc_universe  *universe,
                                          struct mulle_objc_loadversion *version);


//
// create a calloced universe, ready to be placed into a thread. If you don't
// want to do this, just don't use this function.
//
struct _mulle_objc_universe  *mulle_objc_alloc_universe( void);
void  _mulle_objc_universe_dealloc( struct _mulle_objc_universe *universe);


// use
static inline void  _mulle_objc_universe_retain( struct _mulle_objc_universe *universe)
{
   _mulle_atomic_pointer_increment( &universe->retaincount_1);
}


static inline void  _mulle_objc_universe_release( struct _mulle_objc_universe *universe)
{
   if( _mulle_atomic_pointer_decrement( &universe->retaincount_1) == 0)
      _mulle_objc_universe_crunch( universe, _mulle_objc_universe_dealloc);
}



#pragma mark - globals / tables

// keep it to 512 bytes
#define S_MULLE_OBJC_THREADCONFIG_FOUNDATION_SPACE  (256 - sizeof( struct _mulle_objc_universe *))
#define S_MULLE_OBJC_THREADCONFIG_USER_SPACE        (256 - sizeof( struct _mulle_objc_exceptionstackentry *))


struct _mulle_objc_exceptionstackentry;

struct _mulle_objc_threadconfig
{
   struct _mulle_objc_universe              *universe;
   struct _mulle_objc_exceptionstackentry   *exception_stack;

   intptr_t   foundationspace[ S_MULLE_OBJC_THREADCONFIG_FOUNDATION_SPACE / sizeof( intptr_t)];
   intptr_t   userspace[ S_MULLE_OBJC_THREADCONFIG_USER_SPACE / sizeof( intptr_t)];
};


// only __mulle_objc_get_universe should use this
static inline struct _mulle_objc_universe  *__mulle_objc_get_thread_universe( void)
{
   struct _mulle_objc_universe       *universe;
   struct _mulle_objc_threadconfig  *config;

   config  = mulle_objc_get_threadconfig();
   if( ! config)
      return( NULL);

   universe = config->universe;
   return( universe);
}


MULLE_C_CONST_NON_NULL_RETURN  // always returns same value (in same thread)
static inline struct _mulle_objc_universe  *mulle_objc_get_thread_universe( void)
{
   struct _mulle_objc_universe       *universe;
   struct _mulle_objc_threadconfig  *config;

   config  = mulle_objc_get_threadconfig();
   assert( config && "thread not configure for mulle_objc");

   universe = config->universe;
   assert( _mulle_objc_universe_is_initialized( universe) && "universe not initialized yet");

   return( universe);
}


void   mulle_objc_set_thread_universe( struct _mulle_objc_universe *universe);

static inline void   *_mulle_objc_threadconfig_get_foundationspace( struct _mulle_objc_threadconfig  *config)
{
   return( config->foundationspace);
}


static inline void   *_mulle_objc_threadconfig_get_userspace( struct _mulle_objc_threadconfig  *config)
{
   return( config->userspace);
}


#pragma mark - loadbits and tagged pointer support

static inline uintptr_t   _mulle_objc_universe_get_loadbits( struct _mulle_objc_universe *universe)
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

MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_universe  *mulle_objc_get_universe( void);

MULLE_C_CONST_NON_NULL_RETURN  // always returns same value (in same thread)
static inline struct _mulle_objc_universe  *mulle_objc_inlined_get_universe( void)
{
#if __MULLE_OBJC_TRT__
   return( mulle_objc_get_thread_universe());
#else
   return( mulle_objc_get_global_universe());
#endif
}


// use this only during mulle_objc_get_or_create_universe
// always returns same value (in same thread)
MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_universe  *__mulle_objc_get_universe( void);


//
// this call creates a global universe, if there is no thread or global universe
// present yet.
//
// This method is used in the universe for loading and compositing classes
// manually. Others preferably use mulle_objc_inlined_get_universe().
//
MULLE_C_CONST_NON_NULL_RETURN  // always returns same value (in same thread)
struct _mulle_objc_universe  *mulle_objc_get_or_create_universe( void);
//
// "convenience" use this in test cases to tear down all objects
// and check for leaks
//
void  mulle_objc_release_universe( void);


#pragma mark - string tables

struct mulle_concurrent_hashmap  *_mulle_objc_universe_get_hashnames( struct _mulle_objc_universe *universe);
struct _mulle_concurrent_pointerarray  *_mulle_objc_universe_get_staticstrings( struct _mulle_objc_universe *universe);


#pragma mark - exceptions

/* the default implementation for handling exceptions */
mulle_thread_tss_t   mulle_objc_unfailing_get_or_create_exceptionstack_key( void);

MULLE_C_NO_RETURN
void   mulle_objc_allocator_fail( void *block, size_t size);

MULLE_C_NO_RETURN
void   _mulle_objc_universe_throw( struct _mulle_objc_universe *universe, void *exception);
void   _mulle_objc_universe_try_enter( struct _mulle_objc_universe *universe,
                                     void *data);
void   _mulle_objc_universe_try_exit( struct _mulle_objc_universe *universe,
                                    void *data);
void   *_mulle_objc_universe_extract_exception( struct _mulle_objc_universe *universe,
                                              void *data);
int  _mulle_objc_universe_match_exception( struct _mulle_objc_universe *universe,
                                         mulle_objc_classid_t classId,
                                         void *exception);

void   _mulle_objc_universe_raise_fail_exception( struct _mulle_objc_universe *universe, char *format, ...)           MULLE_C_NO_RETURN;
void   _mulle_objc_universe_raise_fail_errno_exception( struct _mulle_objc_universe *universe)                        MULLE_C_NO_RETURN;
void   _mulle_objc_universe_raise_inconsistency_exception( struct _mulle_objc_universe *universe, char *format, ...)  MULLE_C_NO_RETURN;
void   _mulle_objc_universe_raise_class_not_found_exception( struct _mulle_objc_universe *universe, mulle_objc_classid_t classid)  MULLE_C_NO_RETURN;
void   _mulle_objc_class_raise_method_not_found_exception( struct _mulle_objc_class *class, mulle_objc_methodid_t methodid)  MULLE_C_NO_RETURN;

// exceptions

void   mulle_objc_raise_fail_exception( char *format, ...)           MULLE_C_NO_RETURN;
void   mulle_objc_raise_fail_errno_exception( void)                  MULLE_C_NO_RETURN;
void   mulle_objc_raise_inconsistency_exception( char *format, ...)  MULLE_C_NO_RETURN;

void   mulle_objc_raise_taggedpointer_exception( void *obj);


#pragma mark - non concurrent memory allocation

MULLE_C_NON_NULL_RETURN
static inline struct mulle_allocator   *_mulle_objc_foundation_get_allocator( struct _mulle_objc_foundation *foundation)
{
   return( &foundation->allocator);
}


MULLE_C_NON_NULL_RETURN
static inline struct mulle_allocator   *_mulle_objc_universe_get_allocator( struct _mulle_objc_universe *universe)
{
   return( &universe->memory.allocator);
}


//
// a gift must have been allocated with the universe->memory.allocator
//
# pragma mark - gifts (externally allocated memory)

static inline void  _mulle_objc_universe_add_gift( struct _mulle_objc_universe *universe,
                                                 void *gift)
{
   _mulle_concurrent_pointerarray_add( &universe->gifts, gift);
}


static inline void   mulle_objc_universe_unfailing_add_gift( struct _mulle_objc_universe *universe,
                                                           void *gift)
{
   if( ! universe || ! gift)
      return;

   _mulle_objc_universe_add_gift( universe, gift);
}


#pragma mark - memory allocation with "gifting"
void   mulle_objc_raise_fail_errno_exception( void) MULLE_C_NO_RETURN;


static inline void   *_mulle_objc_universe_strdup( struct _mulle_objc_universe *universe, char  *s)
{
   char  *p;

   p = _mulle_allocator_strdup( _mulle_objc_universe_get_allocator( universe), s);
   if( p)
      _mulle_objc_universe_add_gift( universe, p);
   return( p);
}


MULLE_C_NON_NULL_RETURN
static inline void   *_mulle_objc_universe_calloc( struct _mulle_objc_universe *universe, size_t n, size_t size)
{
   void  *p;

   p = _mulle_allocator_calloc( _mulle_objc_universe_get_allocator( universe), n, size);
   _mulle_objc_universe_add_gift( universe, p);
   return( p);
}


MULLE_C_NON_NULL_RETURN
static inline void   *mulle_objc_universe_strdup( struct _mulle_objc_universe *universe, char *s)
{
   if( ! universe || ! s)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }
   return( _mulle_objc_universe_strdup( universe, s));
}


static inline void   *mulle_objc_universe_calloc( struct _mulle_objc_universe *universe, size_t n, size_t size)
{
   if( ! universe)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }
   return( _mulle_objc_universe_calloc( universe, n, size));
}


static inline void   mulle_objc_universe_set_path( struct _mulle_objc_universe *universe, char *s)
{
   if( ! universe || ! s)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }

   // can only set once
   if( universe->path)
      _mulle_objc_universe_raise_inconsistency_exception( universe, "path already set");
   
   universe->path = mulle_objc_universe_strdup( universe, s);
}

#pragma mark - load lock

static inline int   _mulle_objc_universe_waitqueues_lock( struct _mulle_objc_universe  *universe)
{
   return( mulle_thread_mutex_lock( &universe->waitqueues.lock));
}


static inline int   _mulle_objc_universe_waitqueues_trylock( struct _mulle_objc_universe  *universe)
{
   return( mulle_thread_mutex_trylock( &universe->waitqueues.lock));
}


static inline int   _mulle_objc_universe_waitqueues_unlock( struct _mulle_objc_universe  *universe)
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

struct _mulle_objc_garbagecollection  *_mulle_objc_universe_get_garbagecollection( struct _mulle_objc_universe *universe);

//
// the "foundation" autoreleasepool should do the checkin
//
int    _mulle_objc_universe_is_current_thread_registered( struct _mulle_objc_universe *universe);
void   _mulle_objc_universe_checkin_current_thread( struct _mulle_objc_universe *universe);
void   _mulle_objc_universe_register_current_thread( struct _mulle_objc_universe *universe);
void   _mulle_objc_universe_register_current_thread_if_needed( struct _mulle_objc_universe *universe);
void   _mulle_objc_universe_unregister_current_thread( struct _mulle_objc_universe *universe);


#pragma mark - ObjC thread support

#ifndef MULLE_OBJC_NO_CONVENIENCES

// if you are using threads with ObjC, you should create your threads with
// mulle_objc_create_thread or register them with mulle_objc_thread_register.

int   mulle_objc_create_thread( mulle_thread_rval_t (*f)(void *), void *arg, mulle_thread_t *thread);

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


static inline struct _mulle_objc_foundation   *_mulle_objc_universe_get_foundation( struct _mulle_objc_universe *universe)
{
   return( &universe->foundation);
}


static inline void  *_mulle_objc_universe_get_foundationdata( struct _mulle_objc_universe *universe)
{
   return( universe->foundation.universefriend.data);
}


static inline void  _mulle_objc_universe_get_foundationspace( struct _mulle_objc_universe *universe,
                                                             void **dst,
                                                             size_t *size)
{
   if( dst)
      *dst  = universe->foundationspace;
   if( size)
      *size = sizeof( universe->foundationspace);
}


#pragma mark - classes

struct _mulle_objc_classpair   *mulle_objc_universe_new_classpair( struct _mulle_objc_universe *universe,
                                                                  mulle_objc_classid_t  classid,
                                                                  char *name,
                                                                  size_t instancesize,
                                                                  struct _mulle_objc_infraclass *superclass);

#ifndef MULLE_OBJC_NO_CONVENIENCES

struct _mulle_objc_classpair   *mulle_objc_unfailing_new_classpair( mulle_objc_classid_t  classid,
                                                                    char *name,
                                                                    size_t instancesize,
                                                                    struct _mulle_objc_infraclass *superclass);
#endif

//    -1 on error, 0 success
// errno:
//    EFAULT= superclass not present
//    EINVAL= parameter or property NULL
//    ECHILD= missing methodlist, property etc.
//
int   mulle_objc_universe_add_infraclass( struct _mulle_objc_universe *universe,
                                         struct _mulle_objc_infraclass *infra);


void   mulle_objc_universe_unfailing_add_infraclass( struct _mulle_objc_universe *universe,
                                                    struct _mulle_objc_infraclass *infra);


#ifndef MULLE_OBJC_NO_CONVENIENCES
// used in some tests
void   mulle_objc_unfailing_add_infraclass( struct _mulle_objc_infraclass *pair);
#endif

void  _mulle_objc_universe_set_fastclass( struct _mulle_objc_universe *universe,
                                         struct _mulle_objc_infraclass *infra,
                                         unsigned int index);

#pragma mark - method cache

static inline unsigned int   _mulle_objc_universe_number_of_preloadmethods( struct _mulle_objc_universe *universe)
{
   return( universe->methodidstopreload.n);
}


#pragma mark - methods

int    _mulle_objc_universe_add_methoddescriptor( struct _mulle_objc_universe *universe, struct _mulle_objc_methoddescriptor *p);

static inline int   mulle_objc_universe_add_methoddescriptor( struct _mulle_objc_universe *universe, struct _mulle_objc_methoddescriptor *p)
{
   if( ! universe)
   {
      errno = EINVAL;
      return( -1);
   }
   return( _mulle_objc_universe_add_methoddescriptor( universe, p));
}


void   mulle_objc_universe_unfailing_add_methoddescriptor( struct _mulle_objc_universe *universe, struct _mulle_objc_methoddescriptor *p);

// get name from methodid for example
struct _mulle_objc_methoddescriptor   *_mulle_objc_universe_lookup_methoddescriptor( struct _mulle_objc_universe *universe, mulle_objc_methodid_t methodid);


// this function automatically gifts the methodlist to the
// universe, do not free it yourself!

static inline struct _mulle_objc_method  *mulle_objc_universe_alloc_array_of_methods( struct _mulle_objc_universe *universe, unsigned int count_methods)
{
   return( _mulle_objc_universe_calloc( universe, count_methods, sizeof( struct _mulle_objc_method)));
}


char   *mulle_objc_lookup_methodname( mulle_objc_methodid_t methodid);


# pragma mark - protocols

struct _mulle_objc_protocol  *_mulle_objc_universe_lookup_protocol( struct _mulle_objc_universe *universe,
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


void    mulle_objc_universe_unfailing_add_protocol( struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_protocol *protocol);

struct _mulle_objc_protocol  *mulle_objc_lookup_protocol( mulle_objc_protocolid_t protocolid);


#pragma mark - protocol lists

//
// method lists
// these methods are here, because they use the universe allocer
//
// Do not free the returned methodlist, it has been gifted to the
// universe already
//
static inline struct _mulle_objc_protocollist  *mulle_objc_universe_alloc_protocollist( struct _mulle_objc_universe *universe, unsigned int n)
{
   return( _mulle_objc_universe_calloc( universe, 1, mulle_objc_sizeof_protocollist( n)));
}


# pragma mark - categories

char   *_mulle_objc_universe_lookup_category( struct _mulle_objc_universe *universe,
                                             mulle_objc_categoryid_t categoryid);
int   _mulle_objc_universe_add_category( struct _mulle_objc_universe *universe,
                                        mulle_objc_categoryid_t categoryid,
                                        char *name);
void    mulle_objc_universe_unfailing_add_category( struct _mulle_objc_universe *universe,
                                                   mulle_objc_categoryid_t categoryid,
                                                   char *name);
char   *mulle_objc_lookup_category( mulle_objc_categoryid_t categoryid);


#pragma mark - method lists

//
// method lists
// these methods are here, because they use the universe allocer
//
// Do not free the returned methodlist, it has been gifted to the
// universe already
//
static inline struct _mulle_objc_methodlist  *mulle_objc_universe_alloc_methodlist( struct _mulle_objc_universe *universe, unsigned int count_methods)
{
   return( _mulle_objc_universe_calloc( universe, 1, mulle_objc_sizeof_methodlist( count_methods)));
}


#pragma mark - static strings


static inline mulle_objc_classid_t   _mulle_objc_universe_get_rootclassid( struct _mulle_objc_universe *universe)
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
void   _mulle_objc_universe_staticstringclass_did_change( struct _mulle_objc_universe *universe);

void  _mulle_objc_universe_set_staticstringclass( struct _mulle_objc_universe *universe,
                                                 struct _mulle_objc_infraclass *infra);

#pragma mark - hashnames (debug output only)

void   _mulle_objc_universe_add_loadhashedstringlist( struct _mulle_objc_universe *universe,
                                                          struct _mulle_objc_loadhashedstringlist *hashnames);

char   *_mulle_objc_universe_search_debughashname( struct _mulle_objc_universe *universe,
                                                  mulle_objc_uniqueid_t hash);



# pragma mark - debug string

char   *mulle_objc_search_debughashname( mulle_objc_uniqueid_t uniqueid);

// never returns NULL
char   *mulle_objc_string_for_uniqueid( mulle_objc_uniqueid_t classid);

char   *mulle_objc_string_for_classid( mulle_objc_classid_t classid);
char   *mulle_objc_string_for_methodid( mulle_objc_methodid_t methodid);
char   *mulle_objc_string_for_protocolid( mulle_objc_protocolid_t protocolid);
char   *mulle_objc_string_for_categoryid( mulle_objc_categoryid_t categoryid);


# pragma mark - debug waitqueues

enum mulle_objc_universe_status
{
   mulle_objc_universe_is_ok            = 0,
   mulle_objc_universe_is_missing       = -1,
   mulle_objc_universe_is_incomplete    = -2,
   mulle_objc_universe_is_locked        = -3,
   mulle_objc_universe_is_wrong_version = -4,
};


enum mulle_objc_universe_status  _mulle_objc_universe_check_waitqueues( struct _mulle_objc_universe *universe);

// as above just a bit more convenient and does a version check!
enum mulle_objc_universe_status  _mulle_objc_check_universe( uint32_t version);

//
// the universe should be compiled into the user program
// universe mismatches should be caught by the shared library version in
// theory, but while developing that's often not the case though.
// this is cheap...
//
static inline enum mulle_objc_universe_status  mulle_objc_check_universe( void)
{
   return( _mulle_objc_check_universe( MULLE_OBJC_RUNTIME_VERSION));
}

// conveniences

#ifndef MULLE_OBJC_NO_CONVENIENCES

//
// the resulting methodlist is already gifted to the universe
// don't free it yourself
//
struct _mulle_objc_methodlist    *mulle_objc_alloc_methodlist( unsigned int n);
struct _mulle_objc_protocollist  *mulle_objc_alloc_protocollist( unsigned int n);

#endif


/* debug support */

// this walks recursively through the whole universe
mulle_objc_walkcommand_t   mulle_objc_universe_walk( struct _mulle_objc_universe *universe,
                                                    mulle_objc_walkcallback_t   callback,
                                                   void *userinfo);

// this just walks over the classes
mulle_objc_walkcommand_t   _mulle_objc_universe_walk_classes( struct _mulle_objc_universe  *universe,
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
                            void *userinfo)
{
   struct _mulle_objc_universe  *universe;
   
   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_walk_classes( universe, 1, callback, userinfo));
}



# pragma mark - API

static inline void   mulle_objc_register_current_thread( void)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_inlined_get_universe();
   _mulle_objc_universe_register_current_thread( universe);
}


static inline void   mulle_objc_unregister_current_thread( void)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_inlined_get_universe();
   _mulle_objc_universe_unregister_current_thread( universe);
}


static inline void   mulle_objc_checkin_current_thread( void)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_inlined_get_universe();
   _mulle_objc_universe_checkin_current_thread( universe);
}


#pragma mark - getenv 

int  mulle_objc_getenv_yes_no_default( char *name, int default_value);
int  mulle_objc_getenv_yes_no( char *name);


#endif /* mulle_objc_universe_h__*/

/* [^1] When you crash here, you have called this function before the universe
has been setup correctly. Or maybe it is gone already ? Solution a) don't call
this function then. b) Setup the universe before calling this */

