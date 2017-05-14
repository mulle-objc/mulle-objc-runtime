//
//  mulle_objc_runtime.h
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

#ifndef mulle_objc_runtime_h__
#define mulle_objc_runtime_h__

#include "mulle_objc_cache.h"
#include "mulle_objc_fastmethodtable.h"
#include "mulle_objc_fastclasstable.h"
#include "mulle_objc_ivarlist.h"
#include "mulle_objc_methodlist.h"
#include "mulle_objc_propertylist.h"
#include "mulle_objc_load.h"
#include "mulle_objc_walktypes.h"

#include <mulle_allocator/mulle_allocator.h>
#include <mulle_concurrent/mulle_concurrent.h>
#include <mulle_c11/mulle_c11.h>

#include "mulle_objc_runtime_struct.h"
#include "mulle_objc_runtime_global.h"

/*
 * forward declaration:
 * this will be declared by the Foundation (or the test program)
 *
 */
mulle_thread_tss_t   mulle_objc_unfailing_get_or_create_runtimekey( void);
void   mulle_objc_delete_runtimekey( void);


void  __mulle_objc_runtime_setup( struct _mulle_objc_runtime *runtime,
                                  struct mulle_allocator *allocator);
void  _mulle_objc_runtime_assert_version( struct _mulle_objc_runtime  *runtime,
                                          struct mulle_objc_loadversion *version);


//
// create a calloced runtime, ready to be placed into a thread. If you don't
// want to do this, just don't use this function.
//
struct _mulle_objc_runtime  *mulle_objc_alloc_runtime( void);
void  _mulle_objc_runtime_dealloc( struct _mulle_objc_runtime *runtime);


// use
static inline void  _mulle_objc_runtime_retain( struct _mulle_objc_runtime *runtime)
{
   _mulle_atomic_pointer_increment( &runtime->retaincount_1);
}


static inline void  _mulle_objc_runtime_release( struct _mulle_objc_runtime *runtime)
{
   if( _mulle_atomic_pointer_decrement( &runtime->retaincount_1) == 0)
      _mulle_objc_runtime_dealloc( runtime);
}


#pragma mark - globals / tables

// keep it to 512 bytes
#define S_MULLE_OBJC_THREADCONFIG_FOUNDATION_SPACE  (256 - sizeof( struct _mulle_objc_runtime *))
#define S_MULLE_OBJC_THREADCONFIG_USER_SPACE        (256 - sizeof( struct _mulle_objc_exceptionstackentry *))


struct _mulle_objc_exceptionstackentry;

struct _mulle_objc_threadconfig
{
   struct _mulle_objc_runtime               *runtime;
   struct _mulle_objc_exceptionstackentry   *exception_stack;

   intptr_t   foundationspace[ S_MULLE_OBJC_THREADCONFIG_FOUNDATION_SPACE / sizeof( intptr_t)];
   intptr_t   userspace[ S_MULLE_OBJC_THREADCONFIG_USER_SPACE / sizeof( intptr_t)];
};


// only __mulle_objc_get_runtime should use this
static inline struct _mulle_objc_runtime  *__mulle_objc_get_thread_runtime( void)
{
   struct _mulle_objc_runtime       *runtime;
   struct _mulle_objc_threadconfig  *config;

   config  = mulle_objc_get_threadconfig();
   if( ! config)
      return( NULL);

   runtime = config->runtime;
   return( runtime);
}


MULLE_C_CONST_NON_NULL_RETURN  // always returns same value (in same thread)
static inline struct _mulle_objc_runtime  *mulle_objc_get_thread_runtime( void)
{
   struct _mulle_objc_runtime       *runtime;
   struct _mulle_objc_threadconfig  *config;

   config  = mulle_objc_get_threadconfig();
   assert( config && "thread not configure for mulle_objc");

   runtime = config->runtime;
   assert( _mulle_objc_runtime_is_initialized( runtime) && "runtime not initialized yet");

   return( runtime);
}


void   mulle_objc_set_thread_runtime( struct _mulle_objc_runtime *runtime);

static inline void   *_mulle_objc_threadconfig_get_foundationspace( struct _mulle_objc_threadconfig  *config)
{
   return( config->foundationspace);
}


static inline void   *_mulle_objc_threadconfig_get_userspace( struct _mulle_objc_threadconfig  *config)
{
   return( config->userspace);
}


#pragma mark - loadbits and tagged pointer support

static inline uintptr_t   _mulle_objc_runtime_get_loadbits( struct _mulle_objc_runtime *runtime)
{
   return( (uintptr_t) _mulle_atomic_pointer_read( &runtime->loadbits));
}

void   _mulle_objc_runtime_set_loadbit( struct _mulle_objc_runtime *runtime,
                                        uintptr_t value);


//
// returns 1 if index is out of bounds for this cpu
//
int  _mulle_objc_runtime_set_taggedpointerclass_at_index( struct _mulle_objc_runtime  *runtime,
                                                          struct _mulle_objc_infraclass *infra,
                                                          unsigned int index);

//
// this method is used by API consumers. It is generally assumed that the
// foundation sets up each thread properly with a runtime pointer.
//
// It is not possible to have multiple runtimes in the same thread.
//
// This call assumes that a runtime has been setup, it may crash if not.
// If it returns NULL, then you didn't setup a runtime for either your
// current thread or globally. (Hint: thread pools are tough, because you can't
// initialize them easily)
//

MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_runtime  *mulle_objc_get_runtime( void);

MULLE_C_CONST_NON_NULL_RETURN  // always returns same value (in same thread)
static inline struct _mulle_objc_runtime  *mulle_objc_inlined_get_runtime( void)
{
#if __MULLE_OBJC_TRT__
   return( mulle_objc_get_thread_runtime());
#else
   return( mulle_objc_get_global_runtime());
#endif
}


// use this only during mulle_objc_get_or_create_runtime
// always returns same value (in same thread)
MULLE_C_CONST_NON_NULL_RETURN
struct _mulle_objc_runtime  *__mulle_objc_get_runtime( void);


//
// this call creates a global runtime, if there is no thread or global runtime
// present yet.
//
// This method is used in the runtime for loading and compositing classes
// manually. Others preferably use mulle_objc_inlined_get_runtime().
//
MULLE_C_CONST_NON_NULL_RETURN  // always returns same value (in same thread)
struct _mulle_objc_runtime  *mulle_objc_get_or_create_runtime( void);
//
// "convenience" use this in test cases to tear down all objects
// and check for leaks
//
void  mulle_objc_release_runtime( void);


#pragma mark - string tables

struct mulle_concurrent_hashmap  *_mulle_objc_runtime_get_hashnames( struct _mulle_objc_runtime *runtime);
struct _mulle_concurrent_pointerarray  *_mulle_objc_runtime_get_staticstrings( struct _mulle_objc_runtime *runtime);


#pragma mark - exceptions

/* the default implementation for handling exceptions */
mulle_thread_tss_t   mulle_objc_unfailing_get_or_create_exceptionstack_key( void);

MULLE_C_NO_RETURN
void   mulle_objc_allocator_fail( void *block, size_t size);

MULLE_C_NO_RETURN
void   _mulle_objc_runtime_throw( struct _mulle_objc_runtime *runtime, void *exception);
void   _mulle_objc_runtime_try_enter( struct _mulle_objc_runtime *runtime,
                                     void *data);
void   _mulle_objc_runtime_try_exit( struct _mulle_objc_runtime *runtime,
                                    void *data);
void   *_mulle_objc_runtime_extract_exception( struct _mulle_objc_runtime *runtime,
                                              void *data);
int  _mulle_objc_runtime_match_exception( struct _mulle_objc_runtime *runtime,
                                         mulle_objc_classid_t classId,
                                         void *exception);

void   _mulle_objc_runtime_raise_fail_exception( struct _mulle_objc_runtime *runtime, char *format, ...)           MULLE_C_NO_RETURN;
void   _mulle_objc_runtime_raise_fail_errno_exception( struct _mulle_objc_runtime *runtime)                        MULLE_C_NO_RETURN;
void   _mulle_objc_runtime_raise_inconsistency_exception( struct _mulle_objc_runtime *runtime, char *format, ...)  MULLE_C_NO_RETURN;
void   _mulle_objc_runtime_raise_class_not_found_exception( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)  MULLE_C_NO_RETURN;
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
static inline struct mulle_allocator   *_mulle_objc_runtime_get_allocator( struct _mulle_objc_runtime *runtime)
{
   return( &runtime->memory.allocator);
}


// memory routines ot allocate memory, that should belong to the runtime
// you never free it yourself

//
// a gift must have been allocated with the runtime->memory.allocator
//
# pragma mark - gifts (externally allocated memory)

static inline void  _mulle_objc_runtime_add_gift( struct _mulle_objc_runtime *runtime,
                                                 void *gift)
{
   _mulle_concurrent_pointerarray_add( &runtime->gifts, gift);
}


static inline void   mulle_objc_runtime_unfailing_add_gift( struct _mulle_objc_runtime *runtime,
                                                           void *gift)
{
   if( ! runtime || ! gift)
      return;

   _mulle_objc_runtime_add_gift( runtime, gift);
}


#pragma mark - memory allocation with "gifting"
void   mulle_objc_raise_fail_errno_exception( void) MULLE_C_NO_RETURN;


static inline void   *_mulle_objc_runtime_strdup( struct _mulle_objc_runtime *runtime, char  *s)
{
   char  *p;

   p = _mulle_allocator_strdup( _mulle_objc_runtime_get_allocator( runtime), s);
   if( p)
      _mulle_objc_runtime_add_gift( runtime, p);
   return( p);
}


MULLE_C_NON_NULL_RETURN
static inline void   *_mulle_objc_runtime_calloc( struct _mulle_objc_runtime *runtime, size_t n, size_t size)
{
   void  *p;

   p = _mulle_allocator_calloc( _mulle_objc_runtime_get_allocator( runtime), n, size);
   _mulle_objc_runtime_add_gift( runtime, p);
   return( p);
}


MULLE_C_NON_NULL_RETURN
static inline void   *mulle_objc_runtime_strdup( struct _mulle_objc_runtime *runtime, char *s)
{
   if( ! runtime || ! s)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }
   return( _mulle_objc_runtime_strdup( runtime, s));
}


static inline void   *mulle_objc_runtime_calloc( struct _mulle_objc_runtime *runtime, size_t n, size_t size)
{
   if( ! runtime)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }
   return( _mulle_objc_runtime_calloc( runtime, n, size));
}


#pragma mark - load lock

static inline int   _mulle_objc_runtime_waitqueues_lock( struct _mulle_objc_runtime  *runtime)
{
   return( mulle_thread_mutex_lock( &runtime->waitqueues.lock));
}


static inline int   _mulle_objc_runtime_waitqueues_trylock( struct _mulle_objc_runtime  *runtime)
{
   return( mulle_thread_mutex_trylock( &runtime->waitqueues.lock));
}


static inline int   _mulle_objc_runtime_waitqueues_unlock( struct _mulle_objc_runtime  *runtime)
{
   return( mulle_thread_mutex_unlock( &runtime->waitqueues.lock));
}


#pragma mark - user lock support

//
// If your program wants to make some other changes to the runtime,
// rather than adding classes or methods, you should lock the runtime
// to cooperate with others.
//
static inline int   _mulle_objc_runtime_lock( struct _mulle_objc_runtime  *runtime)
{
   return( mulle_thread_mutex_lock( &runtime->lock));
}


static inline int   _mulle_objc_runtime_trylock( struct _mulle_objc_runtime  *runtime)
{
   return( mulle_thread_mutex_trylock( &runtime->lock));
}


static inline int   _mulle_objc_runtime_unlock( struct _mulle_objc_runtime  *runtime)
{
   return( mulle_thread_mutex_unlock( &runtime->lock));
}


#pragma mark - memory garbage collection

// don't use this too much

struct _mulle_objc_garbagecollection  *_mulle_objc_runtime_get_garbagecollection( struct _mulle_objc_runtime *runtime);

//
// the "foundation" autoreleasepool should do the checkin
//
int    _mulle_objc_runtime_is_current_thread_registered( struct _mulle_objc_runtime *runtime);
void   _mulle_objc_runtime_checkin_current_thread( struct _mulle_objc_runtime *runtime);
void   _mulle_objc_runtime_register_current_thread( struct _mulle_objc_runtime *runtime);
void   _mulle_objc_runtime_register_current_thread_if_needed( struct _mulle_objc_runtime *runtime);
void   _mulle_objc_runtime_unregister_current_thread( struct _mulle_objc_runtime *runtime);


#pragma mark - ObjC thread support

#ifndef MULLE_OBJC_NO_CONVENIENCES

// if you are using threads with ObjC, you should create your threads with
// mulle_objc_create_thread or register them with mulle_objc_thread_register.

int   mulle_objc_create_thread( mulle_thread_rval_t (*f)(void *), void *arg, mulle_thread_t *thread);

#endif


#pragma mark - friends

static inline void  _mulle_objc_runtime_set_userinfo( struct _mulle_objc_runtime *runtime,
                                                      struct _mulle_objc_runtimefriend *pfriend)
{
   runtime->userinfo = *pfriend;
}


static inline void  _mulle_objc_runtime_set_foundation( struct _mulle_objc_runtime *runtime,
                                                        struct _mulle_objc_foundation *pfriend)
{
   runtime->foundation = *pfriend;
}


static inline void  *_mulle_objc_runtime_get_userinfodata( struct _mulle_objc_runtime *runtime)
{
   return( runtime->userinfo.data);
}


static inline struct _mulle_objc_foundation   *_mulle_objc_runtime_get_foundation( struct _mulle_objc_runtime *runtime)
{
   return( &runtime->foundation);
}


static inline void  *_mulle_objc_runtime_get_foundationdata( struct _mulle_objc_runtime *runtime)
{
   return( runtime->foundation.runtimefriend.data);
}


static inline void  _mulle_objc_runtime_get_foundationspace( struct _mulle_objc_runtime *runtime,
                                                             void **dst,
                                                             size_t *size)
{
   if( dst)
      *dst  = runtime->foundationspace;
   if( size)
      *size = sizeof( runtime->foundationspace);
}


#pragma mark - classes

struct _mulle_objc_classpair   *mulle_objc_runtime_new_classpair( struct _mulle_objc_runtime *runtime,
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

// classes

int   mulle_objc_runtime_add_infraclass( struct _mulle_objc_runtime *runtime,
                                         struct _mulle_objc_infraclass *infra);


void   mulle_objc_runtime_unfailing_add_infraclass( struct _mulle_objc_runtime *runtime,
                                                    struct _mulle_objc_infraclass *infra);


#ifndef MULLE_OBJC_NO_CONVENIENCES
void   mulle_objc_unfailing_add_infraclass( struct _mulle_objc_infraclass *pair);
#endif

void  _mulle_objc_runtime_set_fastclass( struct _mulle_objc_runtime *runtime,
                                         struct _mulle_objc_infraclass *infra,
                                         unsigned int index);

#pragma mark - method cache

static inline unsigned int   _mulle_objc_runtime_number_of_preloadmethods( struct _mulle_objc_runtime *runtime)
{
   return( runtime->methodidstopreload.n);
}


#pragma mark - methods

int    _mulle_objc_runtime_add_methoddescriptor( struct _mulle_objc_runtime *runtime, struct _mulle_objc_methoddescriptor *p);

static inline int   mulle_objc_runtime_add_methoddescriptor( struct _mulle_objc_runtime *runtime, struct _mulle_objc_methoddescriptor *p)
{
   if( ! runtime)
   {
      errno = EINVAL;
      return( -1);
   }
   return( _mulle_objc_runtime_add_methoddescriptor( runtime, p));
}


void   mulle_objc_runtime_unfailing_add_methoddescriptor( struct _mulle_objc_runtime *runtime, struct _mulle_objc_methoddescriptor *p);

// get name from methodid for example
struct _mulle_objc_methoddescriptor   *_mulle_objc_runtime_lookup_methoddescriptor( struct _mulle_objc_runtime *runtime, mulle_objc_methodid_t methodid);


// this function automatically gifts the methodlist to the
// runtime, do not free it yourself!

static inline struct _mulle_objc_method  *mulle_objc_runtime_alloc_array_of_methods( struct _mulle_objc_runtime *runtime, unsigned int count_methods)
{
   return( _mulle_objc_runtime_calloc( runtime, count_methods, sizeof( struct _mulle_objc_method)));
}


char   *mulle_objc_lookup_methodname( mulle_objc_methodid_t methodid);


#pragma mark - method lists

//
// method lists
// these methods are here, because they use the runtime allocer
//
// Do not free the returned methodlist, it has been gifted to the
// runtime already
//
static inline struct _mulle_objc_methodlist  *mulle_objc_runtime_alloc_methodlist( struct _mulle_objc_runtime *runtime, unsigned int count_methods)
{
   return( _mulle_objc_runtime_calloc( runtime, 1, mulle_objc_size_of_methodlist( count_methods)));
}

#pragma mark - static strings


static inline mulle_objc_classid_t   _mulle_objc_runtime_get_rootclassid( struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_foundation  *foundation;

   foundation = _mulle_objc_runtime_get_foundation( runtime);
   return( foundation ->rootclassid);
}

#pragma mark - static strings

struct _mulle_objc_staticstring  // really an object
{
   char            *_s;
   unsigned int    _len;
};

void   _mulle_objc_runtime_add_staticstring( struct _mulle_objc_runtime *runtime,
                                             struct _mulle_objc_object *string);

// this changes the class of ALL (!) static strings to -> foundation.staticstringclass
void   _mulle_objc_runtime_staticstringclass_did_change( struct _mulle_objc_runtime *runtime);

void  _mulle_objc_runtime_set_staticstringclass( struct _mulle_objc_runtime *runtime,
                                                 struct _mulle_objc_infraclass *infra);

#pragma mark - hashnames (debug output only)

void   _mulle_objc_runtime_add_loadhashedstringlist( struct _mulle_objc_runtime *runtime,
                                                          struct _mulle_objc_loadhashedstringlist *hashnames);

char   *_mulle_objc_runtime_search_debughashname( struct _mulle_objc_runtime *runtime,
                                                  mulle_objc_uniqueid_t hash);



# pragma mark - debug string

char   *mulle_objc_search_debughashname( mulle_objc_uniqueid_t uniqueid);

// never returns NULL
char   *mulle_objc_string_for_uniqueid( mulle_objc_uniqueid_t classid);


static inline char   *mulle_objc_string_for_classid( mulle_objc_classid_t classid)
{
   return( mulle_objc_string_for_uniqueid( classid));
}


static inline char   *mulle_objc_string_for_ivarid( mulle_objc_ivarid_t ivarid)
{
   return( mulle_objc_string_for_uniqueid( ivarid));
}


static inline char   *mulle_objc_string_for_methodid( mulle_objc_methodid_t methodid)
{
   return( mulle_objc_string_for_uniqueid( methodid));
}


static inline char   *mulle_objc_string_for_protocolid( mulle_objc_methodid_t protocolid)
{
   return( mulle_objc_string_for_uniqueid( protocolid));
}


static inline char   *mulle_objc_string_for_categoryid( mulle_objc_categoryid_t categoryid)
{
   return( mulle_objc_string_for_uniqueid( categoryid));
}


static inline char   *mulle_objc_string_for_propertyid( mulle_objc_categoryid_t propertyid)
{
   return( mulle_objc_string_for_uniqueid( propertyid));
}


# pragma mark - debug waitqueues

enum mulle_objc_runtime_status
{
   mulle_objc_runtime_is_ok            = 0,
   mulle_objc_runtime_is_missing       = -1,
   mulle_objc_runtime_is_incomplete    = -2,
   mulle_objc_runtime_is_locked        = -3,
   mulle_objc_runtime_is_wrong_version = -4,
};


enum mulle_objc_runtime_status  _mulle_objc_runtime_check_waitqueues( struct _mulle_objc_runtime *runtime);

// as above just a bit more convenient and does a version check!
enum mulle_objc_runtime_status  _mulle_objc_check_runtime( uint32_t version);

//
// the runtime should be compiled into the user program
// runtime mismatches should be caught by the shared library version in
// theory, but while developing that's often not the case though.
// this is cheap...
//
static inline enum mulle_objc_runtime_status  mulle_objc_check_runtime( void)
{
   return( _mulle_objc_check_runtime( MULLE_OBJC_RUNTIME_VERSION));
}



// conveniences

#ifndef MULLE_OBJC_NO_CONVENIENCES

//
// the resulting methodlist is already gifted to the runtime
// don't free it yourself
//
struct _mulle_objc_methodlist  *mulle_objc_alloc_methodlist( unsigned int count_methods);
void   mulle_objc_free_methodlist( struct _mulle_objc_methodlist *list);
#endif



/* debug support */



// this walks recursively through the whole runtime
mulle_objc_walkcommand_t   mulle_objc_runtime_walk( struct _mulle_objc_runtime *runtime,
                                                    mulle_objc_walkcallback_t   callback,
                                                   void *userinfo);

// this just walks over the classes
mulle_objc_walkcommand_t   mulle_objc_runtime_walk_classes( struct _mulle_objc_runtime  *runtime,
                                                            mulle_objc_walkcallback_t callback,
                                                            void *userinfo);

// for lldb ?
void   mulle_objc_walk_classes( mulle_objc_walkcallback_t callback,
                                void *userinfo);

# pragma mark - API

static inline void   mulle_objc_register_current_thread( void)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   _mulle_objc_runtime_register_current_thread( runtime);
}


static inline void   mulle_objc_unregister_current_thread( void)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   _mulle_objc_runtime_unregister_current_thread( runtime);
}


static inline void   mulle_objc_checkin_current_thread( void)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   _mulle_objc_runtime_checkin_current_thread( runtime);
}


#pragma mark - getenv 

int  mulle_objc_getenv_yes_no_default( char *name, int default_value);
int  mulle_objc_getenv_yes_no( char *name);


#endif /* mulle_objc_runtime_h__*/

/* [^1] When you crash here, you have called this function before the runtime
has been setup correctly. Or maybe it is gone already ? Solution a) don't call
this function then. b) Setup the runtime before calling this */

