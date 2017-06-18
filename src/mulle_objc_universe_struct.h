//
//  mulle_objc_universe_struct.h
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
#ifndef mulle_objc_universe_struct_h__
#define mulle_objc_universe_struct_h__

#include "mulle_objc_cache.h"
#include "mulle_objc_fastclasstable.h"
#include "mulle_objc_fastmethodtable.h"
#include "mulle_objc_ivarlist.h"
#include "mulle_objc_load.h"
#include "mulle_objc_methodlist.h"
#include "mulle_objc_propertylist.h"
#include "mulle_objc_taggedpointer.h"
#include "mulle_objc_uniqueid.h"
#include "mulle_objc_uniqueidarray.h"
#include "mulle_objc_version.h"

#include <mulle_aba/mulle_aba.h>
#include <mulle_allocator/mulle_allocator.h>
#include <mulle_concurrent/mulle_concurrent.h>
#include <mulle_thread/mulle_thread.h>


struct _mulle_objc_class;
struct _mulle_objc_universe;
struct mulle_objc_loadversion;


//
// Configure the universe. Don't change after intialization
//
struct _mulle_objc_universeconfig
{
   unsigned   forget_strings           : 1;  // don't keep track of static strings
   unsigned   min_optlevel             : 3;  // min compiler optimization level: (0)
   unsigned   max_optlevel             : 3;  // max compiler optimization level: (7)
   unsigned   ignore_ivarhash_mismatch : 1;  // do not check for fragility problems
   unsigned   no_tagged_pointers       : 1;  // don't use tagged pointers
   unsigned   thread_local_rt          : 1;  // use thread local universes
   unsigned   repopulate_caches        : 1;  // useful for coverage analysis
};


// loadbits check that code we add to the universe is
// compatible with respect to tagged pointers
enum
{
   MULLE_OBJC_UNIVERSE_HAVE_NO_TPS_LOADS = 0x1,
   MULLE_OBJC_UNIVERSE_HAVE_TPS_LOADS    = 0x2,
   MULLE_OBJC_UNIVERSE_HAVE_TPS_CLASSES  = 0x4
};


//
// Debug the universe. Use environment variables to set these
// bits.
//
struct _mulle_objc_universedebug
{
   struct
   {
      unsigned   method_searches;      // keep this in an int

      unsigned   category_adds        : 1;
      unsigned   class_adds           : 1;
      unsigned   class_cache          : 1;
      unsigned   class_frees          : 1;
      unsigned   dependencies         : 1;
      unsigned   dump_universe        : 1;  // hefty, set manually
      unsigned   fastclass_adds       : 1;
      unsigned   initialize           : 1;
      unsigned   load_calls           : 1; // +initialize, +load, +categoryDependencies
      unsigned   loadinfo             : 1;
      unsigned   method_caches        : 1;
      unsigned   method_calls         : 1;
      unsigned   protocol_adds        : 1;
      unsigned   state_bits           : 1;
      unsigned   string_adds          : 1;
      unsigned   tagged_pointers      : 1;
      unsigned   universe             : 1;
   } trace;

   struct
   {
      unsigned   methodid_types          : 1;
      unsigned   protocolclass           : 1;
      unsigned   stuck_loadables         : 1;  // set by default
      unsigned   pedantic_methodid_types : 1;
   } warn;

   struct
   {
      unsigned   universe_config          : 1;
      unsigned   print_origin            : 1; // set by default
   } print;
};


//
// Objective-C exceptions and @try/@catch are vectored through here
//
struct _mulle_objc_universeexceptionvectors
{
   void   (*throw)( struct _mulle_objc_universe *universe, void *exception);
   void   (*try_enter)( struct _mulle_objc_universe *universe, void *localExceptionData);
   void   (*try_exit)( struct _mulle_objc_universe *universe, void *localExceptionData);
   void   *(*extract)( struct _mulle_objc_universe *universe, void *localExceptionData);
   int    (*match)( struct _mulle_objc_universe *universe, mulle_objc_classid_t classid, void *exception);
};


//
// Failures of the universe itself are vectored though here
//
struct _mulle_objc_universefailures
{
   void   (*uncaughtexception)( void *exception)         MULLE_C_NO_RETURN;
   // fails in unfailing method -> abort
   void   (*fail)( char *format, va_list args)           MULLE_C_NO_RETURN;
   // unexpected happening -> abort
   void   (*inconsistency)( char *format, va_list args)  MULLE_C_NO_RETURN;
   // class not found -> abort
   void   (*class_not_found)( struct _mulle_objc_universe *universe,
                              mulle_objc_methodid_t missing_method)  MULLE_C_NO_RETURN;
   // method not found -> abort
   void   (*method_not_found)( struct _mulle_objc_class *cls,
                               mulle_objc_methodid_t missing_method)  MULLE_C_NO_RETURN;
};


//
// Specify methodids (global) that get preloaded into the method caches.
// This guarantees an optimal spot and no delay during the first call.
// It makes no sense to put "fast" methods here...
//
struct _mulle_objc_preloadmethodids
{
   unsigned int            n;
   mulle_objc_methodid_t   methodids[ 32];
};


//
// Default values to be put into classes, when they are being created
//
struct _mulle_objc_classdefaults
{
   struct _mulle_objc_method   *forwardmethod;
   void                        (*class_is_missing)( struct _mulle_objc_universe *, mulle_objc_classid_t);

   unsigned short              inheritance;
};


struct _mulle_objc_loadcallbacks
{
   int   (*should_load_loadinfo)(  struct _mulle_objc_universe *, struct _mulle_objc_loadinfo *);
};

//
// Garbage collection for the various caches
struct _mulle_objc_garbagecollection
{
   struct mulle_aba   aba;
};


typedef void   mulle_objc_universefriend_destructor_t( struct _mulle_objc_universe *, void *);
typedef void   mulle_objc_universefriend_versionassert_t( struct _mulle_objc_universe *,
                                                         void *,
                                                         struct mulle_objc_loadversion *);


//
// Give friends of the universe a place to store data. The universe will run
// the destructor upon closing. With the versionassert you can ensure that
// your friend is compatible with the currently loaded "loadinfo"
//
struct _mulle_objc_universefriend
{
   void                                         *data;
   mulle_objc_universefriend_destructor_t       *destructor;
   mulle_objc_universefriend_versionassert_t    *versionassert;
};

//
// Foundation information that the universe uses. The string class will place
// itself into the universe during +load using
// `_mulle_objc_universe_add_staticstring`. The allocator should be setup during
// the universe initialization.
// The postponer is used to wait for staticstring (or something else)

typedef int   mulle_objc_waitqueues_postpone_t( struct _mulle_objc_universe *,
                                                struct _mulle_objc_loadinfo *);



struct _mulle_objc_foundation
{
   struct _mulle_objc_universefriend    universefriend;
   struct _mulle_objc_infraclass       *staticstringclass;
   struct mulle_allocator              allocator;   // allocator for objects
   mulle_objc_classid_t                rootclassid; // NSObject = e9e78cbd
};


struct _mulle_objc_memorymanagement
{
   struct mulle_allocator   allocator;
};


//
// Up to seven classes that tagged pointers are constructed for
// Classes register themselves for a certain index during load.
//
struct _mulle_objc_taggedpointers
{
   struct _mulle_objc_class    *pointerclass[ 8];         // only 1 ... are really used
};


struct _mulle_objc_waitqueues
{
   mulle_thread_mutex_t              lock;  // used for
   struct mulle_concurrent_hashmap   classestoload;
   struct mulle_concurrent_hashmap   categoriestoload;
};

#define S_MULLE_OBJC_UNIVERSE_FOUNDATION_SPACE   1024

/*
 * All (?) global variables used by the universe are in this struct.
 * in fact if you setup the universe properly with a root
 * autorelease pool, you should be able to completely remove the
 * universe AND all created instances.
 *
 * All no, unfortunately there is one static class needed for
 * static strings.
 */
struct _mulle_objc_universe
{
   //
   // A: these types are all designed to be concurrent, no locking needed
   //
   struct _mulle_objc_cachepivot            cachepivot;

   // try to keep this region stable for version checks

   mulle_atomic_pointer_t                   version;
   char                                     *path;    

   // try to keep this region stable for loadcallbacks

   struct mulle_concurrent_hashmap          classtable;  /// keep it here for debugger
   struct mulle_concurrent_hashmap          descriptortable;
   struct mulle_concurrent_hashmap          protocoltable;
   struct mulle_concurrent_hashmap          categorytable;

   struct mulle_concurrent_pointerarray     staticstrings;
   struct mulle_concurrent_pointerarray     hashnames;
   struct mulle_concurrent_pointerarray     gifts;  // external (!) allocations that we need to free

   struct _mulle_objc_loadcallbacks         loadcallbacks;

   // unstable region, edit at will

   struct _mulle_objc_waitqueues            waitqueues;

   struct _mulle_objc_fastclasstable        fastclasstable;
   struct _mulle_objc_taggedpointers        taggedpointers;

   mulle_atomic_pointer_t                   retaincount_1;
   mulle_atomic_pointer_t                   cachecount_1; // #1#
   mulle_atomic_pointer_t                   loadbits;
   mulle_thread_mutex_t                     lock;

   //
   // B: the rest is intended to be read only (setup at init time)
   //    if you think you need to change something, use the lock
   //
   char                                     compilation[ 128];   // debugging

   mulle_thread_t                           thread;  // init-done thread

   struct _mulle_objc_memorymanagement      memory;

   struct _mulle_objc_classdefaults         classdefaults;
   struct _mulle_objc_garbagecollection     garbage;
   struct _mulle_objc_preloadmethodids      methodidstopreload;

   struct _mulle_objc_universefailures       failures;
   struct _mulle_objc_universeexceptionvectors   exceptionvectors;
   struct _mulle_objc_universeconfig         config;
   struct _mulle_objc_universedebug          debug;

   // It's all zeroes, so save some space with a union.
   // it would be "nicer" to have these in a const global
   // but due to windows, it's nicer to have a few globals
   // as possible
   //
   union
   {
      struct _mulle_objc_cache           empty_cache;
      struct _mulle_objc_methodlist      empty_methodlist;
      struct _mulle_objc_ivarlist        empty_ivarlist;
      struct _mulle_objc_propertylist    empty_propertylist;
      struct _mulle_objc_uniqueidarray   empty_uniqueidarray;
   };   //
   // this allows the foundation to come up during load without having to do
   // a malloc
   //
   struct _mulle_objc_universefriend     userinfo;    // for user programs
   struct _mulle_objc_foundation         foundation;  // for foundation

   intptr_t                              foundationspace[ S_MULLE_OBJC_UNIVERSE_FOUNDATION_SPACE / sizeof( intptr_t)];
};


static inline uint32_t   _mulle_objc_universe_get_version( struct _mulle_objc_universe *universe)
{
   return( (uint32_t) (uintptr_t) _mulle_atomic_pointer_read( &universe->version));
}


static inline char   *_mulle_objc_universe_get_path( struct _mulle_objc_universe *universe)
{
   return( universe->path);
}


static inline int   _mulle_objc_universe_is_initialized( struct _mulle_objc_universe *universe)
{
   return( (int32_t) _mulle_objc_universe_get_version( universe) >= 0);
}


static inline int   _mulle_objc_universe_is_uninitialized( struct _mulle_objc_universe *universe)
{
   return( (int32_t) _mulle_objc_universe_get_version( universe) < 0);
}


static inline int   _mulle_objc_universe_is_initializing( struct _mulle_objc_universe *universe)
{
   return( (int32_t) _mulle_objc_universe_get_version( universe) == -2);
}



// #1#: whenever a caches contents change, this variable should be incremented
//      if that is adhered to, then we can checkout the value before a
//      methodlist update and afterwards, and deduce if a costly cache flush
//      is necessary.
//

#endif
