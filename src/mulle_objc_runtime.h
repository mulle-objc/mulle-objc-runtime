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
extern struct _mulle_objc_runtime  *__get_or_create_objc_runtime( void);


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
   assert( _mulle_objc_runtime_is_initalized( runtime) && "runtime not initialized yet");

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


#pragma mark - loadbits && tagged pointer support

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
                                                          struct _mulle_objc_class *cls,
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


// use this only during __get_or_create_objc_runtime
// always returns same value (in same thread)
MULLE_C_NON_NULL_RETURN  // always returns same value (in same thread)
struct _mulle_objc_runtime  *__mulle_objc_get_runtime( void);


//
// this call creates a global runtime, if there is no thread or global runtime
// present yet. Unless we compile a standalone runtime (for testing)
// This method is used in the runtime for loading and compositing classes
// manually. Others preferably use mulle_objc_inlined_get_runtime().
//
MULLE_C_NON_NULL_RETURN  // always returns same value (in same thread)
struct _mulle_objc_runtime  *__get_or_create_objc_runtime( void);

struct mulle_concurrent_hashmap  *_mulle_objc_runtime_get_hashnames( struct _mulle_objc_runtime *runtime);
struct _mulle_concurrent_pointerarray  *_mulle_objc_runtime_get_staticstrings( struct _mulle_objc_runtime *runtime);


//
// "convenience" use this in test cases to tear down all objects
// and check for leaks
//
void  mulle_objc_release_runtime( void);


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


static inline void   *_mulle_objc_runtime_calloc( struct _mulle_objc_runtime *runtime, size_t n, size_t size)
{
   return( _mulle_allocator_calloc( _mulle_objc_runtime_get_allocator( runtime), n, size));
}


static inline void   *_mulle_objc_runtime_realloc( struct _mulle_objc_runtime *runtime, void *block, size_t size)
{
   return( _mulle_allocator_realloc( _mulle_objc_runtime_get_allocator( runtime), block, size));
}


static inline void   _mulle_objc_runtime_free( struct _mulle_objc_runtime *runtime, void *block)
{
   _mulle_allocator_free( _mulle_objc_runtime_get_allocator( runtime), block);
}


static inline void   *mulle_objc_runtime_calloc( struct _mulle_objc_runtime *runtime, size_t n, size_t size)
{
   if( ! runtime)
   {
      errno = EINVAL;
      (*runtime->memory.allocator.fail)( NULL, 0);
   }
   return( _mulle_allocator_calloc( &runtime->memory.allocator, n, size));
}


static inline void   *mulle_objc_runtime_realloc( struct _mulle_objc_runtime *runtime, void *block, size_t size)
{
   if( ! runtime)
   {
      errno = EINVAL;
      (*runtime->memory.allocator.fail)( NULL, 0);
   }
   return( _mulle_allocator_realloc( &runtime->memory.allocator, block, size));
}


static inline void   mulle_objc_runtime_free( struct _mulle_objc_runtime *runtime, void *block)
{
   if( ! block)
      return;

   if( ! runtime)
   {
      errno = EINVAL;
      (*runtime->memory.allocator.fail)( NULL, 0);
   }

   _mulle_allocator_free( &runtime->memory.allocator, block);
}


//
// conveniences that call the runtime functions with the current runtime
//
#ifndef MULLE_OBJC_NO_CONVENIENCES

MULLE_C_NON_NULL_RETURN
void   *mulle_objc_calloc( size_t n, size_t size);

MULLE_C_NON_NULL_RETURN
void   *mulle_objc_realloc( void *p, size_t size);

void   mulle_objc_free( void *p);

#endif


#pragma mark - lock support

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
                                                                  struct _mulle_objc_class *superclass);

#ifndef MULLE_OBJC_NO_CONVENIENCES

struct _mulle_objc_classpair   *mulle_objc_unfailing_new_classpair( mulle_objc_classid_t  classid,
                                                                    char *name,
                                                                    size_t instancesize,
                                                                    struct _mulle_objc_class *superclass);
#endif

// classes

int   mulle_objc_runtime_add_class( struct _mulle_objc_runtime *runtime, struct _mulle_objc_class *cls);

void  _mulle_objc_runtime_set_fastclass( struct _mulle_objc_runtime *runtime,
                                         struct _mulle_objc_class *cls,
                                         unsigned int index);


void   mulle_objc_runtime_unfailing_add_class( struct _mulle_objc_runtime *runtime,
                                               struct _mulle_objc_class *cls);


#ifndef MULLE_OBJC_NO_CONVENIENCES

void   mulle_objc_unfailing_add_class( struct _mulle_objc_class *cls);

#endif



#pragma mark - method cache

static inline unsigned int   _mulle_objc_runtime_number_of_preload_methods( struct _mulle_objc_runtime *runtime)
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


static inline struct _mulle_objc_method  *mulle_objc_runtime_alloc_array_of_methods( struct _mulle_objc_runtime *runtime, unsigned int count_methods)
{
   return( _mulle_objc_runtime_calloc( runtime, count_methods, sizeof( struct _mulle_objc_method)));
}


static inline void   mulle_objc_runtime_free_array_of_methods( struct _mulle_objc_runtime *runtime, struct _mulle_objc_method *array)
{
   _mulle_objc_runtime_free( runtime, array);
}


char   *mulle_objc_lookup_methodname( mulle_objc_methodid_t methodid);
char   *mulle_objc_search_debughashname( mulle_objc_uniqueid_t uniqueid);


#pragma mark - method lists

//
// method lists
// these methods are here, because they use the runtime allocer
//

static inline struct _mulle_objc_methodlist  *mulle_objc_runtime_alloc_methodlist( struct _mulle_objc_runtime *runtime, unsigned int count_methods)
{
   return( _mulle_objc_runtime_calloc( runtime, 1, mulle_objc_size_of_methodlist( count_methods)));
}

static inline void   mulle_objc_runtime_free_methodlist( struct _mulle_objc_runtime *runtime, struct _mulle_objc_methodlist *list)
{
   mulle_objc_runtime_free( runtime, list);
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
                                                 struct _mulle_objc_class *cls);

#pragma mark - hashnames (debug output only)

void   _mulle_objc_runtime_add_loadhashedstringlist( struct _mulle_objc_runtime *runtime,
                                                          struct _mulle_objc_loadhashedstringlist *hashnames);

char   *_mulle_objc_runtime_search_debughashname( struct _mulle_objc_runtime *runtime,
                                                  mulle_objc_uniqueid_t hash);



# pragma mark - gifts (externally allocated memory)

//
// a gift must have been allocated with the runtime->memory.allocator
//
void   _mulle_objc_runtime_unfailing_add_gift( struct _mulle_objc_runtime *runtime,
                                               void *gift);

static inline void   mulle_objc_runtime_unfailing_add_gift( struct _mulle_objc_runtime *runtime,
                                              void *gift)
{
   if( ! runtime || ! gift)
      return;
   _mulle_objc_runtime_unfailing_add_gift( runtime, gift);
}


// conveniences

#ifndef MULLE_OBJC_NO_CONVENIENCES

struct _mulle_objc_methodlist  *mulle_objc_alloc_methodlist( unsigned int count_methods);
void   mulle_objc_free_methodlist( struct _mulle_objc_methodlist *list);
#endif



/* debug support */

// rename this
enum mulle_objc_runtime_type_t
{
   mulle_objc_runtime_is_runtime    = 0,
   mulle_objc_runtime_is_class      = 1,
   mulle_objc_runtime_is_meta_class = 2,
   mulle_objc_runtime_is_category   = 3,
   mulle_objc_runtime_is_protocol   = 4,
   mulle_objc_runtime_is_method     = 5,
   mulle_objc_runtime_is_property   = 6,
   mulle_objc_runtime_is_ivar       = 7
};


typedef enum
{
   mulle_objc_runtime_walk_error        = -1,

   mulle_objc_runtime_walk_ok           = 0,
   mulle_objc_runtime_walk_dont_descend = 1,  // skip descent
   mulle_objc_runtime_walk_done         = 2,
   mulle_objc_runtime_walk_cancel       = 3
} mulle_objc_runtime_walkcommand_t;


struct _mulle_objc_runtime;

typedef mulle_objc_runtime_walkcommand_t
   (*mulle_objc_runtime_walkcallback)( struct _mulle_objc_runtime *runtime,
                                       void *p,
                                       enum mulle_objc_runtime_type_t type,
                                       char *key,
                                       void *parent,
                                       void *userinfo);

// this walks recursively through the whole runtime
mulle_objc_runtime_walkcommand_t   mulle_objc_runtime_walk( struct _mulle_objc_runtime *runtime,
                                                            mulle_objc_runtime_walkcallback   callback,
                                                            void *userinfo);

// this just walks over the classes
mulle_objc_runtime_walkcommand_t   mulle_objc_runtime_walk_classes( struct _mulle_objc_runtime  *runtime,
                                                                    mulle_objc_runtime_walkcallback callback,
                                                                    void *userinfo);

// for lldb ?
void   mulle_objc_walk_classes( mulle_objc_runtime_walkcallback callback,
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


#endif /* mulle_objc_runtime_h__*/

/* [^1] When you crash here, you have called this function before the runtime
has been setup correctly. Or maybe it is gone already ? Solution a) don't call
this function then. b) Setup the runtime before calling this */

