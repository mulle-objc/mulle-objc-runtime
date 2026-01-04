//
//  mulle_objc_load.c
//  mulle-objc-runtime
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
#pragma clang diagnostic ignored "-Wparentheses"

#include "mulle-objc-load.h"

#include "mulle-objc-builtin.h"
#include "mulle-objc-callqueue.h"
#include "mulle-objc-class.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-loadinfo.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-propertylist.h"
#include "mulle-objc-protocollist.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-universe-class.h"


#include "include-private.h"
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static struct _mulle_objc_dependency  no_dependency =
{
   MULLE_OBJC_NO_CLASSID,
   MULLE_OBJC_NO_CATEGORYID
};


static void
   mulle_objc_loadcategory_enqueue_nofail( struct _mulle_objc_loadcategory *info,
                                           struct _mulle_objc_callqueue *loads,
                                           struct _mulle_objc_universe *universe);

static void
   mulle_objc_loadclass_enqueue_nofail( struct _mulle_objc_loadclass *info,
                                        struct _mulle_objc_callqueue *loads,
                                        struct _mulle_objc_universe *universe);

typedef void   map_f_callback_t( void *,
                                 struct _mulle_objc_callqueue *,
                                 struct _mulle_objc_universe *);

// this is destructive
static void    map_f( struct mulle_concurrent_hashmap *table,
                      mulle_objc_uniqueid_t uniqueid,
                      map_f_callback_t *f,
                      struct _mulle_objc_callqueue *loads,
                      struct _mulle_objc_universe *universe)
{
   struct mulle_concurrent_pointerarray   *list;
   struct mulle_allocator                 *allocator;
   void                                   *value;

   if( ! table)
      return;

   list = _mulle_concurrent_hashmap_lookup( table, uniqueid);
   if( ! list)
      return;

   // because we are really single-threaded everything is much easier
   // just tear out the table (no-one can write into it concurrently)
   _mulle_concurrent_hashmap_remove( table, uniqueid, list);

   mulle_concurrent_pointerarray_for( list, value)
   {
      (*f)( value, loads, universe);
   }

   _mulle_concurrent_pointerarray_done( list);

   allocator = _mulle_objc_universe_get_allocator( universe);
   _mulle_allocator_free( allocator, list);
}


static mulle_objc_implementation_t
   _mulle_objc_methodlist_bsearch_dependencies_imp( struct _mulle_objc_methodlist *methods)
{
   struct _mulle_objc_method      *method;
   mulle_objc_implementation_t    imp;

   method = mulle_objc_method_bsearch( methods->methods,
                                       methods->n_methods,
                                       MULLE_OBJC_DEPENDENCIES_METHODID);
   if( ! method)
      return( 0);

   imp = _mulle_objc_method_get_implementation( method);
   return( imp);
}


static struct _mulle_objc_dependency
   _mulle_objc_universe_fulfill_dependencies( struct _mulle_objc_universe *universe,
                                              struct _mulle_objc_infraclass *infra,
                                              struct _mulle_objc_dependency *dependencies)
{
   struct _mulle_objc_classpair    *pair;

   while( dependencies->classid)
   {
      if( universe->debug.trace.dependency)
         mulle_objc_universe_trace( universe, "+dependencies check class %08lx \"%s\" ...",
                             (unsigned long)  dependencies->classid,
                             _mulle_objc_universe_describe_classid( universe, dependencies->classid));

      if( ! infra || (_mulle_objc_infraclass_get_classid( infra) != dependencies->classid))
      {
         infra = _mulle_objc_universe_lookup_infraclass( universe, dependencies->classid);
         if( ! infra)
         {
            if( universe->debug.trace.dependency)
            {
               mulle_objc_universe_trace( universe,
                                          "+dependencies class %08lx \"%s\" is not present yet",
                                          (unsigned long) dependencies->classid,
                                          _mulle_objc_universe_describe_classid( universe, dependencies->classid));
            }
            return( *dependencies);
         }
      }

      if( dependencies->categoryid)
      {
         if( universe->debug.trace.dependency)
            mulle_objc_universe_trace( universe, "+dependencies check category %08lx,%08lx \"%s( %s)\" ....",
                                          (unsigned long) dependencies->classid,
                                          (unsigned long) dependencies->categoryid,
                                          _mulle_objc_universe_describe_classid( universe, dependencies->classid),
                                          _mulle_objc_universe_describe_categoryid( universe, dependencies->categoryid));

         pair = _mulle_objc_infraclass_get_classpair( infra);
         if( ! _mulle_objc_classpair_has_categoryid( pair, dependencies->categoryid))
         {
            if( universe->debug.trace.dependency)
            {
               mulle_objc_universe_trace( universe,
                                          "+dependencies category %08lx,%08lx \"%s( %s)\" is not present yet",
                                          (unsigned long) dependencies->classid,
                                          (unsigned long) dependencies->categoryid,
                                          _mulle_objc_universe_describe_classid( universe, dependencies->classid),
                                          _mulle_objc_universe_describe_categoryid( universe, dependencies->categoryid));
            }
            return( *dependencies);
         }
      }
      ++dependencies;
   }

   return( no_dependency);
}


#pragma mark - classes

static void  loadclass_fprintf( FILE *fp,
                               struct _mulle_objc_loadclass *info)
{
   fprintf( fp, "class %08lx \"%s\" (%p)",
           (unsigned long) info->classid, info->classname, info);
}


static void  loadclass_trace( struct _mulle_objc_loadclass *info,
                              struct _mulle_objc_universe *universe,
                              char *format, ...)
{
   va_list   args;

   mulle_objc_universe_trace_nolf( universe, "");
   loadclass_fprintf( stderr, info);
   fputc( ' ', stderr);

   va_start( args, format);
   vfprintf( stderr, format, args);
   va_end( args);

   if( info->origin && universe->debug.print.print_origin)
      fprintf( stderr, " (%s)", info->origin);
   fputc( '\n', stderr);
}


static struct mulle_concurrent_pointerarray   *
   _mulle_objc_map_append_info( struct mulle_concurrent_hashmap *map,
                                mulle_objc_classid_t missingclassid,
                                void *info,
                                struct mulle_allocator *allocator)
{
   struct mulle_concurrent_pointerarray   *list;
   int                                    rval;

   for(;;)
   {
      list = _mulle_concurrent_hashmap_lookup( map, missingclassid);
      if( list)
         break;
      list = _mulle_allocator_calloc( allocator, 1, sizeof( *list));
      _mulle_concurrent_pointerarray_init( list, 1, allocator);

      rval = _mulle_concurrent_hashmap_insert( map, missingclassid, list);
      if( ! rval)
         break;

      // this can't really happen anymore can it ?
      assert( 0);

      _mulle_concurrent_pointerarray_done( list);
      _mulle_allocator_free( allocator, list);  // assume it was never
      if( rval != EEXIST)
         return( NULL);
   }

   _mulle_concurrent_pointerarray_add( list, info);
   return( list);
}


static int   mulle_objc_loadclass_delayedadd( struct _mulle_objc_loadclass *info,
                                              mulle_objc_classid_t missingclassid,
                                              struct _mulle_objc_universe *universe)
{
   struct mulle_concurrent_pointerarray   *list;

   if( ! info)
      mulle_objc_universe_fail_code( universe, EINVAL);

   assert( info->classid != missingclassid);

   list = _mulle_objc_map_append_info( &universe->waitqueues.classestoload,
                                       missingclassid,
                                       info,
                                       &universe->memory.allocator);
   if( ! list)
      mulle_objc_universe_fail_errno( universe);

   if( universe->debug.trace.dependency)
      loadclass_trace( info, universe,
                      "waits on list %p for class %08lx \"%s\" to load "
                      "(or to gain more categories) ",
                      list,
                      (unsigned long) missingclassid,
                      _mulle_objc_universe_describe_classid( universe, missingclassid));

   return( 0);
}


static struct _mulle_objc_dependency
    _mulle_objc_loadclass_fulfill_user_dependencies( struct _mulle_objc_loadclass *info,
                                                     struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_dependency   *dependencies;
   mulle_objc_implementation_t     imp;

   if( ! info->classmethods)
      return( no_dependency);

   imp = _mulle_objc_methodlist_bsearch_dependencies_imp( info->classmethods);
   if( ! imp)
      return( no_dependency);

   if( universe->debug.trace.dependency)
      loadclass_trace( info, universe, "call +[%s dependencies]", info->classname);

   // because we do a "fake" (non-self) call here, we don't route this through
   // mulle_objc_implementation_invoke, which would crash
   dependencies =  (*imp)( NULL, MULLE_OBJC_DEPENDENCIES_METHODID, NULL);
   if( ! dependencies)
      mulle_objc_universe_fail_generic( universe, "error in mulle_objc_universe %p: %s "
                                                  "returned NULL for +dependencies\n",
                                                  universe,
                                                  info->classname);

   return( _mulle_objc_universe_fulfill_dependencies( universe, NULL, dependencies));
}


static struct _mulle_objc_dependency
    _mulle_objc_loadclass_fulfill_dependencies( struct _mulle_objc_loadclass *info,
                                                struct _mulle_objc_universe *universe,
                                                struct _mulle_objc_infraclass  **p_superclass)
{
   struct _mulle_objc_infraclass    *protocolclass;
   struct _mulle_objc_infraclass    *superclass;
   mulle_objc_classid_t             *classid_p;
   struct _mulle_objc_dependency    dependency;

   assert( info);
   assert( universe);
   assert( p_superclass);

   dependency = no_dependency;

   superclass    = NULL;
   *p_superclass = NULL;

#if 0
   if( universe->debug.trace.dependency)
      loadclass_trace( info, universe, "dependency check superclass %08lx \"%s\" ...",
                       info->superclassid,
                       info->superclassname);
#endif

   if( info->superclassid)
   {
      superclass    = _mulle_objc_universe_lookup_infraclass( universe, info->superclassid);
      *p_superclass = superclass;

      if( ! superclass)
      {
         if( universe->debug.trace.dependency)
           loadclass_trace( info, universe, "superclass %08lx \"%s\" is "
                                            "not present yet",
                                            (unsigned long) info->superclassid,
                                            info->superclassname);
         dependency.classid    = info->superclassid;
         dependency.categoryid = MULLE_OBJC_NO_CATEGORYID;
         return( dependency);
      }

      if( strcmp( info->superclassname, superclass->base.name))
         mulle_objc_universe_fail_generic( universe,
             "error in mulle_objc_universe %p: hash collision %08lx "
             "for classnames \"%s\" \"%s\"",
             universe,
             (unsigned long) info->superclassid,
             info->superclassname,
             superclass->base.name);
   }

   if( superclass && superclass->ivarhash != info->superclassivarhash)
   {
      if( ! universe->config.ignore_ivarhash_mismatch)
         mulle_objc_universe_fail_generic( universe,
              "error in mulle_objc_universe %p: superclass \"%s\" of \"%s\" "
              "has changed. Recompile \"%s\" (%s).\n",
              universe,
              info->superclassname,
              info->classname,
              info->classname,
              info->origin ? info->origin : "<unknown file>");
   }

   // protocol classes present ?
   if( info->protocolclassids)
   {
      for( classid_p = info->protocolclassids; *classid_p; ++classid_p)
      {
         // avoid duplication and waiting for seld
         if( *classid_p == info->superclassid || *classid_p == info->classid)
            continue;
#if 0
         if( universe->debug.trace.dependency)
            loadclass_trace( info, universe, "dependency check protocolclass %08lx \"%s\" ...",
                             *classid_p,
                             _mulle_objc_universe_describe_classid( universe, *classid_p));
#endif
         protocolclass = _mulle_objc_universe_lookup_infraclass( universe, *classid_p);
         if( ! protocolclass)
         {
            if( universe->debug.trace.dependency)
            {
               loadclass_trace( info, universe, "protocolclass %08lx \"%s\" is "
                                                "not present yet",
                               (unsigned long) *classid_p,
                               _mulle_objc_universe_describe_classid( universe, *classid_p));
            }

            dependency.classid    = *classid_p;
            dependency.categoryid = MULLE_OBJC_NO_CATEGORYID;
            return( dependency);
         }
      }
   }

   return( _mulle_objc_loadclass_fulfill_user_dependencies( info, universe));
}


void
   mulle_objc_loadclass_print_unfulfilled_dependency( struct _mulle_objc_loadclass *info,
                                                      struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_dependency   dependency;
   struct _mulle_objc_infraclass   *infra;
   char                            *s_class;

   if( ! info || ! universe)
      return;

   dependency = _mulle_objc_loadclass_fulfill_dependencies( info, universe, &infra);
   if( dependency.classid == MULLE_OBJC_NO_CLASSID)
      return;

   s_class = _mulle_objc_universe_describe_classid( universe, dependency.classid);
   if( universe->debug.print.stuck_class_coverage)
      printf( "%08lx;%s;;\n", (unsigned long) dependency.classid, s_class);

   if( universe->debug.trace.waiters_svg)
      fprintf( stderr, "\t\"%s\" -> \"%s\" [ label=\" waits for\" ]\n",
              info->classname,
              s_class);
   else
      fprintf( stderr, "\t%08lx \"%s\" -> %08lx \"%s\" [ label=\" waiting for class\" ]\n",
              (unsigned long) info->classid, info->classname,
              (unsigned long) dependency.classid, s_class);
}


// ensure the load class, minimally makes sense
static int  mulle_objc_loadclass_is_sane( struct _mulle_objc_loadclass *info)
{
   if( ! info)
      return( 0);

   if( ! info->classname)
      return( 0);

   if( ! mulle_objc_uniqueid_is_sane_string( info->classid, info->classname))
      return( 0);

   // class method lists should have no owner
   if( info->classmethods && info->classmethods->loadcategory)
      return( 0);
   if( info->instancemethods && info->instancemethods->loadcategory)
      return( 0);

   return( 1);
}

//
// We call a method classExtraSize which should just return a size_t value
// it must not call any Objective-C code and in fact self is also passed as
// nil.
//
// useless: just use a static variable in the @implementation
//
static size_t    call_classExtraSize( struct _mulle_objc_methodlist  *list)
{
   size_t                        extrasize;
   mulle_objc_implementation_t   imp;
   struct _mulle_objc_method     *method;

   extrasize = 0;
   if( list)
   {
      method = _mulle_objc_methodlist_search( list, 0x185d8c27); // classExtraSize
      if( method)
      {
         imp       = _mulle_objc_method_get_implementation( method);
         extrasize = (size_t) (*imp)( NULL, 0x185d8c27, NULL);
      }
   }
   return( extrasize);
}


static mulle_objc_classid_t   _mulle_objc_loadclass_enqueue( struct _mulle_objc_loadclass *info,
                                                             struct _mulle_objc_callqueue *loads,
                                                             struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_infraclass   *superclass;
   struct _mulle_objc_dependency   dependency;
   size_t                          extrasize;

   // root ?
   superclass = NULL;
   dependency = _mulle_objc_loadclass_fulfill_dependencies( info, universe, &superclass);
   if( dependency.classid != MULLE_OBJC_NO_CLASSID)
      return( dependency.classid);

   //
   // for those that reverse order their .o files in a shared library
   // categories of a class then the class
   // subclass first then superclass
   // this callqueue mechanism does the "right" thing
   //
   // ready to install

   extrasize = call_classExtraSize( info->classmethods);

   pair = mulle_objc_universe_new_classpair( universe, info->classid,
                                                       info->classname,
                                                       info->instancesize,
                                                       extrasize,
                                                       superclass);
   if( ! pair)
      mulle_objc_universe_fail_errno( universe);  // unfailing vectors through there

   _mulle_objc_classpair_set_loadclass( pair, info);
   mulle_objc_classpair_add_protocollist_nofail( pair, info->protocols);
   mulle_objc_classpair_add_protocolclassids_nofail( pair, info->protocolclassids);

   meta = _mulle_objc_classpair_get_metaclass( pair);

   mulle_objc_metaclass_add_methodlist_nofail( meta, info->classmethods);
   mulle_objc_methodlist_add_load_to_callqueue_nofail( info->classmethods, meta, loads);

   infra = _mulle_objc_classpair_get_infraclass( pair);
   assert( meta == _mulle_objc_class_get_metaclass( &infra->base));

   _mulle_objc_infraclass_set_ivarhash( infra, info->classivarhash);

   mulle_objc_infraclass_add_ivarlist_nofail( infra, info->instancevariables);
   mulle_objc_infraclass_add_methodlist_nofail( infra, info->instancemethods);
   mulle_objc_infraclass_add_propertylist_nofail( infra, info->properties);

   if( info->fastclassindex >= 0)
      _mulle_objc_universe_set_fastclass( universe, infra, info->fastclassindex);

   if( mulle_objc_universe_register_infraclass( universe, infra))
   {
      if( errno == EFAULT)
         mulle_objc_universe_fail_generic( universe,
               "error in mulle_objc_universe %p: "
               "superclass %08lx \"%s\" of class %08lx \"%s\" does not exist.\n",
                universe,
                superclass ? (unsigned long) superclass->base.classid : 0L,  // analyzer...
                superclass ? superclass->base.name : "nil", // analyzer...
                (unsigned long) infra->base.classid, infra->base.name);

      if( errno == EEXIST)
         mulle_objc_universe_fail_generic( universe,
               "error in mulle_objc_universe %p: "
               "duplicate class %08lx \"%s\".\n",
                universe, (unsigned long) infra->base.classid, infra->base.name);

      mulle_objc_universe_fail_generic( universe,
            "error addding class %08lx \"%s\" to mulle_objc_universe %p "
            "errno=%d\n",
            (unsigned long) infra->base.classid, infra->base.name, universe, errno);
   }

   //
   // check if categories or classes are waiting for us ?
   //
   map_f( &universe->waitqueues.categoriestoload,
         info->classid,
         (map_f_callback_t *) mulle_objc_loadcategory_enqueue_nofail,
         loads,
         universe);
   map_f( &universe->waitqueues.classestoload,
         info->classid,
         (map_f_callback_t *) mulle_objc_loadclass_enqueue_nofail,
         loads,
         universe);

   return( MULLE_OBJC_NO_CLASSID);
}


static void
   mulle_objc_loadclass_enqueue_nofail( struct _mulle_objc_loadclass *info,
                                        struct _mulle_objc_callqueue *loads,
                                        struct _mulle_objc_universe *universe)
{
   mulle_objc_classid_t   missingclassid;

   // possibly get or create universe..

   if( ! mulle_objc_loadclass_is_sane( info))
      mulle_objc_universe_fail_code( universe, EINVAL);

   missingclassid = _mulle_objc_loadclass_enqueue( info, loads, universe);
   if( missingclassid != MULLE_OBJC_NO_CLASSID)
      if( mulle_objc_loadclass_delayedadd( info, missingclassid, universe))
         mulle_objc_universe_fail_errno( universe);
}


static void   _mulle_objc_loadclass_sort_lists( struct _mulle_objc_loadclass *lcls)
{
   mulle_qsort_r( lcls->protocolclassids,
                  _mulle_objc_uniqueid_arraycount( lcls->protocolclassids),
                  sizeof( mulle_objc_protocolid_t),
                  _mulle_objc_uniqueid_compare_r,
                  NULL);
   mulle_objc_ivarlist_sort( lcls->instancevariables);
   mulle_objc_methodlist_sort( lcls->instancemethods);
   mulle_objc_methodlist_sort( lcls->classmethods);
   mulle_objc_propertylist_sort( lcls->properties);
   mulle_objc_protocollist_sort( lcls->protocols);
}




#pragma mark - classlists

static void   mulle_objc_loadclasslist_enqueue_nofail( struct _mulle_objc_loadclasslist *list,
                                                       int need_sort,
                                                       struct _mulle_objc_callqueue *loads,
                                                       struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_loadclass   **p_class;
   struct _mulle_objc_loadclass   **sentinel;

   if( ! list)
      return;

   p_class = list->loadclasses;
   sentinel = &p_class[ list->n_loadclasses];
   while( p_class < sentinel)
   {
      if( need_sort)
         _mulle_objc_loadclass_sort_lists( *p_class);

      mulle_objc_loadclass_enqueue_nofail( *p_class, loads, universe);
      p_class++;
   }
}


#pragma mark - categories


static void  loadcategory_fprintf( FILE *fp,
                            struct _mulle_objc_loadcategory *info)
{
   fprintf( fp, "category %08lx,%08lx \"%s( %s)\" (%p)",
           (unsigned long) info->classid, (unsigned long) info->categoryid,
           info->classname, info->categoryname,
           info);
}


static void  loadcategory_trace( struct _mulle_objc_loadcategory *info,
                                 struct _mulle_objc_universe *universe,
                                 char *format, ...)
{
   va_list   args;

   mulle_objc_universe_trace_nolf( universe, "");
   loadcategory_fprintf( stderr, info);
   fputc( ' ', stderr);

   va_start( args, format);
   vfprintf( stderr, format, args);
   va_end( args);

   if( info->origin && universe->debug.print.print_origin)
      fprintf( stderr, " (%s)", info->origin);
   fputc( '\n', stderr);
}



static int  mulle_objc_loadcategory_delayedadd( struct _mulle_objc_loadcategory *info,
                                                mulle_objc_classid_t missingclassid,
                                                struct _mulle_objc_universe *universe)
{
   struct mulle_concurrent_pointerarray   *list;

   if( ! info)
   {
      errno = EINVAL;
      return( -1);
   }

   list = _mulle_objc_map_append_info( &universe->waitqueues.categoriestoload,
                                       missingclassid,
                                       info,
                                       &universe->memory.allocator);
   if( ! list)
      mulle_objc_universe_fail_errno( universe);

   if( universe->debug.trace.dependency)
      loadcategory_trace( info, universe, "waits for class %08lx \"%s\" to load "
                         "or gain more categories on list %p",
                         (unsigned long) missingclassid,
                         _mulle_objc_universe_describe_classid( universe, missingclassid),
                         list);

   return( 0);
}


static struct _mulle_objc_dependency
   _mulle_objc_loadcategory_fulfill_user_dependencies( struct _mulle_objc_loadcategory *info,
                                                       struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_universe     *universe;
   struct _mulle_objc_dependency   *dependencies;
   mulle_objc_implementation_t     imp;

   assert( info);
   assert( infra);

   if( ! info->classmethods)
      return( no_dependency);

   imp = _mulle_objc_methodlist_bsearch_dependencies_imp( info->classmethods);
   if( ! imp)
      return( no_dependency);

   universe = _mulle_objc_infraclass_get_universe( infra);
   if( universe->debug.trace.dependency)
      loadcategory_trace( info, universe, "call +[%s(%s) dependencies]",
              info->classname,
              info->categoryname);

   dependencies = (*imp)( infra, MULLE_OBJC_DEPENDENCIES_METHODID, infra);
   if( ! dependencies)
      mulle_objc_universe_fail_generic( universe,
                                               "error in mulle_objc_universe %p: "
                                               "%s(%s) returned NULL for "
                                               "+dependencies\n",
                                               universe,
                                               info->classname,
                                               info->categoryname);

   return( _mulle_objc_universe_fulfill_dependencies( universe, infra, dependencies));
}


static struct _mulle_objc_dependency
   _mulle_objc_loadcategory_fulfill_dependencies( struct _mulle_objc_loadcategory *info,
                                                  struct _mulle_objc_universe *universe,
                                                  struct _mulle_objc_infraclass **p_class)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_infraclass   *protocolclass;
   mulle_objc_classid_t            *classid_p;
   struct _mulle_objc_dependency   dependency;

   assert( info);
   assert( universe);
   assert( p_class);

#if 0
   if( universe->debug.trace.dependency)
      loadcategory_trace( info, universe, "dependency check ...");
#endif
   // check class
   infra    = _mulle_objc_universe_lookup_infraclass( universe, info->classid);
   *p_class = infra;


   if( ! infra)
   {
      if( universe->debug.trace.dependency)
      {
         loadcategory_trace( info, universe,
                             "its class %08lx \"%s\" is not present yet",
                             (unsigned long) info->classid,
                             info->classname);
      }
      dependency.classid    = info->classid;
      dependency.categoryid = MULLE_OBJC_NO_CATEGORYID;
      return( dependency);
   }

   // protocol classes present ?
   if( info->protocolclassids)
   {
      for( classid_p = info->protocolclassids; *classid_p; ++classid_p)
      {
         // avoid duplication
         if( *classid_p == info->classid)
            continue;

         protocolclass = _mulle_objc_universe_lookup_infraclass( universe, *classid_p);
         if( ! protocolclass)
         {
            if( universe->debug.trace.dependency)
            {
               loadcategory_trace( info, universe,
                                   "protocolclass %08lx \"%s\" is not present yet",
                                   (unsigned long) *classid_p,
                                   _mulle_objc_universe_describe_classid( universe, *classid_p));
            }
            dependency.classid    = *classid_p;
            dependency.categoryid = MULLE_OBJC_NO_CATEGORYID;
            return( dependency);
         }
      }
   }

   return( _mulle_objc_loadcategory_fulfill_user_dependencies( info, infra));
}


void
   mulle_objc_loadcategory_print_unfulfilled_dependency( struct _mulle_objc_loadcategory *info,
                                                         struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_dependency   dependency;
   struct _mulle_objc_infraclass   *infra;
   int                             old;
   char                            *s_class;
   char                            *s_category;

   if( ! info || ! universe)
      return;

   infra = NULL;

   // turn this off (it annoys) -- we are assumed to be single-threaded here
   // anyway

   old = universe->debug.trace.dependency;
   universe->debug.trace.dependency = 0;
   {
      dependency = _mulle_objc_loadcategory_fulfill_dependencies( info, universe, &infra);
   }
   universe->debug.trace.dependency = old;

   if( dependency.classid == MULLE_OBJC_NO_CLASSID)
      return;

   s_class = _mulle_objc_universe_describe_classid( universe, dependency.classid);
   if( dependency.categoryid == MULLE_OBJC_NO_CATEGORYID)
   {
      if( universe->debug.print.stuck_class_coverage)
         printf( "%08lx;%s;;\n", (unsigned long) dependency.classid, s_class);

      if( universe->debug.trace.waiters_svg)
         fprintf( stderr, "\t\"%s( %s)\" -> \"%s\" [label=\" waits for\" ]\n",
                 info->classname, info->categoryname,
                 s_class);
      else
         fprintf( stderr, "\tCategory %08lx,%08lx \"%s( %s)\" is waiting for "
                          "class %08lx \"%s\"\n",
                 (unsigned long) info->classid, (unsigned long) info->categoryid,
                 info->classname, info->categoryname,
                 (unsigned long) dependency.classid,
                 s_class);
      return;
   }

   s_category = _mulle_objc_universe_describe_categoryid( universe, dependency.categoryid);
   if( universe->debug.print.stuck_category_coverage)
      printf( "%08lx;%s;%08lx;%s\n", (unsigned long) dependency.classid, s_class, (unsigned long) dependency.categoryid, s_category);

   if( universe->debug.trace.waiters_svg)
      fprintf( stderr, "\t\"%s( %s)\" ->  \"%s( %s)\" [ label=\" waits for\" ]\n",
              info->classname, info->categoryname,
              s_class,
              s_category);
   else
      fprintf( stderr, "\tCategory %08lx,%08lx \"%s( %s)\" is waiting for "
                       "category %08lx,%08lx \"%s( %s)\"\n",
              (unsigned long) info->classid, (unsigned long) info->categoryid,
              info->classname, info->categoryname,
              (unsigned long) dependency.classid,
              (unsigned long) dependency.categoryid,
              s_class,
              s_category);
}


// ensure the load category, minimally makes sense
static int  mulle_objc_loadcategory_is_sane( struct _mulle_objc_loadcategory *info)
{
   if( ! info || ! info->categoryname || ! info->classname)
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! mulle_objc_uniqueid_is_sane_string( info->classid, info->classname))
      return( 0);
   if( ! mulle_objc_uniqueid_is_sane_string( info->categoryid, info->categoryname))
      return( 0);

   return( 1);
}


static void  fail_duplicate_category( struct _mulle_objc_classpair *pair,
                                      struct _mulle_objc_loadcategory *info)
{
   struct _mulle_objc_universe    *universe;
   struct _mulle_objc_methodlist  *category;
   struct _mulle_objc_infraclass  *infra;
   struct _mulle_objc_metaclass   *meta;
   struct _mulle_objc_class       *cls;
   char                           *info_origin;
   char                           *pair_origin;

   universe    = _mulle_objc_classpair_get_universe( pair);
   infra       = _mulle_objc_classpair_get_infraclass( pair);
   cls         = _mulle_objc_infraclass_as_class( infra);
   category    = mulle_objc_class_find_methodlist( cls, info->categoryid);
   if( ! category)
   {
      meta     = _mulle_objc_classpair_get_metaclass( pair);
      cls      = _mulle_objc_metaclass_as_class( meta);
      category = mulle_objc_class_find_methodlist( cls, info->categoryid);
   }

   pair_origin = mulle_objc_methodlist_get_categoryorigin( category);
   if( ! pair_origin)
      pair_origin = "<unknown origin>";

   info_origin = info->origin;
   if( ! info_origin)
      info_origin = "<unknown origin>";

   mulle_objc_universe_fail_generic( universe,
      "error in mulle_objc_universe %p: category %08lx \"%s( %s)\" (%s) "
      "is already present in class %08lx \"%s\" (%s).\n",
          universe,
          (unsigned long) info->categoryid,
          info->classname,
          info->categoryname ? info->categoryname : "<optimized away>",
          info_origin,
          (unsigned long) _mulle_objc_classpair_get_classid( pair),
          _mulle_objc_classpair_get_name( pair),
          pair_origin);
}



static mulle_objc_classid_t
   _mulle_objc_loadcategory_enqueue( struct _mulle_objc_loadcategory *info,
                                     struct _mulle_objc_callqueue *loads,
                                     struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_dependency   dependency;

   infra      = NULL;
   dependency = _mulle_objc_loadcategory_fulfill_dependencies( info, universe, &infra);
   if( dependency.classid != MULLE_OBJC_NO_CLASSID)
      return( dependency.classid);

   if( strcmp( info->classname, infra->base.name))
      mulle_objc_universe_fail_generic( universe,
         "error in mulle_objc_universe %p: hashcollision %08lx "
         "for classnames \"%s\" and \"%s\"\n",
            universe, (unsigned long) info->classid, info->classname, infra->base.name);
   if( info->classivarhash != infra->ivarhash)
      mulle_objc_universe_fail_generic( universe,
         "error in mulle_objc_universe %p: class %08lx \"%s\" of "
         "category %08lx \"%s( %s)\" has changed (load:0x%lx->class:0x%lx). Recompile %s\n",
            universe,
            (unsigned long) info->classid,
            info->classname,
            (unsigned long) info->categoryid,
            info->classname,
            info->categoryname ? info->categoryname : "<optimized away>",
            (unsigned long) info->classivarhash,
            (unsigned long) infra->ivarhash,
            info->origin ? info->origin : "<optimized away>");

   pair = _mulle_objc_infraclass_get_classpair( infra);
   meta = _mulle_objc_classpair_get_metaclass( pair);

   if( info->categoryid && _mulle_objc_classpair_has_categoryid( pair, info->categoryid))
      fail_duplicate_category( pair, info);

   // checks for hash collisions
   mulle_objc_universe_register_category_nofail( universe, info->categoryid, info->categoryname);

   // the loader sets the categoryid as owner
   if( info->instancemethods && info->instancemethods->n_methods)
   {
      info->instancemethods->loadcategory = info;
      if( mulle_objc_class_add_methodlist( &infra->base, info->instancemethods))
         mulle_objc_universe_fail_errno( universe);
   }
   if( info->classmethods && info->classmethods->n_methods)
   {
      info->classmethods->loadcategory = info;
      if( mulle_objc_class_add_methodlist( &meta->base, info->classmethods))
         mulle_objc_universe_fail_errno( universe);
   }

   if( info->properties && info->properties->n_properties)
      if( mulle_objc_infraclass_add_propertylist( infra, info->properties))
         mulle_objc_universe_fail_errno( universe);

   //
   // TODO need to check that protocolids name are actually correct
   // emit protocolnames together with ids. Keep central directory in
   // universe
   //
   mulle_objc_classpair_add_protocollist_nofail( pair, info->protocols);
   mulle_objc_classpair_add_protocolclassids_nofail( pair, info->protocolclassids);
   if( info->categoryid)
      mulle_objc_classpair_add_categoryid_nofail( pair, info->categoryid);

   // this queues things up
   mulle_objc_methodlist_add_load_to_callqueue_nofail( info->classmethods, meta, loads);

   //
   // retrigger those who are waiting for their dependencies
   //
   map_f( &universe->waitqueues.categoriestoload,
          info->classid,
          (map_f_callback_t *) mulle_objc_loadcategory_enqueue_nofail,
          loads,
          universe);
   map_f( &universe->waitqueues.classestoload,
          info->classid,
          (map_f_callback_t *) mulle_objc_loadclass_enqueue_nofail,
          loads,
          universe);

   return( MULLE_OBJC_NO_CLASSID);
}



static void
   mulle_objc_loadcategory_enqueue_nofail( struct _mulle_objc_loadcategory *info,
                                           struct _mulle_objc_callqueue *loads,
                                           struct _mulle_objc_universe *universe)
{
   mulle_objc_classid_t   missingclassid;

   if( ! mulle_objc_loadcategory_is_sane( info))
      mulle_objc_universe_fail_code( universe, EINVAL);

   missingclassid = _mulle_objc_loadcategory_enqueue( info, loads, universe);
   if( missingclassid != MULLE_OBJC_NO_CLASSID)
      if( mulle_objc_loadcategory_delayedadd( info, missingclassid, universe))
         mulle_objc_universe_fail_errno( universe);
}


# pragma mark - categorylists

static void   _mulle_objc_loadcategory_sort_lists( struct _mulle_objc_loadcategory *lcat)
{
   mulle_qsort_r( lcat->protocolclassids,
                  _mulle_objc_uniqueid_arraycount( lcat->protocolclassids),
                  sizeof( mulle_objc_protocolid_t),
                  _mulle_objc_uniqueid_compare_r,
                  NULL);

   mulle_objc_methodlist_sort( lcat->instancemethods);
   mulle_objc_methodlist_sort( lcat->classmethods);
   mulle_objc_propertylist_sort( lcat->properties);
   mulle_objc_protocollist_sort( lcat->protocols);
}


static void   mulle_objc_loadcategorylist_enqueue_nofail( struct _mulle_objc_loadcategorylist *list,
                                                          int need_sort,
                                                          struct _mulle_objc_callqueue *loads,
                                                          struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_loadcategory   **p_category;
   struct _mulle_objc_loadcategory   **sentinel;

   if( ! list)
      return;

   p_category = list->loadcategories;
   sentinel   = &p_category[ list->n_loadcategories];
   while( p_category < sentinel)
   {
      if( need_sort)
         _mulle_objc_loadcategory_sort_lists( *p_category);

      mulle_objc_loadcategory_enqueue_nofail( *p_category, loads, universe);
      p_category++;
   }
}

# pragma mark - stringlists

static void   mulle_objc_loadstringlist_enqueue_nofail( struct _mulle_objc_loadstringlist *list,
                                                        struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_object     **p_string;
   struct _mulle_objc_object     **sentinel;

   if( ! list)
      return;

   p_string = list->loadstrings;
   sentinel = &p_string[ list->n_loadstrings];

   // memo: the actual staticstringclass is likely not installed yet

   while( p_string < sentinel)
   {
      _mulle_objc_universe_add_staticstring( universe, *p_string);
      p_string++;
   }
}


# pragma mark - hashedstring

char   *_mulle_objc_loadhashedstring_bsearch( struct _mulle_objc_loadhashedstring *buf,
                                              unsigned int n,
                                              mulle_objc_uniqueid_t search)
{
   int   first;
   int   last;
   int   middle;
   struct _mulle_objc_loadhashedstring   *p;

   assert( mulle_objc_uniqueid_is_sane( search));

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


char   *_mulle_objc_loadhashedstring_find( struct _mulle_objc_loadhashedstring *buf,
                                           unsigned int n,
                                           mulle_objc_uniqueid_t search)
{
   struct _mulle_objc_loadhashedstring   *p;
   struct _mulle_objc_loadhashedstring   *sentinel;

   p        = buf;
   sentinel = &p[ n];
   while( p < sentinel)
   {
      if( p->uniqueid == search)
         return( p->string);
      ++p;
   }
   return( NULL);
}


int  _mulle_objc_loadhashedstring_compare_r( void *_a, void *_b, void *thunk)
{
   struct _mulle_objc_loadhashedstring    *a = _a;
   struct _mulle_objc_loadhashedstring    *b = _b;
   mulle_objc_uniqueid_t                  a_id;
   mulle_objc_uniqueid_t                  b_id;

   MULLE_C_UNUSED( thunk);

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

   mulle_qsort_r( methods,
                  n,
                  sizeof( struct _mulle_objc_loadhashedstring),
                  _mulle_objc_loadhashedstring_compare_r,
                  NULL);
}


int  mulle_objc_loadhashedstring_is_sane( struct _mulle_objc_loadhashedstring *p)
{
   if( ! p || ! p->string)
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! mulle_objc_uniqueid_is_sane_string( p->uniqueid, p->string))
      return( 0);

   return( 1);
}


# pragma mark - loadsuperlist

static void   mulle_objc_loadsuperlist_enqueue_nofail( struct _mulle_objc_superlist *list,
                                                       struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_super   *p;
   struct _mulle_objc_super   *sentinel;

   if( ! list)
      return;

   p        = &list->supers[ 0];
   sentinel = &p[ list->n_supers];
   while( p < sentinel)
   {
      mulle_objc_universe_register_super_nofail( universe, p);
      ++p;
   }
}





# pragma mark - hashedstringlists

static void   mulle_objc_loadhashedstringlist_enqueue_nofail( struct _mulle_objc_loadhashedstringlist *map,
                                                              struct _mulle_objc_universe   *universe,
                                                              int need_sort)
{
   if( ! map || ! map->n_loadentries)
   {
      if( universe->debug.trace.hashstrings)
         mulle_objc_universe_trace( universe,
                                    "empty hashstrings %p",
                                    map);
      return;
   }

   if( need_sort)
      mulle_objc_loadhashedstringlist_sort( map);

   if( universe->debug.trace.hashstrings)
   {
      struct _mulle_objc_loadhashedstring  *p;
      struct _mulle_objc_loadhashedstring  *sentinel;
      unsigned int                         i;

      p        = map->loadentries;
      sentinel = &p[ map->n_loadentries];
      i        = 0;
      while( p < sentinel)
      {
         mulle_objc_universe_trace( universe,
                                   "#%d: %x is \"%s\"",
                                   i++,
                                   p->uniqueid,
                                   p->string);
         ++p;
      }

   }

   _mulle_objc_universe_add_loadhashedstringlist( universe, map);
}


# pragma mark - info

static void   call_load( struct _mulle_objc_metaclass *meta,
                         mulle_objc_methodid_t sel,
                         mulle_objc_implementation_t imp,
                         struct _mulle_objc_universe *universe)
{
   struct _mulle_objc_infraclass   *infra;

   if( universe->debug.trace.initialize)
     mulle_objc_universe_trace( universe,
                                "%08lx \"%s\" call +[%s load]",
                                (unsigned long) _mulle_objc_metaclass_get_classid( meta),
                                _mulle_objc_metaclass_get_name( meta),
                                _mulle_objc_metaclass_get_name( meta));

   // the "meta" class is not the object passed
   infra = _mulle_objc_metaclass_get_infraclass( meta);
   // we don't invoke `_mulle_objc_implementation_debug`, because we don't want
   // to trigger +initialize

   (*imp)( infra, sel, infra);
}


void    mulle_objc_universe_assert_loadinfo( struct _mulle_objc_universe *universe,
                                             struct _mulle_objc_loadinfo *info)
{
   unsigned int   optlevel;
   int            load_tps;
   int            load_tao;
   int            mismatch;
   uintptr_t      bits;

   if( info->version.load != MULLE_OBJC_RUNTIME_LOAD_VERSION)
   {
      mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
      //
      // if you reach this, and you go huh ? it may mean, that an older
      // shared library version of the Foundation was loaded.
      //
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: the loaded binary was produced for "
         "load version %d, but this universe %u.%u.%u (%s) supports "
         "load version %d only",
            universe, info->version.load,
            mulle_objc_version_get_major( _mulle_objc_universe_get_version( universe)),
            mulle_objc_version_get_minor( _mulle_objc_universe_get_version( universe)),
            mulle_objc_version_get_patch( _mulle_objc_universe_get_version( universe)),
            _mulle_objc_universe_get_path( universe) ? _mulle_objc_universe_get_path( universe) : "???",
            MULLE_OBJC_RUNTIME_LOAD_VERSION);
   }

   //
   // this is used by mulle-objc-list which just want the info "stream" by
   // and doesn't care about tps/fcs/tao consistency, as it never makes actual
   // objective-C calls. but LOAD_VERSION must match. This is like the only
   // imaginable valid reason to use this flag... Otherwise you're just digging
   // yourself into a deeper hole
   //
   if( universe->config.skip_consistency_checks)
      return;

   if( info->version.foundation)
   {
      if( ! universe->foundation.universefriend.versionassert)
      {
         mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
         mulle_objc_universe_fail_inconsistency( universe,
            "mulle_objc_universe %p: foundation version set (0x%x), but "
            "universe foundation provides no versionassert",
               universe, info->version.foundation);
      }
      (*universe->foundation.universefriend.versionassert)( universe, &universe->foundation, &info->version);
   }

   if( info->version.user)
   {
      if( ! universe->userinfo.versionassert)
      {
         mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
         mulle_objc_universe_fail_inconsistency( universe,
            "mulle_objc_universe %p: loadinfo user version set (0x%x), but "
            "universe userinfo provides no versionassert",
               universe, info->version.user);
      }
      (*universe->userinfo.versionassert)( universe, &universe->userinfo, &info->version);
   }


   //
   // check for tagged pointers. What can happen ?
   // Remember that static strings can be tps!
   //
   // universe | Code   | Description
   // ---------|--------|--------------
   // No-TPS   | No-TPS | Works
   // No-TPS   | TPS    | Crashes
   // TPS      | No-TPS | Works, but slower. Does not mix with "TPS Code"
   // TPS      | TPS    | Works
   //
   // Allow loading of "NO TPS"-code into a "TPS" aware universe as long
   // as no "TPS"-code is loaded also.
   //
   load_tps = ! (info->version.bits & _mulle_objc_loadinfo_notaggedptrs);

   bits     = _mulle_objc_universe_get_loadbits_inline( universe);

   mismatch = (load_tps && (bits & MULLE_OBJC_UNIVERSE_HAVE_NO_TPS_LOADS)) ||
              (! load_tps && (bits & MULLE_OBJC_UNIVERSE_HAVE_TPS_LOADS));
   mismatch |= universe->config.no_tagged_pointer && load_tps;
   if( mismatch)
   {
      mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: the universe is %sconfigured for "
         "tagged pointers, but these objects are compiled differently",
             universe,
             universe->config.no_tagged_pointer ? "not " : "");
   }

   _mulle_objc_universe_set_loadbit( universe,
                                     load_tps
                                        ? MULLE_OBJC_UNIVERSE_HAVE_TPS_LOADS
                                        : MULLE_OBJC_UNIVERSE_HAVE_NO_TPS_LOADS);


   //
   // Check for thread affine objects (TAO). What can happen ?
   // A mismatch always crashes, because the object header is different.
   //
   load_tao = ! ! (info->version.bits & _mulle_objc_loadinfo_threadaffineobjects);
#ifdef __MULLE_OBJC_TAO__
   if( ! load_tao)
   {
      mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: the runtime is compiled for "
         "thread affine objects -fobjc-tao, but these objects are compiled -fno-objc-tao",
             universe);
   }
#else
   if( load_tao)
   {
      mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: the runtime is compiled for "
         "no thread affine objects -fno-objc-tao, but these objects are compiled -fobjc-tao",
             universe);
   }
#endif

   // we set this anyway for the debugger (easier to find)
   _mulle_objc_universe_set_loadbit( universe,
                                     load_tao
                                        ? MULLE_OBJC_UNIVERSE_HAVE_TAO_LOADS
                                        : MULLE_OBJC_UNIVERSE_HAVE_NO_TAO_LOADS);


   //
   // check that if a universename is given, that tps is off
   //
   if( info->loaduniverse && ! universe->config.no_tagged_pointer)
   {
      mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
      mulle_objc_universe_fail_inconsistency( universe,
            "mulle_objc_universe %p: the universe is not the default universe "
            "but TPS is enabled", universe);
   }

   // make sure everything is compiled with say -O0 (or -O1 at least)
   // if u want to...
   optlevel = ((info->version.bits >> 16) & 0x7);
   if( optlevel < universe->config.min_optlevel || optlevel > universe->config.max_optlevel)
   {
      mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
      mulle_objc_universe_fail_inconsistency( universe,
          "mulle_objc_universe %p: loadinfo was compiled with optimization "
          "level %d but universe requires between (%d and %d)",
               universe,
               optlevel,
               universe->config.min_optlevel,
               universe->config.max_optlevel);
   }


   //
   // Check for fast methods classes. What can happen ?
   // Compatibility of fast method funtions must be ascertained with version
   // numbering.
   //
   // universe | Code   | Description
   // ---------|--------|--------------
   // No-FCS   | No-FCS | Works
   // No-FCS   | FCS    | Crashes
   // FCS      | No-FCS | Crashes (wrong class size messes up classpair code)
   // FCS      | FCS    | Works
   //
#ifdef __MULLE_OBJC_FCS__
   if( info->version.bits & _mulle_objc_loadinfo_nofastcalls)
   {
      mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: the universe is compiled for fast methods, "
         "but classes and categories are not.", universe);
   }
#endif

#ifdef __MULLE_OBJC_NO_FCS__
   if( ! (info->version.bits & _mulle_objc_loadinfo_nofastcalls))
   {
      mulle_objc_loadinfo_dump( info, "loadinfo:   ", universe);
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: the universe can't handle fast methods, "
         "but classes and categories use them.", universe);
   }
#endif
}


char   *mulle_objc_loadinfo_get_origin( struct _mulle_objc_loadinfo *info)
{
   char  *s;

   if( ! info)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   if( info->origin && *info->origin)
      return( info->origin);

   s = NULL;
   if( info->loadclasslist && info->loadclasslist->n_loadclasses)
      s = info->loadclasslist->loadclasses[ 0]->origin;
   if( ! s)
      if( info->loadcategorylist && info->loadcategorylist->n_loadcategories)
         s = info->loadcategorylist->loadcategories[ 0]->origin;
   return( s);
}


//
// this is the function called per .o file
// it's called indirectly via mulle_atinit on participating platforms
// (those that use ELF)
//
static void   _mulle_objc_loadinfo_enqueue_nofail( void *_info)
{
   struct _mulle_objc_loadinfo             *info = _info;
   struct _mulle_objc_universe             *universe;
   int                                     need_sort;
   static struct _mulle_objc_loaduniverse  empty;
   struct _mulle_objc_loaduniverse         *loaduniverse;
   int                                     trace;

   trace = mulle_objc_environment_get_yes_no( "MULLE_OBJC_TRACE_LOADINFO");
   if( trace)
      fprintf( stderr, "%p: mulle-objc is enqueing loadinfo %p\n",
                           (void *) mulle_thread_self(), info);

   // allow NULL input so mulle_objc_list can call this once, so the
   // linker can't optimize it away
   if( ! info)
      return;

   loaduniverse = info->loaduniverse;
   if( ! info->loaduniverse)
       loaduniverse = &empty;
   else
   {
      if( info->loaduniverse->universeid == MULLE_OBJC_DEFAULTUNIVERSEID)
      {
         fprintf( stderr, "loaduniverse must not use default id 0 (compiler bug!)\n");
         abort();;
      }
   }

   if( trace)
   {
      // only trace non standard universes
      if( loaduniverse->universeid != 0)
      {
         fprintf( stderr, "%p: mulle-objc is acquiring universe %lu \"%s\"\n",
                              (void *) mulle_thread_self(),
                              (unsigned long) loaduniverse->universeid,
                              loaduniverse->universename ? loaduniverse->universename : "");
      }
   }

   universe = mulle_objc_global_register_universe( loaduniverse->universeid,
                                                   loaduniverse->universename);
   assert( universe);

   if( _mulle_objc_universe_is_uninitialized( universe))
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: universe was not properly initialized "
         "by `__register_mulle_objc_universe`.", universe);

   if( ! universe->memory.allocator.calloc)
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: Has no allocator installed.", universe);

   if( ! _mulle_objc_thread_isregistered_universe_gc( universe))
   {
      mulle_objc_universe_fail_inconsistency( universe,
         "mulle_objc_universe %p: The function "
         "\"mulle_objc_loadinfo_enqueue_nofail\" is called from a "
         "non-registered thread.", universe, info->version.foundation);
   }

   _mulle_objc_universe_assert_runtimeversion( universe, &info->version);

   if( universe->callbacks.should_load_loadinfo)
   {
      if( ! (*universe->callbacks.should_load_loadinfo)( universe, info))
      {
         if( universe->debug.trace.loadinfo)
         {
            mulle_objc_universe_trace( universe,
                                       "loadinfo %p ignored on request",
                                       info);
            mulle_objc_loadinfo_dump( info, "   ", universe);
         }
         return;
      }
   }

   // this will also set some bits in the runtime
   mulle_objc_universe_assert_loadinfo( universe, info);

   if( universe->debug.trace.loadinfo)
   {
      mulle_objc_universe_trace( universe,
                                 "loads loadinfo %p in thread %p",
                                 info,
                                 (void *) mulle_thread_self());
      mulle_objc_loadinfo_dump( info, "   ", universe);
   }

   if( universe->debug.trace.loadinfo)
   {
      mulle_objc_universe_trace( universe, "loading strings...");
   }

   // load strings in first, can be done unlocked
   mulle_objc_loadstringlist_enqueue_nofail( info->loadstringlist, universe);

   if( universe->debug.trace.loadinfo || universe->debug.trace.hashstrings)
   {
      char   *s;
      char   *sep;

      s = mulle_objc_loadinfo_get_origin( info);
      sep = ": ";
      if( ! s || !strlen( s))
         s = sep = "";
      mulle_objc_universe_trace( universe, "%s%sloading hashed strings...", s, sep);
   }

   // pass universe thru...
   need_sort = info->version.bits & _mulle_objc_loadinfo_unsorted;

   mulle_objc_loadhashedstringlist_enqueue_nofail( info->loadhashedstringlist,
                                                   universe,
                                                   need_sort);

   if( universe->debug.trace.loadinfo)
   {
      mulle_objc_universe_trace( universe, "loading super strings...");
   }

   // super strings are unproblematic also
   mulle_objc_loadsuperlist_enqueue_nofail( info->loadsuperlist, universe);

   if( universe->debug.trace.loadinfo)
   {
      mulle_objc_universe_trace( universe, "locking waitqueues...");
   }

   _mulle_objc_universe_lock_waitqueues( universe);
   {
      if( universe->debug.trace.loadinfo)
      {
         mulle_objc_universe_trace( universe,  "lock successful");
      }

      //
      // serialize the classes and categories for +load!
      // see dox/load/LOAD.md for more info
      //
      //
      // the load-queue is kinda superflous, since we are single-threaded
      // but the sequencing is nicer, since all classes and categories in
      // .o are now loaded (should document this (or nix it)))
      //
      // TODO: just callin +load in add_infraclass would do it find me thinks
      //
      struct _mulle_objc_callqueue   loads;

      if( mulle_objc_callqueue_init( &loads, &universe->memory.allocator))
         mulle_objc_universe_fail_errno( universe);

      //
      // the wait-queues are maintained in the universe
      // Because these are locked now anyway, the pointerarray is overkill
      // and not that useful, because you can't remove entries
      //

      if( universe->debug.trace.loadinfo)
         mulle_objc_universe_trace( universe,  "loading classes...");

      mulle_objc_loadclasslist_enqueue_nofail( info->loadclasslist,
                                               need_sort,
                                               &loads,
                                               universe);
      if( universe->debug.trace.loadinfo)
         mulle_objc_universe_trace( universe,  "loading categories...");

      mulle_objc_loadcategorylist_enqueue_nofail( info->loadcategorylist,
                                                  need_sort,
                                                  &loads,
                                                  universe);

      if( universe->debug.trace.loadinfo)
         mulle_objc_universe_trace( universe,  "performing +load calls...");

      mulle_objc_callqueue_walk( &loads, (mulle_objc_callqueue_t *) call_load, universe);
      mulle_objc_callqueue_done( &loads);
   }

   if( universe->debug.trace.loadinfo)
      mulle_objc_universe_trace( universe, "unlocking waitqueues...");

   _mulle_objc_universe_unlock_waitqueues( universe);


   if( universe->debug.trace.loadinfo)
      mulle_objc_universe_trace( universe, "finished with loadinfo %p", info);
}


//
// Use of mulle_atinit:
//
// What we would like to have is a scenario, where libraries like
// mulle-testallocator can be transparently placed "underneath" the runtime.
// For that the initializer of the library has to run before the runtime
// gets initialized. The runtime gets initialized during class-loading though.
// That's usually not a problem, but because ELF can not really give any
// guarantees, this is an endless source of problems.
//
// So we delay all incoming class loads from shared libraries and run them
// when the executable initializers, which run last. This scheme doesn't work
// if you link the mulle_testallocator statically and in the wrong order.
//
void   mulle_objc_loadinfo_enqueue_nofail( struct _mulle_objc_loadinfo *info)
{
   int   trace;
   char  *comment;

   trace = mulle_objc_environment_get_yes_no( "MULLE_OBJC_TRACE_LOADINFO");
   if( trace)
      fprintf( stderr, "%p: mulle-objc is pushing loadinfo onto atinit %p\n",
                           (void *) mulle_thread_self(), info);
   //
   // comment must be static, can't and wont be freed
   // (so must remain until mulle_atinit is through)
   //
   comment = info ? mulle_objc_loadinfo_get_origin( info) : NULL;
   comment = comment ? comment : "_mulle_objc_loadinfo";
   mulle_atinit( _mulle_objc_loadinfo_enqueue_nofail, info, 0, comment);

   if( trace)
      fprintf( stderr, "%p: mulle-objc did push loadinfo onto atinit %p\n",
                              (void *) mulle_thread_self(), info);
}


