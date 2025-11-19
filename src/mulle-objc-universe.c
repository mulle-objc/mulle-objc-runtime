//
//  mulle_objc_universe.c
//  mulle-objc-runtime
//
//  Created by Nat! on 09.03.15.
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
#define _GNU_SOURCE     // for <time.h> later on
#include "mulle-objc-universe.h"

#include "mulle-objc-builtin.h"
#include "mulle-objc-universe-global.h"
#include "mulle-objc-class.h"
#include "mulle-objc-class-initialize.h"
#include "mulle-objc-class-lookup.h"
#include "mulle-objc-universe-class.h"
#include "mulle-objc-universe-exception.h"
#include "mulle-objc-universe-fail.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-object.h"
#include "mulle-objc-signature.h"
#include "mulle-objc-walktypes.h"
#include "include-private.h"
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if defined( __linux__) || defined( __APPLE__)
# define HAVE_TRACE_TIMESTAMP
# include <time.h>
#endif


# pragma mark - setup

static void   nop( struct _mulle_objc_universe  *universe,
                   mulle_objc_classid_t classid)
{
   MULLE_C_UNUSED( universe);
   MULLE_C_UNUSED( classid);
}


void   mulle_objc_hang( void)
{
   fprintf( stderr, "Hanging for debugger to attach\n");
   for(;;)
      mulle_thread_yield();  // better for windows than sleep
}


void   mulle_objc_universe_maybe_hang_or_abort( struct _mulle_objc_universe *universe)
{
   if( universe->debug.warn.crash)
      abort();
   if( universe->debug.warn.hang)
      mulle_objc_hang();
}


static void
   _mulle_objc_universeconfig_dump( struct _mulle_objc_universeconfig *config)
{
   fprintf( stderr, "%stagged pointers", config->no_tagged_pointer ? "no " : "");

   fprintf( stderr, ", %sfast calls", config->no_fast_call ? "no " : "");
   if( config->forget_strings)
      fprintf( stderr, ", forget strings");

   if( config->ignore_ivarhash_mismatch)
      fprintf( stderr, ", ignore ivarhash mismatch");
   fprintf( stderr, ", min:-O%u max:-O%u", config->min_optlevel, config->max_optlevel);
   fprintf( stderr, ", cache fillrate: %d%%", config->cache_fillrate ? config->cache_fillrate : 25);
}

# pragma mark - environment

//
// TODO: figure out how fast getenv is and possibly cache it
//       since we are using it a lot
//
static int   get_yes_no( char *s)
{
   switch( *s)
   {
   case 'f' :
   case 'F' :
   case 'n' :
   case 'N' :
   case '0' : return( 0);
   }

   return( 1);
}

int   mulle_objc_environment_get_yes_no_default( char *name, int default_value)
{
   char   *s;

   s = name ? getenv( name) : NULL;
   if( ! s)
      return( default_value);

   return( get_yes_no( s));
}


int   mulle_objc_environment_get_int( char *name, int min, int max, int default_value)
{
   char   *s;
   long   value;

   s = name ? getenv( name) : NULL;
   if( ! s)
      return( default_value);

   // also allow YES/NO (for trace.instance)
   if( (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z'))
      return( get_yes_no( s));

   value = strtol( s, NULL, 0);
   if( value >= min && value <= max)
      return( (int) value);
   return( default_value);
}


int   mulle_objc_environment_get_yes_no( char *name)
{
   return( mulle_objc_environment_get_yes_no_default( name, 0));
}


static inline int   getenv_yes_no( char *name)
{
   return( mulle_objc_environment_get_yes_no( name));
}


static inline int   getenv_yes_no_default( char *name, int default_value)
{
   return( mulle_objc_environment_get_yes_no_default( name, default_value));
}


//
// preserve errno here, as this is used during calls
//
void   mulle_objc_universe_trace_preamble( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_threadinfo   *config;
   int       preserve;

   preserve = errno;
   {
      if( universe->thread != mulle_thread_self())
      {
         config = __mulle_objc_thread_get_threadinfo( universe);
         if( config)
            fprintf( stderr, "t:#%2lu ", (unsigned long) config->nr);
         else
            fprintf( stderr, "t:%p ", (void *) mulle_thread_self());
      }

#ifdef HAVE_TRACE_TIMESTAMP
      if( universe->debug.trace.timestamp)
      {
         struct timespec   now;

         clock_gettime( CLOCK_MONOTONIC_RAW, &now);
         fprintf( stderr, "%ld.%09ld ", now.tv_sec, now.tv_nsec);
      }
#endif
   }
   errno = preserve;
}


static void   trace_preamble( struct _mulle_objc_universe *universe)
{
   size_t   n_universes;
   char     *name;
   char     *format;
   char     *sep;

   mulle_objc_universe_trace_preamble( universe);

   name="";
   sep=":";

   // if we dump a lot, indent traces and be less verbose
   if( (universe->debug.method_call & MULLE_OBJC_UNIVERSE_CALL_TRACE_BIT) ||
       universe->debug.trace.instance)
   {
      format = "    %s%s ";
   }
   else
      if( universe->debug.trace.timestamp)
         format = "%s%s ";
      else
      {
         format = "mulle_objc_universe %strace%s ";
         sep    = ":";
      }

   if( universe)
   {
      n_universes = __mulle_objc_global_get_alluniverses( NULL, 0);
      if( n_universes > 1)
      {
         name = mulle_objc_universe_get_name( universe);
         sep  = ":";
      }
   }
   else
   {
      name = "NULL";
      sep  = ":";
   }

   fprintf( stderr, format, name, sep);
}


void  mulle_objc_universe_trace_nolf( struct _mulle_objc_universe *universe,
                                      char *format,
                                      ...)
{
   va_list   args;
   int       preserve;

   preserve = errno;
   {
      va_start( args, format);

      mulle_thread_mutex_lock( &universe->debug.lock);
      {
         trace_preamble( universe);
         vfprintf( stderr, format, args);
      }
      mulle_thread_mutex_unlock( &universe->debug.lock);

      va_end( args);
   }
   errno = preserve;
}


void   mulle_objc_universe_trace( struct _mulle_objc_universe *universe,
                                  char *format,
                                  ...)
{
   va_list   args;
   int       preserve;

   preserve = errno;
   {
      va_start( args, format);

      mulle_thread_mutex_lock( &universe->debug.lock);
      {
         trace_preamble( universe);
         vfprintf( stderr, format, args);
         fprintf( stderr, "\n");
      }
      mulle_thread_mutex_unlock( &universe->debug.lock);

      va_end( args);
   }
   errno = preserve;
}


void   mulle_objc_universe_fprintf( struct _mulle_objc_universe *universe,
                                    FILE *fp,
                                    char *format,
                                    ...)
{
   va_list   args;
   int       preserve;

   preserve = errno;
   {
      va_start( args, format);

      mulle_thread_mutex_lock( &universe->debug.lock);
      {
         mulle_objc_universe_trace_preamble( universe);

         vfprintf( fp, format, args);
      }
      mulle_thread_mutex_unlock( &universe->debug.lock);
      va_end( args);
   }
   errno = preserve;
}


static void   _mulle_objc_universe_get_environment( struct _mulle_objc_universe  *universe)
{
   char   *s;

   if( getenv_yes_no( "MULLE_OBJC_WARN_ENABLED"))
   {
      universe->debug.warn.protocolclass  = 1;
      universe->debug.warn.stuck_loadable = 1;
      universe->debug.warn.method_bits    = 1;
      universe->debug.warn.method_type    = MULLE_OBJC_WARN_METHOD_TYPE_NORMAL;
   }
   else
   {
      universe->debug.warn.protocolclass  = getenv_yes_no( "MULLE_OBJC_WARN_PROTOCOLCLASS");
      universe->debug.warn.stuck_loadable = getenv_yes_no_default( "MULLE_OBJC_WARN_STUCK_LOADABLE", 1);
      universe->debug.warn.method_bits    = getenv_yes_no_default( "MULLE_OBJC_WARN_METHOD_BITS", 1);
      universe->debug.warn.method_type    = mulle_objc_environment_get_int( "MULLE_OBJC_WARN_METHOD_TYPE",
                                                                            MULLE_OBJC_WARN_METHOD_TYPE_NORMAL,
                                                                            MULLE_OBJC_WARN_METHOD_TYPE_NONE,
#if DEBUG
                                                                            MULLE_OBJC_WARN_METHOD_TYPE_LENIENT
#else
                                                                            MULLE_OBJC_WARN_METHOD_TYPE_NONE
#endif
                                                                     );
   }

   universe->debug.warn.hang             = getenv_yes_no( "MULLE_OBJC_WARN_HANG");
   universe->debug.warn.crash            = getenv_yes_no( "MULLE_OBJC_WARN_CRASH");

   universe->debug.trace.category_add    = getenv_yes_no( "MULLE_OBJC_TRACE_CATEGORY_ADD");
   universe->debug.trace.class_add       = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_ADD");
   universe->debug.trace.class_cache     = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_CACHE");
   universe->debug.trace.class_free      = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_FREE");
   universe->debug.trace.dependency      = getenv_yes_no( "MULLE_OBJC_TRACE_DEPENDENCY");
   universe->debug.trace.fastclass_add   = getenv_yes_no( "MULLE_OBJC_TRACE_FASTCLASS_ADD");
   universe->debug.trace.instance        = mulle_objc_environment_get_int( "MULLE_OBJC_TRACE_INSTANCE", 0, 2, 0);
   universe->debug.trace.initialize      = getenv_yes_no( "MULLE_OBJC_TRACE_INITIALIZE");
   universe->debug.trace.hashstrings     = getenv_yes_no( "MULLE_OBJC_TRACE_HASHSTRINGS");
   universe->debug.trace.loadinfo        = getenv_yes_no( "MULLE_OBJC_TRACE_LOADINFO");
   universe->debug.trace.method_add      = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_ADD");
   universe->debug.trace.method_cache    = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_CACHE");
   universe->debug.trace.method_search   = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_SEARCH");  // fairly excessive!
   universe->debug.trace.preload         = getenv_yes_no( "MULLE_OBJC_TRACE_PRELOAD");
   universe->debug.trace.descriptor_add  = getenv_yes_no( "MULLE_OBJC_TRACE_DESCRIPTOR_ADD");
   universe->debug.trace.propertyid_add  = getenv_yes_no( "MULLE_OBJC_TRACE_PROPERTYID_ADD");
   universe->debug.trace.protocol_add    = getenv_yes_no( "MULLE_OBJC_TRACE_PROTOCOL_ADD");
   universe->debug.trace.state_bit       = getenv_yes_no( "MULLE_OBJC_TRACE_STATE_BIT");
   universe->debug.trace.string_add      = getenv_yes_no( "MULLE_OBJC_TRACE_STRING_ADD");
   universe->debug.trace.super_add       = getenv_yes_no( "MULLE_OBJC_TRACE_SUPER_ADD");
   universe->debug.trace.tagged_pointer  = getenv_yes_no( "MULLE_OBJC_TRACE_TAGGED_POINTER");
   universe->debug.trace.timestamp       = getenv_yes_no( "MULLE_OBJC_TRACE_TIMESTAMP");
   universe->debug.trace.thread          = getenv_yes_no( "MULLE_OBJC_TRACE_THREAD");
   universe->debug.trace.tao             = getenv_yes_no( "MULLE_OBJC_TRACE_TAO");

   universe->debug.method_call           = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_CALL") ? MULLE_OBJC_UNIVERSE_CALL_TRACE_BIT : 0;  // totally excessive!

   // this environment variable is also queried during universe setup
   if( getenv_yes_no( "MULLE_OBJC_TRACE_LEAK"))
   {
      universe->debug.method_call         |= MULLE_OBJC_UNIVERSE_CALL_TRACE_BIT | MULLE_OBJC_UNIVERSE_CALL_BORING_TRACE_BIT;
      universe->debug.trace.thread         = 1;
      universe->debug.trace.instance       = 1;
   }

   if( getenv_yes_no( "MULLE_OBJC_TRACE_ZOMBIE"))
   {
      universe->debug.method_call         |= MULLE_OBJC_UNIVERSE_CALL_TRACE_BIT  | MULLE_OBJC_UNIVERSE_CALL_BORING_TRACE_BIT;
      universe->debug.trace.thread         = 1;
      universe->debug.trace.instance       = 1;
   }

   if( getenv_yes_no( "MULLE_OBJC_TRACE_CACHE"))
   {
      universe->debug.trace.method_cache   = 1;
      universe->debug.trace.class_cache    = 1;
   }

   if( getenv_yes_no( "MULLE_OBJC_TRACE_LOAD"))
   {
      universe->debug.trace.loadinfo       = 1;
      universe->debug.trace.dependency     = 1;
   }

   // don't trace method search and calls, per default... too expensive
   // don't trace caches either, usually that's too boring
   // also don't dump, per default
   if( getenv_yes_no( "MULLE_OBJC_TRACE_ENABLED"))
   {
      universe->debug.trace.category_add   = 1;
      universe->debug.trace.class_add      = 1;
      universe->debug.trace.class_free     = 1;
      universe->debug.trace.fastclass_add  = 1;
      universe->debug.trace.initialize     = 1;
      universe->debug.trace.protocol_add   = 1;
      universe->debug.trace.state_bit      = 1;
      universe->debug.trace.tagged_pointer = 1;
   }


   universe->debug.method_call          |= getenv_yes_no_default( "MULLE_OBJC_CHECK_TAO", MULLE_OBJC_TAO_OBJECT_HEADER)
                                           ? MULLE_OBJC_UNIVERSE_CALL_TAO_BIT
                                           : 0;
   universe->debug.method_call          |= getenv_yes_no( "MULLE_OBJC_TRACE_BORING_METHOD_CALL")
                                           ? MULLE_OBJC_UNIVERSE_CALL_BORING_TRACE_BIT
                                           : 0;

   universe->debug.print.print_origin    = getenv_yes_no_default( "MULLE_OBJC_PRINT_ORIGIN", 1);
   universe->debug.print.universe_config = getenv_yes_no( "MULLE_OBJC_PRINT_UNIVERSE_CONFIG");

   if( universe->debug.print.universe_config)
   {
      fprintf( stderr, "mulle-objc-universe %p: v%d.%d.%d (load-version: %d) (",
         universe,
         MULLE_OBJC_RUNTIME_VERSION_MAJOR,
         MULLE_OBJC_RUNTIME_VERSION_MINOR,
         MULLE_OBJC_RUNTIME_VERSION_PATCH,
         MULLE_OBJC_RUNTIME_LOAD_VERSION);
      _mulle_objc_universeconfig_dump( &universe->config);
      fprintf( stderr, ")\n");

      s = mulle_objc_global_preprocessor_string( &mulle_stdlib_allocator);
      fprintf( stderr, "%s\n", s);
      mulle_allocator_free( &mulle_stdlib_allocator, s);
   }
}


static MULLE_C_NO_RETURN void
   mulle_objc_fail_allocation( struct mulle_allocator *allocator,
                               void *block,
                               size_t size)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( size);

   mulle_objc_universe_fail_code( NULL, ENOMEM);
}


# pragma mark - thread info



void   mulle_objc_thread_setup_threadinfo( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_threadinfo   *config;
   struct mulle_allocator          *allocator;
   mulle_thread_tss_t              threadkey;

   if( ! universe)
      abort();

   // don't use gifting calloc, will be cleaned separately

   allocator = _mulle_objc_universe_get_allocator( universe);
   config    = _mulle_allocator_calloc( allocator, 1, sizeof( struct _mulle_objc_threadinfo));

   config->allocator = allocator;
   config->universe  = universe;

   // merely for tracing, main as its first, gets config->nr 0
   config->nr = (uintptr_t) _mulle_atomic_pointer_increment( &universe->debug.thread_counter);

   // let foundation and userinfo setup their threadinfo space
   // including possibly the destructors of the threadinfo
   if( universe->foundation.universefriend.threadinfoinitializer)
      (*universe->foundation.universefriend.threadinfoinitializer)( config);
   if( universe->userinfo.threadinfoinitializer)
      (*universe->userinfo.threadinfoinitializer)( config);

   threadkey = _mulle_objc_universe_get_threadkey( universe);
   assert( mulle_thread_tss_get( threadkey) == NULL);
   mulle_thread_tss_set( threadkey, config);
   assert( mulle_thread_tss_get( threadkey) == config);

   if( universe->debug.trace.thread)
      mulle_objc_universe_trace( universe, "setup threadinfo %p of thread %p", config, mulle_thread_self());
}


static void   mulle_objc_threadinfo_free( struct _mulle_objc_threadinfo *config)
{
   struct _mulle_objc_universe           *universe;
   struct mulle__pointerarrayenumerator  rover;
   void                                  *linkedlist;

   if( ! config)
      return;

   // get rid of reuse allocs if there are any
   // this code is harmless if there aren't any reuse allocs
   rover = _mulle__pointerarray_enumerate( &config->reuseallocsperclassindex);
   while( _mulle__pointerarrayenumerator_next( &rover, &linkedlist))
   {
      mulle_linkedlistentry_walk( linkedlist,
                                  (mulle_linkedlistentry_walk_callback_t *) mulle_allocator_free,
                                  config->allocator);
   }
   mulle__pointerarrayenumerator_done( &rover);
   _mulle__pointerarray_done( &config->reuseallocsperclassindex, config->allocator);

   universe = config->universe;
   if( config->userspace_destructor)
   {
      if( universe->debug.trace.thread)
         mulle_objc_universe_trace( universe, "call threadinfo %p of thread %p user destructor", config, mulle_thread_self());

      (*config->userspace_destructor)( config, config->userspace);
   }
   if( config->foundation_destructor)
   {
      if( universe->debug.trace.thread)
         mulle_objc_universe_trace( universe, "call threadinfo %p of thread %p foundation destructor", config, mulle_thread_self());

      (*config->foundation_destructor)( config, config->foundationspace);
   }

   if( universe->debug.trace.thread)
      mulle_objc_universe_trace( universe, "free threadinfo %p of thread %p (#%ld)", config, mulle_thread_self(), config->nr);

   _mulle_allocator_free( config->allocator, config);
}


void   mulle_objc_thread_unset_threadinfo( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_threadinfo   *config;
   mulle_thread_tss_t              threadkey;

   if( ! universe)
      return;

   threadkey = _mulle_objc_universe_get_threadkey( universe);
   config    = mulle_thread_tss_get( threadkey);
   assert( config);

   mulle_objc_threadinfo_free( config);
   mulle_thread_tss_set( threadkey, NULL);

   if( universe->debug.trace.thread)
      mulle_objc_universe_trace( universe, "unset threadinfo %p of thread %p", config, mulle_thread_self());
}



# pragma mark - initialization


static int   abafree_nofail( void  *aba,
                             void (*p_free)( void *, void *),
                             void *pointer,
                             void *owner)
{
   if( _mulle_aba_free_owned_pointer( aba, p_free, pointer, owner))
      mulle_objc_universe_fail_errno( NULL);
   return( 0);
}


//
// minimal stuff the universe needs to be setup with immediately.
// *** dont allocate anything***
//
static void   _mulle_objc_universe_set_defaults( struct _mulle_objc_universe  *universe,
                                                 struct mulle_allocator *allocator)
{
   void   mulle_objc_vprintf_abort( char *format, va_list args);
   char   *kind;
   extern struct _mulle_objc_impcache_callback   _mulle_objc_impcache_callback_initial;
   extern struct _mulle_objc_impcache_callback   _mulle_objc_impcache_callback_empty;

   assert( universe);

   // check this also, easily fixable with padding
   if( sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1))
      abort();

   kind = "custom";
   if( ! allocator)
   {
      //
      // The "universe" standard is mulle_stdlib_allocator which is not tracked
      // when running with the testallocator.
      // Reason being, that bugs are expected in the user code more often than
      // in the runtime. The lesser visible allocations mean less usually
      // superflous output.
      //
      if( mulle_objc_environment_get_yes_no( "MULLE_OBJC_UNIVERSE_DEFAULT_ALLOCATOR"))
      {
         allocator = &mulle_default_allocator;
         kind      = "default";
      }
      else
      {
         allocator = &mulle_stdlib_allocator;
         kind      = "stdlib";
      }
   }
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "use %s allocator: %p", kind, allocator);

   strncpy( universe->compilation, __DATE__ " "__TIME__ " " __FILE__, 128);
   universe->compilation[ 127] = 0;

   // TODO: gotta improve this
   universe->compilebits  = 0;
#ifdef __MULLE_OBJC_TPS__
   universe->compilebits |= 1;
#endif
#ifdef __MULLE_OBJC_NO_TPS__
   universe->compilebits |= 2;
#endif
#ifdef __MULLE_OBJC_FCS__
   universe->compilebits |= 4;
#endif
#ifdef __MULLE_OBJC_NO_FCS__
   universe->compilebits |= 8;
#endif
#ifdef __MULLE_OBJC_TAO__
   universe->compilebits |= 16;
#endif
#ifdef __MULLE_OBJC_NO_TAO__
   universe->compilebits |= 32;
#endif
#ifdef MULLE_OBJC_TAO_OBJECT_HEADER
# if MULLE_OBJC_TAO_OBJECT_HEADER
   universe->compilebits |= 64;
# else
   universe->compilebits |= 128;
# endif
#endif

   assert( allocator->calloc);
   assert( allocator->free);
   assert( allocator->realloc);

   assert( _mulle_objc_universe_is_transitioning( universe)); // != 0!
   _mulle_atomic_pointer_write_nonatomic( &universe->cachepivot.entries,
                                          universe->empty_cache.entries);

   // the initial cache is place into classes, that haven't run +initialize
   // yes, with the callbacks need to properly call
   // +initialize and other things

   _mulle_objc_impcache_callback_init( &universe->initial_impcache.callback,
                                       &_mulle_objc_impcache_callback_initial);
   _mulle_objc_impcache_callback_init( &universe->empty_impcache.callback,
                                       &_mulle_objc_impcache_callback_empty);

   universe->memory.allocator         = *allocator;
   universe->memory.allocator.aba     = &universe->garbage.aba;
   universe->memory.allocator.abafree = abafree_nofail;
   mulle_allocator_set_fail( &universe->memory.allocator, mulle_objc_fail_allocation);

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "allocator: %p", allocator);

   _mulle_objc_universe_init_fail( universe);
   _mulle_objc_universe_init_exception( universe);

   universe->classdefaults.inheritance      = MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES;
   universe->classdefaults.class_is_missing = nop;

   _mulle_concurrent_pointerarray_init( &universe->staticstrings, 0, &universe->memory.allocator);
   _mulle_concurrent_pointerarray_init( &universe->hashnames, 0, &universe->memory.allocator);
   _mulle_concurrent_pointerarray_init( &universe->gifts, 0, &universe->memory.allocator);

   universe->path                = NULL;
   universe->config.max_optlevel = 0x7;
#if 0
   universe->config.preload_all_methods = 1;
#endif
   _mulle_objc_universe_get_environment( universe);

   // for named universes this is done in alloc already
#ifdef __MULLE_OBJC_NO_TPS__
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "__MULLE_OBJC_NO_TPS__ disables tagged pointers");
   universe->config.no_tagged_pointer = 1;
#endif

#ifdef __MULLE_OBJC_NO_FCS__
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "__MULLE_OBJC_NO_FCS__ disables fast calls");
   universe->config.no_fast_call = 1;
#endif

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "defaults set done");
}


static void   _mulle_objc_universe_done_gc( struct _mulle_objc_universe *universe);
static void   _mulle_objc_universe_init_gc( struct _mulle_objc_universe *universe);

static int   return_zero( void)
{
   return( 0);
}


void   _mulle_objc_universe_init( struct _mulle_objc_universe *universe,
                                  struct mulle_allocator *allocator)
{
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "init begin");

   if( mulle_thread_tss_create( (void *) mulle_objc_threadinfo_free,
                                &universe->threadkey))
      mulle_objc_universe_fail_perror( NULL, "No more thread local storage keys available");

   _mulle_objc_universe_set_defaults( universe, allocator);

   allocator = &universe->memory.allocator;

   _mulle_objc_universe_init_gc( universe);
   _mulle_objc_thread_register_universe_gc_if_needed( universe);

   if( mulle_thread_mutex_init( &universe->debug.lock))
      abort();
   if( mulle_thread_mutex_init( &universe->waitqueues.lock))
      abort();
   if( mulle_thread_mutex_init( &universe->lock))
      abort();

   universe->debug.count_stackdepth = return_zero;

   universe->thread = mulle_thread_self();
   mulle_objc_thread_setup_threadinfo( universe);

   _mulle_concurrent_hashmap_init( &universe->categorytable, 128, allocator);
   _mulle_concurrent_hashmap_init( &universe->classtable, 128, allocator);
   _mulle_concurrent_hashmap_init( &universe->descriptortable, 2048, allocator);
   _mulle_concurrent_hashmap_init( &universe->varyingtypedescriptortable, 8, allocator);
   _mulle_concurrent_hashmap_init( &universe->protocoltable, 64, allocator);
   _mulle_concurrent_hashmap_init( &universe->supertable, 256, allocator);
   _mulle_concurrent_hashmap_init( &universe->propertyidtable, 1024, allocator);

   _mulle_concurrent_hashmap_init( &universe->waitqueues.classestoload, 64, allocator);
   _mulle_concurrent_hashmap_init( &universe->waitqueues.categoriestoload, 32, allocator);

   if( universe->debug.trace.universe)
   {
      uintptr_t   bits;

      bits  = _mulle_objc_universe_get_loadbits_inline( universe);
      mulle_objc_universe_trace( universe, "universe load bits : %lx", bits);
      mulle_objc_universe_trace( universe, "universe tps       : %lx", universe->config.no_tagged_pointer ? 0 : 1);
      mulle_objc_universe_trace( universe, "init done");
   }

   // TODO: currently I am too lazy to set this up
   // preload a list of know selectors of the universe for easier debugging
   //   _mulle_objc_universe_add_loadhashedstringlist( universe, &map);
}


struct _mulle_objc_universe  *
  mulle_objc_alloc_universe( mulle_objc_universeid_t universeid,
                             char *universename)
{
   struct _mulle_objc_universe  *universe;

   if( universeid && ! universename || ! universeid && universename)
      mulle_objc_universe_fail_perror( NULL, "universeid/universename mismatch");

   if( universename)
   {
      if( ! *universename)
         mulle_objc_universe_fail_perror( NULL, "universename is empty");

      if( mulle_objc_universeid_from_string( universename) != universeid)
         mulle_objc_universe_fail_generic( NULL, "universeid/universename hash mismatch");
   }

#if DEBUG
   if( universeid == 0x96fde243)
      mulle_objc_universe_fail_generic( NULL, "universeid is 0x96fde243 -> \"NULL\"");
#endif

   universe = calloc( 1, sizeof( struct _mulle_objc_universe));
   if( ! universe)
      mulle_objc_universe_fail_perror( NULL, "mulle_objc_create_universe");

   _mulle_atomic_pointer_write_nonatomic( &universe->version,
                                          (void *) mulle_objc_universe_is_uninitialized);

   // should be gifted (afterwards), if this isn't read only
   universe->universeid               = universeid;
   universe->universename             = universename;
   universe->config.no_tagged_pointer = 1;

   universe->debug.trace.universe = getenv_yes_no( "MULLE_OBJC_TRACE_UNIVERSE");
   if( universe->debug.trace.universe)
   {
      mulle_objc_universe_trace( universe, "allocated universe %x \"%s\"\n", universeid, universename);
      mulle_objc_universe_trace( universe, "allocated universe set to: no tagged pointers");
   }
   return( universe);
}


struct _mulle_objc_universe  *
   __mulle_objc_global_get_universe( mulle_objc_universeid_t universeid,
                                     char *universename)
{
   struct _mulle_objc_universe  *universe;
   struct _mulle_objc_universe  *actualuniverse;

   if( ! universeid)
   {
      universe = __mulle_objc_global_get_defaultuniverse();
      return( universe);
   }

   universe = mulle_objc_global_lookup_universe( universeid);
   if( ! universe)
   {
      universe       = mulle_objc_alloc_universe( universeid, universename);
      actualuniverse = __mulle_objc_global_register_universe( universeid, universe);
      if( actualuniverse)
      {
         _mulle_objc_universe_done( universe);
         universe = actualuniverse;
      }
   }
   return( universe);
}

// this should only be used in "mulle_objc_load.c"
struct _mulle_objc_universe  *
   mulle_objc_global_register_universe( mulle_objc_universeid_t universeid,
                                        char *universename)
{
   struct _mulle_objc_universe  *universe;

   universe = __register_mulle_objc_universe( universeid, universename);  // the external function
   if( ! universe)
   {
      fprintf( stderr, "__register_mulle_objc_universe returned NULL\n");
      abort();
   }
   return( universe);
}


void  _mulle_objc_universe_assert_runtimeversion( struct _mulle_objc_universe *universe,
                                                  struct mulle_objc_loadversion *version)
{
   if( (mulle_objc_version_get_major( version->runtime) !=
        mulle_objc_version_get_major( _mulle_objc_universe_get_version( universe))) ||
       //
       // during 0 versions, any minor jump is incompatible
       //
       ( ! mulle_objc_version_get_major( version->runtime) &&
         (mulle_objc_version_get_minor( version->runtime) !=
          mulle_objc_version_get_minor( _mulle_objc_universe_get_version( universe)))))

   {
      mulle_objc_universe_fail_generic( universe,
         "mulle_objc_universe %p fatal: can't load code with incompatible "
         "version %u.%u.%u. This mulle-objc universe is version %u.%u.%u (%s)\n",
            universe,
            mulle_objc_version_get_major( version->runtime),
            mulle_objc_version_get_minor( version->runtime),
            mulle_objc_version_get_patch( version->runtime),
            mulle_objc_version_get_major( _mulle_objc_universe_get_version( universe)),
            mulle_objc_version_get_minor( _mulle_objc_universe_get_version( universe)),
            mulle_objc_version_get_patch( _mulle_objc_universe_get_version( universe)),
            _mulle_objc_universe_get_path( universe) ? _mulle_objc_universe_get_path( universe) : "???");
   }
}


static void
   _mulle_objc_universe_willfinalize( struct _mulle_objc_universe *universe);

//
// this is done for "global" universe configurations
// where threads possibly try to create and release
// universes willy, nilly (usualy tests)
//
// Usually the "crunch" function will be `_mulle_objc_universe_done`
//
void   _mulle_objc_universe_crunch( struct _mulle_objc_universe  *universe,
                                    void (*crunch)( struct _mulle_objc_universe  *universe))
{
   void  *actual;
   int   trace;
   void  (*callback)( struct _mulle_objc_universe  *universe);

   trace = universe->debug.trace.universe;
   if( trace)
      mulle_objc_universe_trace( universe,
                                 "[%p] try to lock the universe for crunch",
                                 (void *) mulle_thread_self());

   // ensure only one thread is going at it
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version,
                                           (void *) (MULLE_OBJC_RUNTIME_VERSION + 1),
                                           (void *) MULLE_OBJC_RUNTIME_VERSION);
      if( actual == (void *) mulle_objc_universe_is_uninitialized)
      {
         if( universe->debug.trace.universe)
            mulle_objc_universe_trace( universe,
                                      "[%p] someone else crunched the "
                                      "universe already",
                                      (void *) mulle_thread_self());
         return;  // someone else did it
      }
      if( actual == (void *) (MULLE_OBJC_RUNTIME_VERSION + 1))
         break;
      mulle_thread_yield();
   }

   //
   // there should be no other thread attempting this now
   // the universe is still in a "happy" state and messaging is going on
   // uninhibited, but we will notify interested classes and parties that
   // the universe will go down (e.g. +willFinalize)
   //
   _mulle_objc_universe_willfinalize( universe);

   // change to deinitializing...
   // __mulle_atomic_pointer_cas is weak and allowed to fail
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version,
                                           (void *) mulle_objc_universe_is_deinitializing,
                                           (void *) (MULLE_OBJC_RUNTIME_VERSION + 1));
      if( actual == (void *) mulle_objc_universe_is_deinitializing)
         break;
   }

   // START OF LOCKED

   if( trace)
      mulle_objc_universe_trace( universe,
                                 "[%p] crunch of the universe is in progress",
                                 (void *) mulle_thread_self());

   callback = universe->callbacks.will_crunch;
   if( callback)
      (*callback)( universe);

   callback = universe->callbacks.did_crunch;
   (*crunch)( universe);

   if( trace)
      mulle_objc_universe_trace( universe,
                                 "[%p] crunch of the universe is done",
                                 (void *) mulle_thread_self());

   mulle_atomic_memory_barrier(); // shared/global memory

   // END OF LOCKED

   // now set to unintialized
   // __mulle_atomic_pointer_cas is weak and allowed to fail
   // don't want to bloat the CAS api though
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version,
                                           (void *) mulle_objc_universe_is_uninitialized,
                                           (void *) mulle_objc_universe_is_deallocating);

      if( actual == (void *) mulle_objc_universe_is_deallocating)
      {
         if( trace)
            mulle_objc_universe_trace( universe,
                                       "[%p] unlocked the universe",
                                       (void *) mulle_thread_self());
         if( callback)
            (*callback)( universe);

         if( ! _mulle_objc_universe_is_default( universe))
         {
            __mulle_objc_global_unregister_universe( universe->universeid, universe);

            // if is superflous, but analyzer can't crack it
            if( universe != __mulle_objc_global_get_defaultuniverse())
               free( universe);
         }
         return;
      }

      if( trace)
         mulle_objc_universe_trace( universe,
                                    "[%p] retrying to unlock the universe",
                                    (void *) mulle_thread_self());
   }
}


static void   mulle_objc_global_atexit( void)
{
   struct _mulle_objc_universe  *universe;

   universe = __mulle_objc_global_get_defaultuniverse();
   if( ! universe)
   {
      fprintf( stderr, "atexit does nothing as there is no default universe\n");
      return;
   }

   if( ! _mulle_objc_universe_is_initialized( universe))
   {
      //if( universe->debug.trace.universe)
         mulle_objc_universe_trace( universe,
                                    "atexit skips release of uninitialized universe");
      return;
   }

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "atexit releases the universe (atexit: %p)",
                                    (void *) mulle_objc_global_atexit);
   _mulle_objc_universe_release( universe);
}


void
   __mulle_objc_universe_atexit_ifneeded( struct _mulle_objc_universe *universe)
{
   static int   did_it;

   if( ! universe->config.wait_threads_on_exit)
   {
      // might have been set earlier by coverage
      universe->config.pedantic_exit |= mulle_objc_environment_get_yes_no( "MULLE_OBJC_PEDANTIC_EXIT")
                                        | mulle_objc_environment_get_yes_no( "MULLE_OBJC_TRACE_LEAK")
                                        | mulle_objc_environment_get_yes_no( "MULLE_OBJC_TRACE_ZOMBIE");
      if( ! universe->config.pedantic_exit)
         return;
   }

   if( ! _mulle_objc_universe_is_default( universe))
   {
      if( universe->debug.trace.universe)
         mulle_objc_universe_trace( universe,
                                    "atexit is not installed as the universe is \
not the default universe. Use mulle_objc_global_main_finish.");
      return;
   }

   // these atexit functions could pile up though, so inhibit it
   if( ! did_it)
   {
      did_it = 1;

      if( mulle_atexit( mulle_objc_global_atexit))
         mulle_objc_universe_fail_perror( universe, "atexit:");
      if( universe->debug.trace.universe)
         mulle_objc_universe_trace( universe,
                                       "atexit \"mulle_objc_global_atexit\" (%p) installed",
                                       (void *) mulle_objc_global_atexit);
   }
}

void   _mulle_objc_universe_defaultbang( struct _mulle_objc_universe  *universe,
                                         struct mulle_allocator *allocator,
                                         void *userinfo)
{
   MULLE_C_UNUSED( userinfo);

   _mulle_objc_universe_init( universe, allocator);
   __mulle_objc_universe_atexit_ifneeded( universe);
}


static void   universe_trace( struct _mulle_objc_universe *universe, char *s)
{
   fprintf( stderr, "mulle_objc_universe %p \"%s\" %x, trace: [%p] %s\n",
           				universe,
                  	mulle_objc_universe_get_name( universe),
                  	_mulle_objc_universe_get_universeid( universe),
                  	(void *) mulle_thread_self(),
   						s);
}

//
// this is done for "global" universe configurations
// where threads possibly try to create and release
// universes willy, nilly (usually tests)
//
static void
   __mulle_objc_universe_bang( struct _mulle_objc_universe  *universe,
                               void (*setup)( struct _mulle_objc_universe *universe,
                                              struct mulle_allocator *allocator,
                                              void *userinfo),
                               struct mulle_allocator *allocator,
                               void *userinfo)
{
   void   *actual;
   int    trace;

   trace = universe->debug.trace.universe;
   if( trace)
   	universe_trace( universe, "trying to lock the universe down for bang");

   // ensure only one thread is going at it
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version,
                                           (void *)mulle_objc_universe_is_initializing,
                                           (void *) mulle_objc_universe_is_uninitialized);
      if( actual == (void *) MULLE_OBJC_RUNTIME_VERSION)
      {
         if( trace)
            universe_trace( universe,
            					 "someone else did the universe bang already");
         return;  // someone else did it
      }
      if( actual == (void *) mulle_objc_universe_is_uninitialized)
         break;
      mulle_thread_yield();
   }

   // BEGIN OF LOCKED
   if( trace)
      universe_trace( universe, "bang of the universe in progress");

   (*setup)( universe, allocator, userinfo);

   if( trace)
      universe_trace( universe, "bang of the universe done");

   mulle_atomic_memory_barrier();  // shared/global memory
   // END OF LOCKED

   // __mulle_atomic_pointer_cas is weak and allowed to fail
   // don't want to bloat the CAS api though
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version,
                                           (void *) MULLE_OBJC_RUNTIME_VERSION,
                                           (void *) mulle_objc_universe_is_initializing);
      if( actual == (void *) mulle_objc_universe_is_initializing)
      {
         if( trace)
            universe_trace( universe, "unlocked the universe");
         return;
      }

      if( trace)
            universe_trace( universe, "retrying to the universe");
   }
}


void   _mulle_objc_universe_bang( struct _mulle_objc_universe  *universe,
                                  void (*bang)( struct _mulle_objc_universe *universe,
                                                struct mulle_allocator *allocator,
                                                void *userinfo),
                                  struct mulle_allocator *allocator,
                                  void *userinfo)
{
   universe->debug.trace.universe = getenv_yes_no( "MULLE_OBJC_TRACE_UNIVERSE");

   if( ! bang)
      bang = _mulle_objc_universe_defaultbang;

   __mulle_objc_universe_bang( universe, bang, allocator, userinfo);
}


# pragma mark - TPS and Loadbits

uintptr_t
   _mulle_objc_universe_get_loadbits( struct _mulle_objc_universe *universe)
{
   return( _mulle_objc_universe_get_loadbits_inline( universe));
}


void  _mulle_objc_universe_set_loadbit( struct _mulle_objc_universe *universe,
                                        uintptr_t bit)
{
   uintptr_t   oldbits;
   uintptr_t   newbits;

   do
   {
      oldbits = _mulle_objc_universe_get_loadbits_inline( universe);
      newbits = oldbits | bit;
      if( oldbits == newbits)
         return;
   }
   while( ! _mulle_atomic_pointer_cas_weak( &universe->loadbits, (void *) newbits, (void *) oldbits));
}


//
// it's assumed that there are no competing classes for the spot
//
int  _mulle_objc_universe_set_taggedpointerclass_at_index( struct _mulle_objc_universe  *universe,
                                                           struct _mulle_objc_infraclass *infra,
                                                           unsigned int index)
{
   assert( index <= 0x7);  // anything over 0x7 is a bug though

   if( ! index || index > mulle_objc_get_taggedpointer_mask())
   {
      errno = EACCES;
      return( -1);
   }

   if( universe->taggedpointers.pointerclass[ index])
      mulle_objc_universe_fail_inconsistency( universe,
                                              "Another class is hogging the "
                                              "tagged pointer index %u already "
                                              "until the end of the universe",
                                              index);
   if( universe->debug.trace.tagged_pointer)
      mulle_objc_universe_trace( universe,
                                 "set tagged pointers with "
                                 "index %u to isa %p (class %08x \"%s\" )",
                                  index,
                                  infra,
                                 _mulle_objc_infraclass_get_classid( infra),
                                 _mulle_objc_infraclass_get_name( infra));

   universe->taggedpointers.pointerclass[ index] = infra;

   _mulle_objc_universe_set_loadbit( universe, MULLE_OBJC_UNIVERSE_HAVE_TPS_CLASSES);

   return( 0);
}


# pragma mark - dealloc


static inline void
   _mulle_objc_universe_willfinalize_friend( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_universefriend *pfriend)
{
   /* we don't mind if pfriend->friend is NULL */
   if( ! pfriend->finalizer)
      return;

   (*pfriend->finalizer)( universe, pfriend->data, mulle_objc_will_finalize);
}


static inline void
   _mulle_objc_universe_finalize_friend( struct _mulle_objc_universe *universe,
                                         struct _mulle_objc_universefriend *pfriend)
{
   /* we don't mind if pfriend->friend is NULL */
   if( ! pfriend->finalizer)
      return;

   (*pfriend->finalizer)( universe, pfriend->data, mulle_objc_finalize);
}



static inline void
   _mulle_objc_universe_free_friend( struct _mulle_objc_universe *universe,
                                     struct _mulle_objc_universefriend *pfriend)
{
   /* we don't mind if pfriend->friend is NULL */
   if( ! pfriend->destructor)
      return;

   (*pfriend->destructor)( universe, pfriend->data);
}


static void   pointerarray_in_hashmap_map( struct _mulle_objc_universe *universe,
                                           struct mulle_concurrent_hashmap *map,
                                           void (*f)( void *, void *))
{
   struct mulle_concurrent_pointerarray   *list;
   intptr_t                               key;

   mulle_concurrent_hashmap_for( map, key, list)
   {
      mulle_concurrent_pointerarray_map( list, f, universe);
   }
}


static int  reverse_compare_classindex( struct _mulle_objc_infraclass **p_a,
                                        struct _mulle_objc_infraclass **p_b)
{
   struct _mulle_objc_classpair   *a;
   struct _mulle_objc_classpair   *b;
   int                            classindex_a;
   int                            classindex_b;

   a = _mulle_objc_infraclass_get_classpair( *p_a);
   b = _mulle_objc_infraclass_get_classpair( *p_b);

   classindex_a = (int) _mulle_objc_classpair_get_classindex( a);
   classindex_b = (int) _mulle_objc_classpair_get_classindex( b);

   return( classindex_b - classindex_a);
}


static void
   _mulle_objc_universe_reversesort_infraclasses_by_classindex( struct _mulle_objc_infraclass **array,
                                                                unsigned int n_classes)
{
   MULLE_C_ASSERT( sizeof( struct _mulle_objc_infraclass **) == sizeof( void *));

   mulle_qsort( array,
                n_classes,
                sizeof( struct _mulle_objc_infraclass *),
                (mulle_qsort_cmp_t *) reverse_compare_classindex);
}


static struct _mulle_objc_infraclass **
   _mulle_objc_universe_all_infraclasses( struct _mulle_objc_universe *universe,
                                          unsigned int *n_classes,
                                          struct mulle_allocator *allocator)
{
   struct _mulle_objc_infraclass               **p_cls;
   struct _mulle_objc_infraclass               **array;
   struct _mulle_objc_infraclass               **sentinel;
   struct mulle_concurrent_hashmapenumerator   rover;
   unsigned int                                n;
   intptr_t                                    classid;

   n = mulle_concurrent_hashmap_count( &universe->classtable);
   if( ! n)
   {
      *n_classes = 0;
      return( NULL);
   }

   array = mulle_allocator_calloc( allocator,
                                   n,
                                   sizeof( struct _mulle_objc_classpair *));

   sentinel = &array[ n];
   p_cls    = array;

   // this code is a bit weird, because of the funny reuse of p_cls, that's
   // why its not as for loop (and haven't had time to pretty it up)
   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &classid, (void **) p_cls))
   {
      assert( *p_cls);
      // poser check: classes posing as another class, will appear later
      //              again, ignore them
      if( (*p_cls)->base.classid != classid)
         continue;
      if( ++p_cls >= sentinel)
         break;
   }
   mulle_concurrent_hashmapenumerator_done( &rover);

   *n_classes = (unsigned int) (p_cls - array);
   return( array);
}


static void   _mulle_objc_constantobject_dealloc( struct _mulle_objc_object *obj,
                                                  mulle_objc_methodid_t deallocSel)
{
   mulle_objc_implementation_t     imp;
   struct _mulle_objc_class        *cls;

   cls   = _mulle_objc_object_get_isa( obj);
   imp   = _mulle_objc_class_lookup_implementation_noforward( cls, deallocSel);
   if ( _mulle_objc_object_is_constant( obj))
      _mulle_objc_object_deconstantify_noatomic( obj);

   if( imp)
      mulle_objc_implementation_invoke( imp, obj, deallocSel, obj);
   else
      mulle_objc_object_call( obj, 0x9929eb3d /* @selector( dealloc) */, NULL);
}



static void   _mulle_objc_classcluster_dealloc( struct _mulle_objc_object *placeholder)
{
   _mulle_objc_constantobject_dealloc( placeholder, 0x0f0ab9f6); // @selector( __deallocClassCluster));
}


static void   _mulle_objc_instantiate_dealloc( struct _mulle_objc_object *placeholder)
{
   _mulle_objc_constantobject_dealloc( placeholder, 0x74cf60ba); // @selector( __deallocInstantiate));
}


static void   _mulle_objc_singleton_dealloc( struct _mulle_objc_object *placeholder)
{
   _mulle_objc_constantobject_dealloc( placeholder, 0xcdfeb729); // @selector( __deallocSingleton));
}


static void   (*dealloc_functions[])( struct _mulle_objc_object *) =
{
   _mulle_objc_classcluster_dealloc,
   _mulle_objc_instantiate_dealloc,
   _mulle_objc_singleton_dealloc
};


static void
   _mulle_objc_universe_dealloc_placeholders( struct _mulle_objc_universe *universe,
                                              enum _mulle_objc_infraclass_placeholder_index index)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_object       *obj;
   static void                     (*f)( struct _mulle_objc_object *);
   intptr_t                        classid;

   f     = dealloc_functions[ index];

   mulle_concurrent_hashmap_for( &universe->classtable, classid, infra)
   {
      // poser check: classes posing as another class, will appear later
      //              again, ignore them

      if( infra->base.classid != classid)
         continue;

      obj = _mulle_objc_infraclass_get_placeholder( infra, index);
      if( obj)
      {
         // MEMO:
         // this "NULL" is useful in poser situations where we get the
         // same class possibly twice for dealloc
         //_mulle_atomic_pointer_cas( &infra->placeholders[ index].pointer,
         //                           NULL,
         //                           obj);
         (*f)( obj);
      }
   }
}


static void
   _mulle_objc_universe_call_infraclasses( struct _mulle_objc_universe *universe,
                                           void (*f)( struct _mulle_objc_infraclass *))
{
   unsigned int                    n_classes;
   struct _mulle_objc_infraclass   **array;
   struct _mulle_objc_infraclass   **p;
   struct _mulle_objc_infraclass   **sentinel;
   struct _mulle_objc_infraclass   *infra;
   struct mulle_allocator          *allocator;

   allocator = _mulle_objc_universe_get_allocator( universe);
   array     = _mulle_objc_universe_all_infraclasses( universe, &n_classes, allocator);
   _mulle_objc_universe_reversesort_infraclasses_by_classindex( array, n_classes);

   // then call deinitialize to get rid of static cvars
   p        = array;
   sentinel = &p[ n_classes];
   while( p < sentinel)
   {
      infra = *p++;
      (*f)( infra);
   }
   mulle_allocator_free( allocator, array);
}


static void
   _mulle_objc_universe_deinitialize_infraclasses( struct _mulle_objc_universe *universe)
{
   _mulle_objc_universe_call_infraclasses( universe, _mulle_objc_infraclass_call_deinitialize);
}


static void
   _mulle_objc_universe_finalize_infraclasses( struct _mulle_objc_universe *universe)
{
   _mulle_objc_universe_call_infraclasses( universe, _mulle_objc_infraclass_call_finalize);
}


static void
   _mulle_objc_universe_unload_infraclasses( struct _mulle_objc_universe *universe)
{
   _mulle_objc_universe_call_infraclasses( universe, _mulle_objc_infraclass_call_unload);
}


static void
   _mulle_objc_universe_willfinalize_infraclasses( struct _mulle_objc_universe *universe)
{
   _mulle_objc_universe_call_infraclasses( universe, _mulle_objc_infraclass_call_willfinalize);
}



static void
   _mulle_objc_universe_free_classpairs( struct _mulle_objc_universe *universe)
{
   unsigned int                    n_classes;
   struct _mulle_objc_infraclass   **array;
   struct _mulle_objc_infraclass   **p;
   struct _mulle_objc_infraclass   **sentinel;
   struct mulle_allocator          *allocator;

   allocator = _mulle_objc_universe_get_allocator( universe);
   array     = _mulle_objc_universe_all_infraclasses( universe, &n_classes, allocator);

   p        = array;
   sentinel = &p[ n_classes];
   while( p < sentinel)
   {
      if( universe->debug.trace.class_free)
         mulle_objc_universe_trace( universe,
                                    "destroying classpair %p \"%s\"",
                                    _mulle_objc_infraclass_get_classpair( *p),
                                    _mulle_objc_infraclass_get_name( *p));

      _mulle_objc_classpair_free( _mulle_objc_infraclass_get_classpair( *p),
                                  allocator);
      ++p;
   }

   mulle_allocator_free( allocator, array);
}


static void   free_gift( void *p, struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, p);
}


enum mulle_objc_universe_status
   _mulle_objc_universe_check_waitqueues( struct _mulle_objc_universe *universe)
{
   int           rval;

   unsigned int  n_classes;
   unsigned int  n_categories;

   rval         = mulle_objc_universe_is_ok;
   n_classes    = mulle_concurrent_hashmap_count( &universe->waitqueues.classestoload);
   n_categories = mulle_concurrent_hashmap_count( &universe->waitqueues.categoriestoload);
   if( ! (n_classes + n_categories))
      return( rval);

   /*
    * free various stuff
    */
   universe->debug.trace.waiters_svg = getenv_yes_no( "MULLE_OBJC_TRACE_WAITERS_SVG");
   if( universe->debug.trace.waiters_svg)
      fprintf( stderr, "digraph waiters\n{\trankdir=\"LR\"\n\tnode [ shape=\"box\"]\n");

   if( n_classes)
   {
      rval = mulle_objc_universe_is_incomplete;
      if( ! universe->debug.trace.waiters_svg)
         fprintf( stderr, "mulle_objc_universe %p warning: the following "
                          "classes failed to load:\n",
                          universe);
      if( _mulle_objc_universe_trylock_waitqueues( universe))
      {
         fprintf( stderr, "mulle_objc_universe %p error: the waitqueues "
                          "are still locked!\n",
                          universe);
         return( mulle_objc_universe_is_locked);
      }

      pointerarray_in_hashmap_map( universe,
                                   &universe->waitqueues.classestoload,
                                   (void (*)()) mulle_objc_loadclass_print_unfulfilled_dependency);
      _mulle_objc_universe_unlock_waitqueues( universe);
   }

   if( n_categories)
   {
      rval = mulle_objc_universe_is_incomplete;
      if( ! universe->debug.trace.waiters_svg)
         fprintf( stderr, "mulle_objc_universe %p warning: the following "
                          "categories failed to load:\n",
                          universe);
      if( _mulle_objc_universe_trylock_waitqueues( universe))
      {
         fprintf( stderr, "mulle_objc_universe %p error: the waitqueues "
                          "are still locked!\n",
                          universe);
         return( mulle_objc_universe_is_locked);
      }

      pointerarray_in_hashmap_map( universe,
                                   &universe->waitqueues.categoriestoload,
                                   (void (*)()) mulle_objc_loadcategory_print_unfulfilled_dependency);
      _mulle_objc_universe_unlock_waitqueues( universe);
   }

   if( universe->debug.trace.waiters_svg)
      fprintf( stderr, "}\n");
   else
      fprintf( stderr, "You can get graphviz output by setting MULLE_OBJC_TRACE_WAITERS_SVG=YES\n");

   mulle_objc_universe_maybe_hang_or_abort( universe);
   return( rval);
}


enum mulle_objc_universe_status
   __mulle_objc_universe_check( struct _mulle_objc_universe *universe,
                                uint32_t version)
{
   uint32_t   universe_version;

   universe_version = _mulle_objc_universe_get_version( universe);

   if( mulle_objc_version_get_major( version) != mulle_objc_version_get_major( universe_version))
      return( mulle_objc_universe_is_wrong_version);

   // during 0 development, a minor change is major
   if( ! mulle_objc_version_get_major( version) &&
       (mulle_objc_version_get_minor( version) != mulle_objc_version_get_minor( universe_version)))
      return( mulle_objc_universe_is_wrong_version);

   return( _mulle_objc_universe_check_waitqueues( universe));
}



enum mulle_objc_universe_status
   __mulle_objc_global_check_universe( char *name, uint32_t version)
{
   struct _mulle_objc_universe   *universe;
   mulle_objc_universeid_t       universeid;

   universeid = MULLE_OBJC_DEFAULTUNIVERSEID;
   if( name && *name)
      universeid = mulle_objc_universeid_from_string( name);

   universe = mulle_objc_global_get_universe( universeid);
   if( ! universe)
      return( mulle_objc_universe_is_missing);

   return( __mulle_objc_universe_check( universe, version));
}


static void
   _mulle_objc_universe_free_classgraph( struct _mulle_objc_universe *universe)
{
   /* free classes */
   _mulle_objc_universe_free_classpairs( universe);

   _mulle_concurrent_hashmap_done( &universe->waitqueues.categoriestoload);
   _mulle_concurrent_hashmap_done( &universe->waitqueues.classestoload);
   _mulle_concurrent_hashmap_done( &universe->propertyidtable);
   _mulle_concurrent_hashmap_done( &universe->supertable);
   _mulle_concurrent_hashmap_done( &universe->protocoltable);
   _mulle_concurrent_hashmap_done( &universe->descriptortable);
   _mulle_concurrent_hashmap_done( &universe->varyingtypedescriptortable);
   _mulle_concurrent_hashmap_done( &universe->classtable);
   _mulle_concurrent_hashmap_done( &universe->categorytable);

   _mulle_concurrent_pointerarray_done( &universe->staticstrings);
   _mulle_concurrent_pointerarray_done( &universe->hashnames);

   /* free gifts */
   mulle_concurrent_pointerarray_map( &universe->gifts,
                                      (void (*)()) free_gift,
                                      &universe->memory.allocator);
   _mulle_concurrent_pointerarray_done( &universe->gifts);
}


static int   fake_aba_free( void *aba,
                            void (*free)( void *, void *),
                            void *block,
                            void *owner)
{
   MULLE_C_UNUSED( aba);

   (*free)( block, owner);
   return( 0);
}


static void
   _mulle_objc_universe_willfinalize( struct _mulle_objc_universe *universe)
{
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe,
                                 "universe willfinalize classes");

   _mulle_objc_universe_willfinalize_infraclasses( universe);

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "universe willfinalize userinfo");
   _mulle_objc_universe_willfinalize_friend( universe, &universe->userinfo);

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "universe willfinalize foundation");
   // say goodbye to root classes here
   _mulle_objc_universe_willfinalize_friend( universe, &universe->foundation.universefriend);
}


// this is called by release, when all retainers are gone
void   _mulle_objc_universe_done( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_cache   *cache;
   struct mulle_allocator     *allocator;

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "universe is winding down");

   if( universe->thread && universe->thread != mulle_thread_self())
      mulle_objc_universe_fail_inconsistency( universe,
         "universe must be deallocated by the same thread that created it (sorry)");

   // check for lost classes now, its soon enough
   if( universe->debug.warn.stuck_loadable)
      _mulle_objc_universe_check_waitqueues( universe);

   // the friends are freed first, and everything is still fairly fine
   // you can still message around

   //
   // call +finalize on classes (done before the foundation winds down)
   // What is tricky here, is that the classes are finalized, yet the
   // autoreleasepool and the thread dictionary may still exist!
   //
   // MEMO: maybe collect infraclasses once and the pass them to the
   // three finalizer functions ?
   //
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe,
                                 "universe finalizes classes");
   _mulle_objc_universe_finalize_infraclasses( universe);

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "universe finalizes userinfo");
   _mulle_objc_universe_finalize_friend( universe, &universe->userinfo);

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "universe finalizes foundation");
   // say goodbye to root classes here
   _mulle_objc_universe_finalize_friend( universe, &universe->foundation.universefriend);

   //
   // TODO: wrap above statements up till here into an autoreleasepool ?
   //

   // call +deinitialize on classes
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe,
                                 "universe deinitializes classes");
   _mulle_objc_universe_deinitialize_infraclasses( universe);

   // call +unload on classes
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe,
                                 "universe unloads classes and categories");
   _mulle_objc_universe_unload_infraclasses( universe);


   //
   // dealloc singletons now
   //
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe,
                                 "universe removes singletons");
   _mulle_objc_universe_dealloc_placeholders( universe, MULLE_OBJC_INFRACLASS_SINGLETON_INDEX);

   //
   // dealloc classclusters, they should not care about +unload having run
   //
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe,
                                 "universe removes classclusters");
   _mulle_objc_universe_dealloc_placeholders( universe, MULLE_OBJC_INFRACLASS_CLASSCLUSTER_INDEX);

   //
   // dealloc instantiates, they should not care about +unload having run either
   //
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe,
                                 "universe removes instantiates");
   _mulle_objc_universe_dealloc_placeholders( universe, MULLE_OBJC_INFRACLASS_INSTANTIATE_INDEX);

   // the after the unload we tell the friends to suicide
   // the last autoreleasepool will be gone then
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "universe frees userinfo");
   _mulle_objc_universe_free_friend( universe, &universe->userinfo);

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "universe frees foundation (last autoreleasepool dies)");
   // this should put down the last autoreleasepool of the main thread
   _mulle_objc_universe_free_friend( universe, &universe->foundation.universefriend);

   if( universe->callbacks.will_dealloc)
      (*universe->callbacks.will_dealloc)( universe);

   // ******* END OF USABLE CLASSES IN UNIVERSE *****

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "universe stops messaging. Noone can message anymore from this point");
   // this can't really fail, but keep it atomic anyway
   if( ! _mulle_atomic_pointer_cas( &universe->version,
                                    (void *) mulle_objc_universe_is_deallocating,
                                    (void *) mulle_objc_universe_is_deinitializing))
   {
      mulle_objc_universe_fail_inconsistency( universe,
         "someone is messing around with the universe");
   }


   // ******* JUST FREE STUFF NOW *****

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe,
                                 "universe is now unusable and freeing stuff");

   // do it like this, because we are not initialized anymore
   // every universe has its own aba
   //
   _mulle_objc_universe_done_gc( universe);

   //
   // because we are winding the universe down, its useless to abafree stuff
   // so we change the allocator to unsafe
   //
   allocator          = _mulle_objc_universe_get_allocator( universe);
   allocator->abafree = fake_aba_free;
   allocator->aba     = NULL;

   _mulle_objc_universe_free_classgraph( universe);

   cache = _mulle_objc_cachepivot_get_cache_atomic( &universe->cachepivot);
   if( cache != &universe->empty_cache)
      _mulle_objc_cache_free( cache, allocator);

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "deallocing aba");

   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "deallocing threadlocal");

   mulle_objc_thread_unset_threadinfo( universe);
   mulle_thread_tss_free( universe->threadkey);

   mulle_thread_mutex_done( &universe->waitqueues.lock);
   mulle_thread_mutex_done( &universe->debug.lock);
   mulle_thread_mutex_done( &universe->lock);

   if( _mulle_objc_universe_is_default( universe))
   {
      // idle waste of cpu, but useful if we get reclaimed as global
      // but don't clobber version
      memset( &universe->cachepivot, 0, sizeof( universe->cachepivot));
      memset( &universe->path, 0, sizeof( *universe) - offsetof( struct _mulle_objc_universe, path));
   }
}


intptr_t  _mulle_objc_universe_retain( struct _mulle_objc_universe *universe)
{
   intptr_t  rc;

   rc = (intptr_t) _mulle_atomic_pointer_increment( &universe->retaincount_1);
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "retain the universe (%ld)", rc + 1);

   return( rc);
}


intptr_t   _mulle_objc_universe_release( struct _mulle_objc_universe *universe)
{
   intptr_t   rc;

   if( universe->config.wait_threads_on_exit || universe->config.pedantic_exit)
   {
      if( mulle_thread_self() == _mulle_objc_universe_get_thread( universe))
      {
         if( universe->debug.trace.universe)
         {
            rc = (intptr_t) _mulle_atomic_pointer_read( &universe->retaincount_1);
            if( rc)
               mulle_objc_universe_trace( universe, "main thread is waiting "
                     "on %ld threads to finish (possibly indefinitely)", (long) rc);
         }

         for(;;)
         {
            rc = (intptr_t) _mulle_atomic_pointer_read( &universe->retaincount_1);
            if( ! rc)
               break;
            mulle_thread_yield();
         }
      }
   }

   rc = (intptr_t) _mulle_atomic_pointer_decrement( &universe->retaincount_1);
   if( universe->debug.trace.universe)
      mulle_objc_universe_trace( universe, "release the universe (%ld)", rc + 1);

   if( universe->config.pedantic_exit)
   {
      if( rc == 0)
      {
         assert( universe->thread == mulle_thread_self());
         _mulle_objc_universe_crunch( universe, _mulle_objc_universe_done);
      }
   }
   return( rc);
}



# pragma mark - universe release convenience

// a convenience for testing universes
void  mulle_objc_global_release_defaultuniverse( void)
{
   struct _mulle_objc_universe   *universe;

   universe = __mulle_objc_global_get_defaultuniverse();
   if( universe && _mulle_objc_universe_is_initialized( universe))
      _mulle_objc_universe_release( universe);
}


int   mulle_objc_universe_is_initialized( struct _mulle_objc_universe *universe)
{
   return( universe ? _mulle_objc_universe_is_initialized( universe) : -1);
}


# pragma mark - garbage collection

static void   _mulle_objc_universe_init_gc( struct _mulle_objc_universe *universe)
{
   struct mulle_allocator  *allocator;

   allocator = _mulle_objc_universe_get_allocator( universe);
   if( _mulle_aba_init( &universe->garbage.aba, allocator))
      mulle_objc_universe_fail_perror( universe, "_mulle_aba_init");
}


static void   _mulle_objc_universe_done_gc( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   gc = _mulle_objc_universe_get_gc( universe);
   if( _mulle_aba_unregister_current_thread( &gc->aba))
      mulle_objc_universe_fail_perror( universe, "_mulle_aba_unregister_current_thread");

   _mulle_aba_done( &gc->aba);
}


void   _mulle_objc_thread_register_universe_gc( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   assert( _mulle_objc_universe_is_initialized( universe));

   gc = _mulle_objc_universe_get_gc( universe);
   if( _mulle_aba_register_current_thread( &gc->aba))
      mulle_objc_universe_fail_perror( universe, "_mulle_aba_register_current_thread");
}


int    _mulle_objc_thread_isregistered_universe_gc( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   assert( _mulle_objc_universe_is_messaging( universe));

   gc = _mulle_objc_universe_get_gc( universe);
   return( _mulle_aba_is_current_thread_registered( &gc->aba));
}


void   _mulle_objc_thread_register_universe_gc_if_needed( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

// don't assert here as its called by the "bang"s
//   assert( _mulle_objc_universe_is_initialized( universe));

   gc = _mulle_objc_universe_get_gc( universe);
   if( _mulle_aba_is_current_thread_registered( &gc->aba))
      return;

   if( _mulle_aba_register_current_thread( &gc->aba))
      mulle_objc_universe_fail_perror( universe, "_mulle_aba_register_current_thread");
}


void   _mulle_objc_thread_remove_universe_gc( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   assert( _mulle_objc_universe_is_messaging( universe));

   gc = _mulle_objc_universe_get_gc( universe);
   if( _mulle_aba_unregister_current_thread( &gc->aba))
      mulle_objc_universe_fail_perror( universe, "_mulle_aba_unregister_current_thread");
}


void   _mulle_objc_thread_checkin_universe_gc( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   assert( _mulle_objc_universe_is_messaging( universe));

   gc = _mulle_objc_universe_get_gc( universe);
   if( _mulle_aba_checkin_current_thread( &gc->aba))
      mulle_objc_universe_fail_perror( universe, "_mulle_aba_checkin_current_thread");
}



# pragma mark - "classes"

//
// can be useful if you are using the thread local universe, and the
// const thing doesn't work to your advantage
//
struct _mulle_objc_universe  *
   mulle_objc_global_get_universe( mulle_objc_universeid_t universeid)
{
   return( mulle_objc_global_get_universe_inline( universeid));
}



int   _mulle_objc_universe_register_infraclass_for_classid( struct _mulle_objc_universe *universe,
                                                            struct _mulle_objc_infraclass *infra,
                                                            mulle_objc_classid_t classid)
{
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_infraclass   *superclass;
   struct _mulle_objc_metaclass    *dup;

   dup = _mulle_concurrent_hashmap_register( &universe->classtable,
                                             classid,
                                             infra);
   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      return( -1);
   if( dup)
   {
      errno = EEXIST;
      return( -1);
   }

   if( universe->debug.trace.class_add || universe->debug.trace.dependency)
   {
      mulle_objc_universe_trace_nolf( universe,
                                      "added class #%ld %08x \"%s\"",
                                      (long) _mulle_objc_infraclass_get_classindex( infra),
                                      _mulle_objc_infraclass_get_classid( infra),
                                      _mulle_objc_infraclass_get_name( infra));

      superclass = _mulle_objc_infraclass_get_superclass( infra);
      if( superclass)
         fprintf( stderr,
                  " with superclass %08x \"%s\"",
                 _mulle_objc_infraclass_get_classid( superclass),
                 _mulle_objc_infraclass_get_name( superclass));

      meta = _mulle_objc_infraclass_get_metaclass( infra);
      fprintf( stderr, " (-:%p +:%p)\n", infra, meta);
   }
   return( 0);
}


/* don't check for ivar_hash, as this is too painful for application
   universe hacks. Only during loading
 */
int   mulle_objc_universe_register_infraclass( struct _mulle_objc_universe *universe,
                                          struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_infraclass   *superclass;
   struct _mulle_objc_classpair    *pair;
   uintptr_t                       classindex;


   if( ! universe || ! infra)
   {
      errno = EINVAL;
      return( -1);
   }

   meta  = _mulle_objc_infraclass_get_metaclass( infra);
   if( ! mulle_objc_infraclass_is_sane( infra) ||
       ! mulle_objc_metaclass_is_sane( meta))
   {
      // errno is set by is_sane;
      return( -1);
   }

   superclass = _mulle_objc_infraclass_get_superclass( infra);
   if( superclass)
   {
      if( ! _mulle_concurrent_hashmap_lookup( &universe->classtable,
                                              _mulle_objc_infraclass_get_classid( superclass)))
      {
         errno = EFAULT;
         return( -1);
      }
   }

   classindex = (uintptr_t) _mulle_atomic_pointer_increment( &universe->classindex);
   pair       = _mulle_objc_infraclass_get_classpair( infra);
   _mulle_objc_classpair_set_classindex( pair, (uint32_t) classindex);

   return( _mulle_objc_universe_register_infraclass_for_classid( universe,
                                                                 infra,
                                                                 _mulle_objc_infraclass_get_classid( infra)));
}


//
// This will use the superclasss of infra, there is no other way
// So the next time a class is looked up by "classid" (or via string)
// this will not find "infra", which means:
// Foo : Bar and mulle_objc_universe_patch_infraclass( ..., [Foo class])
// [Bar alloc] will produce a Foo now. By itself this isn't sufficient for
// poseAs: though, as FooBar : Bar. A FooBar instance will still message
// Foo when calling super.
//
int   mulle_objc_universe_patch_infraclass( struct _mulle_objc_universe *universe,
                                            struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_infraclass   *superclass;
   struct _mulle_objc_classpair    *pair;
   uintptr_t                       classindex;
   int                             rval;
   int                             index;

   if( ! universe || ! infra)
   {
      errno = EINVAL;
      return( -1);
   }

   meta  = _mulle_objc_infraclass_get_metaclass( infra);
   if( ! mulle_objc_infraclass_is_sane( infra) ||
       ! mulle_objc_metaclass_is_sane( meta))
   {
      // errno is set by is_sane;
      return( -1);
   }

   superclass = _mulle_objc_infraclass_get_superclass( infra);
   if( superclass)
   {
      if( ! _mulle_concurrent_hashmap_lookup( &universe->classtable,
                                              _mulle_objc_infraclass_get_classid( superclass)))
      {
         errno = EFAULT;
         return( -1);
      }
   }

   index = mulle_objc_get_fastclasstable_index( _mulle_objc_infraclass_get_classid( superclass));
   if( index != -1)
   {
      errno = EDOM;  // TODO: maybe not impossible, but NYI
      return( -1);
   }


   classindex = (uintptr_t) _mulle_atomic_pointer_increment( &universe->classindex);
   pair       = _mulle_objc_infraclass_get_classpair( infra);
   _mulle_objc_classpair_set_classindex( pair, (uint32_t) classindex);

   rval = mulle_concurrent_hashmap_patch( &universe->classtable,
                                          _mulle_objc_infraclass_get_classid( superclass),
                                          infra,
                                          superclass);
   if( rval)
   {
      errno = rval;
      return( -1);
   }

   if( universe->debug.trace.class_add || universe->debug.trace.dependency)
   {
      mulle_objc_universe_trace_nolf( universe,
                                      "class #%ld %08x \"%s\"",
                                      (long) classindex,
                                      _mulle_objc_infraclass_get_classid( infra),
                                      _mulle_objc_infraclass_get_name( infra));
      if( superclass)
         fprintf( stderr,
                  " poses now as superclass %08x \"%s\"",
                 _mulle_objc_infraclass_get_classid( superclass),
                 _mulle_objc_infraclass_get_name( superclass));
      fprintf( stderr, " (-:%p +:%p)\n", infra, meta);
   }
   return( 0);
}



void   _mulle_objc_universe_remove_fastclass_at_index( struct _mulle_objc_universe *universe,
                                                       unsigned int index)
{
   struct _mulle_objc_infraclass   *infra;

   assert( universe);
   assert( index < MULLE_OBJC_S_FASTCLASSES);

   infra = mulle_objc_fastclasstable_get_infraclass( &universe->fastclasstable, index);
   if( _mulle_atomic_pointer_cas( &universe->fastclasstable.classes[ index].pointer, NULL, infra))
   {
      if( universe->debug.trace.fastclass_add)
         mulle_objc_universe_trace( universe,
                                    "remove fastclass \"%s\" at index %d",
                                    infra->base.name,
                                    index);
   }
}


int   mulle_objc_universe_remove_infraclass( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_infraclass *infra)
{
   int   rval;
   int   index;

   if( ! universe || ! infra)
   {
      errno = EINVAL;
      return( -1);
   }

   // remove from main storage
   // so wont appear in cache again
   rval = _mulle_concurrent_hashmap_remove( &universe->classtable,
                                            infra->base.classid,
                                            infra);

   if( rval)
   {
      errno = rval;
      return( -1);
   }

   if( universe->debug.trace.class_add || universe->debug.trace.dependency)
   {
      mulle_objc_universe_trace( universe,
                                 "removed class %08x \"%s\"",
                                 _mulle_objc_infraclass_get_classid( infra),
                                 _mulle_objc_infraclass_get_name( infra));
   }

   // remove from class caches
    _mulle_objc_universe_invalidate_classcache( universe);

   // remove from fastclass table
   index = mulle_objc_get_fastclasstable_index( _mulle_objc_infraclass_get_classid( infra));
   if( index > 0)
      _mulle_objc_universe_remove_fastclass_at_index( universe, index);

   return( 0);
}


void   mulle_objc_universe_register_infraclass_nofail( struct _mulle_objc_universe *universe,
                                                  struct _mulle_objc_infraclass *infra)
{
   if( mulle_objc_universe_register_infraclass( universe, infra))
      mulle_objc_universe_fail_errno( universe);
}


void   _mulle_objc_universe_set_fastclass( struct _mulle_objc_universe *universe,
                                           struct _mulle_objc_infraclass *infra,
                                           unsigned int index)
{
   struct _mulle_objc_infraclass  *old;

   assert( universe);
   assert( infra);

   if( universe->debug.trace.fastclass_add)
      mulle_objc_universe_trace( universe,
                                 "add fastclass \"%s\" at index %d",
                                 infra->base.name,
                                 index);

   if( index >= MULLE_OBJC_S_FASTCLASSES)
      mulle_objc_universe_fail_generic( universe,
                  "error in mulle_objc_universe %p: "
                  "fastclass index %d for %s (id %08lx) out of bounds\n",
                  universe,
                  index,
                  _mulle_objc_infraclass_get_name( infra),
                  _mulle_objc_infraclass_get_classid( infra));

   if( ! _mulle_atomic_pointer_cas( &universe->fastclasstable.classes[ index].pointer, infra, NULL))
   {
      old = _mulle_atomic_pointer_read( &universe->fastclasstable.classes[ index].pointer);
      mulle_objc_universe_fail_inconsistency( universe,
                  "mulle_objc_universe %p: classes \"%s\" "
                  "and \"%s\", both want to occupy fastclass spot %u",
                  universe,
                  _mulle_objc_infraclass_get_name( infra),
                  _mulle_objc_infraclass_get_name( old),
                  index);
   }
}


# pragma mark - method descriptors

static void  class_list_methods( struct _mulle_objc_class *cls,
                                 char prefix,
                                 mulle_objc_methodid_t methodid,
                                 FILE *fp)
{
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   struct _mulle_objc_method            *method;
   char                                 *s;

   for( search = mulle_objc_searcharguments_make_default( methodid);
        ;
        search = mulle_objc_searcharguments_make_previous( method))
   {
      _mulle_objc_searcharguments_set_initialize( &search, 0); // do not trigger +initialize
      method = mulle_objc_class_search_method( cls,
                                               &search,
                                               MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_META \
                                               | MULLE_OBJC_CLASS_DONT_INHERIT_SUPERCLASS \
                                               | MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS \
                                               | MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES,
                                               &result);
      if( ! method)
         break;

      mulle_fprintf( fp, "%c[%s", prefix, cls->name);
      s = _mulle_objc_methodlist_get_categoryname( result.list);
      if( s && ! s[ 0])
         mulle_fprintf( fp, "( %s)", s);
      mulle_fprintf( fp, " %s] bits=0x%lx signature=%s\n",
                         method->descriptor.name,
                         method->descriptor.bits,
                         method->descriptor.signature);
   }
}



static void   mulle_objc_universe_list_methods( struct _mulle_objc_universe *universe,
                                                mulle_objc_methodid_t methodid,
                                                FILE *fp)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_class        *infra_cls;
   struct _mulle_objc_class        *meta_cls;
   intptr_t                        classid;

   if( ! universe || ! fp)
      return;

   mulle_concurrent_hashmap_for( &universe->classtable, classid, infra_cls)
   {
      infra    = _mulle_objc_class_as_infraclass( infra_cls);
      meta     = _mulle_objc_infraclass_get_metaclass( infra);
      meta_cls = _mulle_objc_metaclass_as_class( meta);
      class_list_methods( meta_cls, '+', methodid, fp);
      class_list_methods( infra_cls, '-', methodid, fp);
   }
}


static struct _mulle_objc_descriptor *
   _mulle_objc_universe_register_descriptor( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_descriptor *p,
                                             struct _mulle_objc_class *cls,
                                             struct _mulle_objc_methodlist *list)
{
   int                             signature_comparison;
   int                             bit_comparison;
   struct _mulle_objc_descriptor   *dup;
   int                             is_meta;

   is_meta = (cls && _mulle_objc_class_is_metaclass( cls));

   dup = _mulle_concurrent_hashmap_register( &universe->descriptortable,
                                             p->methodid,
                                             p);
   if( ! dup)
   {
      if( universe->debug.trace.descriptor_add)
      {
         if( cls)
         {
            if( list && _mulle_objc_methodlist_get_categoryname( list))
               fprintf( stderr, "added descriptor %08x \"%s\" by %c%s( %s) (%p)\n",
                                p->methodid,
                                p->name,
                                _mulle_objc_class_is_metaclass( cls) ? '+' : '-',
                                cls->name,
                                _mulle_objc_methodlist_get_categoryname( list),
                                p);
            else
               fprintf( stderr, "added descriptor %08x \"%s\" by %c%s (%p)\n",
                                p->methodid,
                                p->name,
                                _mulle_objc_class_is_metaclass( cls) ? '+' : '-',
                                cls->name,
                                p);
         }
         else
            mulle_objc_universe_trace( universe,
                                       "added descriptor %08x \"%s\" (%p)",
                                       p->methodid,
                                       p->name,
                                       p);
      }
      return( p);
   }

   // must be out of mem
   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      return( 0);

   assert( p->methodid == dup->methodid);

   // hash clash is very bad
   if( strcmp( dup->name, p->name))
      mulle_objc_universe_fail_generic( universe,
            "mulle_objc_universe %p error: duplicate methods \"%s\" and \"%s\" "
            "with same id %08lx\n", universe, dup->name, p->name, (long) p->methodid);

   //
   // update the descriptor, so we know if its used by class and instance
   // methods...
   //
   switch( universe->debug.warn.method_type)
   {
   default :
   case MULLE_OBJC_WARN_METHOD_TYPE_NONE :
      signature_comparison = 0;
      break;
   case MULLE_OBJC_WARN_METHOD_TYPE_LENIENT :
      signature_comparison = _mulle_objc_methodsignature_compare_lenient( dup->signature, p->signature);
      break;
   case MULLE_OBJC_WARN_METHOD_TYPE_NORMAL :
      signature_comparison = _mulle_objc_methodsignature_compare( dup->signature, p->signature);
      break;
   case MULLE_OBJC_WARN_METHOD_TYPE_STRICT :
      signature_comparison = _mulle_objc_signature_compare_strict( dup->signature, p->signature);
      break;
   }

   bit_comparison = 0;
   if( universe->debug.warn.method_bits)
      bit_comparison = ! _mulle_objc_method_bits_type_equal( dup->bits, p->bits);

   if( signature_comparison ||  bit_comparison)
   {
      // the value in the table is unimportant. I might write a hashset
      // for this, but I am too lazy now.
      _mulle_concurrent_hashmap_register( &universe->varyingtypedescriptortable,
                                          p->methodid,
                                          (void *) 0x1848);

      //
      // hack: so ':' can be used without warning as a shortcut selector
      //       also hack string and data for the time being (sight)
      switch( p->methodid)
      {
      case 0x7c165381 : // string
      case 0xf0cb86d3 : // :
      case 0x872e2a5d : // data
         break;

      default         :
         if( ! bit_comparison)
            mulle_fprintf( stderr, "mulle_objc_universe %p warning: varying signatures \"%s\" "
                             "and registered \"%s\" ",
                             universe,
                             dup->signature,
                             p->signature);
         else
            if( ! signature_comparison)
               mulle_fprintf( stderr, "mulle_objc_universe %p warning: varying bits 0x%lx "
                                "and registered 0x%lx ",
                                universe,
                                (unsigned long) dup->bits, (unsigned long) p->bits);
            else
               mulle_fprintf( stderr, "mulle_objc_universe %p warning: varying signatures/bits \"%s\"/0x%lx "
                                "and registered \"%s\"/0x%lx ",
                                universe,
                                dup->signature, (unsigned long) dup->bits,
                                p->signature, (unsigned long) p->bits);
         mulle_fprintf( stderr, "for method %08lx ", (long) p->methodid);
         if( cls)
         {
            if( list && _mulle_objc_methodlist_get_categoryname( list))
               mulle_fprintf( stderr, "%c[%s( %s) %s]",
                                is_meta ? '+' : '-',
                                cls->name,
                                _mulle_objc_methodlist_get_categoryname( list),
                                p->name);
            else
               mulle_fprintf( stderr, "%c[%s %s]",
                                is_meta ? '+' : '-',
                                cls->name,
                                p->name);
         }
         else
            mulle_fprintf( stderr, "\"%s\"",
                             p->name);
         mulle_fprintf( stderr, " (Tip: MULLE_OBJC_TRACE_DESCRIPTOR_ADD=YES)\n");

         mulle_objc_universe_list_methods( universe, p->methodid, stderr);
         mulle_objc_universe_maybe_hang_or_abort( universe);
      }
   }

   // update bits for successful second registration
   return( dup);
}



//
// returns duplicate if found!
//
struct _mulle_objc_descriptor *
   _mulle_objc_universe_register_descriptor_nofail( struct _mulle_objc_universe *universe,
                                                    struct _mulle_objc_descriptor *p,
                                                    struct _mulle_objc_class *cls,
                                                    struct _mulle_objc_methodlist *list)
{
   struct _mulle_objc_descriptor   *dup;

   dup = _mulle_objc_universe_register_descriptor( universe, p, cls, list);
   if( ! dup)
      mulle_objc_universe_fail_errno( NULL);
   return( dup);
}


struct _mulle_objc_descriptor *
   mulle_objc_universe_register_descriptor_nofail( struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_descriptor *p,
                                                   struct _mulle_objc_class *cls,
                                                   struct _mulle_objc_methodlist *list)
{
   if( ! universe)
      mulle_objc_universe_fail_code( universe, EINVAL);
   if( ! mulle_objc_descriptor_is_sane( p))
      mulle_objc_universe_fail_code( universe, EINVAL);

   return( _mulle_objc_universe_register_descriptor_nofail( universe, p, cls, list));
}


struct _mulle_objc_descriptor *
   mulle_objc_universe_lookup_descriptor_nofail( struct _mulle_objc_universe *universe,
                                                 mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_descriptor *p;

   if( ! universe)
      mulle_objc_universe_fail_code( universe, EINVAL);

   p = _mulle_objc_universe_lookup_descriptor( universe, methodid);
   if( ! p)
      mulle_objc_universe_fail_descriptornotfound( universe, methodid);
   return( p);
}



// function kept for tests, register is the way to go though
int   _mulle_objc_universe_add_descriptor( struct _mulle_objc_universe *universe,
                                           struct _mulle_objc_descriptor *p,
                                           struct _mulle_objc_class *cls,
                                           struct _mulle_objc_methodlist *list)
{
   return( _mulle_objc_universe_register_descriptor( universe, p, cls, list) != NULL ? 0 : -1);
}



int   mulle_objc_universe_add_descriptor( struct _mulle_objc_universe *universe,
                                          struct _mulle_objc_descriptor *p,
                                          struct _mulle_objc_class *cls,
                                          struct _mulle_objc_methodlist *list)
{
   if( ! universe)
   {
      errno = EINVAL;
      return( -1);
   }

   if( ! mulle_objc_descriptor_is_sane( p))
      return( -1);

   return( _mulle_objc_universe_add_descriptor( universe, p, cls, list));
}



char   *mulle_objc_universe_lookup_methodname( struct _mulle_objc_universe *universe,
                                               mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_descriptor   *desc;

   if( ! universe)
   {
      errno = EINVAL;
      return( (char *) -1);
   }

   desc    = _mulle_objc_universe_lookup_descriptor( universe, methodid);
   return( desc ? desc->name : NULL);
}


// just used by some tests
char   *mulle_objc_global_lookup_methodname( mulle_objc_universeid_t universeid,
                                             mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_descriptor   *desc;

   universe = mulle_objc_global_get_universe_inline( universeid);

   desc    = _mulle_objc_universe_lookup_descriptor( universe, methodid);
   return( desc ? desc->name : NULL);
}


# pragma mark - generic hashmap walker


mulle_objc_walkcommand_t
   _mulle_objc_universe_walk_hashmap( struct _mulle_objc_universe  *universe,
                                      struct mulle_concurrent_hashmap *hashmap,
                                      mulle_objc_walk_hashmap_callback_t callback,
                                      void *userinfo)
{
   mulle_objc_walkcommand_t   cmd;
   void                       *item;
   void                       *skip;
   void                       *hash;
   int                        rval;

   if( ! universe || ! callback)
      return( mulle_objc_walk_error);

   cmd  = mulle_objc_walk_done;
   skip = NULL;
again:
   mulle_concurrent_hashmap_for_rval( hashmap, hash, item, rval)
   {
      if( skip)
      {
         if( item == skip)
            skip = NULL;
         continue;
      }
      cmd = (*callback)( universe, item, userinfo);
      if( mulle_objc_walkcommand_is_stopper( cmd))
         break;
   }

   if( rval == EBUSY)
   {
      skip = item;
      goto again;
   }

   return( cmd);
}


# pragma mark - property id lookup

static void  infraclass_list_properties( struct _mulle_objc_infraclass *infra,
                                         mulle_objc_propertyid_t propertyid,
                                         FILE *fp)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_property   *property;
   char                          *sep;
   char                          *sep2;

   property = mulle_objc_infraclass_search_property( infra, propertyid);
   if( ! property)
      return;

   universe = _mulle_objc_infraclass_get_universe( infra);
   sep  = "(";
   sep2 = " ";
   mulle_fprintf( fp, "@interface %s @property");
   if( property->propertyid != property->getter)
   {
      mulle_fprintf( fp, "%sgetter=%s", _mulle_objc_universe_describe_methodid( universe, property->getter));
      sep = ", ";
      sep2 = ") ";
   }
   if( property->setter)
   {
      mulle_fprintf( fp, "%ssetter=%s", sep, _mulle_objc_universe_describe_methodid( universe, property->setter));
      sep = ", ";
      sep2 = ") ";
   }
   if( property->adder)
   {
      mulle_fprintf( fp, "%adder=%s", _mulle_objc_universe_describe_methodid( universe, property->adder));
      sep = ", ";
      sep2 = ") ";
   }
   if( property->remover)
   {
      mulle_fprintf( fp, "%remover=%s", _mulle_objc_universe_describe_methodid( universe, property->remover));
      sep = ", ";
      sep2 = ") ";
   }

   mulle_fprintf( fp, "%s%s; @end\n", sep2, property->name);
}


static void   mulle_objc_universe_list_properties( struct _mulle_objc_universe *universe,
                                                   mulle_objc_propertyid_t propertyid,
                                                   FILE *fp)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_class        *infra_cls;
   intptr_t                        classid;

   if( ! universe || ! fp)
      return;

   mulle_concurrent_hashmap_for( &universe->classtable, classid, infra_cls)
   {
      infra = _mulle_objc_class_as_infraclass( infra_cls);
      infraclass_list_properties( infra, propertyid, fp);
   }
}


static mulle_objc_propertyid_t
   _mulle_objc_universe_register_propertyid_for_methodid( struct _mulle_objc_universe *universe,
                                                          mulle_objc_propertyid_t propertyid,
                                                          mulle_objc_methodid_t methodid)
{
   void                      *dup;

   dup = _mulle_concurrent_hashmap_register( &universe->propertyidtable,
                                             methodid,
                                             (void *) (uintptr_t) propertyid);
   if( ! dup)
   {
      if( universe->debug.trace.propertyid_add)
      {
         mulle_objc_universe_trace( universe,
                                    "added propertyid %08x for methodid %08x",
                                    propertyid,
                                    methodid);
      }
      return( propertyid);
   }

   // must be out of mem
   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      return( 0);

   if( (mulle_objc_propertyid_t) (uintptr_t) dup != propertyid)
   {
      mulle_fprintf( stderr,
                     "mulle_objc_universe %p warning: method %08x \"%s\" already "
                     "registered for protocolid %08x\n",
                     universe,
                     methodid, _mulle_objc_universe_describe_methodid( universe, methodid),
                     propertyid);
      mulle_objc_universe_list_properties( universe, propertyid, stderr);
      mulle_objc_universe_maybe_hang_or_abort( universe);
      propertyid = 0;
   }
   return( propertyid);
}


void
    mulle_objc_universe_register_propertyid_for_methodid_nofail( struct _mulle_objc_universe *universe,
                                                                 mulle_objc_propertyid_t propertyid,
                                                                 mulle_objc_methodid_t methodid)
{
   mulle_objc_propertyid_t   dupid;

   if( ! universe)
      mulle_objc_universe_fail_code( universe, EINVAL);
   if( ! mulle_objc_propertyid_is_sane( propertyid))
      mulle_objc_universe_fail_code( universe, EINVAL);
   if( ! mulle_objc_methodid_is_sane( methodid))
      mulle_objc_universe_fail_code( universe, EINVAL);

   dupid = _mulle_objc_universe_register_propertyid_for_methodid( universe, propertyid, methodid);
   if( ! dupid)
      mulle_objc_universe_fail_errno( NULL);
}


# pragma mark - super map

struct _mulle_objc_super   *
   _mulle_objc_universe_lookup_super( struct _mulle_objc_universe *universe,
                                      mulle_objc_superid_t superid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->supertable, superid));
}


struct _mulle_objc_super   *
   _mulle_objc_universe_lookup_super_nofail( struct _mulle_objc_universe *universe,
                                             mulle_objc_superid_t superid)
{
   struct _mulle_objc_super   *p;

   p = _mulle_objc_universe_lookup_super( universe, superid);
   if( ! p)
      mulle_objc_universe_fail_supernotfound( universe, superid);
   return( p);
}


int   _mulle_objc_universe_register_super( struct _mulle_objc_universe *universe,
                                           struct _mulle_objc_super *p)
{
   struct _mulle_objc_super   *dup;

   if( ! mulle_objc_super_is_sane( p))
      return( -1);

   dup = _mulle_concurrent_hashmap_register( &universe->supertable, p->superid, p);
   if( ! dup)
   {
      if( universe->debug.trace.super_add)
         mulle_objc_universe_trace( universe,
                                    "add super %08x,%08x,%08x (%p)",
                                    p->superid,
                                    p->classid,
                                    p->methodid,
                                    p);
      return( 0);
   }

   if( dup->classid != p->classid || dup->methodid != p->methodid)
      mulle_objc_universe_fail_generic( universe,
            "mulle_objc_universe %p error: duplicate supers %08x,%08x "
            "and %08x,%08x with same id %08x\n",
            universe,
            dup->classid, dup->methodid,
            p->classid, p->methodid,
            p->superid);

   return( 0);
}


void    mulle_objc_universe_register_super_nofail( struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_super *p)
{
   assert( universe);

   if( _mulle_objc_universe_register_super( universe, p))
      mulle_objc_universe_fail_errno( universe);
}




# pragma mark - protocols

struct _mulle_objc_protocol   *
   _mulle_objc_universe_lookup_protocol( struct _mulle_objc_universe *universe,
                                         mulle_objc_protocolid_t protocolid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->protocoltable, protocolid));
}


int   _mulle_objc_universe_register_protocol( struct _mulle_objc_universe *universe,
                                              struct _mulle_objc_protocol *p)

{
   struct _mulle_objc_protocol   *dup;

   if( p->protocolid == MULLE_OBJC_NO_PROTOCOLID ||
       p->protocolid == MULLE_OBJC_INVALID_PROTOCOLID)
      return( -1);

   dup = _mulle_concurrent_hashmap_register( &universe->protocoltable, p->protocolid, p);
   if( ! dup)
   {
      if( universe->debug.trace.protocol_add)
         mulle_objc_universe_trace( universe,
                                    "add protocol %08x \"%s\" (%p)",
                                    p->protocolid,
                                    p->name,
                                    p);
      return( 0);
   }

   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      return( -1);

   assert( dup->protocolid == p->protocolid);

   if( strcmp( dup->name, p->name))
      mulle_objc_universe_fail_generic( universe,
            "mulle_objc_universe %p error: duplicate protocols \"%s\" and \"%s\" "
            "with same id %08lx\n", universe, dup->name, p->name, (long) p->protocolid);

   return( 0);
}


void    mulle_objc_universe_register_protocol_nofail( struct _mulle_objc_universe *universe,
                                                      struct _mulle_objc_protocol *protocol)
{
   assert( universe);

   if( _mulle_objc_universe_register_protocol( universe, protocol))
      mulle_objc_universe_fail_errno( universe);
}


size_t   _mulle_objc_universe_count_protocols( struct _mulle_objc_universe  *universe)
{
   return( mulle_concurrent_hashmap_count( &universe->protocoltable));
}


# pragma mark - categories

char   *_mulle_objc_universe_lookup_category( struct _mulle_objc_universe *universe,
                                             mulle_objc_categoryid_t categoryid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->categorytable, categoryid));
}


int   _mulle_objc_universe_register_category( struct _mulle_objc_universe *universe,
                                             mulle_objc_categoryid_t categoryid,
                                             char *name)

{
   char   *dup;

   if( categoryid == MULLE_OBJC_NO_CATEGORYID || categoryid == MULLE_OBJC_INVALID_CATEGORYID)
      return( -1);

   dup = _mulle_concurrent_hashmap_register( &universe->categorytable, categoryid, name);
   if( ! dup)
      return( 0);

   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      return( -1);

   if( strcmp( dup, name))
      mulle_objc_universe_fail_generic( universe,
            "mulle_objc_universe %p error: duplicate categories \"%s\" and \"%s\" "
            "with same id %08lx\n", universe, dup, name, (long) categoryid);

   return( 0);
}


void    mulle_objc_universe_register_category_nofail( struct _mulle_objc_universe *universe,
                                                      mulle_objc_categoryid_t categoryid,
                                                      char *name)
{
   char  *dup;

   assert( universe);

   if( _mulle_objc_universe_register_category( universe, categoryid, name))
   {
      dup = _mulle_objc_universe_lookup_category( universe, categoryid);
      mulle_objc_universe_fail_generic( universe,
         "mulle_objc_universe %p error: duplicate categories \"%s\" and \"%s\" "
         "with same id %08x\n", universe, dup, name, categoryid);
   }
}

# pragma mark - string cache

//
// this cache is needed until, the foundation sets the proper string class
// "usually" the universe stores all these strings, so that all objects in
// the universe system can be accounted for
//

struct _NSConstantString
{
   char           *_storage;   // ivar #0:: must be defined EXACTLY like this
   unsigned int   _length;     // ivar #1:: must be defined EXACTLY like this
};


void   _mulle_objc_universe_add_staticstring( struct _mulle_objc_universe *universe,
                                              struct _mulle_objc_object *string)
{
   assert( universe);
   assert( string);

   if( ! universe->foundation.staticstringclass)
   {
      if( universe->debug.trace.string_add)
         mulle_objc_universe_trace( universe,
                                    "delay add of string @\"%s\" (%p)",
                                    ((struct _NSConstantString *) string)->_storage,
                                    string);

      // memorize it anyway
      _mulle_concurrent_pointerarray_add( &universe->staticstrings, (void *) string);
      return;
   }

   _mulle_objc_object_set_isa( string,
                               _mulle_objc_infraclass_as_class( universe->foundation.staticstringclass));
   if( universe->debug.trace.string_add)
      mulle_objc_universe_trace( universe,
                                 "add string @\"%s\" (%p)",
                                 ((struct _NSConstantString *) string)->_storage,
                                 string);

   if( ! universe->config.forget_strings)
      _mulle_concurrent_pointerarray_add( &universe->staticstrings, (void *) string);
}


void   _mulle_objc_universe_didchange_staticstringclass( struct _mulle_objc_universe *universe,
                                                         int constantify)
{
   struct _mulle_objc_object   *string;
   struct mulle_allocator      *allocator;
   int                         flag;

   if( constantify)
      mulle_concurrent_pointerarray_for( &universe->staticstrings, string)
      {
         _mulle_objc_object_constantify_noatomic( string);
      }

   flag = universe->debug.trace.string_add;
   mulle_concurrent_pointerarray_for( &universe->staticstrings, string)
   {
      _mulle_objc_object_set_isa( string,
                                 _mulle_objc_infraclass_as_class( universe->foundation.staticstringclass));
      if( flag)
         mulle_objc_universe_trace( universe,
                                    "patch string class @\"%s\" (%p)",
                                    ((struct _NSConstantString *) string)->_storage,
                                    string);
   }

   // if so configured wipe the list
   // effectivey _mulle_objc_universe_didchange_staticstringclass should then
   // only be called once ever, which is its intention anyway
   //
   if( universe->config.forget_strings)
   {
      allocator = _mulle_objc_universe_get_allocator( universe);
      _mulle_concurrent_pointerarray_done( &universe->staticstrings);
      _mulle_concurrent_pointerarray_init( &universe->staticstrings,
                                           0,
                                           allocator);
   }
}


void  _mulle_objc_universe_set_staticstringclass( struct _mulle_objc_universe *universe,
                                                  struct _mulle_objc_infraclass *infra,
                                                  int constantify)
{
   assert( universe);
   assert( infra);

   universe->foundation.staticstringclass = infra;
   _mulle_objc_universe_didchange_staticstringclass( universe, constantify);
}


# pragma mark - hashnames (debug output only)

void   _mulle_objc_universe_add_loadhashedstringlist( struct _mulle_objc_universe *universe,
                                                      struct _mulle_objc_loadhashedstringlist *hashnames)
{
   _mulle_concurrent_pointerarray_add( &universe->hashnames, (void *) hashnames);
}


char  *_mulle_objc_universe_search_hashstring( struct _mulle_objc_universe *universe,
                                               mulle_objc_uniqueid_t hash)
{
   struct _mulle_objc_loadhashedstringlist   *map;
   char                                      *s;

   s = NULL;
   mulle_concurrent_pointerarray_for( &universe->hashnames, map)
   {
      s = mulle_objc_loadhashedstringlist_search( map, hash);
      if( s)
         break;
   }

   return( s);
}


#pragma mark - uniqueid to string conversions

MULLE_C_NONNULL_RETURN
char   *_mulle_objc_universe_describe_uniqueid( struct _mulle_objc_universe *universe,
                                                mulle_objc_uniqueid_t uniqueid)
{
   extern char   *mulle_objc_known_name_for_uniqueid( mulle_objc_uniqueid_t uniqueid);

   char   *s;

   if( uniqueid == MULLE_OBJC_NO_UNIQUEID)
      return( "NOT AN ID");
   if( uniqueid == MULLE_OBJC_INVALID_UNIQUEID)
      return( "INVALID ID");
   s = _mulle_objc_universe_search_hashstring( universe, uniqueid);
   if( ! s)
      s = mulle_objc_known_name_for_uniqueid( uniqueid);
   return( s ? s : "???");
}


MULLE_C_NONNULL_RETURN char
   *_mulle_objc_universe_describe_classid( struct _mulle_objc_universe *universe,
                                           mulle_objc_classid_t classid)
{
   struct _mulle_objc_infraclass   *infra;

   infra = _mulle_objc_universe_lookup_infraclass( universe, classid);
   if( infra)
      return( _mulle_objc_infraclass_get_name( infra));
   return( _mulle_objc_universe_describe_uniqueid( universe, classid));
}


MULLE_C_NONNULL_RETURN
char   *_mulle_objc_universe_describe_methodid( struct _mulle_objc_universe *universe,
                                                  mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_descriptor   *desc;

   desc = _mulle_objc_universe_lookup_descriptor( universe, methodid);
   if( desc)
      return( _mulle_objc_descriptor_get_name( desc));
   return( _mulle_objc_universe_describe_uniqueid( universe, methodid));
}


MULLE_C_NONNULL_RETURN
char   *_mulle_objc_universe_describe_protocolid( struct _mulle_objc_universe *universe,
                                                    mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_protocol   *protocol;

   protocol = _mulle_objc_universe_lookup_protocol( universe, protocolid);
   if( protocol)
      return( _mulle_objc_protocol_get_name( protocol));
   return( _mulle_objc_universe_describe_uniqueid( universe, protocolid));
}


MULLE_C_NONNULL_RETURN
char   *_mulle_objc_universe_describe_categoryid( struct _mulle_objc_universe *universe,
                                                    mulle_objc_categoryid_t categoryid)
{
   char    *name;

   name = _mulle_objc_universe_lookup_category( universe, categoryid);
   if( name)
      return( name);
   return( _mulle_objc_universe_describe_uniqueid( universe, categoryid));
}


MULLE_C_NONNULL_RETURN
char   *_mulle_objc_universe_describe_superid( struct _mulle_objc_universe *universe,
                                                 mulle_objc_superid_t superid)
{
   struct _mulle_objc_super   *p;

   p = _mulle_objc_universe_lookup_super( universe, superid);
   if( p)
      return( p->name);
   return( _mulle_objc_universe_describe_uniqueid( universe, superid));
}


# pragma mark - debug support

/* debug support */

mulle_objc_walkcommand_t
    mulle_objc_universe_walk( struct _mulle_objc_universe *universe,
                              mulle_objc_walkcallback_t   callback,
                              void *userinfo)
{
   mulle_objc_walkcommand_t   cmd;
   struct _mulle_objc_class   *cls;
   intptr_t                   classid;

   if( ! universe || ! callback)
      return( mulle_objc_walk_error);

   cmd = (*callback)( universe, universe, mulle_objc_walkpointer_is_universe, NULL, NULL, userinfo);
   if( mulle_objc_walkcommand_is_stopper( cmd))
      return( cmd);

   mulle_concurrent_hashmap_for( &universe->classtable, classid, cls)  // slow
   {
      cmd = mulle_objc_classpair_walk( _mulle_objc_class_get_classpair( cls),
                                       callback,
                                       userinfo);
      if( mulle_objc_walkcommand_is_stopper( cmd))
         return( cmd);
   }

   return( cmd);
}


mulle_objc_walkcommand_t
   _mulle_objc_universe_walk_classes( struct _mulle_objc_universe  *universe,
                                      int with_meta,
                                      mulle_objc_walkcallback_t callback,
                                      void *userinfo)
{
   mulle_objc_walkcommand_t        cmd;
   struct _mulle_objc_infraclass   *cls;
   struct _mulle_objc_metaclass    *meta;
   intptr_t                        classid;

   cmd = mulle_objc_walk_done;

   mulle_concurrent_hashmap_for( &universe->classtable, classid, cls)  // slow
   {
      cmd = (*callback)( universe, cls, mulle_objc_walkpointer_is_infraclass, NULL, NULL, userinfo);
      if( mulle_objc_walkcommand_is_stopper( cmd))
         return( cmd);

      if( with_meta)
      {
         meta = _mulle_objc_infraclass_get_metaclass( cls);
         if( meta)
         {
            cmd = (*callback)( universe, meta, mulle_objc_walkpointer_is_metaclass, NULL, NULL, userinfo);
            if( mulle_objc_walkcommand_is_stopper( cmd))
               return( cmd);
         }
      }
   }

   return( cmd);
}


static mulle_objc_walkcommand_t
   invalidate_classcaches_callback( struct _mulle_objc_universe *universe,
                                     void *p,
                                     enum mulle_objc_walkpointertype_t type,
                                     char *key,
                                     void *parent,
                                     void *userinfo)
{
   struct _mulle_objc_infraclass   *infra = p;
   struct _mulle_objc_infraclass   *kindofcls = userinfo;
   struct _mulle_objc_metaclass    *meta;

   MULLE_C_UNUSED( universe);
   MULLE_C_UNUSED( type);
   MULLE_C_UNUSED( key);
   MULLE_C_UNUSED( parent);

   if( mulle_objc_infraclass_is_subclass( infra, kindofcls))
   {
      mulle_objc_class_invalidate_caches( _mulle_objc_infraclass_as_class( infra), NULL);

      meta = _mulle_objc_infraclass_get_metaclass( infra);

      mulle_objc_class_invalidate_caches( _mulle_objc_metaclass_as_class( meta), NULL);
   }
   return( mulle_objc_walk_ok);
}


void  _mulle_objc_universe_invalidate_classcaches( struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_infraclass *kindofcls)
{
   _mulle_objc_universe_walk_classes( universe, 0, invalidate_classcaches_callback, kindofcls);
}




int   _mulle_objc_universe_cache_should_grow( struct _mulle_objc_universe *universe,
                                              struct _mulle_objc_cache *cache)
{
   int            should;
   unsigned int   fillrate;

   fillrate = _mulle_objc_universe_get_cache_fillrate( universe);
   should   = _mulle_objc_cache_should_grow( cache, fillrate);

   if( should && universe->debug.trace.method_cache)
      mulle_objc_universe_trace( universe,
                                 "cache %p should grow because its count %lu "
                                 "exceeds the fillrate (%u%%) of %lu",
                                 cache,
                                 (unsigned long) _mulle_objc_cache_get_count( cache),
                                 fillrate ? fillrate : 33,
                                 (unsigned long) _mulle_objc_cache_get_size( cache));

   return( should);
}


static void   _mulle_objc_universe_assert_tao_object_header( struct _mulle_objc_universe *universe, int define)
{
   //
   // Either or must be set, this can't really fail unless the includes
   // are messed up. If you hit this, you probably are calling an uninitialized
   // value. Where by luck, the pointers don't SIGSEV beforehand
   //
   assert( universe->compilebits & (64|128));

   if( define)
      assert( (universe->compilebits & 64) &&
              "ensure that your C compiler uses the same __MULLE_OBJC_TPS__ flags as mulle-objc");
   else
      assert( (universe->compilebits & 128) &&
               "ensure that your C compiler uses the same __MULLE_OBJC_TPS__ flags as mulle-objc");

   MULLE_C_UNUSED( universe);
}


void   _mulle_objc_object_assert_tao_object_header_no_tps( struct _mulle_objc_object *obj, int define)
{
   struct _mulle_objc_universe   *universe;
   struct _mulle_objc_class      *cls;

   assert( ! mulle_objc_object_get_taggedpointerindex( obj));

   // we can't easily get the universe here, because we would hit the header
   // and then we likely get into recursion. So do it manually
   cls = ((struct _mulle_objc_class **) obj)[ -1];

   // NULL can happen, if the header isn't filled up yet
   if( cls)
   {
      universe = _mulle_objc_class_get_universe( cls);
      _mulle_objc_universe_assert_tao_object_header( universe, define);
   }
}
