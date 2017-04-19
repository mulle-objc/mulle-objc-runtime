//
//  mulle_objc_load.c
//  mulle-objc
//
//  Created by Nat! on 19.11.14.
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

#include "mulle_objc_load.h"

#include "mulle_objc_callqueue.h"
#include "mulle_objc_class.h"
#include "mulle_objc_classpair.h"
#include "mulle_objc_class_runtime.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_metaclass.h"
#include "mulle_objc_runtime.h"
#include "mulle_objc_runtime_dotdump.h"


#include <mulle_concurrent/mulle_concurrent.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static size_t   mulle_objc_count_protocolids( mulle_objc_protocolid_t *ids)
{
   size_t   n;

   n = 0;
   if( ids)
      while( *ids++)
         ++n;
   return( n);
}


static int  _mulle_objc_protocolid_compare( mulle_objc_protocolid_t *a, mulle_objc_protocolid_t *b)
{
   intptr_t   diff;

   diff = (intptr_t) *a - (intptr_t) *b;
   if( diff < 0)
      return( -1);
   return( ! ! diff);
}


static void    map_f( struct mulle_concurrent_hashmap *table,
                      mulle_objc_uniqueid_t uniqueid,
                      void (*f)( void *,
                                 struct _mulle_objc_callqueue *),
                      struct _mulle_objc_callqueue *loads)
{
   struct mulle_concurrent_pointerarray            *list;
   struct mulle_concurrent_pointerarrayenumerator  rover;
   void                                            *value;

   if( ! table)
      return;

   list = _mulle_concurrent_hashmap_lookup( table, uniqueid);
   if( ! list)
      return;

   // because we are really single-threaded everything is much easier
   // just tear out the table (no-one can write into it concurrently)
   _mulle_concurrent_hashmap_remove( table, uniqueid, list);

   rover = mulle_concurrent_pointerarray_enumerate( list);
   while( value = mulle_concurrent_pointerarrayenumerator_next( &rover))
      (*f)( value, loads);
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   _mulle_concurrent_pointerarray_done( list);
}


#pragma mark - classes

static void  trace_preamble( struct _mulle_objc_runtime *runtime)
{
   fprintf( stderr, "mulle_objc_runtime %p trace: ", runtime);
}


static void  loadclass_fprintf( FILE *fp,
                               struct _mulle_objc_loadclass *info)
{
   fprintf( fp, "class %08x \"%s\"",
           info->classid, info->classname);
}


static void  loadclass_trace( struct _mulle_objc_loadclass *info,
                              struct _mulle_objc_runtime *runtime,
                              char *format, ...)
{
   va_list   args;

   trace_preamble( runtime);
   loadclass_fprintf( stderr, info);
   fputc( ' ', stderr);

   va_start( args, format);
   vfprintf( stderr, format, args);
   va_end( args);

   if( info->origin && runtime->debug.trace.print_origin)
      fprintf( stderr, " (%s)", info->origin);
   fputc( '\n', stderr);
}



static int   mulle_objc_loadclass_delayedadd( struct _mulle_objc_loadclass *info,
                                              mulle_objc_classid_t missingclassid,
                                              struct _mulle_objc_runtime *runtime)
{
   struct mulle_concurrent_pointerarray   *list;
   int                                    rval;

   // possibly get or create runtime..

   if( ! info)
   {
      errno = EINVAL;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   assert( info->classid != missingclassid);

   for(;;)
   {
      list = _mulle_concurrent_hashmap_lookup( &runtime->waitqueues.classestoload, missingclassid);
      if( ! list)
      {
         list = _mulle_allocator_calloc( &runtime->memory.allocator, 1, sizeof( *list));
         _mulle_concurrent_pointerarray_init( list, 1, &runtime->memory.allocator);

         rval = _mulle_concurrent_hashmap_insert( &runtime->waitqueues.classestoload, missingclassid, list);
         if( ! rval)
            break;

         _mulle_concurrent_pointerarray_done( list);
         _mulle_allocator_free( &runtime->memory.allocator, list);  // assume it was never
         if( rval == EEXIST)
            continue;

         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
      }
      break;
   }

   _mulle_concurrent_pointerarray_add( list, info);
   return( 0);
}


static mulle_objc_classid_t
    mulle_objc_loadclass_missingdependencies_classid( struct _mulle_objc_loadclass *info,
                                                      struct _mulle_objc_runtime   *runtime)
{
   struct _mulle_objc_method           *method;
   mulle_objc_classid_t                *classids;
   mulle_objc_methodimplementation_t   imp;

   if( ! info->classmethods)
      return( 0);

   method = mulle_objc_method_bsearch( info->classmethods->methods,
                                       info->classmethods->n_methods,
                                       MULLE_OBJC_CLASS_DEPENDENCIES_METHODID);
   if( ! method)
      return( 0);

   if( runtime->debug.trace.load_calls)
      loadclass_trace( info, runtime, "call +[%s classDependencies]", info->classname);

   imp      = _mulle_objc_method_get_implementation( method);
   classids = (*imp)( NULL, MULLE_OBJC_CLASS_DEPENDENCIES_METHODID, NULL);
   if( ! classids)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: %s returned NULL for +categoryDependencies\n",
                                               runtime, info->classname);

   while( *classids)
   {
      if( ! _mulle_objc_runtime_lookup_infraclass( runtime, *classids))
      {
         if( runtime->debug.trace.delayed_category_adds)
            loadclass_trace( info, runtime, "waits for class %08x \"%s\"",
                             *classids,
                             mulle_objc_string_for_classid( *classids));
         return( *classids);
      }
      ++classids;
   }

   return( 0);
}


mulle_objc_classid_t   mulle_objc_loadclass_missingclassid( struct _mulle_objc_loadclass *info,
                                                            struct _mulle_objc_runtime *runtime,
                                                            struct _mulle_objc_infraclass  **p_superclass)
{
   struct _mulle_objc_infraclass   *protocolclass;
   struct _mulle_objc_infraclass   *superclass;
   mulle_objc_classid_t            *classid_p;
   mulle_objc_classid_t             missingid;

   assert( info);
   assert( runtime);

   superclass = NULL;
   if( info->superclassid)
   {
      superclass = _mulle_objc_runtime_lookup_infraclass( runtime, info->superclassid);
      if( ! superclass)
      {
         if( runtime->debug.trace.delayed_class_adds)
            loadclass_trace( info, runtime,
                         "waits for superclass %08x \"%s\"",
                         info->superclassid, info->superclassname);

         return( info->superclassid);
      }

      if( strcmp( info->superclassname, superclass->base.name))
         _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: hash collision %08x for classnames \"%s\" \"%s\"",
                                                  runtime,
                                                  info->superclassid,
                                                  info->superclassname,
                                                  superclass->base.name);
   }

   if( superclass && superclass->ivarhash != info->superclassivarhash)
   {
      if( ! runtime->config.ignore_ivarhash_mismatch)
         _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: superclass \"%s\" of \"%s\" has changed. Recompile \"%s\" (%s).\n", runtime, info->superclassname, info->classname, info->classname, info->origin ? info->origin : "<unknown file>");
   }

   // protocol classes present ?
   if( info->protocolclassids)
   {
      for( classid_p = info->protocolclassids; *classid_p; ++classid_p)
      {
         // avoid duplication and waiting for seld
         if( *classid_p == info->superclassid || *classid_p == info->classid)
            continue;

         protocolclass = _mulle_objc_runtime_lookup_infraclass( runtime, *classid_p);
         if( ! protocolclass)
         {
            if( runtime->debug.trace.delayed_class_adds)
               loadclass_trace( info, runtime,
                           "waits for protocol class %08x \"%s\"\n",
                           *classid_p, mulle_objc_string_for_protocolid( *classid_p));
            return( *classid_p);
         }
      }
   }

   missingid = mulle_objc_loadclass_missingdependencies_classid( info, runtime);
   if( missingid != MULLE_OBJC_NO_CLASSID)
      return( missingid);

   if( p_superclass)
      *p_superclass = superclass;

   return( 0);
}


// ensure the load class, minimally make sense
static int  mulle_objc_loadclass_is_sane( struct _mulle_objc_loadclass *info)
{
   if( ! info)
      return( 0);

   if( info->classid == MULLE_OBJC_NO_CLASSID ||
       info->classid == MULLE_OBJC_INVALID_CLASSID)
      return( 0);

   if( ! info->classname)
      return( 0);

   return( 1);
}


void   mulle_objc_loadclass_unfailing_enqueue( struct _mulle_objc_loadclass *info,
                                               struct _mulle_objc_callqueue *loads)
{
   struct _mulle_objc_classpair   *pair;
   struct _mulle_objc_metaclass   *meta;
   struct _mulle_objc_infraclass  *superclass;
   struct _mulle_objc_runtime     *runtime;
   mulle_objc_classid_t           missingclassid;

   // possibly get or create runtime..

   runtime = __get_or_create_objc_runtime();

   if( ! mulle_objc_loadclass_is_sane( info))
   {
      errno = EINVAL;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   // root ?
   superclass = NULL;
   missingclassid = mulle_objc_loadclass_missingclassid( info, runtime, &superclass);
   if( missingclassid)
   {
      if( mulle_objc_loadclass_delayedadd( info, missingclassid, runtime))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
      return;
   }

   //
   // for those that reverse order their .o files in a shared library
   // categories of a class then the class
   // subclass first then superclass
   // this callqueue mechanism does the "right" thing
   //
   // ready to install
   pair = mulle_objc_unfailing_new_classpair( info->classid, info->classname, info->instancesize, superclass);

   _mulle_objc_classpair_set_origin( pair, info->origin);
   mulle_objc_classpair_unfailing_add_protocols( pair, info->protocolids);

   meta   = _mulle_objc_classpair_get_metaclass( pair);

   mulle_objc_metaclass_unfailing_add_methodlist( meta, info->classmethods);
   mulle_objc_methodlist_unfailing_add_load_to_callqueue( info->classmethods, meta, loads);

   infra  = _mulle_objc_classpair_get_infraclass( pair);
   assert( meta == _mulle_objc_class_get_metaclass( &infra->base));

   _mulle_objc_infraclass_set_ivarhash( infra, info->classivarhash);

   mulle_objc_infraclass_unfailing_add_ivarlist( infra, info->instancevariables);
   mulle_objc_infraclass_unfailing_add_propertylist( infra, info->properties);
   mulle_objc_infraclass_unfailing_add_methodlist( infra, info->instancemethods);

   if( info->fastclassindex >= 0)
      _mulle_objc_runtime_set_fastclass( runtime, infra, info->fastclassindex);

   if( mulle_objc_runtime_add_infraclass( runtime, infra))
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: duplicate class %08x \"%s\".\n", runtime, infra->base.classid, infra->base.name);

   if( runtime->debug.trace.dump_runtime)
      mulle_objc_dotdump_runtime_to_tmp();
   
   //
   // check if categories or classes are waiting for us ?
   //
   map_f( &runtime->waitqueues.categoriestoload, info->classid, (void (*)()) mulle_objc_loadcategory_unfailing_enqueue, loads);
   map_f( &runtime->waitqueues.classestoload, info->classid, (void (*)())  mulle_objc_loadclass_unfailing_enqueue, loads);
}


static void   mulle_objc_loadclass_listssort( struct _mulle_objc_loadclass *lcls)
{
   qsort( lcls->protocolids, mulle_objc_count_protocolids( lcls->protocolids), sizeof( mulle_objc_protocolid_t), (int (*)()) _mulle_objc_protocolid_compare);

   mulle_objc_ivarlist_sort( lcls->instancevariables);
   mulle_objc_methodlist_sort( lcls->instancemethods);
   mulle_objc_methodlist_sort( lcls->classmethods);
   mulle_objc_propertylist_sort( lcls->properties);
}


static void   loadprotocols_dump( mulle_objc_protocolid_t *protocolids,
                                  mulle_objc_protocolid_t *protocolclassids,
                                  struct _mulle_objc_loadhashedstringlist *strings)

{
   mulle_objc_protocolid_t    protoid;
   char                       *pre;
   char                       *sep;
   char                       *s;

   fprintf( stderr, " <");
   sep = " ";
   for(; *protocolids; ++protocolids)
   {
      protoid = *protocolids;
      pre     = "";

      if( protocolclassids)
         for(; *protocolclassids; ++protocolclassids)
            if( *protocolclassids == protoid)
               pre="*";

      s = NULL;
      if( strings)
         s = mulle_objc_loadhashedstringlist_bsearch( strings, protoid);
      if( s)
         fprintf( stderr, "%s%s%s", sep, pre, s);
      else
         fprintf( stderr, "%s%s%lx", sep, pre, (unsigned long) protoid);
      sep = ", ";
   }
   fprintf( stderr, ">");
}


static void   loadclass_dump( struct _mulle_objc_loadclass *p,
                              char *prefix,
                              struct _mulle_objc_loadhashedstringlist *strings)

{

   fprintf( stderr, "%s@implementation %s", prefix, p->classname);
   if( p->superclassname)
      fprintf( stderr, " : %s", p->superclassname);

   if( p->protocolids)
      loadprotocols_dump( p->protocolids, p->protocolclassids, strings);

   if( p->origin)
      fprintf( stderr, " // %s", p->origin);

   fprintf( stderr, "\n");
}



#pragma mark - classlists

static void   mulle_objc_loadclasslist_unfailing_enqueue( struct _mulle_objc_loadclasslist *list,
                                                          int need_sort,
                                                          struct _mulle_objc_callqueue *loads)
{
   struct _mulle_objc_loadclass   **p_class;
   struct _mulle_objc_loadclass   **sentinel;

   p_class = list->loadclasses;
   sentinel = &p_class[ list->n_loadclasses];
   while( p_class < sentinel)
   {
      if( need_sort)
         mulle_objc_loadclass_listssort( *p_class);

      mulle_objc_loadclass_unfailing_enqueue( *p_class, loads);
      p_class++;
   }
}

static void   loadclasslist_dump( struct _mulle_objc_loadclasslist *list,
                                  char *prefix,
                                  struct _mulle_objc_loadhashedstringlist *strings)
{
   struct _mulle_objc_loadclass   **p;
   struct _mulle_objc_loadclass   **sentinel;

   if( ! list)
      return;

   p        = list->loadclasses;
   sentinel = &p[ list->n_loadclasses];
   while( p < sentinel)
      loadclass_dump( *p++, prefix, strings);
}


#pragma mark - categories


static void  loadcategory_fprintf( FILE *fp,
                            struct _mulle_objc_loadcategory *info)
{
   fprintf( fp, "category %08x \"%s( %s)\"",
           info->categoryid, info->classname, info->categoryname);
}


static void  loadcategory_trace( struct _mulle_objc_loadcategory *info,
                             struct _mulle_objc_runtime *runtime,
                            char *format, ...)
{
   va_list   args;

   trace_preamble( runtime);
   loadcategory_fprintf( stderr, info);
   fputc( ' ', stderr);

   va_start( args, format);
   vfprintf( stderr, format, args);
   va_end( args);

   if( info->origin && runtime->debug.trace.print_origin)
      fprintf( stderr, " (%s)", info->origin);
   fputc( '\n', stderr);
}



static int  mulle_objc_loadcategory_delayedadd( struct _mulle_objc_loadcategory *info,
                                                mulle_objc_classid_t missingclassid,
                                                struct _mulle_objc_runtime *runtime)
{
   struct mulle_concurrent_pointerarray   *list;

   if( ! info)
   {
      errno = EINVAL;
      return( -1);
   }

   list = _mulle_concurrent_hashmap_lookup( &runtime->waitqueues.categoriestoload, missingclassid);
   if( ! list)
   {
      list = _mulle_allocator_calloc( &runtime->memory.allocator, 1, sizeof( *list));
      _mulle_concurrent_pointerarray_init( list, 1, &runtime->memory.allocator);

      if( _mulle_concurrent_hashmap_insert( &runtime->waitqueues.categoriestoload, missingclassid, list))
      {
         _mulle_concurrent_pointerarray_done( list);
         _mulle_allocator_free( &runtime->memory.allocator, list);  // assume it was never alive
         return( -1);
      }
   }

   _mulle_concurrent_pointerarray_add( list, info);
   return( 0);
}


mulle_objc_categoryid_t   mulle_objc_loadcategory_missingcategoryid( struct _mulle_objc_loadcategory *info,
                                                                     struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_classpair        *pair;
   struct _mulle_objc_method           *method;
   struct _mulle_objc_runtime          *runtime;
   mulle_objc_categoryid_t             *categoryids;
   mulle_objc_methodimplementation_t   imp;

   assert( info);
   assert( infra);

   if( ! info->classmethods)
      return( 0);

   method = mulle_objc_method_bsearch( info->classmethods->methods,
                                       info->classmethods->n_methods,
                                       MULLE_OBJC_CATEGORY_DEPENDENCIES_METHODID);
   if( ! method)
      return( 0);

   runtime = _mulle_objc_infraclass_get_runtime( infra);

   if( runtime->debug.trace.load_calls)
   {
      loadcategory_trace( info, runtime, "call +[%s(%s) categoryDependencies]",
                         _mulle_objc_infraclass_get_name( infra),
                         info->categoryname);
   }

   imp         = _mulle_objc_method_get_implementation( method);
   categoryids = (*imp)( infra, MULLE_OBJC_CATEGORY_DEPENDENCIES_METHODID, infra);
   if( ! categoryids)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: %s(%s) returned NULL for +categoryDependencies\n",
                                               runtime, info->classname, info->categoryname);

   pair = _mulle_objc_infraclass_get_classpair( infra);
   while( *categoryids)
   {
      if( ! _mulle_objc_classpair_has_category( pair, *categoryids))
      {
         if( runtime->debug.trace.delayed_category_adds)
            loadcategory_trace( info, runtime, "waits for category %08x \"%s( %s)\"",
                        *categoryids,
                           _mulle_objc_infraclass_get_name( infra),
                           mulle_objc_string_for_categoryid( *categoryids));
         return( *categoryids);
      }
      ++categoryids;
   }

   return( 0);
}


mulle_objc_classid_t   mulle_objc_loadcategory_missingclassid( struct _mulle_objc_loadcategory *info,
                                              struct _mulle_objc_runtime *runtime,
                                              struct _mulle_objc_infraclass **p_class)
{
   struct _mulle_objc_infraclass     *infra;
   struct _mulle_objc_infraclass     *protocolclass;
   mulle_objc_classid_t              *classid_p;

   // check class
   infra = _mulle_objc_runtime_lookup_infraclass( runtime, info->classid);
   if( ! infra)
   {
      if( runtime->debug.trace.delayed_category_adds)
         loadcategory_trace( info, runtime, "waits for its class %08x \"%s\"",
                 info->classid, info->classname);
      return( info->classid);
   }

   // protocol classes present ?
   if( info->protocolclassids)
   {
      for( classid_p = info->protocolclassids; *classid_p; ++classid_p)
      {
         // avoid duplication
         if( *classid_p == info->classid)
            continue;

         protocolclass = _mulle_objc_runtime_lookup_infraclass( runtime, *classid_p);
         if( ! protocolclass)
         {
            if( runtime->debug.trace.delayed_category_adds)
               loadcategory_trace( info, runtime, "waits for protocol class %08x \"%s\"",
                       *classid_p, mulle_objc_string_for_protocolid( *classid_p));
            return( *classid_p);
         }
      }
   }

   if( p_class)
      *p_class = infra;

   return( 0);
}

// ensure the load category, minimally make sense
static int  mulle_objc_loadcategory_is_sane( struct _mulle_objc_loadcategory *info)
{
   if( ! info)
      return( 0);
   if( info->classid == MULLE_OBJC_NO_CLASSID ||
       info->classid == MULLE_OBJC_INVALID_CLASSID)
      return( 0);
   if( info->categoryid == MULLE_OBJC_NO_CLASSID ||
       info->categoryid == MULLE_OBJC_INVALID_CLASSID)
      return( 0);

   if( ! info->categoryname ||
       ! info->classname)
      return( 0);

   return( 1);
}


static void  raise_duplicate_category_exception( struct _mulle_objc_classpair *pair,
                                                 struct _mulle_objc_loadcategory *info)
{
   struct _mulle_objc_runtime     *runtime;
   char                           *info_origin;
   char                           *pair_origin;

   runtime     = _mulle_objc_classpair_get_runtime( pair);
   pair_origin = _mulle_objc_classpair_get_origin( pair);
   if( ! pair_origin)
      pair_origin = "<unknown origin>";

   info_origin = info->origin;
   if( ! info_origin)
      info_origin = "<unknown origin>";

   _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: category %08x \"%s( %s)\" (%s) is already present in class %08x \"%s\" (%s).\n",
                                            runtime,
                                            info->categoryid,
                                            info->classname,
                                            info->categoryname ? info->categoryname : "???",
                                            info_origin,
                                            _mulle_objc_classpair_get_classid( pair),
                                            _mulle_objc_classpair_get_name( pair),
                                            pair_origin);
}


void   mulle_objc_loadcategory_unfailing_enqueue( struct _mulle_objc_loadcategory *info,
                                                  struct _mulle_objc_callqueue *loads)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_runtime      *runtime;
   mulle_objc_classid_t            missingclassid;
   mulle_objc_categoryid_t         missingcategoryid;

   runtime = __get_or_create_objc_runtime();

   if( ! mulle_objc_loadcategory_is_sane( info))
   {
      errno = EINVAL;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   infra = NULL;
   missingclassid = mulle_objc_loadcategory_missingclassid( info, runtime, &infra);
   if( missingclassid)
   {
      if( mulle_objc_loadcategory_delayedadd( info, missingclassid, runtime))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
      return;
   }
   assert( infra);

   missingcategoryid = mulle_objc_loadcategory_missingcategoryid( info, infra);
   if( missingcategoryid)
   {
      // wait on class, not category
      if( mulle_objc_loadcategory_delayedadd( info, info->classid, runtime))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
      return;
   }


   if( strcmp( info->classname, infra->base.name))
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: hashcollision %08x for classnames \"%s\" and \"%s\"\n",
                                               runtime, info->classid, info->classname, infra->base.name);

   if( info->classivarhash != infra->ivarhash)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: class %08x \"%s\" of category %08x \"%s( %s)\" has changed. Recompile %s\n",
            runtime,
            info->classid,
            info->classname,
            info->categoryid,
            info->classname,
            info->categoryname ? info->categoryname : "???",
            info->origin ? info->origin : "<unknown origin>");

   pair = _mulle_objc_infraclass_get_classpair( infra);
   meta = _mulle_objc_classpair_get_metaclass( pair);

   if( info->categoryid && _mulle_objc_classpair_has_category( pair, info->categoryid))
      raise_duplicate_category_exception( pair, info);


   // the loader sets the categoryid as owner
   if( info->instancemethods && info->instancemethods->n_methods)
   {
      info->instancemethods->owner = (void *) info->categoryid;
      if( mulle_objc_class_add_methodlist( &infra->base, info->instancemethods))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }
   if( info->classmethods && info->classmethods->n_methods)
   {
      info->classmethods->owner = (void *) info->categoryid;
      if( mulle_objc_class_add_methodlist( &meta->base, info->classmethods))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   if( info->properties && info->properties->n_properties)
      if( mulle_objc_infraclass_add_propertylist( infra, info->properties))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);

   mulle_objc_classpair_unfailing_add_protocols( pair, info->protocolids);
   if( info->categoryid)
      mulle_objc_classpair_unfailing_add_category( pair, info->categoryid);

   // this queues things up
   mulle_objc_methodlist_unfailing_add_load_to_callqueue( info->classmethods, meta, loads);

   if( runtime->debug.trace.dump_runtime)
      mulle_objc_dotdump_runtime_to_tmp();

   //
   // retrigger "special" categories, that are waiting for their dependencies
   //
   map_f( &runtime->waitqueues.categoriestoload,
          info->classid,
          (void (*)()) mulle_objc_loadcategory_unfailing_enqueue,
          loads);
}


static void   loadcategory_dump( struct _mulle_objc_loadcategory *p,
                                 char *prefix,
                                 struct _mulle_objc_loadhashedstringlist *strings)
{
   fprintf( stderr, "%s@implementation %s( %s)", prefix, p->classname, p->categoryname);

   if( p->protocolids)
      loadprotocols_dump( p->protocolids, p->protocolclassids, strings);

   if( p->origin)
      fprintf( stderr, " // %s", p->origin);

   fprintf( stderr, "\n");
}


# pragma mark - categorylists


static void   mulle_objc_loadcategory_listssort( struct _mulle_objc_loadcategory *lcat)
{
   qsort( lcat->protocolids, mulle_objc_count_protocolids( lcat->protocolids), sizeof( mulle_objc_protocolid_t), (int (*)()) _mulle_objc_protocolid_compare);

   mulle_objc_methodlist_sort( lcat->instancemethods);
   mulle_objc_methodlist_sort( lcat->classmethods);
   mulle_objc_propertylist_sort( lcat->properties);
}


static void   mulle_objc_loadcategorylist_unfailing_enqueue( struct _mulle_objc_loadcategorylist *list,
                                                             int need_sort,
                                                             struct _mulle_objc_callqueue *loads)
{
   struct _mulle_objc_loadcategory   **p_category;
   struct _mulle_objc_loadcategory   **sentinel;

   p_category = list->loadcategories;
   sentinel   = &p_category[ list->n_loadcategories];
   while( p_category < sentinel)
   {
      if( need_sort)
         mulle_objc_loadcategory_listssort( *p_category);

      mulle_objc_loadcategory_unfailing_enqueue( *p_category, loads);
      p_category++;
   }
}


static void   loadcategorylist_dump( struct _mulle_objc_loadcategorylist *list,
                                     char *prefix,
                                     struct _mulle_objc_loadhashedstringlist *strings)
{
   struct _mulle_objc_loadcategory   **p;
   struct _mulle_objc_loadcategory   **sentinel;

   if( ! list)
      return;

   p        = list->loadcategories;
   sentinel = &p[ list->n_loadcategories];
   while( p < sentinel)
      loadcategory_dump( *p++, prefix, strings);
}


# pragma mark - stringlists

static void   mulle_objc_loadstringlist_unfailing_enqueue( struct _mulle_objc_loadstringlist *list)
{
   struct _mulle_objc_object    **p_string;
   struct _mulle_objc_object    **sentinel;
   struct _mulle_objc_runtime   *runtime;

   runtime = __get_or_create_objc_runtime();

   p_string = list->loadstrings;
   sentinel = &p_string[ list->n_loadstrings];

   // memo: the actual staticstringclass is likely not installed yet

   while( p_string < sentinel)
   {
      _mulle_objc_runtime_add_staticstring( runtime, *p_string);
      p_string++;
   }
}


# pragma mark - hashedstring

char   *_mulle_objc_loadhashedstring_bsearch( struct _mulle_objc_loadhashedstring *buf,
                                             unsigned int n,
                                             mulle_objc_ivarid_t search)
{
   int   first;
   int   last;
   int   middle;
   struct _mulle_objc_loadhashedstring   *p;

   assert( search != MULLE_OBJC_NO_IVARID && search != MULLE_OBJC_INVALID_IVARID);

   first  = 0;
   last   = n - 1;  // unsigned not good (need extra if)
   middle = (first + last) / 2;

   while( first <= last)
   {
      p = &buf[ middle];
      if( p->uniqueid <= search)
      {
         if( p->uniqueid == search)
            return( p->string);

         first = middle + 1;
      }
      else
         last = middle - 1;

      middle = (first + last) / 2;
   }

   return( NULL);
}


int  _mulle_objc_loadhashedstring_compare( struct _mulle_objc_loadhashedstring *a,
                                          struct _mulle_objc_loadhashedstring *b)
{
   mulle_objc_uniqueid_t   a_id;
   mulle_objc_uniqueid_t   b_id;

   a_id = a->uniqueid;
   b_id = b->uniqueid;
   if( a_id < b_id)
      return( -1);
   return( a_id != b_id);
}


void   mulle_objc_loadhashedstring_sort( struct _mulle_objc_loadhashedstring *methods,
                                        unsigned int n)
{
   if( ! methods)
      return;

   qsort( methods, n, sizeof( struct _mulle_objc_loadhashedstring), (int(*)())  _mulle_objc_loadhashedstring_compare);
}


# pragma mark - hashedstringlists

static void   mulle_objc_loadhashedstringlist_unfailing_enqueue( struct _mulle_objc_loadhashedstringlist *map, int need_sort)
{
   struct _mulle_objc_runtime   *runtime;

   if( need_sort)
      mulle_objc_loadhashedstringlist_sort( map);

   runtime = __get_or_create_objc_runtime();
   _mulle_objc_runtime_add_loadhashedstringlist( runtime, map);
}


# pragma mark - info

static void  dump_bits( unsigned int bits)
{
   char   *delim;

   delim ="";
   if( bits & _mulle_objc_loadinfo_unsorted)
   {
      fprintf( stderr, "unsorted");
      delim=", ";
   }

   if( bits & _mulle_objc_loadinfo_aaomode)
   {
      fprintf( stderr, "%sAAM", delim);
      delim=", ";
   }

   if( bits & _mulle_objc_loadinfo_notaggedptrs)
   {
      fprintf( stderr, "%sno tagged pointers", delim);
      delim=", ";
   }

   if( bits & _mulle_objc_loadinfo_threadlocalrt)
   {
      fprintf( stderr, "%sthread local runtime", delim);
      delim=", ";
   }

   fprintf( stderr, "%s-O%d", delim, (bits >> 8) & 0x7);
}


static void   print_version( char *prefix, uint32_t version)
{
   fprintf( stderr, "%s=%u.%u.%u", prefix,
            mulle_objc_version_get_major( version),
            mulle_objc_version_get_minor( version),
            mulle_objc_version_get_patch( version));
}


static void   loadinfo_dump( struct _mulle_objc_loadinfo *info, char *prefix)
{
   fprintf( stderr, "%sversion: ", prefix);
   print_version( "runtime", info->version.runtime);
   print_version( ", foundation", info->version.foundation);
   print_version( ", user", info->version.user);
   fprintf( stderr, " (");
   dump_bits( info->version.bits);
   fprintf( stderr, ")\n");

   loadclasslist_dump( info->loadclasslist, prefix, info->loadhashedstringlist);
   loadcategorylist_dump( info->loadcategorylist, prefix, info->loadhashedstringlist);
}


static void   call_load( struct _mulle_objc_metaclass *meta,
                         mulle_objc_methodid_t sel,
                         mulle_objc_methodimplementation_t imp,
                         struct _mulle_objc_runtime *runtime)
{
   struct _mulle_objc_infraclass   *infra;

   if( runtime->debug.trace.load_calls)
   {
      trace_preamble( runtime);
      fprintf( stderr, "%08x \"%s\" call +[%s load]\n",
              _mulle_objc_metaclass_get_classid( meta),
              _mulle_objc_metaclass_get_name( meta),
              _mulle_objc_metaclass_get_name( meta));
   }

   // the "meta" class is not the object passed
   infra = _mulle_objc_metaclass_get_infraclass( meta);
   (*imp)( (struct _mulle_objc_object *) _mulle_objc_infraclass_as_class( infra),
                                         sel,
                                         _mulle_objc_metaclass_as_class( meta));
}


//
// this is function called per .o file
//

void   mulle_objc_loadinfo_unfailing_enqueue( struct _mulle_objc_loadinfo *info)
{
   struct _mulle_objc_runtime         *runtime;
   int                                need_sort;
   int                                optlevel;
   int                                load_tps;
   int                                mismatch;
   uintptr_t                          bits;
   assert( info);

   runtime = __get_or_create_objc_runtime();

   if( ! mulle_objc_class_is_current_thread_registered( NULL))
   {
      loadinfo_dump( info, "loadinfo:   ");
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: The function \"mulle_objc_loadinfo_unfailing_enqueue\" is called from a non-registered thread.", runtime, info->version.foundation);
   }

   if( info->version.load != MULLE_OBJC_RUNTIME_LOAD_VERSION)
   {
      loadinfo_dump( info, "loadinfo:   ");
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: the loaded binary was produced for load version %d, but this runtime supports %d only",
            runtime, info->version.load, MULLE_OBJC_RUNTIME_LOAD_VERSION);
   }

   _mulle_objc_runtime_assert_version( runtime, &info->version);

   if( info->version.foundation)
   {
      if( ! runtime->foundation.runtimefriend.versionassert)
      {
         loadinfo_dump( info, "loadinfo:   ");
         _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: foundation version set (0x%x), but runtime foundation provides no versionassert", runtime, info->version.foundation);
      }
      (*runtime->foundation.runtimefriend.versionassert)( runtime, &runtime->foundation, &info->version);
   }

   if( info->version.user)
   {
      if( ! runtime->userinfo.versionassert)
      {
         loadinfo_dump( info, "loadinfo:   ");
         _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: loadinfo user version set (0x%x), but runtime userinfo provides no versionassert", runtime, info->version.user);
      }
      (*runtime->userinfo.versionassert)( runtime, &runtime->userinfo, &info->version);
   }

   // pass runtime thru...
   need_sort = info->version.bits & _mulle_objc_loadinfo_unsorted;

   //
   // check for tagged pointers. What can happen ?
   // Remember that static strings can be tps!
   //
   // Runtime | Code   | Description
   // --------|--------|--------------
   // No-TPS  | No-TPS | Works
   // No-TPS  | TPS    | Crashes
   // TPS     | No-TPS | Works, but slower. Does not mix with "TPS Code"
   // TPS     | YES    | Works
   //
   // Allow loading of "NO TPS"-code into a "TPS" aware runtime as long
   // as no "TPS"-code is loaded also.
   //
   load_tps = ! (info->version.bits & _mulle_objc_loadinfo_notaggedptrs);

   bits     = _mulle_objc_runtime_get_loadbits( runtime);
   mismatch = (load_tps && (bits & MULLE_OBJC_RUNTIME_HAVE_NO_TPS_LOADS)) ||
              (! load_tps && (bits & MULLE_OBJC_RUNTIME_HAVE_TPS_LOADS));
   mismatch |= runtime->config.no_tagged_pointers && load_tps;
   if( mismatch)
   {
      loadinfo_dump( info, "loadinfo:   ");
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: the runtime is %sconfigured for tagged pointers, but classes are compiled differently",
                                                        runtime,
                                                        runtime->config.no_tagged_pointers ? "not " : "");
   }
   _mulle_objc_runtime_set_loadbit( runtime, load_tps ? MULLE_OBJC_RUNTIME_HAVE_TPS_LOADS
                                   : MULLE_OBJC_RUNTIME_HAVE_NO_TPS_LOADS);

   //
   // check for thread local runtime
   //
   // Runtime | Code   | Description
   // --------|--------|--------------
   // Global  | Global | Default
   // Global  | TRT    | Works, but slower. Mixes with "Global Code" too
   // TRT     | Global | Crashes
   // TRT     | TRT    | Works
   //
   if( runtime->config.thread_local_rt)
      if( ! (info->version.bits & _mulle_objc_loadinfo_threadlocalrt))
      {
         loadinfo_dump( info, "loadinfo:   ");
         _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: the runtime is thead local, but classes are compiled for a global runtime");
      }

   // make sure everything is compiled with say -O0 (or -O1 at least)
   // if u want to...
   optlevel = ((info->version.bits >> 16) & 0x7);
   if( optlevel < runtime->config.min_optlevel || optlevel > runtime->config.max_optlevel)
   {
      loadinfo_dump( info, "loadinfo:   ");
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: loadinfo was compiled with optimization level %d but runtime requires between (%d and %d)",
                                                        runtime,
                                                        optlevel,
                                                        runtime->config.min_optlevel,
                                                        runtime->config.max_optlevel);
   }

   if( runtime->debug.trace.loadinfo)
   {
      trace_preamble( runtime);
      fprintf( stderr, "loads loadinfo %p\n", info);
      loadinfo_dump( info, "   ");
   }

   // load strings in first, can be done unlocked
   if( info->loadstringlist)
      mulle_objc_loadstringlist_unfailing_enqueue( info->loadstringlist);
   if( runtime->debug.trace.dump_runtime)
      mulle_objc_dotdump_runtime_to_tmp();
   
   if( info->loadhashedstringlist)
      mulle_objc_loadhashedstringlist_unfailing_enqueue( info->loadhashedstringlist, need_sort);
   if( runtime->debug.trace.dump_runtime)
      mulle_objc_dotdump_runtime_to_tmp();

   _mulle_objc_runtime_waitqueues_lock( runtime);
   {
      //
      // serialize the classes and categories for +load!
      // see dox/load/LOAD.md for more info
      //
      //
      // the load-queue is kinda superflous, since we are single-threaded
      // but the sequencing is nicer, since all classes and categories in
      // .o are now loaded (should document this (or nix it)))
      //
      struct _mulle_objc_callqueue   loads;

      if( mulle_objc_callqueue_init( &loads, &runtime->memory.allocator))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);

      //
      // the wait-queues are maintained in the runtime
      //
      if( info->loadclasslist)
         mulle_objc_loadclasslist_unfailing_enqueue( info->loadclasslist, need_sort, &loads);
      if( info->loadcategorylist)
         mulle_objc_loadcategorylist_unfailing_enqueue( info->loadcategorylist, need_sort, &loads);

      mulle_objc_callqueue_walk( &loads, (void (*)()) call_load, runtime);
      mulle_objc_callqueue_done( &loads);
   }

   _mulle_objc_runtime_waitqueues_unlock( runtime);
}



