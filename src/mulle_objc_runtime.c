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

//

//

#include "mulle_objc_runtime.h"

#include "mulle_objc_runtime_global.h"
#include "mulle_objc_class.h"
#include "mulle_objc_class_runtime.h"
#include "mulle_objc_object.h"

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


void   mulle_objc_runtime_dump_to_tmp( void);

void   mulle_objc_runtime_dump_to_tmp( void)
{
   void   mulle_objc_dump_runtime_as_html_to_tmp( void);
   void   mulle_objc_runtime_dump_graphviz_tmp( void);

   struct _mulle_objc_runtime   *runtime;
   size_t                       count;

   runtime = mulle_objc_get_runtime();
   count   = mulle_concurrent_hashmap_count( &runtime->classtable);
   if( count > 5)
      mulle_objc_dump_runtime_as_html_to_tmp();
   else
      mulle_objc_runtime_dump_graphviz_tmp();
}


MULLE_C_NO_RETURN MULLE_C_NEVER_INLINE
static void   _mulle_objc_printf_abort( char *format, ...)
{
   va_list   args;

   va_start( args, format);

   vfprintf( stderr, format, args);
   fprintf( stderr, "\n");

   va_end( args);

#if DEBUG
   mulle_objc_runtime_dump_to_tmp();
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
static void   _mulle_objc_class_not_found_abort( struct _mulle_objc_runtime *runtime, mulle_objc_classid_t missing_class)
{
   _mulle_objc_printf_abort( "mulle_objc_runtime: missing class with id %08x",
                            missing_class);
}


MULLE_C_NO_RETURN
static void   _mulle_objc_method_not_found_abort( struct _mulle_objc_class *cls,
                                                 mulle_objc_methodid_t missing_method)
{
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

   if( ! methodname)
   {
      desc = _mulle_objc_runtime_lookup_methoddescriptor( cls->runtime, missing_method);
      if( desc)
         methodname = desc->name;
      else
         methodname = _mulle_objc_runtime_search_debughashname(cls->runtime, missing_method);
   }

   if( ! methodname)
      _mulle_objc_printf_abort( "mulle_objc_runtime: missing %s method with id %08x in class \"%s\"",
                                _mulle_objc_class_is_metaclass( cls) ? "class" : "instance",
                                missing_method,
                                name);
   _mulle_objc_printf_abort( "mulle_objc_runtime: missing method \"%c%s\" (id %08x) in class \"%s\"",
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


static void   _mulle_objc_runtime_set_debug_defaults_from_environment( struct _mulle_objc_runtime  *runtime)
{
   runtime->debug.warn.methodid_types        = getenv( "MULLE_OBJC_WARN_METHODID_TYPES") != NULL;
   runtime->debug.warn.protocol_class        = getenv( "MULLE_OBJC_WARN_PROTOCOL_CLASS") != NULL;
   runtime->debug.warn.not_loaded_classes    = getenv( "MULLE_OBJC_WARN_NOTLOADED_CLASSES") != NULL;
   runtime->debug.warn.not_loaded_categories = getenv( "MULLE_OBJC_WARN_NOTLOADED_CATEGORIES") != NULL;

#if ! DEBUG
   if( getenv( "MULLE_OBJC_WARN_ENABLED"))
#endif
   {
      runtime->debug.warn.methodid_types        = 1;
      runtime->debug.warn.protocol_class        = 1;
      runtime->debug.warn.not_loaded_categories = 1;
      runtime->debug.warn.not_loaded_categories = 1;
   }

   runtime->debug.trace.runtime_config     = getenv( "MULLE_OBJC_TRACE_RUNTIME_CONFIG") != NULL;
   runtime->debug.trace.category_adds      = getenv( "MULLE_OBJC_TRACE_CATEGORY_ADDS") != NULL;
   runtime->debug.trace.class_adds         = getenv( "MULLE_OBJC_TRACE_CLASS_ADDS") != NULL;
   runtime->debug.trace.class_frees        = getenv( "MULLE_OBJC_TRACE_CLASS_FREES") != NULL;
   runtime->debug.trace.delayed_class_adds = getenv( "MULLE_OBJC_TRACE_DELAYED_CLASS_ADDS") != NULL;
   runtime->debug.trace.fastclass_adds     = getenv( "MULLE_OBJC_TRACE_FASTCLASS_ADDS") != NULL;
   runtime->debug.trace.method_calls       = getenv( "MULLE_OBJC_TRACE_METHOD_CALLS") != NULL;  // totally excessive!
   runtime->debug.trace.method_searches    = getenv( "MULLE_OBJC_TRACE_METHOD_SEARCHES") != NULL;  // fairly excessive!
   runtime->debug.trace.load_calls         = getenv( "MULLE_OBJC_TRACE_LOAD_CALLS") != NULL;
   runtime->debug.trace.string_adds        = getenv( "MULLE_OBJC_TRACE_STRING_ADDS") != NULL;
   runtime->debug.trace.tagged_pointers    = getenv( "MULLE_OBJC_TRACE_TAGGED_POINTERS") != NULL;

   runtime->debug.trace.delayed_category_adds = getenv( "MULLE_OBJC_TRACE_DELAYED_CLASS_ADDS") != NULL;

   // don't trace method search and calls, per default... too expensive
   if( getenv( "MULLE_OBJC_TRACE_ENABLED"))
   {
      runtime->debug.trace.category_adds         = 1;
      runtime->debug.trace.class_adds            = 1;
      runtime->debug.trace.class_frees           = 1;
      runtime->debug.trace.delayed_class_adds    = 1;
      runtime->debug.trace.delayed_category_adds = 1;
      runtime->debug.trace.fastclass_adds        = 1;
      runtime->debug.trace.string_adds           = 1;
      runtime->debug.trace.tagged_pointers       = 1;
      runtime->debug.trace.runtime_config        = 1;
      runtime->debug.trace.load_calls            = 1;
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

   assert( ! _mulle_objc_runtime_is_initalized( runtime)); // != 0!
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

   _mulle_concurrent_pointerarray_init( &runtime->staticstrings, 16, &runtime->memory.allocator);
   _mulle_concurrent_pointerarray_init( &runtime->hashnames, 16, &runtime->memory.allocator);
   _mulle_concurrent_pointerarray_init( &runtime->gifts, 16, &runtime->memory.allocator);

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
   _mulle_concurrent_hashmap_init( &runtime->classestoload, 64, &runtime->memory.allocator);
   _mulle_concurrent_hashmap_init( &runtime->categoriestoload, 32, &runtime->memory.allocator);

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


void  _mulle_objc_runtime_assert_version( struct _mulle_objc_runtime  *runtime,
                                          struct mulle_objc_loadversion *version)
{
   if( mulle_objc_version_get_major( version->runtime) !=
       mulle_objc_version_get_major( runtime->version))
   {
      errno = ENOEXEC;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   //
   // during 0 versions, any minor jump is incompatible
   //
   if( ! mulle_objc_version_get_major( version->runtime) &&
         (mulle_objc_version_get_minor( version->runtime) !=
          mulle_objc_version_get_minor( runtime->version)))
   {
      errno = ENOEXEC;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
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
                                                          struct _mulle_objc_class *cls,
                                                          unsigned int index)
{
   if( ! index || index > mulle_objc_get_taggedpointer_mask())
      return( -1);

   assert( ! runtime->taggedpointers.pointerclass[ index]);
   if( runtime->debug.trace.tagged_pointers)
      fprintf( stderr, "mulle_objc_runtime %p trace: set tagged pointers with index %d to isa %p (class \"%s\" with id %08x)\n", runtime, index, cls, cls->name, cls->classid);
   runtime->taggedpointers.pointerclass[ index] = cls;

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


static void   print_loadclass( struct _mulle_objc_loadclass *cls)
{
   fprintf( stderr, "\t%s\n", cls->classname);
}


static void   print_loadcategory( struct _mulle_objc_loadcategory *category)
{
   fprintf( stderr, "\t%s (%s)\n", category->classname, category->categoryname);
}


static void   pointerarray_in_hashmap_map( struct mulle_concurrent_hashmap *map,
                                           void (*f)( void *, void *))
{
   struct mulle_concurrent_hashmapenumerator  rover;
   struct mulle_concurrent_pointerarray       *list;

   rover = mulle_concurrent_hashmap_enumerate( map);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &list))
   {
      mulle_concurrent_pointerarray_map( list, f, NULL);
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

      _mulle_objc_classpair_destroy( _mulle_objc_class_get_classpair( *p), &runtime->memory.allocator);
      ++p;
   }

   mulle_allocator_free( &runtime->memory.allocator, array);
}


static void  free_gift( void *p, struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, p);
}


static void   _mulle_objc_runtime_free_classgraph( struct _mulle_objc_runtime *runtime)
{
   /*
    * free various stuff
    */
   if( runtime->debug.warn.not_loaded_classes && mulle_concurrent_hashmap_count( &runtime->classestoload))
   {
      fprintf( stderr, "mulle_objc_runtime %p warning: the following classes failed to load:\n", runtime);
      pointerarray_in_hashmap_map( &runtime->classestoload, (void (*)()) print_loadclass);
   }

   if( runtime->debug.warn.not_loaded_categories && mulle_concurrent_hashmap_count( &runtime->categoriestoload))
   {
      fprintf( stderr, "mulle_objc_runtime %p warning: the following categories failed to load:\n", runtime);
      pointerarray_in_hashmap_map( &runtime->categoriestoload, (void (*)()) print_loadcategory);
   }

   /* free classes */
   _mulle_objc_runtime_free_classpairs( runtime);

   _mulle_concurrent_hashmap_done( &runtime->categoriestoload);
   _mulle_concurrent_hashmap_done( &runtime->classestoload);
   _mulle_concurrent_hashmap_done( &runtime->descriptortable);
   _mulle_concurrent_hashmap_done( &runtime->classtable);

   _mulle_concurrent_pointerarray_done( &runtime->staticstrings);
   _mulle_concurrent_pointerarray_done( &runtime->hashnames);

   /* free gifts */
   mulle_concurrent_pointerarray_map( &runtime->gifts, (void (*)()) free_gift, &runtime->memory.allocator);
   _mulle_concurrent_pointerarray_done( &runtime->gifts);
}


static inline void  _mulle_objc_runtime_uninitalize( struct _mulle_objc_runtime *runtime)
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

   _mulle_objc_runtime_uninitalize( runtime);

   if( _mulle_objc_is_global_runtime( runtime))
      return;

   free( runtime);
}


# pragma mark - runtime release convenience

void  mulle_objc_release_runtime( void)
{
   struct _mulle_objc_runtime *runtime;

   runtime = __mulle_objc_get_runtime();
   if( runtime && _mulle_objc_runtime_is_initalized( runtime))
      _mulle_objc_runtime_release( runtime);
}


struct _mulle_objc_garbagecollection  *_mulle_objc_runtime_get_garbagecollection( struct _mulle_objc_runtime *runtime)
{
   assert( _mulle_objc_runtime_is_initalized( runtime));

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


#ifndef MULLE_OBJC_NO_CONVENIENCES

/* conveniences, need runtime already setup to work */

MULLE_C_NON_NULL_RETURN
void   *mulle_objc_calloc( size_t n, size_t size)
{
   return( _mulle_allocator_calloc( &mulle_objc_inlined_get_runtime()->memory.allocator, n, size));
}


MULLE_C_NON_NULL_RETURN
void   *mulle_objc_realloc( void *block, size_t size)
{
   return( _mulle_allocator_realloc( &mulle_objc_inlined_get_runtime()->memory.allocator, block, size));
}


void   mulle_objc_free( void *block)
{
   _mulle_allocator_free( &mulle_objc_inlined_get_runtime()->memory.allocator, block);
}

#endif


//
// admittedly its fairly tricky, to set mulle_objc_globals before any of the
// class loads hit...
//
# pragma mark - "exceptions"


long   __mulle_objc_personality_v0 = 1848;  // no idea what this is used for

MULLE_C_NO_RETURN
void   _mulle_objc_runtime_raise_fail_errno_exception( struct _mulle_objc_runtime *runtime)
{
   _mulle_objc_runtime_raise_fail_exception( runtime, "%s", strerror( errno));
}


MULLE_C_NO_RETURN
void   _mulle_objc_runtime_raise_inconsistency_exception( struct _mulle_objc_runtime *runtime, char *format, ...)
{
   va_list   args;

   va_start( args, format);
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
                                          mulle_objc_classid_t classId,
                                          void *exception)
{
   struct _mulle_objc_class   *cls;
   struct _mulle_objc_class   *exceptionCls;

   assert( classId);
   assert( exception);

   cls          = _mulle_objc_runtime_unfailing_lookup_class( runtime, classId);
   exceptionCls = _mulle_objc_object_get_isa( exception);

   do
   {
      if( cls == exceptionCls)
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
   _mulle_objc_runtime_raise_fail_exception( __get_or_create_objc_runtime(), format, args);
   va_end( args);
}


void   mulle_objc_raise_fail_errno_exception( void)
{
   _mulle_objc_runtime_raise_fail_errno_exception( __get_or_create_objc_runtime());
}


void   mulle_objc_raise_inconsistency_exception( char *format, ...)
{
   va_list   args;

   va_start( args, format);
   _mulle_objc_runtime_raise_inconsistency_exception( __get_or_create_objc_runtime(), format, args);
   va_end( args);
}
#endif


void   mulle_objc_raise_taggedpointer_exception( void *obj)
{
   _mulle_objc_runtime_raise_inconsistency_exception( __get_or_create_objc_runtime(), "%p is a tagged pointer", obj);
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

   config = _mulle_objc_runtime_calloc( runtime, 1, sizeof( struct _mulle_objc_threadconfig));

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
                                      struct _mulle_objc_class *superclass)
{
   extern int     _mulle_objc_class_is_sane( struct _mulle_objc_class *cls);
   struct _mulle_objc_classpair   *pair;
   struct _mulle_objc_class       *super_meta;
   struct _mulle_objc_class       *super_meta_isa;

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
      super_meta     = _mulle_objc_class_get_metaclass( superclass);
      super_meta_isa = _mulle_objc_class_get_metaclass( super_meta);
      assert( super_meta_isa);
   }

   pair = (struct _mulle_objc_classpair *)  _mulle_objc_runtime_calloc( runtime, 1, sizeof( struct _mulle_objc_classpair) + sizeof( uint32_t));

   _mulle_objc_objectheader_init( &pair->metaclassheader, super_meta_isa ? super_meta_isa : &pair->infraclass);
   _mulle_objc_objectheader_init( &pair->infraclassheader, &pair->metaclass);

   _mulle_objc_class_init( &pair->infraclass, name, instancesize, classid, superclass, runtime);
   _mulle_objc_class_init( &pair->metaclass, name, sizeof( struct _mulle_objc_class), classid, super_meta ? super_meta : &pair->infraclass, runtime);

   _mulle_concurrent_hashmap_init( &pair->infraclass.cvars, 0, &runtime->memory.allocator);
   _mulle_objc_class_set_infraclass( &pair->metaclass, &pair->infraclass);

   _mulle_objc_class_setup_pointerarrays( &pair->infraclass, runtime);
   _mulle_objc_class_setup_pointerarrays( &pair->metaclass, runtime);

   _mulle_concurrent_pointerarray_add( &pair->metaclass.propertylists, &runtime->empty_propertylist);
   _mulle_concurrent_pointerarray_add( &pair->metaclass.ivarlists, &runtime->empty_ivarlist);

   assert( _mulle_objc_class_is_sane( &pair->metaclass));
   assert( _mulle_objc_class_is_sane( &pair->infraclass));

   return( pair);
}


// convenience during loading
struct _mulle_objc_classpair   *mulle_objc_unfailing_new_classpair( mulle_objc_classid_t  classid,
                                                                    char *name,
                                                                    size_t instancesize,
                                                                    struct _mulle_objc_class *superclass)
{
   struct _mulle_objc_classpair     *pair;
   struct _mulle_objc_runtime       *runtime;

   runtime = __get_or_create_objc_runtime();
   pair     = mulle_objc_runtime_new_classpair( runtime, classid, name, instancesize, superclass);
   if( ! pair)
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);  // unfailing vectors through there
   return( pair);
}


struct _mulle_objc_class   *mulle_objc_runtime_unfailing_lookup_class( struct _mulle_objc_runtime *runtime,
                                                                       mulle_objc_classid_t classid)
{
   struct _mulle_objc_class   *cls;

   cls = _mulle_concurrent_hashmap_lookup( &runtime->classtable, classid);
   if( ! cls)
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   assert( mulle_objc_class_is_current_thread_registered( cls));
   return( cls);
}


/* don't check for ivar_hash, as this is too painful for application
   runtime hacks. Only during loading
 */
int   mulle_objc_runtime_add_class( struct _mulle_objc_runtime *runtime, struct _mulle_objc_class *cls)
{
   struct _mulle_objc_class   *superclass;

   if( ! runtime || ! cls)
   {
      errno = EINVAL;
      return( -1);
   }

   if( ! mulle_objc_class_is_sane( cls))
      return( -1);

   if( _mulle_objc_class_is_metaclass( cls))
   {
      errno = EDOM;
      return( -1);
   }

   if( _mulle_concurrent_hashmap_lookup( &runtime->classtable, cls->classid))
   {
      errno = EEXIST;
      return( -1);
   }

   superclass = cls->superclass;
   if( superclass)
   {
      if( ! _mulle_concurrent_hashmap_lookup( &runtime->classtable, superclass->classid))
      {
         errno = EFAULT;
         return( -1);
      }
   }

   if( runtime->debug.trace.class_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: add class \"%s\" with id %08x (-:%p +:%p s:%p)\n", runtime, cls->name, cls->classid, cls, _mulle_objc_class_get_metaclass( cls), _mulle_objc_class_get_superclass( cls));

   return( _mulle_concurrent_hashmap_insert( &runtime->classtable, cls->classid, cls));
}


void   _mulle_objc_runtime_set_fastclass( struct _mulle_objc_runtime *runtime,
                                          struct _mulle_objc_class *cls,
                                          unsigned int index)
{
   struct _mulle_objc_class  *old;

   assert( runtime);
   assert( cls);
   assert( _mulle_objc_class_is_metaclass( cls));

   if( runtime->debug.trace.fastclass_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: add fastclass \"%s\" at index %d\n", runtime, cls->name, index);

   if( index >= MULLE_OBJC_S_FASTCLASSES)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: "
            "fastclass index %d for %s (id %08lx) out of bounds\n",
             runtime, index, _mulle_objc_class_get_name( cls), _mulle_objc_class_get_classid( cls));

   if( ! _mulle_atomic_pointer_compare_and_swap( &runtime->fastclasstable.classes[ index].pointer, cls, NULL))
   {
      old = _mulle_atomic_pointer_read( &runtime->fastclasstable.classes[ index].pointer);
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: classes \"%s\" "
                                                        "and \"%s\", both want to occupy fastclass spot %u",
                                                        runtime,
                                                        _mulle_objc_class_get_name( cls), _mulle_objc_class_get_name( old), index);
   }
}



void   mulle_objc_runtime_unfailing_add_class( struct _mulle_objc_runtime *runtime,
                                               struct _mulle_objc_class *cls)
{
   if( mulle_objc_runtime_add_class( runtime, cls))
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
}


#ifndef MULLE_OBJC_NO_CONVENIENCES
// conveniences

MULLE_C_CONST_RETURN  // always returns same value (in same thread)
struct _mulle_objc_class   *mulle_objc_unfailing_lookup_class( mulle_objc_classid_t classid)
{
   struct _mulle_objc_class   *cls;

   cls = mulle_objc_runtime_unfailing_lookup_class( mulle_objc_inlined_get_runtime(), classid);
   return( cls);
}


void   mulle_objc_unfailing_add_class( struct _mulle_objc_class *cls)
{
   mulle_objc_runtime_unfailing_add_class( __get_or_create_objc_runtime(), cls);
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

   dup   = _mulle_concurrent_hashmap_lookup( &runtime->descriptortable, p->methodid);
   if( ! dup)
      return( _mulle_concurrent_hashmap_insert( &runtime->descriptortable, p->methodid, p));

   if( strcmp( dup->name, p->name))
   {
      errno = EEXIST;
      return( -1);
   }

   if( runtime->debug.warn.methodid_types)
      if( strcmp( dup->signature, p->signature))
         fprintf( stderr, "mulle_objc_runtime %p warning: varying types \"%s\" and \"%s\" for method \"%s\"\n",
                 runtime,
                 dup->signature, p->signature, p->name);
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

   runtime = __get_or_create_objc_runtime();
   desc    = _mulle_objc_runtime_lookup_methoddescriptor( runtime, methodid);
   return( desc ? desc->name : NULL);
}


char   *mulle_objc_search_debughashname( mulle_objc_uniqueid_t uniqueid)
{
   struct _mulle_objc_runtime            *runtime;

   runtime = __get_or_create_objc_runtime();
   return( _mulle_objc_runtime_search_debughashname( runtime, uniqueid));
}


# pragma mark - method cache

// cheap conveniences
#ifndef MULLE_OBJC_NO_CONVENIENCES
struct _mulle_objc_methodlist  *mulle_objc_alloc_methodlist( unsigned int count_methods)
{
   return( mulle_objc_runtime_calloc( __get_or_create_objc_runtime(),
                                      1,
                                      mulle_objc_size_of_methodlist( count_methods)));
}

void   mulle_objc_free_methodlist( struct _mulle_objc_methodlist *list)
{
   _mulle_objc_runtime_free( __get_or_create_objc_runtime(), list);
}

#endif


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
         fprintf( stderr, "mulle_objc_runtime %p trace: delay add of string \"%s\" at %p\n",
               runtime, ((struct _NSConstantString *) string)->_storage, string);
      _mulle_concurrent_pointerarray_add( &runtime->staticstrings, (void *) string);
   }

   _mulle_objc_object_set_isa( string, runtime->foundation.staticstringclass);
   if( runtime->debug.trace.string_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: add string \"%s\" at %p\n",
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
      _mulle_objc_object_set_isa( string, runtime->foundation.staticstringclass);
      if( flag)
         fprintf( stderr, "mulle_objc_runtime %p trace: patch string \"%s\" at %p\n",
               runtime, ((struct _NSConstantString *) string)->_storage, string);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);
}


void  _mulle_objc_runtime_set_staticstringclass( struct _mulle_objc_runtime *runtime,
                                                 struct _mulle_objc_class *cls)
{
   assert( runtime);
   assert( cls);

   runtime->foundation.staticstringclass = cls;
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


# pragma mark - gifts (externally allocated memory)

void  _mulle_objc_runtime_unfailing_add_gift( struct _mulle_objc_runtime *runtime,
                                              void *gift)
{
   _mulle_concurrent_pointerarray_add( &runtime->gifts, gift);
}


# pragma mark - debug support

/* debug support */

static inline int   is_stopper_command( mulle_objc_runtime_walkcommand_t cmd)
{
   switch( cmd)
   {
   case mulle_objc_runtime_walk_error  :
   case mulle_objc_runtime_walk_done   :
   case mulle_objc_runtime_walk_cancel :
      return( 1);

   default :
      return( 0);
   }
}


struct bouncy_info
{
   void                               *userinfo;
   struct _mulle_objc_runtime         *runtime;
   void                               *parent;
   mulle_objc_runtime_walkcallback    callback;
   mulle_objc_runtime_walkcommand_t   rval;
};


static int   bouncy_method( struct _mulle_objc_method *method, struct _mulle_objc_class *cls, void *userinfo)
{
   struct bouncy_info   *info;

   info       = userinfo;
   info->rval = (info->callback)( info->runtime, method, mulle_objc_runtime_is_method, NULL, cls, info->userinfo);
   return( is_stopper_command( info->rval));
}


static int   bouncy_property( struct _mulle_objc_property *property, struct _mulle_objc_class *cls, void *userinfo)
{
   struct bouncy_info   *info;

   info       = userinfo;
   info->rval = (info->callback)( info->runtime, property, mulle_objc_runtime_is_property, NULL, cls, info->userinfo);
   return( is_stopper_command( info->rval));
}


static int   bouncy_ivar( struct _mulle_objc_ivar *ivar, struct _mulle_objc_class *cls, void *userinfo)
{
   struct bouncy_info   *info;

   info       = userinfo;
   info->rval = (info->callback)( info->runtime, ivar, mulle_objc_runtime_is_ivar, NULL, cls, info->userinfo);
   return( is_stopper_command( info->rval));
}


// don't expose, because it's bit too weird

mulle_objc_runtime_walkcommand_t
   mulle_objc_runtime_class_walk( struct _mulle_objc_runtime *runtime,
                                  struct _mulle_objc_class   *cls,
                                  enum mulle_objc_runtime_type_t  type,
                                  mulle_objc_runtime_walkcallback   callback,
                                  void *parent,
                                  void *userinfo);

mulle_objc_runtime_walkcommand_t
   mulle_objc_runtime_class_walk( struct _mulle_objc_runtime *runtime,
                                  struct _mulle_objc_class   *cls,
                                  enum mulle_objc_runtime_type_t  type,
                                  mulle_objc_runtime_walkcallback   callback,
                                  void *parent,
                                  void *userinfo)
{
   mulle_objc_runtime_walkcommand_t   cmd;
   struct bouncy_info                 info;

   cmd = (*callback)( runtime, cls, type, NULL, parent, userinfo);
   if( cmd != mulle_objc_runtime_walk_ok)
      return( cmd);

   info.callback = callback;
   info.parent   = parent;
   info.userinfo = userinfo;
   info.runtime  = runtime;

   cmd = _mulle_objc_class_walk_methods( cls, _mulle_objc_class_get_inheritance( cls), bouncy_method, &info);
   if( cmd != mulle_objc_runtime_walk_ok)
      return( cmd);

   cmd = _mulle_objc_class_walk_properties( cls, _mulle_objc_class_get_inheritance( cls), bouncy_property, &info);
   if( cmd != mulle_objc_runtime_walk_ok)
      return( cmd);

   cmd = _mulle_objc_class_walk_ivars( cls, _mulle_objc_class_get_inheritance( cls), bouncy_ivar, &info);
   return( cmd);
}


mulle_objc_runtime_walkcommand_t   mulle_objc_runtime_walk( struct _mulle_objc_runtime *runtime,
      mulle_objc_runtime_walkcallback   callback,
      void *userinfo)
{
   mulle_objc_runtime_walkcommand_t           cmd;
   struct _mulle_objc_class                   *cls;
   struct _mulle_objc_class                   *meta;
   struct mulle_concurrent_hashmapenumerator  rover;

   if( ! runtime || ! callback)
      return( mulle_objc_runtime_walk_error);

   cmd = (*callback)( runtime, runtime, mulle_objc_runtime_is_runtime, NULL, NULL, userinfo);
   if( cmd != mulle_objc_runtime_walk_ok)
      return( cmd);

   rover = mulle_concurrent_hashmap_enumerate( &runtime->classtable);  // slow!
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &cls))
   {
      cmd = mulle_objc_runtime_class_walk( runtime, cls, mulle_objc_runtime_is_class, callback, runtime, userinfo);
      if( is_stopper_command( cmd))
         return( cmd);

      meta = _mulle_objc_class_get_metaclass( cls);
      if( meta && meta != cls)
      {
         cmd = mulle_objc_runtime_class_walk( runtime, meta, mulle_objc_runtime_is_meta_class, callback, cls, userinfo);
         if( is_stopper_command( cmd))
            return( cmd);
      }
   }
   mulle_concurrent_hashmapenumerator_done( &rover);

   return( cmd);
}


mulle_objc_runtime_walkcommand_t   mulle_objc_runtime_walk_classes( struct _mulle_objc_runtime  *runtime,
                                                                    mulle_objc_runtime_walkcallback callback,
                                                                    void *userinfo)
{
   mulle_objc_runtime_walkcommand_t            cmd;
   struct _mulle_objc_class                    *cls;
   struct _mulle_objc_class                    *meta;
   struct mulle_concurrent_hashmapenumerator   rover;

   if( ! runtime || ! callback)
      return( mulle_objc_runtime_walk_error);

   cmd   = mulle_objc_runtime_walk_done;
   rover = mulle_concurrent_hashmap_enumerate( &runtime->classtable);  // slow!
   while( _mulle_concurrent_hashmapenumerator_next( &rover, NULL, (void **) &cls))
   {
      cmd = (*callback)( runtime, cls, mulle_objc_runtime_is_class, NULL, NULL, userinfo);
      if( is_stopper_command( cmd))
         return( cmd);

      meta = _mulle_objc_class_get_metaclass( cls);
      if( meta && meta != cls)
      {
         cmd = (*callback)( runtime, meta, mulle_objc_runtime_is_meta_class, NULL, NULL, userinfo);
         if( is_stopper_command( cmd))
            return( cmd);
      }
   }
   mulle_concurrent_hashmapenumerator_done( &rover);

   return( cmd);
}


void   mulle_objc_walk_classes( mulle_objc_runtime_walkcallback callback,
                                void *userinfo)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_get_runtime();
   mulle_objc_runtime_walk_classes( runtime, callback, userinfo);
}
