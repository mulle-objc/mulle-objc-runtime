//
//  mulle_objc_runtime.c
//  mulle-objc
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

#include "mulle_objc_runtime.h"

#include "mulle_objc_runtime_global.h"
#include "mulle_objc_class.h"
#include "mulle_objc_class_runtime.h"
#include "mulle_objc_classpair.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_metaclass.h"
#include "mulle_objc_object.h"
#include "mulle_objc_signature.h"
#include "mulle_objc_csvdump.h"
#include <mulle_test_allocator/mulle_test_allocator.h>
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


# pragma mark - errors

static void   _mulle_objc_vprintf_abort( char *format, va_list args) MULLE_C_NO_RETURN;
static void   _mulle_objc_perror_abort( char *s)                     MULLE_C_NO_RETURN;


MULLE_C_NO_RETURN
static void   _mulle_objc_perror_abort( char *s)
{
   perror( s);
   abort();
}


MULLE_C_NO_RETURN
static void   _mulle_objc_vprintf_abort( char *format, va_list args)
{
   vfprintf( stderr, format, args);
   fprintf( stderr, "\n");
   abort();
}


MULLE_C_NO_RETURN MULLE_C_NEVER_INLINE
static void   _mulle_objc_printf_abort( char *format, ...)
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


MULLE_C_NO_RETURN
void   _mulle_objc_runtime_raise_fail_exception( struct _mulle_objc_runtime *runtime, char *format, ...)
{
   va_list   args;

   va_start( args, format);
   (*runtime->failures.fail)( format, args);
   va_end( args);
}


MULLE_C_NO_RETURN
static void   _mulle_objc_class_not_found_abort( struct _mulle_objc_runtime *runtime,
                                                 mulle_objc_classid_t missing_classid)
{
   _mulle_objc_printf_abort( "mulle_objc_runtime %p fatal: missing class %08x \"%s\"",
                                 runtime,
                                 missing_classid,
                                 mulle_objc_string_for_classid( missing_classid));
}


MULLE_C_NO_RETURN
static void   _mulle_objc_method_not_found_abort( struct _mulle_objc_class *cls,
                                                 mulle_objc_methodid_t missing_method)
{
   struct _mulle_objc_runtime           *runtime;
   struct _mulle_objc_methoddescriptor  *desc;
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

   runtime = _mulle_objc_class_get_runtime( cls);
   if( ! methodname)
   {
      desc = _mulle_objc_runtime_lookup_methoddescriptor( runtime, missing_method);
      if( desc)
         methodname = desc->name;
      else
         methodname = _mulle_objc_runtime_search_debughashname( runtime, missing_method);
   }

   // keep often seen output more user friendly
   if( ! methodname)
      _mulle_objc_printf_abort( "mulle_objc_runtime %p fatal: missing %s method with id %08x in class \"%s\"",
                                runtime,
                                _mulle_objc_class_is_metaclass( cls) ? "class" : "instance",
                                missing_method,
                                name);
   _mulle_objc_printf_abort( "mulle_objc_runtime %p fatal: missing method \"%c%s\" (%08x) in class \"%s\"",
                             runtime,
                             _mulle_objc_class_is_metaclass( cls) ? '+' : '-',
                             methodname,
                             missing_method,
                             name);
}


# pragma mark - setup

static void   nop( struct _mulle_objc_runtime  *runtime, mulle_objc_classid_t classid)
{
}


static void   _mulle_objc_runtimeconfig_dump( struct _mulle_objc_runtimeconfig  *config)
{
   fprintf( stderr, "%s", config->thread_local_rt ? "thread local" : "global");
   fprintf( stderr, ", %stagged pointers", config->no_tagged_pointers ? "no " : "");
   if( config->forget_strings)
      fprintf( stderr, ", forget strings");

   if( config->ignore_ivarhash_mismatch)
      fprintf( stderr, ", ignore ivarhash mismatch");
   fprintf( stderr, ", min:-O%u max:-O%u", config->min_optlevel, config->max_optlevel);
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

static void   _mulle_objc_runtime_set_debug_defaults_from_environment( struct _mulle_objc_runtime  *runtime)
{
   runtime->debug.warn.methodid_types          = getenv_yes_no( "MULLE_OBJC_WARN_METHODID_TYPES");
   runtime->debug.warn.pedantic_methodid_types = getenv_yes_no( "MULLE_OBJC_WARN_PEDANTIC_METHODID_TYPES");
   runtime->debug.warn.protocolclass           = getenv_yes_no( "MULLE_OBJC_INFRA_WARN_PROTOCOLCLASS");
   runtime->debug.warn.stuck_loadables         = getenv_yes_no_default( "MULLE_OBJC_WARN_STUCK_LOADABLES", 1);

#if ! DEBUG
   if( getenv_yes_no( "MULLE_OBJC_WARN_ENABLED"))
#endif
   {
      runtime->debug.warn.methodid_types = 1;
      runtime->debug.warn.protocolclass  = 1;
   }

   runtime->debug.trace.category_adds      = getenv_yes_no( "MULLE_OBJC_TRACE_CATEGORY_ADDS");
   runtime->debug.trace.class_adds         = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_ADDS");
   runtime->debug.trace.class_frees        = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_FREES");
   runtime->debug.trace.class_cache        = getenv_yes_no( "MULLE_OBJC_TRACE_CLASS_CACHE");
   runtime->debug.trace.dependencies       = getenv_yes_no( "MULLE_OBJC_TRACE_DEPENDENCIES");
   runtime->debug.trace.dump_runtime       = getenv_yes_no( "MULLE_OBJC_TRACE_DUMP_RUNTIME");
   runtime->debug.trace.fastclass_adds     = getenv_yes_no( "MULLE_OBJC_TRACE_FASTCLASS_ADDS");
   runtime->debug.trace.initialize         = getenv_yes_no( "MULLE_OBJC_TRACE_INITIALIZE");
   runtime->debug.trace.load_calls         = getenv_yes_no( "MULLE_OBJC_TRACE_LOAD_CALLS");
   runtime->debug.trace.loadinfo           = getenv_yes_no( "MULLE_OBJC_TRACE_LOADINFO");
   runtime->debug.trace.method_caches      = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_CACHES");
   runtime->debug.trace.method_calls       = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_CALLS");  // totally excessive!
   runtime->debug.trace.method_searches    = getenv_yes_no( "MULLE_OBJC_TRACE_METHOD_SEARCHES");  // fairly excessive!
   runtime->debug.trace.print_origin       = getenv_yes_no_default( "MULLE_OBJC_TRACE_PRINT_ORIGIN", 1);
   runtime->debug.trace.protocol_adds      = getenv_yes_no( "MULLE_OBJC_TRACE_PROTOCOL_ADDS");
   runtime->debug.trace.runtime_config     = getenv_yes_no( "MULLE_OBJC_TRACE_RUNTIME_CONFIG");
   runtime->debug.trace.state_bits         = getenv_yes_no( "MULLE_OBJC_TRACE_STATE_BITS");
   runtime->debug.trace.string_adds        = getenv_yes_no( "MULLE_OBJC_TRACE_STRING_ADDS");
   runtime->debug.trace.tagged_pointers    = getenv_yes_no( "MULLE_OBJC_TRACE_TAGGED_POINTERS");

   // don't trace method search and calls, per default... too expensive
   // don't trace caches either, usually that's too boring
   // also don't dump, per default
   if( getenv_yes_no( "MULLE_OBJC_TRACE_ENABLED"))
   {
      runtime->debug.trace.category_adds         = 1;
      runtime->debug.trace.class_adds            = 1;
      runtime->debug.trace.class_frees           = 1;
      runtime->debug.trace.dependencies          = 1;
      runtime->debug.trace.fastclass_adds        = 1;
      runtime->debug.trace.initialize            = 1;
      runtime->debug.trace.load_calls            = 1;
      runtime->debug.trace.loadinfo              = 1;
      runtime->debug.trace.protocol_adds         = 1;
      runtime->debug.trace.runtime_config        = 1;
      runtime->debug.trace.state_bits            = 1;
      runtime->debug.trace.string_adds           = 1;
      runtime->debug.trace.tagged_pointers       = 1;
   }

   if( runtime->debug.trace.runtime_config)
   {
      fprintf( stderr, "mulle-objc: v%u.%u.%u (load-version: %u) (",
         MULLE_OBJC_RUNTIME_VERSION_MAJOR,
         MULLE_OBJC_RUNTIME_VERSION_MINOR,
         MULLE_OBJC_RUNTIME_VERSION_PATCH,
         MULLE_OBJC_RUNTIME_LOAD_VERSION);
      _mulle_objc_runtimeconfig_dump( &runtime->config);
      fprintf( stderr, ")\n");
   }
}


MULLE_C_NO_RETURN
void   mulle_objc_allocator_fail( void *block, size_t size)
{
   mulle_objc_raise_fail_errno_exception();
}


static int   unfailing_abafree( void  *aba,
                                void (*p_free)( void *),
                                void *pointer)
{
   if( _mulle_aba_free( aba, p_free, pointer))
      mulle_objc_raise_fail_errno_exception();
   return( 0);
}


//
// minimal stuff the runtime needs to be setup with immediately
// *** dont allocate anything***
//
static void   _mulle_objc_runtime_set_defaults( struct _mulle_objc_runtime  *runtime,
                                                struct mulle_allocator *allocator)
{
   void    mulle_objc_vprintf_abort( char *format, va_list args);

   assert( runtime);
   assert( MULLE_THREAD_VERSION    >= ((2 << 20) | (2 << 8) | 0));
   assert( MULLE_ALLOCATOR_VERSION >= ((1 << 20) | (5 << 8) | 0));
   assert( MULLE_ABA_VERSION       >= ((1 << 20) | (1 << 8) | 1));

   // check this also, easily fixable with padding
   if( sizeof( struct _mulle_objc_cacheentry) & (sizeof( struct _mulle_objc_cacheentry) - 1))
      abort();

   if( ! allocator)
      allocator = &mulle_default_allocator;

   strncpy( runtime->compilation, __DATE__ " "__TIME__ " " __FILE__, 128);

   assert( allocator->calloc);
   assert( allocator->free);
   assert( allocator->realloc);

   assert( ! _mulle_objc_runtime_is_initialized( runtime)); // != 0!
   runtime->version = MULLE_OBJC_RUNTIME_VERSION;
   _mulle_atomic_pointer_nonatomic_write( &runtime->cachepivot.entries, runtime->empty_cache.entries);

   runtime->memory.allocator         = *allocator;
   mulle_allocator_set_fail( &runtime->memory.allocator, mulle_objc_allocator_fail);

   runtime->memory.allocator.aba     = &runtime->garbage.aba;
   runtime->memory.allocator.abafree = unfailing_abafree;

   runtime->failures.fail             = _mulle_objc_vprintf_abort;
   runtime->failures.inconsistency    = _mulle_objc_vprintf_abort;
   runtime->failures.class_not_found  = _mulle_objc_class_not_found_abort;
   runtime->failures.method_not_found = _mulle_objc_method_not_found_abort;

   runtime->exceptionvectors.throw     = _mulle_objc_runtime_throw;
   runtime->exceptionvectors.try_enter = _mulle_objc_runtime_try_enter;
   runtime->exceptionvectors.try_exit  = _mulle_objc_runtime_try_exit;
   runtime->exceptionvectors.extract   = _mulle_objc_runtime_extract_exception;
   runtime->exceptionvectors.match     = _mulle_objc_runtime_match_exception;

   runtime->classdefaults.inheritance      = MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOL_CATEGORIES;
   runtime->classdefaults.class_is_missing = nop;

   _mulle_concurrent_pointerarray_init( &runtime->staticstrings, 0, &runtime->memory.allocator);
   _mulle_concurrent_pointerarray_init( &runtime->hashnames, 0, &runtime->memory.allocator);
   _mulle_concurrent_pointerarray_init( &runtime->gifts, 0, &runtime->memory.allocator);

   runtime->config.max_optlevel = 0x7;
#if __MULLE_OBJC_TRT__
   runtime->config.thread_local_rt = 1;
#endif
#if __MULLE_OBJC_NO_TPS__
   runtime->config.no_tagged_pointers = 1;
#endif
   _mulle_objc_runtime_set_debug_defaults_from_environment( runtime);
}


void   __mulle_objc_runtime_setup( struct _mulle_objc_runtime *runtime,
                                   struct mulle_allocator *allocator)
{
   _mulle_objc_runtime_set_defaults( runtime, allocator);

   _mulle_concurrent_hashmap_init( &runtime->classtable, 128, &runtime->memory.allocator);
   _mulle_concurrent_hashmap_init( &runtime->descriptortable, 2048, &runtime->memory.allocator);
   _mulle_concurrent_hashmap_init( &runtime->protocoltable, 64, &runtime->memory.allocator);
   _mulle_concurrent_hashmap_init( &runtime->categorytable, 128, &runtime->memory.allocator);

   mulle_thread_mutex_init( &runtime->waitqueues.lock);
   _mulle_concurrent_hashmap_init( &runtime->waitqueues.classestoload, 64, &runtime->memory.allocator);
   _mulle_concurrent_hashmap_init( &runtime->waitqueues.categoriestoload, 32, &runtime->memory.allocator);

   mulle_objc_unfailing_get_or_create_runtimekey();

   mulle_thread_mutex_init( &runtime->lock);
   runtime->thread = mulle_thread_self();

   mulle_objc_set_thread_runtime( runtime);
   _mulle_objc_runtime_register_current_thread_if_needed( runtime);
}


struct _mulle_objc_runtime  *mulle_objc_alloc_runtime( void)
{
   struct _mulle_objc_runtime  *runtime;

   runtime = calloc( 1, sizeof( struct _mulle_objc_runtime));
   if( ! runtime)
      _mulle_objc_perror_abort( "mulle_objc_create_runtime");

   return( runtime);
}


struct _mulle_objc_runtime  *__mulle_objc_get_runtime( void)
{
   struct _mulle_objc_runtime  *runtime;

#if __MULLE_OBJC_TRT__
   if( mulle_objc_runtime_thread_key)
   {
      runtime = __mulle_objc_get_thread_runtime();
      if( ! runtime)
         runtime = mulle_objc_alloc_runtime();
   }
#else
   runtime = __mulle_objc_get_global_runtime();
#endif
   return( runtime);
}


struct _mulle_objc_runtime  *mulle_objc_get_or_create_runtime( void)
{
   struct _mulle_objc_runtime  *runtime;
   extern MULLE_C_CONST_RETURN
   struct _mulle_objc_runtime  *__get_or_create_mulle_objc_runtime( void);

   runtime = __get_or_create_mulle_objc_runtime();  // the external function
   if( ! runtime)
   {
      fprintf( stderr, "__get_or_create_mulle_objc_runtime returned NULL\n");
      abort();
   }
   return( runtime);
}


void  _mulle_objc_runtime_assert_version( struct _mulle_objc_runtime  *runtime,
                                          struct mulle_objc_loadversion *version)
{
   if( (mulle_objc_version_get_major( version->runtime) !=
        mulle_objc_version_get_major( runtime->version)) ||
       //
       // during 0 versions, any minor jump is incompatible
       //
       ( ! mulle_objc_version_get_major( version->runtime) &&
         (mulle_objc_version_get_minor( version->runtime) !=
          mulle_objc_version_get_minor( runtime->version))))
      
   {
      _mulle_objc_runtime_raise_fail_exception( runtime,
         "mulle_objc_runtime %p fatal: runtime version %u.%u.%u (%s) is incompatible with "
         "compiled version %u.%u.%u\n",
            runtime,
            mulle_objc_version_get_major( runtime->version),
            mulle_objc_version_get_minor( runtime->version),
            mulle_objc_version_get_patch( runtime->version),
            _mulle_objc_runtime_get_path( runtime) ? _mulle_objc_runtime_get_path( runtime) : "???",
            mulle_objc_version_get_major( version->runtime),
            mulle_objc_version_get_minor( version->runtime),
            mulle_objc_version_get_patch( version->runtime));
   }
}


# pragma mark - TPS and Loadbits

void  _mulle_objc_runtime_set_loadbit( struct _mulle_objc_runtime *runtime,
                                       uintptr_t bit)
{
   uintptr_t   oldbits;
   uintptr_t   newbits;

   do
   {
      oldbits = _mulle_objc_runtime_get_loadbits( runtime);
      newbits = oldbits | bit;
      if( oldbits != newbits)
         return;
   }
   while( ! _mulle_atomic_pointer_compare_and_swap( &runtime->loadbits, (void *) newbits, (void *) oldbits));
}


//
// it's assumed that there are no competing classes for the spot
//
int  _mulle_objc_runtime_set_taggedpointerclass_at_index( struct _mulle_objc_runtime  *runtime,
                                                          struct _mulle_objc_infraclass *infra,
                                                          unsigned int index)
{
   if( ! index || index > mulle_objc_get_taggedpointer_mask())
      return( -1);

   assert( ! runtime->taggedpointers.pointerclass[ index]);
   if( runtime->debug.trace.tagged_pointers)
      fprintf( stderr, "mulle_objc_runtime %p trace: set tagged pointers with "
                       "index %d to isa %p (class %08x \"%s\" )\n",
                       runtime, index, infra,
                      _mulle_objc_infraclass_get_classid( infra),
                      _mulle_objc_infraclass_get_name( infra));

   runtime->taggedpointers.pointerclass[ index] = _mulle_objc_infraclass_as_class( infra);

   _mulle_objc_runtime_set_loadbit( runtime, MULLE_OBJC_RUNTIME_HAVE_TPS_CLASSES);

   return( 0);
}


# pragma mark - thread local

static void   mulle_objc_thread_runtime_destructor( struct _mulle_objc_threadconfig *config)
{
   _mulle_allocator_free( &config->runtime->memory.allocator, config);
}


mulle_thread_tss_t   mulle_objc_unfailing_get_or_create_runtimekey( void)
{
   extern mulle_thread_tss_t   mulle_objc_runtime_thread_key;

   if( ! mulle_objc_runtime_thread_key_is_intitialized())
   {
      if( mulle_thread_tss_create( (void *) mulle_objc_thread_runtime_destructor,
                                   &mulle_objc_runtime_thread_key))
      {
         _mulle_objc_perror_abort( "mulle_objc_unfailing_get_or_create_runtimekey");
      }
   }
   return( mulle_objc_runtime_thread_key);
}


static void   mulle_objc_unset_thread_runtime( void)
{
   struct _mulle_objc_threadconfig *config;
   extern mulle_thread_tss_t   mulle_objc_runtime_thread_key;

   if( ! mulle_objc_runtime_thread_key_is_intitialized())
      return;

   config = mulle_thread_tss_get( mulle_objc_runtime_thread_key);
   assert( config);
   if( config)
   {
      mulle_thread_tss_set( mulle_objc_runtime_thread_key, NULL);
      mulle_objc_thread_runtime_destructor( config);
   }
}


void   mulle_objc_delete_runtimekey( void)
{
   extern mulle_thread_tss_t   mulle_objc_runtime_thread_key;
   if( mulle_objc_runtime_thread_key_is_intitialized())
      return;

   assert( ! mulle_thread_tss_get( mulle_objc_runtime_thread_key));

   mulle_thread_tss_free( mulle_objc_runtime_thread_key);
   mulle_objc_runtime_thread_key = -1;
}


# pragma mark - dealloc

static void   _mulle_objc_runtime_free_friend( struct _mulle_objc_runtime *runtime,
                                               struct _mulle_objc_runtimefriend *pfriend)
{
   /* we don't mind if pfriend->friend is NULL */
   if( ! pfriend->destructor)
      return;

   (*pfriend->destructor)( runtime, pfriend->data);
}


static void   pointerarray_in_hashmap_map( struct _mulle_objc_runtime *runtime,
                                           struct mulle_concurrent_hashmap *map,
                                           void (*f)( void *, void *))
{
   struct mulle_concurrent_hashmapenumerator  rover;
   struct mulle_concurrent_pointerarray       *list;

   rover = mulle_concurrent_hashmap_enumerate( map);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &list))
   {
      mulle_concurrent_pointerarray_map( list, f, runtime);
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


static void   _mulle_objc_runtime_sort_classes_by_depth( struct _mulle_objc_class **array,
                                                         unsigned int n_classes)
{
   qsort( array, n_classes, sizeof( struct _mulle_objc_class *), (int (*)()) compare_depth_deeper_first);
}


static struct _mulle_objc_class   **_mulle_objc_runtime_all_classes( struct _mulle_objc_runtime *runtime,
                                                                     unsigned int *n_classes,
                                                                     struct mulle_allocator *allocator)
{
   struct _mulle_objc_class                    **p_cls;
   struct _mulle_objc_class                    **array;
   struct mulle_concurrent_hashmapenumerator   rover;

   *n_classes = mulle_concurrent_hashmap_count( &runtime->classtable);
   array      = mulle_allocator_calloc( allocator, *n_classes, sizeof( struct _mulle_objc_classpair *));

   p_cls = array;
   rover = mulle_concurrent_hashmap_enumerate( &runtime->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) p_cls))
      p_cls++;
   mulle_concurrent_hashmapenumerator_done( &rover);

   return( array);
}


static void   _mulle_objc_runtime_free_classpairs( struct _mulle_objc_runtime *runtime)
{
   unsigned int               n_classes;
   struct _mulle_objc_class   **array;
   struct _mulle_objc_class   **p;
   struct _mulle_objc_class   **sentinel;

   array = _mulle_objc_runtime_all_classes( runtime, &n_classes, &runtime->memory.allocator);
   _mulle_objc_runtime_sort_classes_by_depth( array, n_classes);

   p        = array;
   sentinel = &p[ n_classes];
   while( p < sentinel)
   {
      if( runtime->debug.trace.class_frees)
         fprintf( stderr, "mulle_objc_runtime %p trace: destroying class pair %p \"%s\"\n", runtime, _mulle_objc_class_get_classpair( *p), _mulle_objc_class_get_name( *p));

      _mulle_objc_classpair_free( _mulle_objc_class_get_classpair( *p),
                                  &runtime->memory.allocator);
      ++p;
   }

   mulle_allocator_free( &runtime->memory.allocator, array);
}


static void  free_gift( void *p, struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, p);
}


enum mulle_objc_runtime_status  _mulle_objc_runtime_check_waitqueues( struct _mulle_objc_runtime *runtime)
{
   int   rval;

   rval = mulle_objc_runtime_is_ok;
   /*
    * free various stuff
    */
   if( mulle_concurrent_hashmap_count( &runtime->waitqueues.classestoload))
   {
      rval = mulle_objc_runtime_is_incomplete;
      fprintf( stderr, "mulle_objc_runtime %p warning: the following classes failed to load:\n",
                  runtime);
      if( _mulle_objc_runtime_waitqueues_trylock( runtime))
      {
         fprintf( stderr, "mulle_objc_runtime %p error: the waitqueues are still locked!\n", runtime);
         return( mulle_objc_runtime_is_locked);
      }

      pointerarray_in_hashmap_map( runtime, &runtime->waitqueues.classestoload, (void (*)()) mulle_objc_loadclass_print_unfulfilled_dependency);
      _mulle_objc_runtime_waitqueues_unlock( runtime);
   }

   if( mulle_concurrent_hashmap_count( &runtime->waitqueues.categoriestoload))
   {
      rval = mulle_objc_runtime_is_incomplete;
      fprintf( stderr, "mulle_objc_runtime %p warning: the following categories failed to load:\n", runtime);
      if( _mulle_objc_runtime_waitqueues_trylock( runtime))
      {
         fprintf( stderr, "mulle_objc_runtime %p error: the waitqueues are still locked!\n",
                     runtime);
         return( mulle_objc_runtime_is_locked);
      }

      pointerarray_in_hashmap_map( runtime, &runtime->waitqueues.categoriestoload, (void (*)()) mulle_objc_loadcategory_print_unfulfilled_dependency);
      _mulle_objc_runtime_waitqueues_unlock( runtime);
   }
   return( rval);
}


enum mulle_objc_runtime_status   _mulle_objc_check_runtime( uint32_t version)
{
   struct _mulle_objc_runtime   *runtime;
   uint32_t                     runtime_version;

   runtime = mulle_objc_get_runtime();
   if( ! runtime)
      return( mulle_objc_runtime_is_missing);

   runtime_version = _mulle_objc_runtime_get_version( runtime);

   if( mulle_objc_version_get_major( version) != mulle_objc_version_get_major( runtime_version))
      return( mulle_objc_runtime_is_wrong_version);

   // during 0 development, a minor change is major
   if( ! mulle_objc_version_get_major( version) && (mulle_objc_version_get_minor( version) != mulle_objc_version_get_minor( runtime_version)))
      return( mulle_objc_runtime_is_wrong_version);

   return( _mulle_objc_runtime_check_waitqueues( runtime));
}


static void   _mulle_objc_runtime_free_classgraph( struct _mulle_objc_runtime *runtime)
{
   if( runtime->debug.warn.stuck_loadables)
      _mulle_objc_runtime_check_waitqueues( runtime);

   /* free classes */
   _mulle_objc_runtime_free_classpairs( runtime);

   _mulle_concurrent_hashmap_done( &runtime->waitqueues.categoriestoload);
   _mulle_concurrent_hashmap_done( &runtime->waitqueues.classestoload);
   _mulle_concurrent_hashmap_done( &runtime->descriptortable);
   _mulle_concurrent_hashmap_done( &runtime->protocoltable);
   _mulle_concurrent_hashmap_done( &runtime->categorytable);
   _mulle_concurrent_hashmap_done( &runtime->classtable);

   _mulle_concurrent_pointerarray_done( &runtime->staticstrings);
   _mulle_concurrent_pointerarray_done( &runtime->hashnames);

   /* free gifts */
   mulle_concurrent_pointerarray_map( &runtime->gifts, (void (*)()) free_gift, &runtime->memory.allocator);
   _mulle_concurrent_pointerarray_done( &runtime->gifts);
}


static inline void  _mulle_objc_runtime_uninitialize( struct _mulle_objc_runtime *runtime)
{
   runtime->version = -1;
}


void   _mulle_objc_runtime_dealloc( struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_cache    *cache;

   if( runtime->thread && runtime->thread != mulle_thread_self())
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "runtime must be deallocated by the same thread that created it (sorry)");

   mulle_objc_set_thread_runtime( NULL);

   if( ! _mulle_objc_runtime_is_current_thread_registered( runtime))
      _mulle_objc_runtime_register_current_thread( runtime);

   _mulle_objc_runtime_free_friend( runtime, &runtime->userinfo);
   _mulle_objc_runtime_free_friend( runtime, &runtime->foundation.runtimefriend);

   _mulle_objc_runtime_free_classgraph( runtime);

   cache = _mulle_objc_cachepivot_atomic_get_cache( &runtime->cachepivot);
   if( cache != &runtime->empty_cache)
      _mulle_objc_cache_free( cache, &runtime->memory.allocator);
   _mulle_objc_runtime_unregister_current_thread( runtime);

   _mulle_aba_done( &_mulle_objc_runtime_get_garbagecollection( runtime)->aba);
   mulle_thread_mutex_done( &runtime->lock);

   mulle_objc_delete_runtimekey();

   _mulle_objc_runtime_uninitialize( runtime);

   if( _mulle_objc_is_global_runtime( runtime))
      return;

   free( runtime);
}


# pragma mark - runtime release convenience

void  mulle_objc_release_runtime( void)
{
   struct _mulle_objc_runtime *runtime;

   runtime = __mulle_objc_get_runtime();
   if( runtime && _mulle_objc_runtime_is_initialized( runtime))
      _mulle_objc_runtime_release( runtime);
}


struct _mulle_objc_garbagecollection  *_mulle_objc_runtime_get_garbagecollection( struct _mulle_objc_runtime *runtime)
{
   assert( _mulle_objc_runtime_is_initialized( runtime));

   if( ! _mulle_aba_is_setup( &runtime->garbage.aba))
   {
      if( _mulle_aba_init( &runtime->garbage.aba, &runtime->memory.allocator))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }
   return( &runtime->garbage);
}


void   _mulle_objc_runtime_register_current_thread( struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_garbagecollection   *gc;

   gc = _mulle_objc_runtime_get_garbagecollection( runtime);
   if( _mulle_aba_register_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_register_current_thread");
}


int    _mulle_objc_runtime_is_current_thread_registered( struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_garbagecollection   *gc;

   gc = _mulle_objc_runtime_get_garbagecollection( runtime);
   return( _mulle_aba_is_current_thread_registered( &gc->aba));
}


void   _mulle_objc_runtime_register_current_thread_if_needed( struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_garbagecollection   *gc;

   gc = _mulle_objc_runtime_get_garbagecollection( runtime);
   if( _mulle_aba_is_current_thread_registered( &gc->aba))
      return;

   if( _mulle_aba_register_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_register_current_thread");
}


void   _mulle_objc_runtime_unregister_current_thread( struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_garbagecollection   *gc;

   gc = _mulle_objc_runtime_get_garbagecollection( runtime);
   if( _mulle_aba_unregister_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_unregister_current_thread");
}


void   _mulle_objc_runtime_checkin_current_thread( struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_garbagecollection   *gc;

   gc = _mulle_objc_runtime_get_garbagecollection( runtime);
   if( _mulle_aba_checkin_current_thread( &gc->aba))
      _mulle_objc_perror_abort( "_mulle_aba_checkin_current_thread");
}

//
// admittedly its fairly tricky, to set mulle_objc_globals before any of the
// class loads hit...
//
# pragma mark - "exceptions"


long   __mulle_objc_personality_v0 = 1848;  // no idea what this is used for

MULLE_C_NO_RETURN
void   _mulle_objc_runtime_raise_fail_errno_exception( struct _mulle_objc_runtime *runtime)
{
   _mulle_objc_runtime_raise_fail_exception( runtime, "errno: %s (%d)", strerror( errno), errno);
}


MULLE_C_NO_RETURN
void   _mulle_objc_runtime_raise_inconsistency_exception( struct _mulle_objc_runtime *runtime, char *format, ...)
{
   va_list   args;

   va_start( args, format);

   //
   // inconsistency ? even runtime might not be setup properly. This is the only
   // exception doing this check
   //
   if( ! runtime || ! runtime->failures.inconsistency)
   {
      vfprintf( stderr, format, args);
      abort();;
   }
   (*runtime->failures.inconsistency)( format, args);
   va_end( args);
}


MULLE_C_NO_RETURN
void   _mulle_objc_runtime_raise_class_not_found_exception( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t classid)
{
   (*runtime->failures.class_not_found)( runtime, classid);
}


MULLE_C_NO_RETURN
void   _mulle_objc_class_raise_method_not_found_exception( struct _mulle_objc_class *cls, mulle_objc_methodid_t methodid)
{
   (*cls->runtime->failures.method_not_found)( cls, methodid);
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


void   _mulle_objc_runtime_throw( struct _mulle_objc_runtime *runtime, void *exception)
{
   struct _mulle_objc_exceptionstackentry  *entry;
   struct _mulle_objc_threadconfig         *config;

   config = mulle_objc_get_threadconfig();
   entry  = config->exception_stack;
   if( ! entry)
   {
      if( runtime->failures.uncaughtexception)
         (*runtime->failures.uncaughtexception)( exception);

      fprintf( stderr, "uncaught exception %p", exception);
      abort();
   }

   entry->exception        = exception;
   config->exception_stack = entry->previous;

   // from Apple objc_runtime.mm
#if _WIN32
    longjmp( entry->buf, 1);
#else
    _longjmp( entry->buf, 1);
#endif
}


//
//  objc_exception_try_enter pushes a catch buffer onto the EH stack.
//
void   _mulle_objc_runtime_try_enter( struct _mulle_objc_runtime *runtime,
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
void   _mulle_objc_runtime_try_exit( struct _mulle_objc_runtime *runtime,
                                     void *data)
{
   struct _mulle_objc_exceptionstackentry *entry = data;
   struct _mulle_objc_threadconfig        *config;

   config                  = mulle_objc_get_threadconfig();
   config->exception_stack = entry->previous;
}


void   *_mulle_objc_runtime_extract_exception( struct _mulle_objc_runtime *runtime,
                                               void *data)
{
   struct _mulle_objc_exceptionstackentry *entry = data;

   return( entry->exception);
}


int  _mulle_objc_runtime_match_exception( struct _mulle_objc_runtime *runtime,
                                          mulle_objc_classid_t classid,
                                          void *exception)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_class        *exceptionCls;

   assert( classid != MULLE_OBJC_NO_CLASSID && classid != MULLE_OBJC_INVALID_CLASSID);
   assert( exception);

   infra        = _mulle_objc_runtime_unfailing_get_or_lookup_infraclass( runtime, classid);
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


#ifndef MULLE_OBJC_NO_CONVENIENCES

// conveniences
void   mulle_objc_raise_fail_exception( char *format, ...)
{
   va_list   args;

   va_start( args, format);
   _mulle_objc_runtime_raise_fail_exception( mulle_objc_get_or_create_runtime(), format, args);
   va_end( args);
}


void   mulle_objc_raise_fail_errno_exception( void)
{
   _mulle_objc_runtime_raise_fail_errno_exception( mulle_objc_get_or_create_runtime());
}


void   mulle_objc_raise_inconsistency_exception( char *format, ...)
{
   va_list   args;

   va_start( args, format);
   _mulle_objc_runtime_raise_inconsistency_exception( mulle_objc_get_or_create_runtime(), format, args);
   va_end( args);
}
#endif


void   mulle_objc_raise_taggedpointer_exception( void *obj)
{
   _mulle_objc_runtime_raise_inconsistency_exception( mulle_objc_get_or_create_runtime(), "%p is a tagged pointer", obj);
}


# pragma mark - "classes"

//
// can be useful if you are using the thread local runtime, and the
// const thing doesn't work to your advantage
//
struct _mulle_objc_runtime  *mulle_objc_get_runtime( void)
{
#if __MULLE_OBJC_TRT__
   return( mulle_objc_get_thread_runtime());
#else
   return( mulle_objc_get_global_runtime());
#endif
}


void  mulle_objc_set_thread_runtime( struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_threadconfig    *config;

   if( ! runtime)
   {
      mulle_objc_unset_thread_runtime();
      return;
   }

   // don't use gifting calloc, will be cleaned separately
   config = _mulle_allocator_calloc( _mulle_objc_runtime_get_allocator( runtime), 1, sizeof( struct _mulle_objc_threadconfig));

   config->runtime = runtime;
   mulle_thread_tss_set( mulle_objc_unfailing_get_or_create_runtimekey(), config);
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
struct _mulle_objc_classpair
   *mulle_objc_runtime_new_classpair( struct _mulle_objc_runtime *runtime,
                                      mulle_objc_classid_t  classid,
                                      char *name,
                                      size_t instancesize,
                                      struct _mulle_objc_infraclass *superclass)
{
   struct _mulle_objc_classpair       *pair;
   struct _mulle_objc_metaclass       *super_meta;
   struct _mulle_objc_metaclass       *super_meta_isa;
   size_t                             size;

   if( ! runtime || ! runtime->memory.allocator.calloc)
   {
      errno = EINVAL;
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

   assert( mulle_objc_classid_from_string( name) == classid);

   super_meta     = NULL;
   super_meta_isa = NULL;
   if( superclass)
   {
      super_meta     = _mulle_objc_infraclass_get_metaclass( superclass);
      super_meta_isa = _mulle_objc_class_get_metaclass( &super_meta->base);
      assert( super_meta_isa);
   }

   // classes are freed by hand so don't use gifting calloc
   // TODO: is the sizeof( uint32_t) a code remnant ?
   size = sizeof( struct _mulle_objc_classpair) + sizeof( uint32_t);
   pair = _mulle_allocator_calloc( _mulle_objc_runtime_get_allocator( runtime), 1, size);

   _mulle_objc_objectheader_init( &pair->metaclassheader,
                                  super_meta_isa ? &super_meta_isa->base : &pair->infraclass.base);
   _mulle_objc_objectheader_init( &pair->infraclassheader, &pair->metaclass.base);

   _mulle_objc_class_init( &pair->infraclass.base,
                           name,
                           instancesize,
                           classid,
                           &superclass->base,
                           runtime);
   _mulle_objc_class_init( &pair->metaclass.base,
                           name,
                           sizeof( struct _mulle_objc_class),
                           classid,
                           super_meta ? &super_meta->base : &pair->infraclass.base,
                           runtime);

   _mulle_objc_infraclass_plusinit( &pair->infraclass, &runtime->memory.allocator);
   _mulle_objc_metaclass_plusinit( &pair->metaclass, &runtime->memory.allocator);
   _mulle_objc_classpair_plusinit( pair, &runtime->memory.allocator);

   _mulle_objc_class_set_infraclass( &pair->metaclass.base, &pair->infraclass);

   return( pair);
}


// convenience during loading
struct _mulle_objc_classpair   *mulle_objc_unfailing_new_classpair( mulle_objc_classid_t  classid,
                                                                    char *name,
                                                                    size_t instancesize,
                                                                    struct _mulle_objc_infraclass *superclass)
{
   struct _mulle_objc_classpair     *pair;
   struct _mulle_objc_runtime       *runtime;

   runtime = mulle_objc_get_or_create_runtime();
   pair     = mulle_objc_runtime_new_classpair( runtime, classid, name, instancesize, superclass);
   if( ! pair)
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);  // unfailing vectors through there
   return( pair);
}


/* don't check for ivar_hash, as this is too painful for application
   runtime hacks. Only during loading
 */
int   mulle_objc_runtime_add_infraclass( struct _mulle_objc_runtime *runtime,
                                         struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_infraclass   *superclass;

   if( ! runtime || ! infra)
   {
      errno = EINVAL;
      return( -1);
   }

   meta  = _mulle_objc_infraclass_get_metaclass( infra);

   if( ! mulle_objc_infraclass_is_sane( infra) ||
       ! mulle_objc_metaclass_is_sane( meta))
   {
      errno = EINVAL;
      return( -1);
   }

   if( _mulle_concurrent_hashmap_lookup( &runtime->classtable, infra->base.classid))
   {
      errno = EEXIST;
      return( -1);
   }

   superclass = _mulle_objc_infraclass_get_superclass( infra);
   if( superclass)
   {
      if( ! _mulle_concurrent_hashmap_lookup( &runtime->classtable,
                               _mulle_objc_infraclass_get_classid( superclass)))
      {
         errno = EFAULT;
         return( -1);
      }
   }

   if( runtime->debug.trace.class_adds || runtime->debug.trace.dependencies)
   {
      fprintf( stderr, "mulle_objc_runtime %p trace: add class %08x \"%s\"",
                 runtime,
                 _mulle_objc_infraclass_get_classid( infra),
                 _mulle_objc_infraclass_get_name( infra));
      if( superclass)
         fprintf( stderr, " with superclass %08x \"%s\"",
                 _mulle_objc_infraclass_get_classid( superclass),
                 _mulle_objc_infraclass_get_name( superclass));
      fprintf( stderr, " (-:%p +:%p)\n", infra, meta);
   }

   return( _mulle_concurrent_hashmap_insert( &runtime->classtable,
                                             infra->base.classid,
                                             infra));
}


void   mulle_objc_runtime_unfailing_add_infraclass( struct _mulle_objc_runtime *runtime,
                                                    struct _mulle_objc_infraclass *infra)
{
   if( mulle_objc_runtime_add_infraclass( runtime, infra))
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
}


void   _mulle_objc_runtime_set_fastclass( struct _mulle_objc_runtime *runtime,
                                          struct _mulle_objc_infraclass *infra,
                                          unsigned int index)
{
   struct _mulle_objc_infraclass  *old;

   assert( runtime);
   assert( infra);

   if( runtime->debug.trace.fastclass_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: add fastclass \"%s\" at index %d\n", runtime, infra->base.name, index);

   if( index >= MULLE_OBJC_S_FASTCLASSES)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: "
            "fastclass index %d for %s (id %08lx) out of bounds\n",
             runtime, index, _mulle_objc_infraclass_get_name( infra), _mulle_objc_infraclass_get_classid( infra));

   if( ! _mulle_atomic_pointer_compare_and_swap( &runtime->fastclasstable.classes[ index].pointer, infra, NULL))
   {
      old = _mulle_atomic_pointer_read( &runtime->fastclasstable.classes[ index].pointer);
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: classes \"%s\" "
                                                        "and \"%s\", both want to occupy fastclass spot %u",
                                                        runtime,
                                                        _mulle_objc_infraclass_get_name( infra), _mulle_objc_infraclass_get_name( old), index);
   }
}


#ifndef MULLE_OBJC_NO_CONVENIENCES
// conveniences

void   mulle_objc_unfailing_add_infraclass( struct _mulle_objc_infraclass *infra)
{
   mulle_objc_runtime_unfailing_add_infraclass( mulle_objc_get_or_create_runtime(), infra);
}

#endif


# pragma mark - methods

struct _mulle_objc_methoddescriptor   *_mulle_objc_runtime_lookup_methoddescriptor( struct _mulle_objc_runtime *runtime, mulle_objc_methodid_t methodid)
{
   return( _mulle_concurrent_hashmap_lookup( &runtime->descriptortable, methodid));
}


int   _mulle_objc_runtime_add_methoddescriptor( struct _mulle_objc_runtime *runtime, struct _mulle_objc_methoddescriptor *p)
{
   struct _mulle_objc_methoddescriptor   *dup;

   if( ! mulle_objc_methoddescriptor_is_sane( p))
      return( -1);

   dup = _mulle_concurrent_hashmap_lookup( &runtime->descriptortable, p->methodid);
   if( ! dup)
      return( _mulle_concurrent_hashmap_insert( &runtime->descriptortable, p->methodid, p));

   if( strcmp( dup->name, p->name))
   {
      errno = EEXIST;
      return( -1);
   }

   if( runtime->debug.warn.methodid_types)
   {
      int   comparison;

      if( runtime->debug.warn.pedantic_methodid_types)
         comparison = _mulle_objc_signature_pedantic_compare( dup->signature, p->signature);
      else
         comparison = _mulle_objc_signature_compare( dup->signature, p->signature);
      if( comparison)
         fprintf( stderr, "mulle_objc_runtime %p warning: varying types \"%s\" and \"%s\" for method \"%s\"\n",
                 runtime,
                 dup->signature, p->signature, p->name);
   }
   return( 0);
}


void    mulle_objc_runtime_unfailing_add_methoddescriptor( struct _mulle_objc_runtime *runtime, struct _mulle_objc_methoddescriptor *p)
{
   struct _mulle_objc_methoddescriptor   *dup;

   assert( runtime);

   if( _mulle_objc_runtime_add_methoddescriptor( runtime, p))
   {
      dup = _mulle_objc_runtime_lookup_methoddescriptor( runtime, p->methodid);
      _mulle_objc_runtime_raise_fail_exception( runtime, "mulle_objc_runtime %p error: duplicate methods \"%s\" and \"%s\" with same id %08lx\n", runtime, dup->name, p->name, (long) p->methodid);
   }
}


char   *mulle_objc_lookup_methodname( mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_runtime            *runtime;
   struct _mulle_objc_methoddescriptor   *desc;

   runtime = mulle_objc_get_or_create_runtime();
   desc    = _mulle_objc_runtime_lookup_methoddescriptor( runtime, methodid);
   return( desc ? desc->name : NULL);
}


char   *mulle_objc_search_debughashname( mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_runtime            *runtime;

   runtime = mulle_objc_get_or_create_runtime();
   return( _mulle_objc_runtime_search_debughashname( runtime, uniqueid));
}


# pragma mark - method cache

// cheap conveniences
#ifndef MULLE_OBJC_NO_CONVENIENCES
struct _mulle_objc_methodlist  *mulle_objc_alloc_methodlist( unsigned int n)
{
   return( mulle_objc_runtime_calloc( mulle_objc_get_or_create_runtime(),
                                      1,
                                      mulle_objc_sizeof_methodlist( n)));
}

#endif


# pragma mark - protocols

struct _mulle_objc_protocol   *_mulle_objc_runtime_lookup_protocol( struct _mulle_objc_runtime *runtime,
                                                                    mulle_objc_protocolid_t protocolid)
{
   return( _mulle_concurrent_hashmap_lookup( &runtime->protocoltable, protocolid));
}


int   _mulle_objc_runtime_add_protocol( struct _mulle_objc_runtime *runtime,
                                        struct _mulle_objc_protocol *protocol)

{
   struct _mulle_objc_methoddescriptor   *dup;

   if( protocol->protocolid == MULLE_OBJC_NO_PROTOCOLID || protocol->protocolid == MULLE_OBJC_INVALID_PROTOCOLID)
      return( -1);

   dup = _mulle_concurrent_hashmap_lookup( &runtime->protocoltable, protocol->protocolid);
   if( ! dup)
      return( _mulle_concurrent_hashmap_insert( &runtime->protocoltable, protocol->protocolid, protocol));

   if( strcmp( dup->name, protocol->name))
   {
      errno = EEXIST;
      return( -1);
   }

   return( 0);
}


void    mulle_objc_runtime_unfailing_add_protocol( struct _mulle_objc_runtime *runtime,
                                                   struct _mulle_objc_protocol *protocol)
{
   struct _mulle_objc_protocol  *dup;

   assert( runtime);

   if( _mulle_objc_runtime_add_protocol( runtime, protocol))
   {
      dup = _mulle_objc_runtime_lookup_protocol( runtime, protocol->protocolid);
      _mulle_objc_runtime_raise_fail_exception( runtime, "mulle_objc_runtime %p error: duplicate protocols \"%s\" and \"%s\" with same id %08x\n", runtime, dup->name, protocol->name, protocol->protocolid);
   }
}


struct _mulle_objc_protocol  *mulle_objc_lookup_protocol( mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_get_or_create_runtime();
   return( _mulle_objc_runtime_lookup_protocol( runtime, protocolid));
}

#ifndef MULLE_OBJC_NO_CONVENIENCES
struct _mulle_objc_protocollist  *mulle_objc_alloc_protocollist( unsigned int n)
{
   return( mulle_objc_runtime_calloc( mulle_objc_get_or_create_runtime(),
                                      1,
                                      mulle_objc_sizeof_protocollist( n)));
}
#endif


# pragma mark - categories

char   *_mulle_objc_runtime_lookup_category( struct _mulle_objc_runtime *runtime,
                                             mulle_objc_categoryid_t categoryid)
{
   return( _mulle_concurrent_hashmap_lookup( &runtime->categorytable, categoryid));
}


int   _mulle_objc_runtime_add_category( struct _mulle_objc_runtime *runtime,
                                        mulle_objc_categoryid_t categoryid,
                                        char *name)

{
   char   *dup;

   if( categoryid == MULLE_OBJC_NO_CATEGORYID || categoryid == MULLE_OBJC_INVALID_CATEGORYID)
      return( -1);

   dup = _mulle_concurrent_hashmap_lookup( &runtime->categorytable, categoryid);
   if( ! dup)
      return( _mulle_concurrent_hashmap_insert( &runtime->categorytable, categoryid, name));

   if( strcmp( dup, name))
   {
      errno = EEXIST;
      return( -1);
   }

   return( 0);
}


void    mulle_objc_runtime_unfailing_add_category( struct _mulle_objc_runtime *runtime,
                                                   mulle_objc_categoryid_t categoryid,
                                                   char *name)
{
   char  *dup;

   assert( runtime);

   if( _mulle_objc_runtime_add_category( runtime, categoryid, name))
   {
      dup = _mulle_objc_runtime_lookup_category( runtime, categoryid);
      _mulle_objc_runtime_raise_fail_exception( runtime, "mulle_objc_runtime %p error: duplicate categories \"%s\" and \"%s\" with same id %08x\n", runtime, dup, name, categoryid);
   }
}


char   *mulle_objc_lookup_category( mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_get_or_create_runtime();
   return( _mulle_objc_runtime_lookup_category( runtime, categoryid));
}


# pragma mark - string cache

//
// this cache is needed until, the foundation sets the proper string class
// "usually" the runtime stores all these strings, so that all objects in
// the runtime system can be accounted for
//

struct _NSConstantString
{
   char           *_storage;   // ivar #0:: must be defined EXACTLY like this
   unsigned int   _length;     // ivar #1:: must be defined EXACTLY like this
};


void   _mulle_objc_runtime_add_staticstring( struct _mulle_objc_runtime *runtime,
                                             struct _mulle_objc_object *string)
{
   assert( runtime);
   assert( string);

   if( ! runtime->foundation.staticstringclass)
   {
      if( runtime->debug.trace.string_adds)
         fprintf( stderr, "mulle_objc_runtime %p trace: delay add of string @\"%s\" at %p\n",
               runtime, ((struct _NSConstantString *) string)->_storage, string);

      // memorize it anyway
      _mulle_concurrent_pointerarray_add( &runtime->staticstrings, (void *) string);
      return;
   }

   _mulle_objc_object_set_isa( string,
                               _mulle_objc_infraclass_as_class( runtime->foundation.staticstringclass));
   if( runtime->debug.trace.string_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: add string @\"%s\" at %p\n",
            runtime, ((struct _NSConstantString *) string)->_storage, string);

   if( ! runtime->config.forget_strings)
      _mulle_concurrent_pointerarray_add( &runtime->staticstrings, (void *) string);
}


void   _mulle_objc_runtime_staticstringclass_did_change( struct _mulle_objc_runtime *runtime)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_object                        *string;
   int                                              flag;

   flag  = runtime->debug.trace.string_adds;
   rover = mulle_concurrent_pointerarray_enumerate( &runtime->staticstrings);
   while( string = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      _mulle_objc_object_set_isa( string,
                                 _mulle_objc_infraclass_as_class( runtime->foundation.staticstringclass));
      if( flag)
         fprintf( stderr, "mulle_objc_runtime %p trace: patch string class @\"%s\" at %p\n",
               runtime, ((struct _NSConstantString *) string)->_storage, string);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   // if so configured wipe the list
   // effectivey _mulle_objc_runtime_staticstringclass_did_change should then
   // only be called once ever, which is its intention anyway
   //
   if( runtime->config.forget_strings)
   {
      _mulle_concurrent_pointerarray_done( &runtime->staticstrings);
      _mulle_concurrent_pointerarray_init( &runtime->staticstrings,
                                           0,
                                           &runtime->memory.allocator);
   }
}


void  _mulle_objc_runtime_set_staticstringclass( struct _mulle_objc_runtime *runtime,
                                                 struct _mulle_objc_infraclass *infra)
{
   assert( runtime);
   assert( infra);

   runtime->foundation.staticstringclass = infra;
   _mulle_objc_runtime_staticstringclass_did_change( runtime);
}


# pragma mark - hashnames (debug output only)

void   _mulle_objc_runtime_add_loadhashedstringlist( struct _mulle_objc_runtime *runtime,
                                                          struct _mulle_objc_loadhashedstringlist *hashnames)
{
   _mulle_concurrent_pointerarray_add( &runtime->hashnames, (void *) hashnames);
}


char  *_mulle_objc_runtime_search_debughashname( struct _mulle_objc_runtime *runtime,
                                                 mulle_objc_uniqueid_t hash)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_loadhashedstringlist          *map;
   char                                             *s;

   s     = NULL;
   rover = mulle_concurrent_pointerarray_enumerate( &runtime->hashnames);
   while( map = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      s = mulle_objc_loadhashedstringlist_bsearch( map, hash);
      if( s)
         break;
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   return( s);
}


char   *mulle_objc_string_for_uniqueid( mulle_objc_uniqueid_t uniqueid)
{
   char   *s;
   struct _mulle_objc_runtime   *runtime;
   
   if( uniqueid == MULLE_OBJC_NO_UNIQUEID)
      return( "NOT A ID");
   if( uniqueid == MULLE_OBJC_INVALID_UNIQUEID)
      return( "INVALID ID");
   runtime = mulle_objc_get_or_create_runtime();
   s       = _mulle_objc_runtime_search_debughashname( runtime, uniqueid);
   return( s ? s : "???");
}


char   *mulle_objc_string_for_classid( mulle_objc_classid_t classid)
{
   struct _mulle_objc_runtime      *runtime;
   struct _mulle_objc_infraclass   *infra;
   
   runtime = mulle_objc_get_or_create_runtime();
   infra   = _mulle_objc_runtime_lookup_uncached_infraclass( runtime, classid);
   if( infra)
      return( _mulle_objc_infraclass_get_name( infra));
   return( mulle_objc_string_for_uniqueid( classid));
}


char   *mulle_objc_string_for_methodid( mulle_objc_methodid_t methodid)
{
   struct _mulle_objc_runtime            *runtime;
   struct _mulle_objc_methoddescriptor   *desc;
   
   runtime = mulle_objc_get_or_create_runtime();
   desc   = _mulle_objc_runtime_lookup_methoddescriptor( runtime, methodid);
   if( desc)
      return( _mulle_objc_methoddescriptor_get_name( desc));
   return( mulle_objc_string_for_uniqueid( methodid));
}


char   *mulle_objc_string_for_protocolid( mulle_objc_protocolid_t protocolid)
{
   struct _mulle_objc_runtime    *runtime;
   struct _mulle_objc_protocol   *protocol;
   
   runtime  = mulle_objc_get_or_create_runtime();
   protocol = _mulle_objc_runtime_lookup_protocol( runtime, protocolid);
   if( protocol)
      return( _mulle_objc_protocol_get_name( protocol));
   return( mulle_objc_string_for_uniqueid( protocolid));
}


char   *mulle_objc_string_for_categoryid( mulle_objc_categoryid_t categoryid)
{
   struct _mulle_objc_runtime    *runtime;
   char                          *name;
   
   runtime  = mulle_objc_get_or_create_runtime();
   name    = _mulle_objc_runtime_lookup_category( runtime, categoryid);
   if( name)
      return( name);
   return( mulle_objc_string_for_uniqueid( categoryid));
}


# pragma mark - debug support

/* debug support */

mulle_objc_walkcommand_t
    mulle_objc_runtime_walk( struct _mulle_objc_runtime *runtime,
                             mulle_objc_walkcallback_t   callback,
                             void *userinfo)
{
   mulle_objc_walkcommand_t           cmd;
   struct _mulle_objc_class                   *cls;
   struct mulle_concurrent_hashmapenumerator  rover;

   if( ! runtime || ! callback)
      return( mulle_objc_walk_error);

   cmd = (*callback)( runtime, runtime, mulle_objc_walkpointer_is_runtime, NULL, NULL, userinfo);
   if( mulle_objc_walkcommand_is_stopper( cmd))
      return( cmd);

   rover = mulle_concurrent_hashmap_enumerate( &runtime->classtable);  // slow!
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
   _mulle_objc_runtime_walk_classes( struct _mulle_objc_runtime  *runtime,
                                     int with_meta,
                                     mulle_objc_walkcallback_t callback,
                                     void *userinfo)
{
   mulle_objc_walkcommand_t                    cmd;
   struct _mulle_objc_infraclass               *cls;
   struct _mulle_objc_metaclass                *meta;
   struct mulle_concurrent_hashmapenumerator   rover;

   if( ! runtime || ! callback)
      return( mulle_objc_walk_error);

   cmd   = mulle_objc_walk_done;
   rover = mulle_concurrent_hashmap_enumerate( &runtime->classtable);  // slow!
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &cls))
   {
      cmd = (*callback)( runtime, cls, mulle_objc_walkpointer_is_infraclass, NULL, NULL, userinfo);
      if( mulle_objc_walkcommand_is_stopper( cmd))
         return( cmd);

      if( with_meta)
      {
         meta = _mulle_objc_infraclass_get_metaclass( cls);
         if( meta)
         {
            cmd = (*callback)( runtime, meta, mulle_objc_walkpointer_is_metaclass, NULL, NULL, userinfo);
            if( mulle_objc_walkcommand_is_stopper( cmd))
               return( cmd);
         }
      }
   }
   mulle_concurrent_hashmapenumerator_done( &rover);

   return( cmd);
}

