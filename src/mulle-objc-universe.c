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
#include "mulle-objc-universe.h"

#include "mulle-objc-universe-global.h"
#include "mulle-objc-class.h"
#include "mulle-objc-universe-class.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-object.h"
#include "mulle-objc-signature.h"
#include "mulle-objc-csvdump.h"
#include "mulle-objc-walktypes.h"
#include "include-private.h"
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <mulle-test-allocator/mulle-test-allocator.h> // only used once in project

# pragma mark - errors

MULLE_C_NO_RETURN static void
   _mulle_objc_perror_abort( char *s)
{
   perror( s);
   abort();
}


MULLE_C_NO_RETURN static void
   _mulle_objc_vprintf_abort( char *format, va_list args)
{
   vfprintf( stderr, format, args);
   fprintf( stderr, "\n");
   abort();
}


MULLE_C_NO_RETURN MULLE_C_NEVER_INLINE static void
   _mulle_objc_printf_abort( char *format, ...)
{
   extern void   mulle_objc_dotdump_to_tmp( void);

   va_list   args;

   va_start( args, format);

   vfprintf( stderr, format, args);
   fprintf( stderr, "\n");

   va_end( args);

#if DEBUG
   mulle_objc_dotdump_to_tmp();
#endif
   abort();
}


MULLE_C_NO_RETURN void
   _mulle_objc_universe_raise_generic_exception( struct _mulle_objc_universe *universe,
                                                 char *format, ...)
{
   va_list   args;

   va_start( args, format);

   if( ! universe || _mulle_objc_universe_is_uninitialized( universe))
      _mulle_objc_vprintf_abort( format, args);
   (*universe->failures.fail)( format, args);
   va_end( args);

   abort();  // just make sure if fail returns
}


MULLE_C_NO_RETURN static void
   _mulle_objc_class_not_found_abort( struct _mulle_objc_universe *universe,
                                      mulle_objc_classid_t missing_classid)
{
   _mulle_objc_printf_abort( "mulle_objc_universe %p fatal: unknown class %08x \"%s\"",
                                 universe,
                                 missing_classid,
                                 _mulle_objc_universe_string_for_classid( universe, missing_classid));
}


MULLE_C_NO_RETURN static void
   _mulle_objc_method_not_found_abort( struct _mulle_objc_class *cls,
                                       mulle_objc_methodid_t missing_method)
{
   struct _mulle_objc_universe    *universe;
   struct _mulle_objc_descriptor  *desc;
   char   *methodname;
   char   *name;

   name       = _mulle_objc_class_get_name( cls);
   methodname = NULL;

   // known methodids are not necessarily compiled in
   switch( missing_method)
   {
   case MULLE_OBJC_AUTORELEASE_METHODID : methodname = "autorelease"; break;
   case MULLE_OBJC_COPY_METHODID        : methodname = "copy"; break;
   case MULLE_OBJC_DEALLOC_METHODID     : methodname = "dealloc"; break;
   case MULLE_OBJC_FINALIZE_METHODID    : methodname = "finalize"; break;
   case MULLE_OBJC_INITIALIZE_METHODID  : methodname = "initialize"; break;
   case MULLE_OBJC_INIT_METHODID        : methodname = "init"; break;
   case MULLE_OBJC_LOAD_METHODID        : methodname = "load"; break;
   }

   universe = _mulle_objc_class_get_universe( cls);
   if( ! methodname)
   {
      desc = _mulle_objc_universe_lookup_descriptor( universe, missing_method);
      if( desc)
         methodname = desc->name;
      else
         methodname = _mulle_objc_universe_search_debughashname( universe, missing_method);
   }

   // keep often seen output more user friendly
   if( ! methodname)
      _mulle_objc_printf_abort( "mulle_objc_universe %p fatal: unknown %s "
                                "method %08x in class %08x \"%s\"",
                                universe,
                                _mulle_objc_class_is_metaclass( cls) ? "class" : "instance",
                                missing_method,
                               _mulle_objc_class_get_classid( cls),
                                name);
   _mulle_objc_printf_abort( "mulle_objc_universe %p fatal: unknown method "
                             "%08x \"%c%s\" in class %08x \"%s\"",
                             universe,
                             missing_method,
                             _mulle_objc_class_is_metaclass( cls) ? '+' : '-',
                             methodname,
                             _mulle_objc_class_get_classid( cls),
                             name);
}


MULLE_C_NO_RETURN
static void   _mulle_objc_super_not_found_abort( struct _mulle_objc_universe *universe,
                                                 mulle_objc_superid_t missing_superid)
{
   _mulle_objc_printf_abort( "mulle_objc_universe %p fatal: missing super %08x",
                                 universe,
                                 missing_superid,
                                 _mulle_objc_universe_string_for_superid( universe, missing_superid));
}


# pragma mark - setup

static void   nop( struct _mulle_objc_universe  *universe, mulle_objc_classid_t classid)
{
}


static void   _mulle_objc_universeconfig_dump( struct _mulle_objc_universeconfig  *config)
{
   fprintf( stderr, "%s", config->thread_local_rt ? "thread local" : "global");
   fprintf( stderr, ", %stagged pointers", config->no_tagged_pointer ? "no " : "");
   fprintf( stderr, ", %sfast method calls", config->no_fast_method_call ? "no " : "");
   if( config->forget_strings)
      fprintf( stderr, ", forget strings");

   if( config->ignore_ivarhash_mismatch)
      fprintf( stderr, ", ignore ivarhash mismatch");
   fprintf( stderr, ", min:-O%u max:-O%u", config->min_optlevel, config->max_optlevel);
   fprintf( stderr, ", cache fillrate: %u%%", config->cache_fillrate ? config->cache_fillrate : 25);
}


int   mulle_objc_getenv_yes_no_default( char *name, int default_value)
{
   char   *s;

   s = getenv( name);
   if( ! s)
      return( default_value);

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


int  mulle_objc_getenv_yes_no( char *name)
{
   return( mulle_objc_getenv_yes_no_default( name, 0));
}


static inline int  getenv_yes_no( char *name)
{
   return( mulle_objc_getenv_yes_no( name));
}


static inline int  getenv_yes_no_default( char *name, int default_value)
{
   return( mulle_objc_getenv_yes_no_default( name, default_value));
}


static void   _mulle_objc_universe_set_debug_defaults_from_environment( struct _mulle_objc_universe  *universe)
{
   universe->debug.warn.methodid_type          = getenv_yes_no( "MULLE_OBJC_WARN_METHODID_TYPE");
   universe->debug.warn.pedantic_methodid_type = getenv_yes_no( "MULLE_OBJC_WARN_PEDANTIC_METHODID_TYPE");
   universe->debug.warn.protocolclass          = getenv_yes_no( "MULLE_OBJC_WARN_PROTOCOLCLASS");
   universe->debug.warn.stuck_loadable         = getenv_yes_no_default( "MULLE_OBJC_WARN_STUCK_LOADABLE", 1);

#if ! DEBUG
   if( getenv_yes_no( "MULLE_OBJC_WARN_ENABLED"))
#endif
   {
      universe->debug.warn.methodid_type   = 1;
      universe->debug.warn.protocolclass    = 1;
      universe->debug.warn.stuck_loadable  = 1;
   }

   universe->debug.trace.category_add    = getenv_yes_no( "MULLE_OBJC_TRACE_CATEGORY_ADD");
   universe->debug.trace.class_add       = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_ADD");
   universe->debug.trace.class_free      = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_FREE");
   universe->debug.trace.class_cache     = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_CACHE");
   universe->debug.trace.dependency      = getenv_yes_no( "MULLE_OBJC_TRACE_DEPENDENCY");
   universe->debug.trace.dump_universe   = getenv_yes_no( "MULLE_OBJC_TRACE_DUMP_RUNTIME");
   universe->debug.trace.fastclass_add   = getenv_yes_no( "MULLE_OBJC_TRACE_FASTCLASS_ADD");
   universe->debug.trace.initialize      = getenv_yes_no( "MULLE_OBJC_TRACE_INITIALIZE");
   universe->debug.trace.load_call       = getenv_yes_no( "MULLE_OBJC_TRACE_LOAD_CALL");
   universe->debug.trace.loadinfo        = getenv_yes_no( "MULLE_OBJC_TRACE_LOADINFO");
   universe->debug.trace.method_cache    = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_CACHE");
   universe->debug.trace.method_call     = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_CALL");  // totally excessive!
   universe->debug.trace.method_searches = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_SEARCH");  // fairly excessive!
   universe->debug.trace.descriptor_add  = getenv_yes_no( "MULLE_OBJC_TRACE_DESCRIPTOR_ADD");
   universe->debug.trace.protocol_add    = getenv_yes_no( "MULLE_OBJC_TRACE_PROTOCOL_ADD");
   universe->debug.trace.state_bit       = getenv_yes_no( "MULLE_OBJC_TRACE_STATE_BIT");
   universe->debug.trace.string_add      = getenv_yes_no( "MULLE_OBJC_TRACE_STRING_ADD");
   universe->debug.trace.super_add       = getenv_yes_no( "MULLE_OBJC_TRACE_SUPER_ADD");
   universe->debug.trace.tagged_pointer  = getenv_yes_no( "MULLE_OBJC_TRACE_TAGGED_POINTER");

   if( getenv_yes_no( "MULLE_OBJC_TRACE_CACHE"))
   {
      universe->debug.trace.method_cache  = 1;
      universe->debug.trace.class_cache   = 1;
   }

   if( getenv_yes_no( "MULLE_OBJC_TRACE_LOAD"))
   {
      universe->debug.trace.load_call  = 1;
      universe->debug.trace.loadinfo   = 1;
      universe->debug.trace.dependency = 1;
      universe->debug.trace.initialize = 1;
      universe->debug.trace.state_bit  = 1;
   }


   // don't trace method search and calls, per default... too expensive
   // don't trace caches either, usually that's too boring
   // also don't dump, per default
   if( getenv_yes_no( "MULLE_OBJC_TRACE_ENABLED"))
   {
      universe->debug.trace.category_add         = 1;
      universe->debug.trace.class_add            = 1;
      universe->debug.trace.class_free           = 1;
      universe->debug.trace.dependency           = 1;
      universe->debug.trace.fastclass_add        = 1;
      universe->debug.trace.initialize           = 1;
      universe->debug.trace.load_call            = 1;
      universe->debug.trace.loadinfo             = 1;
      universe->debug.trace.protocol_add         = 1;
      universe->debug.trace.state_bit            = 1;
      universe->debug.trace.tagged_pointer       = 1;
   }

   universe->debug.print.print_origin    = getenv_yes_no_default( "MULLE_OBJC_PRINT_ORIGIN", 1);
   universe->debug.print.universe_config = getenv_yes_no( "MULLE_OBJC_PRINT_UNIVERSE_CONFIG");

   if( universe->debug.print.universe_config)
   {
      fprintf( stderr, "mulle-objc-universe %p: v%u.%u.%u (load-version: %u) (",
         universe,
         MULLE_OBJC_RUNTIME_VERSION_MAJOR,
         MULLE_OBJC_RUNTIME_VERSION_MINOR,
         MULLE_OBJC_RUNTIME_VERSION_PATCH,
         MULLE_OBJC_RUNTIME_LOAD_VERSION);
      _mulle_objc_universeconfig_dump( &universe->config);
      fprintf( stderr, ")\n");
   }

}


MULLE_C_NO_RETURN
void   mulle_objc_allocator_fail( void *block, size_t size)
{
   mulle_objc_raise_errno_exception();
}


static int   unfailing_abafree( void  *aba,
                                void (*p_free)( void *),
                                void *pointer)
{
   if( _mulle_aba_free( aba, p_free, pointer))
      mulle_objc_raise_errno_exception();
   return( 0);
}


//
// minimal stuff the universe needs to be setup with immediately
// *** dont allocate anything***
//
static void   _mulle_objc_universe_set_defaults( struct _mulle_objc_universe  *universe,
                                                 struct mulle_allocator *allocator)
{
   void    mulle_objc_vprintf_abort( char *format, va_list args);

   assert( universe);
   assert( MULLE_THREAD_VERSION    >= ((2 << 20) | (2 << 8) | 0));
   assert( MULLE_ALLOCATOR_VERSION >= ((1 << 20) | (5 << 8) | 0));
   assert( MULLE_ABA_VERSION       >= ((1 << 20) | (1 << 8) | 1));

   // check this also, easily fixable with padding
   if( sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1))
      abort();

   if( ! allocator)
      allocator = &mulle_default_allocator;

   strncpy( universe->compilation, __DATE__ " "__TIME__ " " __FILE__, 128);

   assert( allocator->calloc);
   assert( allocator->free);
   assert( allocator->realloc);

   assert( _mulle_objc_universe_is_transitioning( universe)); // != 0!
   _mulle_atomic_pointer_nonatomic_write( &universe->cachepivot.entries, universe->empty_cache.entries);

   universe->memory.allocator         = *allocator;
   mulle_allocator_set_fail( &universe->memory.allocator, mulle_objc_allocator_fail);

   universe->memory.allocator.aba     = &universe->garbage.aba;
   universe->memory.allocator.abafree = unfailing_abafree;

   universe->failures.fail             = _mulle_objc_vprintf_abort;
   universe->failures.inconsistency    = _mulle_objc_vprintf_abort;
   universe->failures.class_not_found  = _mulle_objc_class_not_found_abort;
   universe->failures.method_not_found = _mulle_objc_method_not_found_abort;
   universe->failures.super_not_found  = _mulle_objc_super_not_found_abort;

   universe->exceptionvectors.throw     = _mulle_objc_universe_throw;
   universe->exceptionvectors.try_enter = _mulle_objc_universe_try_enter;
   universe->exceptionvectors.try_exit  = _mulle_objc_universe_try_exit;
   universe->exceptionvectors.extract   = _mulle_objc_universe_extract_exception;
   universe->exceptionvectors.match     = _mulle_objc_universe_match_exception;

   universe->classdefaults.inheritance      = MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES;
   universe->classdefaults.class_is_missing = nop;

   _mulle_concurrent_pointerarray_init( &universe->staticstrings, 0, &universe->memory.allocator);
   _mulle_concurrent_pointerarray_init( &universe->hashnames, 0, &universe->memory.allocator);
   _mulle_concurrent_pointerarray_init( &universe->gifts, 0, &universe->memory.allocator);

   universe->config.max_optlevel = 0x7;
#ifdef __MULLE_OBJC_TRT__
   universe->config.thread_local_rt = 1;
#endif
#ifdef __MULLE_OBJC_NO_TPS__
   universe->config.no_tagged_pointer = 1;
#endif
#ifdef __MULLE_OBJC_NO_FMC__
   universe->config.no_fast_method_call = 1;
#endif
   _mulle_objc_universe_set_debug_defaults_from_environment( universe);
}


void   __mulle_objc_universe_setup( struct _mulle_objc_universe *universe,
                                    struct mulle_allocator *allocator)
{
   _mulle_objc_universe_set_defaults( universe, allocator);

   allocator = &universe->memory.allocator;
   _mulle_concurrent_hashmap_init( &universe->categorytable, 128, allocator);
   _mulle_concurrent_hashmap_init( &universe->classtable, 128, allocator);
   _mulle_concurrent_hashmap_init( &universe->descriptortable, 2048, allocator);
   _mulle_concurrent_hashmap_init( &universe->protocoltable, 64, allocator);
   _mulle_concurrent_hashmap_init( &universe->supertable, 256, allocator);

   mulle_thread_mutex_init( &universe->waitqueues.lock);
   _mulle_concurrent_hashmap_init( &universe->waitqueues.classestoload, 64, allocator);
   _mulle_concurrent_hashmap_init( &universe->waitqueues.categoriestoload, 32, allocator);

   mulle_objc_unfailingget_or_create_threadkey();

   mulle_thread_mutex_init( &universe->lock);
   universe->thread = mulle_thread_self();

   mulle_objc_set_thread_universe( universe);
   _mulle_objc_universe_register_current_thread_if_needed( universe);
}


struct _mulle_objc_universe  *mulle_objc_alloc_universe( void)
{
   struct _mulle_objc_universe  *universe;

   universe = calloc( 1, sizeof( struct _mulle_objc_universe));
   if( ! universe)
      _mulle_objc_perror_abort( "mulle_objc_create_universe");

   return( universe);
}


struct _mulle_objc_universe  *__mulle_objc_get_universe( void)
{
   struct _mulle_objc_universe  *universe;

#if __MULLE_OBJC_TRT__
   if( mulle_objc_thread_key)
   {
      universe = __mulle_objc_get_thread_universe();
      if( ! universe)
         universe = mulle_objc_alloc_universe();
   }
#else
   universe = __mulle_objc_get_global_universe();
#endif
   return( universe);
}

// this should only be used in "mulle_objc_load.c"
struct _mulle_objc_universe  *mulle_objc_get_or_create_universe( void)
{
   struct _mulle_objc_universe  *universe;

   universe = __get_or_create_mulle_objc_universe();  // the external function
   if( ! universe)
   {
      fprintf( stderr, "__get_or_create_mulle_objc_universe returned NULL\n");
      abort();
   }
   return( universe);
}


void  _mulle_objc_universe_assert_version( struct _mulle_objc_universe *universe,
                                           struct mulle_objc_loadversion *version)
{
   if( (mulle_objc_version_get_major( version->universe) !=
        mulle_objc_version_get_major( _mulle_objc_universe_get_version( universe))) ||
       //
       // during 0 versions, any minor jump is incompatible
       //
       ( ! mulle_objc_version_get_major( version->universe) &&
         (mulle_objc_version_get_minor( version->universe) !=
          mulle_objc_version_get_minor( _mulle_objc_universe_get_version( universe)))))

   {
      _mulle_objc_universe_raise_generic_exception( universe,
         "mulle_objc_universe %p fatal: can't load code with incompatible version %u.%u.%u. This mulle-objc universe is version %u.%u.%u (%s)\n",
            universe,
            mulle_objc_version_get_major( version->universe),
            mulle_objc_version_get_minor( version->universe),
            mulle_objc_version_get_patch( version->universe),
            mulle_objc_version_get_major( _mulle_objc_universe_get_version( universe)),
            mulle_objc_version_get_minor( _mulle_objc_universe_get_version( universe)),
            mulle_objc_version_get_patch( _mulle_objc_universe_get_version( universe)),
                                                   _mulle_objc_universe_get_path( universe) ? _mulle_objc_universe_get_path( universe) : "???");
   }
}


void  _mulle_objc_universe_defaultexitus()
{
   // no autoreleasepools here

   mulle_objc_release_universe();

   if( mulle_objc_getenv_yes_no( "MULLE_OBJC_TEST_ALLOCATOR"))
      mulle_test_allocator_reset();
}


//
// this is done for "global" universe configurations
// where threads possibly try to create and release
// universes willy, nilly (usualy tests)
//
void   _mulle_objc_universe_crunch( struct _mulle_objc_universe  *universe,
                                    void (*crunch)( struct _mulle_objc_universe  *universe))
{
   void  *actual;
   int   trace;

   trace = universe->debug.trace.universe;
   if( trace)
      fprintf( stderr, "mulle_objc_universe %p trace: [%p] trying to lock the universe down for crunch\n",
                  universe, (void *) mulle_thread_self());

   // ensure only one thread is going at it
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version, (void *) mulle_objc_universe_is_deinitializing, (void *) MULLE_OBJC_RUNTIME_VERSION);
      if( actual == (void *) mulle_objc_universe_is_uninitialized)
      {
         if( universe->debug.trace.universe)
            fprintf( stderr, "mulle_objc_universe %p trace: [%p] someone else crunched the universe already\n",
                  universe, (void *) mulle_thread_self());
         return;  // someone else did it
      }
      if( actual == (void *) MULLE_OBJC_RUNTIME_VERSION)
         break;
      mulle_thread_yield();
   }

   // START OF LOCKED

   if( trace)
      fprintf( stderr, "mulle_objc_universe %p trace: [%p] crunch of the universe in progress\n",
              universe, (void *) mulle_thread_self());

   (*crunch)( universe);

   if( trace)
      fprintf( stderr, "mulle_objc_universe %p trace: [%p] crunch of the universe done\n",
              universe, (void *) mulle_thread_self());

   mulle_atomic_memory_barrier(); // shared/global memory

   // END OF LOCKED

   // now set to unintialized
   // __mulle_atomic_pointer_cas is weak and allowed to fail
   // don't want to bloat the CAS api though
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version, (void *) mulle_objc_universe_is_uninitialized, (void *) mulle_objc_universe_is_deinitializing);
      if( actual == (void *) mulle_objc_universe_is_deinitializing)
      {
         if( trace)
            fprintf( stderr, "mulle_objc_universe %p trace: [%p] unlocked the universe\n",
                  universe, (void *) mulle_thread_self());
         return;  // someone else did it
      }

      if( trace)
         fprintf( stderr, "mulle_objc_universe %p trace: [%p] retrying to unlock the universe\n",
               universe, (void *) mulle_thread_self());
   }
}


void   _mulle_objc_universe_defaultbang( struct _mulle_objc_universe  *universe,
                                         void (*exitus)( void),
                                         void *userinfo)
{
   struct mulle_allocator   *allocator;
   int                      is_test;
   int                      is_pedantic;

   allocator = NULL;
   is_test   = getenv_yes_no( "MULLE_OBJC_TEST_ALLOCATOR");
   if( is_test)
   {
      // call this because we are probably also in +load here
      mulle_test_allocator_initialize();
      allocator = &mulle_test_allocator;
#if DEBUG
      fprintf( stderr, "mulle_objc_universe uses \"mulle_test_allocator\" to detect leaks.\n");
#endif
   }
   __mulle_objc_universe_setup( universe, allocator);

   is_pedantic = getenv_yes_no( "MULLE_OBJC_PEDANTIC_EXIT");
   if( is_test || is_pedantic)
   {
      static int   did_it;

      // these atexit functions could pile up though
      if( ! did_it)
      {
         did_it = 1;
         if( atexit( exitus))
            perror( "atexit:");
      }
   }
}


//
// this is done for "global" universe configurations
// where threads possibly try to create and release
// universes willy, nilly (usualy tests)
//
static void   __mulle_objc_universe_bang( struct _mulle_objc_universe  *universe,
                                          void (*setup)( struct _mulle_objc_universe  *universe,
                                                  void (*exitus)( void),
                                                  void *userinfo),
                                          void (*exitus)( void),
                                          void *userinfo)
{
   void   *actual;
   int    trace;

   trace = universe->debug.trace.universe;
   if( trace)
      fprintf( stderr, "mulle_objc_universe %p trace: [%p] trying to lock the universe down for bang\n",
                  universe, (void *) mulle_thread_self());


   // ensure only one thread is going at it
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version, (void *)mulle_objc_universe_is_initializing, (void *) mulle_objc_universe_is_uninitialized);
      if( actual == (void *) MULLE_OBJC_RUNTIME_VERSION)
      {
         if( trace)
            fprintf( stderr, "mulle_objc_universe %p trace: [%p] someone else did the universe bang already\n",
                  universe, (void *) mulle_thread_self());
         return;  // someone else did it
      }
      if( actual == (void *) mulle_objc_universe_is_uninitialized)
         break;
      mulle_thread_yield();
   }

   // BEGIN OF LOCKED
   if( trace)
      fprintf( stderr, "mulle_objc_universe %p trace: [%p] bang of the universe in progress\n",
              universe, (void *) mulle_thread_self());

   (*setup)( universe, exitus, userinfo);

   if( trace)
      fprintf( stderr, "mulle_objc_universe %p trace: [%p] bang of the universe done\n",
              universe, (void *) mulle_thread_self());

   mulle_atomic_memory_barrier();  // shared/global memory
   // END OF LOCKED

   // __mulle_atomic_pointer_cas is weak and allowed to fail
   // don't want to bload the CAS api though
   for(;;)
   {
      actual = __mulle_atomic_pointer_cas( &universe->version, (void *) MULLE_OBJC_RUNTIME_VERSION, (void *) mulle_objc_universe_is_initializing);
      if( actual == (void *) mulle_objc_universe_is_initializing)
      {
         if( trace)
            fprintf( stderr, "mulle_objc_universe %p trace: [%p] unlocked the universe\n",
                  universe, (void *) mulle_thread_self());
         return;  // someone else did it
      }

      if( trace)
         fprintf( stderr, "mulle_objc_universe %p trace: [%p] retrying to unlock the universe\n",
               universe, (void *) mulle_thread_self());
   }
}


void   _mulle_objc_universe_bang( struct _mulle_objc_universe  *universe,
                                  void (*bang)( struct _mulle_objc_universe *universe,
                                                void (*exitus)( void),
                                                void *userinfo),
                                  void (*exitus)( void),
                                  void *userinfo)
{
   universe->debug.trace.universe = getenv_yes_no( "MULLE_OBJC_TRACE_UNIVERSE");

   if( ! bang)
      bang = _mulle_objc_universe_defaultbang;
   if( ! exitus)
      exitus = _mulle_objc_universe_defaultexitus;

   __mulle_objc_universe_bang( universe, bang, exitus, userinfo);
}


# pragma mark - TPS and Loadbits

void  _mulle_objc_universe_set_loadbit( struct _mulle_objc_universe *universe,
                                       uintptr_t bit)
{
   uintptr_t   oldbits;
   uintptr_t   newbits;

   do
   {
      oldbits = _mulle_objc_universe_get_loadbits( universe);
      newbits = oldbits | bit;
      if( oldbits != newbits)
         return;
   }
   while( ! _mulle_atomic_pointer_weakcas( &universe->loadbits, (void *) newbits, (void *) oldbits));
}


//
// it's assumed that there are no competing classes for the spot
//
int  _mulle_objc_universe_set_taggedpointerclass_at_index( struct _mulle_objc_universe  *universe,
                                                          struct _mulle_objc_infraclass *infra,
                                                          unsigned int index)
{
   if( ! index || index > mulle_objc_get_taggedpointer_mask())
      return( -1);

   assert( ! universe->taggedpointers.pointerclass[ index]);
   if( universe->debug.trace.tagged_pointer)
      fprintf( stderr, "mulle_objc_universe %p trace: set tagged pointers with "
                       "index %d to isa %p (class %08x \"%s\" )\n",
                       universe, index, infra,
                      _mulle_objc_infraclass_get_classid( infra),
                      _mulle_objc_infraclass_get_name( infra));

   universe->taggedpointers.pointerclass[ index] = _mulle_objc_infraclass_as_class( infra);

   _mulle_objc_universe_set_loadbit( universe, MULLE_OBJC_UNIVERSE_HAVE_TPS_CLASSES);

   return( 0);
}


# pragma mark - thread local

static void   mulle_objc_thread_universe_destructor( struct _mulle_objc_threadconfig *config)
{
   _mulle_allocator_free( &config->universe->memory.allocator, config);
}


mulle_thread_tss_t   mulle_objc_unfailingget_or_create_threadkey( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
      mulle_thread_tss_t   mulle_objc_thread_key;

   if( ! mulle_objc_thread_key_is_intitialized())
   {
      if( mulle_thread_tss_create( (void *) mulle_objc_thread_universe_destructor,
                                   &mulle_objc_thread_key))
      {
         _mulle_objc_perror_abort( "mulle_objc_unfailingget_or_create_threadkey");
      }
   }
   return( mulle_objc_thread_key);
}


static void   mulle_objc_unset_thread_universe( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
      mulle_thread_tss_t   mulle_objc_thread_key;
   struct _mulle_objc_threadconfig *config;

   if( ! mulle_objc_thread_key_is_intitialized())
      return;

   config = mulle_thread_tss_get( mulle_objc_thread_key);
   assert( config);
   if( config)
   {
      mulle_thread_tss_set( mulle_objc_thread_key, NULL);
      mulle_objc_thread_universe_destructor( config);
   }
}



void   mulle_objc_delete_threadkey( void)
{
   MULLE_OBJC_RUNTIME_EXTERN_GLOBAL
      mulle_thread_tss_t   mulle_objc_thread_key;

   if( mulle_objc_thread_key_is_intitialized())
      return;

   assert( ! mulle_thread_tss_get( mulle_objc_thread_key));

   mulle_thread_tss_free( mulle_objc_thread_key);
   mulle_objc_thread_key = -1;
}


# pragma mark - dealloc

static void   _mulle_objc_universe_free_friend( struct _mulle_objc_universe *universe,
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
   struct mulle_concurrent_hashmapenumerator  rover;
   struct mulle_concurrent_pointerarray       *list;

   rover = mulle_concurrent_hashmap_enumerate( map);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &list))
   {
      mulle_concurrent_pointerarray_map( list, f, universe);
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
}


static int  compare_depth_deeper_first( struct _mulle_objc_class **a, struct _mulle_objc_class **b)
{
   int  depth_a;
   int  depth_b;

   depth_a = (int) _mulle_objc_class_count_depth( *a);
   depth_b = (int) _mulle_objc_class_count_depth( *b);
   return( depth_b - depth_a);
}


static void   _mulle_objc_universe_sort_classes_by_depth( struct _mulle_objc_class **array,
                                                         unsigned int n_classes)
{
   qsort( array, n_classes, sizeof( struct _mulle_objc_class *), (int (*)()) compare_depth_deeper_first);
}


static struct _mulle_objc_class **
   _mulle_objc_universe_all_classes( struct _mulle_objc_universe *universe,
                                     unsigned int *n_classes,
                                    struct mulle_allocator *allocator)
{
   struct _mulle_objc_class                    **p_cls;
   struct _mulle_objc_class                    **array;
   struct mulle_concurrent_hashmapenumerator   rover;

   *n_classes = mulle_concurrent_hashmap_count( &universe->classtable);
   array      = mulle_allocator_calloc( allocator, *n_classes, sizeof( struct _mulle_objc_classpair *));

   p_cls = array;
   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) p_cls))
      p_cls++;
   mulle_concurrent_hashmapenumerator_done( &rover);

   return( array);
}


static void   _mulle_objc_universe_free_classpairs( struct _mulle_objc_universe *universe)
{
   unsigned int               n_classes;
   struct _mulle_objc_class   **array;
   struct _mulle_objc_class   **p;
   struct _mulle_objc_class   **sentinel;
   struct mulle_allocator     *allocator;

   allocator = _mulle_objc_universe_get_allocator( universe);
   array     = _mulle_objc_universe_all_classes( universe, &n_classes, allocator);
   _mulle_objc_universe_sort_classes_by_depth( array, n_classes);

   p        = array;
   sentinel = &p[ n_classes];
   while( p < sentinel)
   {
      if( universe->debug.trace.class_free)
         fprintf( stderr, "mulle_objc_universe %p trace: destroying class pair %p \"%s\"\n", universe, _mulle_objc_class_get_classpair( *p), _mulle_objc_class_get_name( *p));

      _mulle_objc_classpair_free( _mulle_objc_class_get_classpair( *p),
                                  allocator);
      ++p;
   }

   mulle_allocator_free( allocator, array);
}


static void   free_gift( void *p, struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, p);
}


enum mulle_objc_universe_status  _mulle_objc_universe_check_waitqueues( struct _mulle_objc_universe *universe)
{
   int   rval;

   rval = mulle_objc_universe_is_ok;
   /*
    * free various stuff
    */
   if( mulle_concurrent_hashmap_count( &universe->waitqueues.classestoload))
   {
      rval = mulle_objc_universe_is_incomplete;
      fprintf( stderr, "mulle_objc_universe %p warning: the following classes failed to load:\n",
                  universe);
      if( _mulle_objc_universe_waitqueues_trylock( universe))
      {
         fprintf( stderr, "mulle_objc_universe %p error: the waitqueues are still locked!\n", universe);
         return( mulle_objc_universe_is_locked);
      }

      pointerarray_in_hashmap_map( universe, &universe->waitqueues.classestoload, (void (*)()) mulle_objc_loadclass_print_unfulfilled_dependency);
      _mulle_objc_universe_waitqueues_unlock( universe);
   }

   if( mulle_concurrent_hashmap_count( &universe->waitqueues.categoriestoload))
   {
      rval = mulle_objc_universe_is_incomplete;
      fprintf( stderr, "mulle_objc_universe %p warning: the following categories failed to load:\n", universe);
      if( _mulle_objc_universe_waitqueues_trylock( universe))
      {
         fprintf( stderr, "mulle_objc_universe %p error: the waitqueues are still locked!\n",
                     universe);
         return( mulle_objc_universe_is_locked);
      }

      pointerarray_in_hashmap_map( universe, &universe->waitqueues.categoriestoload, (void (*)()) mulle_objc_loadcategory_print_unfulfilled_dependency);
      _mulle_objc_universe_waitqueues_unlock( universe);
   }
   return( rval);
}


enum mulle_objc_universe_status   _mulle_objc_check_universe( uint32_t version)
{
   struct _mulle_objc_universe   *universe;
   uint32_t                     universe_version;

   universe = mulle_objc_get_universe();
   if( ! universe)
      return( mulle_objc_universe_is_missing);

   universe_version = _mulle_objc_universe_get_version( universe);

   if( mulle_objc_version_get_major( version) != mulle_objc_version_get_major( universe_version))
      return( mulle_objc_universe_is_wrong_version);

   // during 0 development, a minor change is major
   if( ! mulle_objc_version_get_major( version) && (mulle_objc_version_get_minor( version) != mulle_objc_version_get_minor( universe_version)))
      return( mulle_objc_universe_is_wrong_version);

   return( _mulle_objc_universe_check_waitqueues( universe));
}


static void   _mulle_objc_universe_free_classgraph( struct _mulle_objc_universe *universe)
{
   if( universe->debug.warn.stuck_loadable)
      _mulle_objc_universe_check_waitqueues( universe);

   /* free classes */
   _mulle_objc_universe_free_classpairs( universe);

   _mulle_concurrent_hashmap_done( &universe->waitqueues.categoriestoload);
   _mulle_concurrent_hashmap_done( &universe->waitqueues.classestoload);
   _mulle_concurrent_hashmap_done( &universe->supertable);
   _mulle_concurrent_hashmap_done( &universe->protocoltable);
   _mulle_concurrent_hashmap_done( &universe->descriptortable);
   _mulle_concurrent_hashmap_done( &universe->classtable);
   _mulle_concurrent_hashmap_done( &universe->categorytable);

   _mulle_concurrent_pointerarray_done( &universe->staticstrings);
   _mulle_concurrent_pointerarray_done( &universe->hashnames);

   /* free gifts */
   mulle_concurrent_pointerarray_map( &universe->gifts, (void (*)()) free_gift, &universe->memory.allocator);
   _mulle_concurrent_pointerarray_done( &universe->gifts);
}


void   _mulle_objc_universe_dealloc( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_cache               *cache;
   struct _mulle_objc_garbagecollection   *gc;
   struct mulle_allocator                 *allocator;

   if( universe->debug.trace.universe)
      fprintf( stderr, "mulle_objc_universe %p: deallocs\n", universe);

   if( universe->thread && universe->thread != mulle_thread_self())
      _mulle_objc_universe_raise_inconsistency_exception( universe,
         "universe must be deallocated by the same thread that created it (sorry)");

   if( universe->callbacks.will_dealloc)
      (*universe->callbacks.will_dealloc)( universe);

   mulle_objc_set_thread_universe( NULL);

   // do it like this, because we are not initialized anymore
   gc = _mulle_objc_universe_get_garbagecollection( universe);
   if( ! _mulle_aba_is_current_thread_registered( &gc->aba))
   {
      if( _mulle_aba_register_current_thread( &gc->aba))
         _mulle_objc_perror_abort( "_mulle_aba_register_current_thread");
   }

   _mulle_objc_universe_free_friend( universe, &universe->userinfo);
   _mulle_objc_universe_free_friend( universe, &universe->foundation.universefriend);

   _mulle_objc_universe_free_classgraph( universe);

   cache = _mulle_objc_cachepivot_atomicget_cache( &universe->cachepivot);
   if( cache != &universe->empty_cache)
   {
      allocator = _mulle_objc_universe_get_allocator( universe);
      _mulle_objc_cache_free( cache, allocator);
   }

   if( _mulle_aba_unregister_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_unregister_current_thread");
   _mulle_aba_done( &_mulle_objc_universe_get_garbagecollection( universe)->aba);

   mulle_thread_mutex_done( &universe->lock);

   if( _mulle_objc_is_global_universe( universe))
   {
      // idle waste of cpu, but useful if we get reclaimed as global
      // but don't clobber version
      memset( &universe->cachepivot, 0, sizeof( universe->cachepivot));
      memset( &universe->path, 0, sizeof( *universe) - offsetof( struct _mulle_objc_universe, path));

      return;
   }

   free( universe);
}


# pragma mark - universe release convenience

void  mulle_objc_release_universe( void)
{
   struct _mulle_objc_universe *universe;

   universe = __mulle_objc_get_universe();
   if( universe && _mulle_objc_universe_is_initialized( universe))
      _mulle_objc_universe_release( universe);
}


struct _mulle_objc_garbagecollection  *
   _mulle_objc_universe_get_garbagecollection( struct _mulle_objc_universe *universe)
{
   struct mulle_allocator   *allocator;

   if( ! _mulle_aba_is_setup( &universe->garbage.aba))
   {
      allocator = _mulle_objc_universe_get_allocator( universe);
      if( _mulle_aba_init( &universe->garbage.aba, allocator))
         _mulle_objc_universe_raise_errno_exception( universe);
   }
   return( &universe->garbage);
}


void   _mulle_objc_universe_register_current_thread( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   assert( _mulle_objc_universe_is_initialized( universe));

   gc = _mulle_objc_universe_get_garbagecollection( universe);
   if( _mulle_aba_register_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_register_current_thread");
}


int    _mulle_objc_universe_is_current_thread_registered( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   assert( _mulle_objc_universe_is_initialized( universe));

   gc = _mulle_objc_universe_get_garbagecollection( universe);
   return( _mulle_aba_is_current_thread_registered( &gc->aba));
}


void   _mulle_objc_universe_register_current_thread_if_needed( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

// don't assert here as its called by the "bang"s
//   assert( _mulle_objc_universe_is_initialized( universe));

   gc = _mulle_objc_universe_get_garbagecollection( universe);
   if( _mulle_aba_is_current_thread_registered( &gc->aba))
      return;

   if( _mulle_aba_register_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_register_current_thread");
}


void   _mulle_objc_universe_unregister_current_thread( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   assert( _mulle_objc_universe_is_initialized( universe));

   gc = _mulle_objc_universe_get_garbagecollection( universe);
   if( _mulle_aba_unregister_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_unregister_current_thread");
}


void   _mulle_objc_universe_checkin_current_thread( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_garbagecollection   *gc;

   assert( _mulle_objc_universe_is_initialized( universe));

   gc = _mulle_objc_universe_get_garbagecollection( universe);
   if( _mulle_aba_checkin_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_checkin_current_thread");
}

//
// admittedly its fairly tricky, to set mulle_objc_globals before any of the
// class loads hit...
//
# pragma mark - "exceptions"


long   __mulle_objc_personality_v0 = 1848;  // no idea what this is used for

MULLE_C_NO_RETURN void
   _mulle_objc_universe_raise_errno_exception( struct _mulle_objc_universe *universe)
{
   _mulle_objc_universe_raise_generic_exception( universe, "errno: %s (%d)", strerror( errno), errno);
}


MULLE_C_NO_RETURN void
   _mulle_objc_universe_raise_inconsistency_exception( struct _mulle_objc_universe *universe,
                                                       char *format, ...)
{
   va_list   args;

   va_start( args, format);

   //
   // inconsistency ? even universe might not be setup properly. This is the only
   // exception doing this check
   //
   if( ! universe || ! universe->failures.inconsistency)
   {
      vfprintf( stderr, format, args);
      abort();;
   }
   (*universe->failures.inconsistency)( format, args);
   va_end( args);

   abort();  // just make sure if fail returns
}


MULLE_C_NO_RETURN void
   _mulle_objc_universe_raise_super_not_found_exception( struct _mulle_objc_universe *universe,
                                                         mulle_objc_superid_t superid)
{
   (*universe->failures.super_not_found)( universe, superid);
   abort();  // just make sure if fail returns
}


MULLE_C_NO_RETURN void
   _mulle_objc_universe_raise_class_not_found_exception( struct _mulle_objc_universe *universe,
                                                         mulle_objc_classid_t classid)
{
   (*universe->failures.class_not_found)( universe, classid);
   abort();  // just make sure if fail returns
}


MULLE_C_NO_RETURN void
   _mulle_objc_class_raise_method_not_found_exception( struct _mulle_objc_class *cls,
                                                       mulle_objc_methodid_t methodid)
{
   (*cls->universe->failures.method_not_found)( cls, methodid);
   abort();  // just make sure if fail returns
}


/* from clang:

  A catch buffer is a setjmp buffer plus:
    - a pointer to the exception that was caught
    - a pointer to the previous exception data buffer
    - two pointers of reserved storage
  Therefore catch buffers form a stack, with a pointer to the top
  of the stack kept in thread-local storage.

  objc_exception_throw pops the top of the EH stack, writes the
    thrown exception into the appropriate field, and longjmps
    to the setjmp buffer.  It crashes the process (with a printf
    and an abort()) if there are no catch buffers on the stack.
  objc_exception_extract just reads the exception pointer out of the
    catch buffer.
*/

struct _mulle_objc_exceptionstackentry
{
   void                                     *exception;
   struct _mulle_objc_exceptionstackentry   *previous;
   void                                     *unused[ 2];
   jmp_buf                                  buf;
};


void   _mulle_objc_universe_throw( struct _mulle_objc_universe *universe, void *exception)
{
   struct _mulle_objc_exceptionstackentry  *entry;
   struct _mulle_objc_threadconfig         *config;

   config = mulle_objc_get_threadconfig();
   entry  = config->exception_stack;
   if( ! entry)
   {
      if( universe->failures.uncaughtexception)
         (*universe->failures.uncaughtexception)( exception);

      fprintf( stderr, "uncaught exception %p", exception);
      abort();
   }

   entry->exception        = exception;
   config->exception_stack = entry->previous;

   // from Apple objc_universe.mm
#if _WIN32
    longjmp( entry->buf, 1);
#else
    _longjmp( entry->buf, 1);
#endif
}


//
//  objc_exception_try_enter pushes a catch buffer onto the EH stack.
//
void   _mulle_objc_universe_try_enter( struct _mulle_objc_universe *universe,
                                      void *data)
{
   struct _mulle_objc_exceptionstackentry  *entry = data;
   struct _mulle_objc_exceptionstackentry  *top;
   struct _mulle_objc_threadconfig         *config;

   config                  = mulle_objc_get_threadconfig();
   top                     = config->exception_stack;
   entry->exception        = NULL;
   entry->previous         = top;
   config->exception_stack = entry;
}


//
//  objc_exception_try_exit pops the given catch buffer, which is
//    required to be the top of the EH stack.
//
void   _mulle_objc_universe_try_exit( struct _mulle_objc_universe *universe,
                                     void *data)
{
   struct _mulle_objc_exceptionstackentry *entry = data;
   struct _mulle_objc_threadconfig        *config;

   config                  = mulle_objc_get_threadconfig();
   config->exception_stack = entry->previous;
}


void   *_mulle_objc_universe_extract_exception( struct _mulle_objc_universe *universe,
                                               void *data)
{
   struct _mulle_objc_exceptionstackentry *entry = data;

   return( entry->exception);
}


int  _mulle_objc_universe_match_exception( struct _mulle_objc_universe *universe,
                                          mulle_objc_classid_t classid,
                                          void *exception)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_class        *exceptionCls;

   assert( classid != MULLE_OBJC_NO_CLASSID && classid != MULLE_OBJC_INVALID_CLASSID);
   assert( exception);

   infra        = _mulle_objc_universe_unfailingfastlookup_infraclass( universe, classid);
   exceptionCls = _mulle_objc_object_get_isa( exception);

   do
   {
      if( _mulle_objc_infraclass_as_class( infra) == exceptionCls)
         return( 1);
      exceptionCls = _mulle_objc_class_get_superclass( exceptionCls);
   }
   while( exceptionCls);

   return( 0);
}


void   mulle_objc_raise_generic_exception( char *format, ...)
{
   va_list   args;

   va_start( args, format);
   _mulle_objc_universe_raise_generic_exception( mulle_objc_get_universe(), format, args);
   va_end( args);
}


void   mulle_objc_raise_errno_exception( void)
{
   _mulle_objc_universe_raise_errno_exception( mulle_objc_get_universe());
}


void   mulle_objc_raise_inconsistency_exception( char *format, ...)
{
   va_list   args;

   va_start( args, format);
   _mulle_objc_universe_raise_inconsistency_exception( mulle_objc_get_universe(), format, args);
   va_end( args);
}


void   mulle_objc_raise_taggedpointer_exception( void *obj)
{
   _mulle_objc_universe_raise_inconsistency_exception( mulle_objc_get_universe(),
                                                       "%p is a tagged pointer", obj);
}


# pragma mark - "classes"

//
// can be useful if you are using the thread local universe, and the
// const thing doesn't work to your advantage
//
struct _mulle_objc_universe  *mulle_objc_get_universe( void)
{
#if __MULLE_OBJC_TRT__
   return( mulle_objc_get_thread_universe());
#else
   return( mulle_objc_get_global_universe());
#endif
}


void  mulle_objc_set_thread_universe( struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_threadconfig   *config;
   struct mulle_allocator            *allocator;

   if( ! universe)
   {
      mulle_objc_unset_thread_universe();
      return;
   }

   // don't use gifting calloc, will be cleaned separately

   allocator = _mulle_objc_universe_get_allocator( universe);
   config    = _mulle_allocator_calloc( allocator, 1, sizeof( struct _mulle_objc_threadconfig));

   config->universe = universe;
   mulle_thread_tss_set( mulle_objc_unfailingget_or_create_threadkey(), config);
}


//
// this is low level, you don't use it
// the outcome is a class (piece) that in itself must be sane
//


// (no instances here yet..)
//   @interface NSObject
//     NSObject ---isa--> meta-NSObject  (1)
//     NSObject ---superclass--> nil
//     meta-NSObject ---isa--> meta-NSObject (2)
//     meta-NSObject ---superclass--> NSObject (3)
//
//   @interface Foo : NSObject
//     Foo ---isa--> meta-Foo (1)
//     Foo ---superclass--> NSObject
//     meta-Foo ---isa--> meta-NSObject (2)
//     meta-Foo ---superclass--> meta-NSObject
//
//   @interface Bar : Foo
//     Bar ---isa--> meta-Bar (1)
//     Bar ---superclass--> Foo
//     meta-Bar ---isa--> meta-NSObject (2)
//     meta-Bar ---superclass--> meta-Foo
//
// a class-pair is a class and a meta-class
//
// every class has a superclass except the root class
// every meta-class has a superclass
//
// (1) every class's isa points to its meta-class sibling
// (2) every meta-class's isa points to the root meta-class
// (3) the root meta-class's superclass is the root class
//
// what is the use of the meta-class. It's basically just
// there to hold a method cache for class methods.
//
struct _mulle_objc_classpair *
   mulle_objc_universe_new_classpair( struct _mulle_objc_universe *universe,
                                      mulle_objc_classid_t  classid,
                                      char *name,
                                      size_t instancesize,
                                      struct _mulle_objc_infraclass *superclass)
{
   struct _mulle_objc_classpair   *pair;
   struct _mulle_objc_metaclass   *super_meta;
   struct _mulle_objc_metaclass   *super_meta_isa;
   struct mulle_allocator         *allocator;
   mulle_objc_classid_t           correct;
   size_t                         size;

   if( ! universe)
   {
      errno = EINVAL;
      return( NULL);
   }

   allocator = _mulle_objc_universe_get_allocator( universe);
   if( _mulle_objc_universe_is_uninitialized( universe) || ! allocator)
   {
      fprintf( stderr, "mulle_objc_universe error: The universe %p has not "
                       "been set up yet. You probably forgot to link the "
                       "startup library\n", universe);
      errno = ENXIO;
      return( NULL);
   }

   if( classid == MULLE_OBJC_NO_CLASSID || classid == MULLE_OBJC_INVALID_CLASSID)
   {
      errno = EINVAL;
      return( NULL);
   }

   if( ! name || ! strlen( name))
   {
      errno = EINVAL;
      return( NULL);
   }

   correct = mulle_objc_classid_from_string( name);
   if( classid != correct)
   {
      fprintf( stderr, "mulle_objc_universe error: Class \"%s\" should have "
                       "classid %08x but has classid %08x\n", name,
                       correct, classid);
      errno = EINVAL;
      return( NULL);
   }

   super_meta     = NULL;
   super_meta_isa = NULL;
   if( superclass)
   {
      super_meta     = _mulle_objc_infraclass_get_metaclass( superclass);
      super_meta_isa = _mulle_objc_class_get_metaclass( &super_meta->base);
      assert( super_meta_isa);
   }

   // classes are freed by hand so don't use gifting calloc
   size = sizeof( struct _mulle_objc_classpair);
   pair = _mulle_allocator_calloc( allocator, 1, size);

   _mulle_objc_objectheader_init( &pair->metaclassheader,
                                  super_meta_isa ? &super_meta_isa->base : &pair->infraclass.base);
   _mulle_objc_objectheader_init( &pair->infraclassheader, &pair->metaclass.base);

   _mulle_objc_class_init( &pair->infraclass.base,
                           name,
                           instancesize,
                           classid,
                           &superclass->base,
                           universe);
   _mulle_objc_class_init( &pair->metaclass.base,
                           name,
                           sizeof( struct _mulle_objc_class),
                           classid,
                           super_meta ? &super_meta->base : &pair->infraclass.base,
                           universe);

   _mulle_objc_infraclass_plusinit( &pair->infraclass, allocator);
   _mulle_objc_metaclass_plusinit( &pair->metaclass, allocator);
   _mulle_objc_classpair_plusinit( pair, allocator);

   _mulle_objc_class_set_infraclass( &pair->metaclass.base, &pair->infraclass);

   return( pair);
}


#ifndef MULLE_OBJC_NO_CONVENIENCES
// convenience during loading
struct _mulle_objc_classpair *
   mulle_objc_unfailingnew_classpair( mulle_objc_classid_t  classid,
                                      char *name,
                                      size_t instancesize,
                                      struct _mulle_objc_infraclass *superclass)
{
   struct _mulle_objc_classpair   *pair;
   struct _mulle_objc_universe    *universe;

   universe = mulle_objc_get_universe();
   pair     = mulle_objc_universe_new_classpair( universe, classid, name, instancesize, superclass);
   if( ! pair)
      _mulle_objc_universe_raise_errno_exception( universe);  // unfailing vectors through there
   return( pair);
}
#endif


/* don't check for ivar_hash, as this is too painful for application
   universe hacks. Only during loading
 */
int   mulle_objc_universe_add_infraclass( struct _mulle_objc_universe *universe,
                                          struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_metaclass    *dup;
   struct _mulle_objc_infraclass   *superclass;

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

   if( _mulle_concurrent_hashmap_lookup( &universe->classtable, infra->base.classid))
   {
      errno = EEXIST;
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

   dup = _mulle_concurrent_hashmap_register( &universe->classtable,
                                             infra->base.classid,
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
      fprintf( stderr, "mulle_objc_universe %p trace: add class %08x \"%s\"",
                 universe,
                 _mulle_objc_infraclass_get_classid( infra),
                 _mulle_objc_infraclass_get_name( infra));
      if( superclass)
         fprintf( stderr, " with superclass %08x \"%s\"",
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
         fprintf( stderr, "mulle_objc_universe %p trace: remove fastclass "
                           "\"%s\" at index %d\n", universe, infra->base.name, index);
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
      fprintf( stderr, "mulle_objc_universe %p trace: removed class %08x \"%s\"\n",
                 universe,
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


void   mulle_objc_universe_unfailingadd_infraclass( struct _mulle_objc_universe *universe,
                                                    struct _mulle_objc_infraclass *infra)
{
   if( mulle_objc_universe_add_infraclass( universe, infra))
      _mulle_objc_universe_raise_errno_exception( universe);
}


void   _mulle_objc_universe_set_fastclass( struct _mulle_objc_universe *universe,
                                           struct _mulle_objc_infraclass *infra,
                                           unsigned int index)
{
   struct _mulle_objc_infraclass  *old;

   assert( universe);
   assert( infra);

   if( universe->debug.trace.fastclass_add)
      fprintf( stderr, "mulle_objc_universe %p trace: add fastclass "
                        "\"%s\" at index %d\n", universe, infra->base.name, index);

   if( index >= MULLE_OBJC_S_FASTCLASSES)
      _mulle_objc_universe_raise_generic_exception( universe,
                  "error in mulle_objc_universe %p: "
                  "fastclass index %d for %s (id %08lx) out of bounds\n",
                  universe,
                  index,
                  _mulle_objc_infraclass_get_name( infra),
                  _mulle_objc_infraclass_get_classid( infra));

   if( ! _mulle_atomic_pointer_cas( &universe->fastclasstable.classes[ index].pointer, infra, NULL))
   {
      old = _mulle_atomic_pointer_read( &universe->fastclasstable.classes[ index].pointer);
      _mulle_objc_universe_raise_inconsistency_exception( universe,
                  "mulle_objc_universe %p: classes \"%s\" "
                  "and \"%s\", both want to occupy fastclass spot %u",
                  universe,
                  _mulle_objc_infraclass_get_name( infra),
                  _mulle_objc_infraclass_get_name( old),
                  index);
   }
}


#ifndef MULLE_OBJC_NO_CONVENIENCES
// conveniences

void   mulle_objc_unfailingadd_infraclass( struct _mulle_objc_infraclass *infra)
{
   mulle_objc_universe_unfailingadd_infraclass( mulle_objc_get_universe(), infra);
}

#endif


# pragma mark - cache


int   _mulle_objc_universe_should_grow_cache( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_cache *cache)
{
   size_t   used;
   size_t   size;

   used = (size_t) _mulle_atomic_pointer_read( &cache->n);
   size = cache->size;

   if( ! universe->config.cache_fillrate)
      return( used >= (size >> 2));

   return( used * 100 >= size * universe->config.cache_fillrate);
}


# pragma mark - method descriptors

struct _mulle_objc_descriptor   *_mulle_objc_universe_lookup_descriptor( struct _mulle_objc_universe *universe, mulle_objc_methodid_t methodid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->descriptortable, methodid));
}


//
// returns duplicate if found!
// returns MULLE_OBJC_INVALID_DESCRIPTOR, if error
//
struct _mulle_objc_descriptor *_mulle_objc_universe_unfailingregister_descriptor(
   struct _mulle_objc_universe *universe,
   struct _mulle_objc_descriptor *p)
{
   struct _mulle_objc_descriptor   *dup;

   dup = _mulle_concurrent_hashmap_register( &universe->descriptortable,
                                             p->methodid,
                                             p);
   if( ! dup)
   {
      if( universe->debug.trace.descriptor_add)
         fprintf( stderr, "mulle_objc_universe %p trace: add descriptor %08x \"%s\" (%p)\n",
                 universe, p->methodid, p->name, p);
      return( p);
   }

   // must be out of mem: is catastrophic
   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      mulle_objc_raise_errno_exception();

   assert( p->methodid == dup->methodid);

   // hash clash is also a catastrophy
   if( strcmp( dup->name, p->name))
      _mulle_objc_universe_raise_generic_exception( universe,
      		"mulle_objc_universe %p error: duplicate selectors \"%s\" and \"%s\" "
      		"with same id %08lx\n", universe, dup->name, p->name, (long) p->methodid);

   if( universe->debug.warn.methodid_type)
   {
      int   comparison;

      if( universe->debug.warn.pedantic_methodid_type)
         comparison = _mulle_objc_signature_pedantic_compare( dup->signature, p->signature);
      else
         comparison = _mulle_objc_signature_compare( dup->signature, p->signature);
      if( comparison)
         fprintf( stderr, "mulle_objc_universe %p warning: varying types \"%s\" and \"%s\" for method \"%s\"\n",
                 universe,
                 dup->signature, p->signature, p->name);
   }

   return( dup);
}


struct _mulle_objc_descriptor *
   mulle_objc_universe_unfailingregister_descriptor(
      struct _mulle_objc_universe *universe,
      struct _mulle_objc_descriptor *p)
{
   if( ! universe)
   {
      errno = EINVAL;
      mulle_objc_raise_errno_exception();
   }

   if( ! mulle_objc_descriptor_is_sane( p))
   {
      errno = EINVAL;
      _mulle_objc_universe_raise_errno_exception( universe);
   }

   return( _mulle_objc_universe_unfailingregister_descriptor( universe, p));
}



// function kept for tests, register is the way to go though
int   _mulle_objc_universe_add_descriptor( struct _mulle_objc_universe *universe,
                                           struct _mulle_objc_descriptor *p)
{
   struct _mulle_objc_descriptor   *dup;

   dup = _mulle_concurrent_hashmap_register( &universe->descriptortable, p->methodid, p);
   if( ! dup)
   {
      if( universe->debug.trace.descriptor_add)
         fprintf( stderr, "mulle_objc_universe %p trace: add descriptor %08x \"%s\" (%p)\n",
                 universe, p->methodid, p->name, p);
      return( 0);
   }

   // must be out of mem
   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      return( -1);

   assert( p->methodid == dup->methodid);

   // hash clash is bad
   if( strcmp( dup->name, p->name))
      _mulle_objc_universe_raise_generic_exception( universe,
      		"mulle_objc_universe %p error: duplicate methods \"%s\" and \"%s\" "
      		"with same id %08lx\n", universe, dup->name, p->name, (long) p->methodid);

   if( universe->debug.warn.methodid_type)
   {
      int   comparison;

      if( universe->debug.warn.pedantic_methodid_type)
         comparison = _mulle_objc_signature_pedantic_compare( dup->signature, p->signature);
      else
         comparison = _mulle_objc_signature_compare( dup->signature, p->signature);
      if( comparison)
         fprintf( stderr, "mulle_objc_universe %p warning: varying types \"%s\" and \"%s\" for method \"%s\"\n",
                 universe,
                 dup->signature, p->signature, p->name);
   }
   return( 0);
}



int   mulle_objc_universe_add_descriptor( struct _mulle_objc_universe *universe,
                                          struct _mulle_objc_descriptor *p)
{
   if( ! universe)
   {
      errno = EINVAL;
      return( -1);
   }

   if( ! mulle_objc_descriptor_is_sane( p))
      return( -1);

   return( _mulle_objc_universe_add_descriptor( universe, p));
}


char   *mulle_objc_lookup_methodname( mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_descriptor   *desc;

   universe = mulle_objc_get_universe();
   desc    = _mulle_objc_universe_lookup_descriptor( universe, methodid);
   return( desc ? desc->name : NULL);
}


char   *mulle_objc_search_debughashname( mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_search_debughashname( universe, uniqueid));
}


// cheap conveniences
#ifndef MULLE_OBJC_NO_CONVENIENCES
struct _mulle_objc_methodlist  *mulle_objc_alloc_methodlist( unsigned int n)
{
   return( mulle_objc_universe_calloc( mulle_objc_get_universe(),
                                       1,
                                       mulle_objc_sizeof_methodlist( n)));
}

#endif


# pragma mark - super map

struct _mulle_objc_super   *
   _mulle_objc_universe_lookup_super( struct _mulle_objc_universe *universe,
                                      mulle_objc_superid_t superid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->supertable, superid));
}


struct _mulle_objc_super   *
   _mulle_objc_universe_unfailinglookup_super( struct _mulle_objc_universe *universe,
                                               mulle_objc_superid_t superid)
{
   struct _mulle_objc_super   *p;

   p = _mulle_objc_universe_lookup_super( universe, superid);
   if( ! p)
      _mulle_objc_universe_raise_super_not_found_exception( universe, superid);
   return( p);
}


int   _mulle_objc_universe_add_super( struct _mulle_objc_universe *universe,
                                      struct _mulle_objc_super *p)
{
   struct _mulle_objc_super   *dup;

   if( ! mulle_objc_super_is_sane( p))
      return( -1);

   dup = _mulle_concurrent_hashmap_register( &universe->supertable, p->superid, p);
   if( ! dup)
   {
      if( universe->debug.trace.super_add)
         fprintf( stderr, "mulle_objc_universe %p trace: add super %08x,%08x,%08x (%p)\n",
                 universe,
                 p->superid,
                 p->classid,
                 p->methodid,
                 p);
      return( 0);
   }

   if( dup->classid != p->classid || dup->methodid != p->methodid)
      _mulle_objc_universe_raise_generic_exception( universe,
            "mulle_objc_universe %p error: duplicate supers %08x,%08x "
            "and %08x,%08x with same id %08x\n",
            universe,
            dup->classid, dup->methodid,
            p->classid, p->methodid,
            p->superid);

   return( 0);
}


void    mulle_objc_universe_unfailingadd_super( struct _mulle_objc_universe *universe,
                                                struct _mulle_objc_super *p)
{
   assert( universe);

   if( _mulle_objc_universe_add_super( universe, p))
      _mulle_objc_universe_raise_errno_exception( universe);
}


struct _mulle_objc_super   *mulle_objc_lookup_super( mulle_objc_superid_t superid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_lookup_super( universe, superid));
}


# pragma mark - protocols

struct _mulle_objc_protocol   *
   _mulle_objc_universe_lookup_protocol( struct _mulle_objc_universe *universe,
                                         mulle_objc_protocolid_t protocolid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->protocoltable, protocolid));
}


int   _mulle_objc_universe_add_protocol( struct _mulle_objc_universe *universe,
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
         fprintf( stderr, "mulle_objc_universe %p trace: add protocol %08x \"%s\" (%p)\n",
                 universe,
                 p->protocolid,
                 p->name,
                 p);
     	return( 0);
   }

   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      return( -1);

   assert( dup->protocolid == p->protocolid);

   if( strcmp( dup->name, p->name))
      _mulle_objc_universe_raise_generic_exception( universe,
      		"mulle_objc_universe %p error: duplicate protocols \"%s\" and \"%s\" "
      		"with same id %08lx\n", universe, dup->name, p->name, (long) p->protocolid);

   return( 0);
}


void    mulle_objc_universe_unfailingadd_protocol( struct _mulle_objc_universe *universe,
                                                   struct _mulle_objc_protocol *protocol)
{
   struct _mulle_objc_protocol  *dup;

   assert( universe);

   if( _mulle_objc_universe_add_protocol( universe, protocol))
      _mulle_objc_universe_raise_errno_exception( universe);
}


struct _mulle_objc_protocol  *mulle_objc_lookup_protocol( mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_lookup_protocol( universe, protocolid));
}


#ifndef MULLE_OBJC_NO_CONVENIENCES
struct _mulle_objc_protocollist  *mulle_objc_alloc_protocollist( unsigned int n)
{
   return( mulle_objc_universe_calloc( mulle_objc_get_universe(),
                                       1,
                                       mulle_objc_sizeof_protocollist( n)));
}
#endif


size_t   _mulle_objc_universe_count_protocols( struct _mulle_objc_universe  *universe)
{
	return( mulle_concurrent_hashmap_count( &universe->protocoltable));
}


mulle_objc_walkcommand_t
   _mulle_objc_universe_walk_protocols( struct _mulle_objc_universe  *universe,
                                        mulle_objc_walk_protocols_callback callback,
  	                                     void *userinfo)
{
   mulle_objc_walkcommand_t                    cmd;
   struct _mulle_objc_protocol                 *protocol;
   struct mulle_concurrent_hashmapenumerator   rover;

   if( ! universe || ! callback)
      return( mulle_objc_walk_error);

   cmd   = mulle_objc_walk_done;
   rover = mulle_concurrent_hashmap_enumerate( &universe->protocoltable);  // slow!
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &protocol))
   {
      cmd = (*callback)( universe, protocol, userinfo);
      if( mulle_objc_walkcommand_is_stopper( cmd))
      	break;
   }
   mulle_concurrent_hashmapenumerator_done( &rover);

   return( cmd);
}


# pragma mark - categories

char   *_mulle_objc_universe_lookup_category( struct _mulle_objc_universe *universe,
                                             mulle_objc_categoryid_t categoryid)
{
   return( _mulle_concurrent_hashmap_lookup( &universe->categorytable, categoryid));
}


int   _mulle_objc_universe_add_category( struct _mulle_objc_universe *universe,
                                        mulle_objc_categoryid_t categoryid,
                                        char *name)

{
   char   *dup;

   if( categoryid == MULLE_OBJC_NO_CATEGORYID || categoryid == MULLE_OBJC_INVALID_CATEGORYID)
      return( -1);

   dup = _mulle_concurrent_hashmap_lookup( &universe->categorytable, categoryid);
   if( ! dup)
      return( _mulle_concurrent_hashmap_insert( &universe->categorytable, categoryid, name));

   if( dup == MULLE_CONCURRENT_INVALID_POINTER)
      return( -1);

   if( strcmp( dup, name))
      _mulle_objc_universe_raise_generic_exception( universe,
            "mulle_objc_universe %p error: duplicate categories \"%s\" and \"%s\" "
            "with same id %08lx\n", universe, dup, name, (long) categoryid);

   return( 0);
}


void    mulle_objc_universe_unfailingadd_category( struct _mulle_objc_universe *universe,
                                                   mulle_objc_categoryid_t categoryid,
                                                   char *name)
{
   char  *dup;

   assert( universe);

   if( _mulle_objc_universe_add_category( universe, categoryid, name))
   {
      dup = _mulle_objc_universe_lookup_category( universe, categoryid);
      _mulle_objc_universe_raise_generic_exception( universe,
    	   "mulle_objc_universe %p error: duplicate categories \"%s\" and \"%s\" "
    	   "with same id %08x\n", universe, dup, name, categoryid);
   }
}


char   *mulle_objc_lookup_category( mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_lookup_category( universe, categoryid));
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
         fprintf( stderr, "mulle_objc_universe %p trace: delay add of string @\"%s\" (%p)\n",
               universe, ((struct _NSConstantString *) string)->_storage, string);

      // memorize it anyway
      _mulle_concurrent_pointerarray_add( &universe->staticstrings, (void *) string);
      return;
   }

   _mulle_objc_object_set_isa( string,
                               _mulle_objc_infraclass_as_class( universe->foundation.staticstringclass));
   if( universe->debug.trace.string_add)
      fprintf( stderr, "mulle_objc_universe %p trace: add string @\"%s\" (%p)\n",
            universe, ((struct _NSConstantString *) string)->_storage, string);

   if( ! universe->config.forget_strings)
      _mulle_concurrent_pointerarray_add( &universe->staticstrings, (void *) string);
}


void   _mulle_objc_universe_staticstringclass_did_change( struct _mulle_objc_universe *universe)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_object                        *string;
   struct mulle_allocator                           *allocator;
   int                                              flag;

   flag  = universe->debug.trace.string_add;
   rover = mulle_concurrent_pointerarray_enumerate( &universe->staticstrings);
   while( string = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      _mulle_objc_object_set_isa( string,
                                 _mulle_objc_infraclass_as_class( universe->foundation.staticstringclass));
      if( flag)
         fprintf( stderr, "mulle_objc_universe %p trace: patch string class @\"%s\" (%p)\n",
               universe, ((struct _NSConstantString *) string)->_storage, string);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   // if so configured wipe the list
   // effectivey _mulle_objc_universe_staticstringclass_did_change should then
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
                                                  struct _mulle_objc_infraclass *infra)
{
   assert( universe);
   assert( infra);

   universe->foundation.staticstringclass = infra;
   _mulle_objc_universe_staticstringclass_did_change( universe);
}


# pragma mark - hashnames (debug output only)

void   _mulle_objc_universe_add_loadhashedstringlist( struct _mulle_objc_universe *universe,
                                                      struct _mulle_objc_loadhashedstringlist *hashnames)
{
   _mulle_concurrent_pointerarray_add( &universe->hashnames, (void *) hashnames);
}


char  *_mulle_objc_universe_search_debughashname( struct _mulle_objc_universe *universe,
                                                 mulle_objc_uniqueid_t hash)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_loadhashedstringlist          *map;
   char                                             *s;

   s     = NULL;
   rover = mulle_concurrent_pointerarray_enumerate( &universe->hashnames);
   while( map = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      s = mulle_objc_loadhashedstringlist_bsearch( map, hash);
      if( s)
         break;
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   return( s);
}


# pragma mark - uniqueid to string conversions

MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_string_for_uniqueid( struct _mulle_objc_universe *universe,
                                                  mulle_objc_uniqueid_t uniqueid)
{
   char   *s;

   if( uniqueid == MULLE_OBJC_NO_UNIQUEID)
      return( "NOT AN ID");
   if( uniqueid == MULLE_OBJC_INVALID_UNIQUEID)
      return( "INVALID ID");
   s = _mulle_objc_universe_search_debughashname( universe, uniqueid);
   return( s ? s : "???");
}


MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_string_for_classid( struct _mulle_objc_universe *universe,
                                                 mulle_objc_classid_t classid)
{
   struct _mulle_objc_infraclass   *infra;

   infra = _mulle_objc_universe_uncachedlookup_infraclass( universe, classid);
   if( infra)
      return( _mulle_objc_infraclass_get_name( infra));
   return( _mulle_objc_universe_string_for_uniqueid( universe, classid));
}


MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_string_for_methodid( struct _mulle_objc_universe *universe,
                                                  mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_descriptor   *desc;

   desc   = _mulle_objc_universe_lookup_descriptor( universe, methodid);
   if( desc)
      return( _mulle_objc_descriptor_get_name( desc));
   return( _mulle_objc_universe_string_for_uniqueid( universe, methodid));
}


MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_string_for_protocolid( struct _mulle_objc_universe *universe,
                                                    mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_protocol   *protocol;

   protocol = _mulle_objc_universe_lookup_protocol( universe, protocolid);
   if( protocol)
      return( _mulle_objc_protocol_get_name( protocol));
   return( _mulle_objc_universe_string_for_uniqueid( universe, protocolid));
}


MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_string_for_categoryid( struct _mulle_objc_universe *universe,
                                                    mulle_objc_categoryid_t categoryid)
{
   char    *name;

   name = _mulle_objc_universe_lookup_category( universe, categoryid);
   if( name)
      return( name);
   return( _mulle_objc_universe_string_for_uniqueid( universe, categoryid));
}


MULLE_C_NON_NULL_RETURN
char   *_mulle_objc_universe_string_for_superid( struct _mulle_objc_universe *universe,
                                                 mulle_objc_superid_t superid)
{
   struct _mulle_objc_super   *p;

   p = _mulle_objc_universe_lookup_super( universe, superid);
   if( p)
      return( p->name);
   return( _mulle_objc_universe_string_for_uniqueid( universe, superid));
}


#ifndef MULLE_OBJC_NO_CONVENIENCES

# pragma mark - string conveniences

MULLE_C_NON_NULL_RETURN
char   *mulle_objc_string_for_uniqueid( mulle_objc_uniqueid_t uniqueid)
{
   char   *s;
   struct _mulle_objc_universe   *universe;

   if( uniqueid == MULLE_OBJC_NO_UNIQUEID)
      return( "NOT AN ID");
   if( uniqueid == MULLE_OBJC_INVALID_UNIQUEID)
      return( "INVALID ID");

   universe = mulle_objc_get_universe();
   s       = _mulle_objc_universe_search_debughashname( universe, uniqueid);
   return( s ? s : "???");
}


MULLE_C_NON_NULL_RETURN
char   *mulle_objc_string_for_classid( mulle_objc_classid_t classid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_string_for_classid( universe, classid));
}


MULLE_C_NON_NULL_RETURN
char   *mulle_objc_string_for_methodid( mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_string_for_methodid( universe, methodid));
}


MULLE_C_NON_NULL_RETURN
char   *mulle_objc_string_for_protocolid( mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_string_for_protocolid( universe, protocolid));
}


MULLE_C_NON_NULL_RETURN
char   *mulle_objc_string_for_categoryid( mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_string_for_categoryid( universe, categoryid));
}


MULLE_C_NON_NULL_RETURN
char   *mulle_objc_string_for_superid( mulle_objc_superid_t superid)
{
   struct _mulle_objc_universe   *universe;

   universe = mulle_objc_get_universe();
   return( _mulle_objc_universe_string_for_superid( universe, superid));
}

#endif


# pragma mark - debug support

/* debug support */

mulle_objc_walkcommand_t
    mulle_objc_universe_walk( struct _mulle_objc_universe *universe,
                              mulle_objc_walkcallback_t   callback,
                              void *userinfo)
{
   mulle_objc_walkcommand_t                   cmd;
   struct _mulle_objc_class                   *cls;
   struct mulle_concurrent_hashmapenumerator  rover;

   if( ! universe || ! callback)
      return( mulle_objc_walk_error);

   cmd = (*callback)( universe, universe, mulle_objc_walkpointer_is_universe, NULL, NULL, userinfo);
   if( mulle_objc_walkcommand_is_stopper( cmd))
      return( cmd);

   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);  // slow!
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &cls))
   {
      cmd = mulle_objc_classpair_walk( _mulle_objc_class_get_classpair( cls),
                                       callback,
                                       userinfo);
      if( mulle_objc_walkcommand_is_stopper( cmd))
         return( cmd);
   }
   mulle_concurrent_hashmapenumerator_done( &rover);

   return( cmd);
}


mulle_objc_walkcommand_t
   _mulle_objc_universe_walk_classes( struct _mulle_objc_universe  *universe,
                                      int with_meta,
                                      mulle_objc_walkcallback_t callback,
                                      void *userinfo)
{
   mulle_objc_walkcommand_t                    cmd;
   struct _mulle_objc_infraclass               *cls;
   struct _mulle_objc_metaclass                *meta;
   struct mulle_concurrent_hashmapenumerator   rover;

   if( ! universe || ! callback)
      return( mulle_objc_walk_error);

   cmd   = mulle_objc_walk_done;
   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);  // slow!
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &cls))
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
   mulle_concurrent_hashmapenumerator_done( &rover);

   return( cmd);
}


static int   mulle_objc_infraclass_is_subclass( struct _mulle_objc_infraclass *infra,
                                                 struct _mulle_objc_infraclass *kindofcls)
{
   if( ! kindofcls)
      return( 1);

   while( infra)
   {
      if( infra == kindofcls)
         return( 1);
      infra = _mulle_objc_infraclass_get_superclass( infra);
   }
   return( 0);
}


static mulle_objc_walkcommand_t
      invalidate_class_caches_callback( struct _mulle_objc_universe *universe,
                                        void *p,
                                        enum mulle_objc_walkpointertype_t type,
                                        char *key,
                                        void *parent,
                                        void *userinfo)
{
   struct _mulle_objc_infraclass   *infra = p;
   struct _mulle_objc_infraclass   *kindofcls = userinfo;
   struct _mulle_objc_infraclass   *superclass;
   struct _mulle_objc_metaclass    *meta;

   if( mulle_objc_infraclass_is_subclass( infra, kindofcls))
   {
      mulle_objc_class_invalidate_methodcache( _mulle_objc_infraclass_as_class( infra));
      _mulle_objc_class_invalidate_kvccache( _mulle_objc_infraclass_as_class( infra));

      meta = _mulle_objc_infraclass_get_metaclass( infra);

      mulle_objc_class_invalidate_methodcache( _mulle_objc_metaclass_as_class( meta));
      _mulle_objc_class_invalidate_kvccache( _mulle_objc_metaclass_as_class( meta));
   }
   return( mulle_objc_walk_ok);
}


void  _mulle_objc_universe_invalidate_class_caches( struct _mulle_objc_universe *universe,
                                                    struct _mulle_objc_infraclass *kindofcls)
{
   _mulle_objc_universe_walk_classes( universe, 0, invalidate_class_caches_callback, kindofcls);
}


