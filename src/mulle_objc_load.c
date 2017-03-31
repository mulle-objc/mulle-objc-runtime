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

#include "mulle_objc_class.h"
#include "mulle_objc_callqueue.h"
#include "mulle_objc_runtime.h"

#include "mulle_objc_class_runtime.h"

#include <mulle_concurrent/mulle_concurrent.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


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


static void    map_f( struct mulle_concurrent_hashmap *table, mulle_objc_uniqueid_t uniqueid, void (*f)( void *, struct _mulle_objc_callqueue  *), struct _mulle_objc_callqueue  *loads)
{
   struct mulle_concurrent_pointerarray   *list;
   unsigned int                     i;
   struct _mulle_objc_runtime       *runtime;

   // possibly get or create runtime..

   runtime = __get_or_create_objc_runtime();

   if( ! table)
      return;

   list = _mulle_concurrent_hashmap_lookup( table, uniqueid);
   if( ! list)
      return;

   // note that this array could grow while we iterate
   i    = 0;
   while( i < mulle_concurrent_pointerarray_get_count( list))
   {
      (*f)( _mulle_concurrent_pointerarray_get( list, i), loads);
      ++i;
   }

   _mulle_concurrent_hashmap_remove( table, uniqueid, list);

   _mulle_concurrent_pointerarray_done( list);
   _mulle_allocator_abafree( &runtime->memory.allocator, list);
}


#pragma mark - classes

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
      list = _mulle_concurrent_hashmap_lookup( &runtime->classestoload, missingclassid);
      if( ! list)
      {
         list = _mulle_allocator_calloc( &runtime->memory.allocator, 1, sizeof( *list));
         _mulle_concurrent_pointerarray_init( list, 16, &runtime->memory.allocator);

         rval =  _mulle_concurrent_hashmap_insert( &runtime->classestoload, missingclassid, list);
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

   if( runtime->debug.trace.delayed_class_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: class %s with id %08x waits for class with id %08x to load\n", runtime, info->classname, info->classid, missingclassid);

   _mulle_concurrent_pointerarray_add( list, info);
   return( 0);
}


mulle_objc_classid_t   mulle_objc_loadclass_missingclassid( struct _mulle_objc_loadclass *info,
                                                            struct _mulle_objc_runtime *runtime,
                                                            struct _mulle_objc_class  **p_superclass)
{
   struct _mulle_objc_class   *protocolclass;
   struct _mulle_objc_class   *superclass;
   mulle_objc_classid_t       *classid_p;

   assert( info);
   assert( runtime);
   assert( p_superclass);

   superclass = NULL;
   if( info->superclassid)
   {
      superclass = _mulle_objc_runtime_lookup_class( runtime, info->superclassid);
      if( ! superclass)
         return( info->superclassid);

      if( strcmp( info->superclassname, superclass->name))
         _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: duplicate classes \"%s\" \"%s\" with id %08x\n", runtime, info->superclassname, superclass->name, info->superclassid);
   }

   if( superclass && superclass->ivarhash != info->superclassivarhash)
   {
      if( ! runtime->config.ignore_ivarhash_mismatch)
         _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: superclass \"%s\" of \"%s\" has changed. Recompile \"%s\".\n", runtime, info->superclassname, info->classname, info->classname);
   }

   // protocol classes present ?
   if( info->protocolclassids)
   {
      for( classid_p = info->protocolclassids; *classid_p; ++classid_p)
      {
         // avoid duplication and waiting for seld
         if( *classid_p == info->superclassid || *classid_p == info->classid)
            continue;

         protocolclass = _mulle_objc_runtime_lookup_class( runtime, *classid_p);
         if( ! protocolclass)
            return( *classid_p);
      }
   }

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


void   mulle_objc_loadclass_unfailing_enqueue( struct _mulle_objc_loadclass *info)
{
   struct _mulle_objc_callqueue   loads;
   struct _mulle_objc_classpair   *pair;
   struct _mulle_objc_class       *cls;
   struct _mulle_objc_class       *meta;
   struct _mulle_objc_class       *superclass;
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
   if( mulle_objc_callqueue_init( &loads, &runtime->memory.allocator))
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);

   // ready to install
   pair = mulle_objc_unfailing_new_classpair( info->classid, info->classname, info->instancesize, superclass);

   cls  = _mulle_objc_classpair_get_infraclass( pair);
   meta = _mulle_objc_classpair_get_metaclass( pair);
   assert( meta == _mulle_objc_class_get_metaclass( cls));

   _mulle_objc_class_set_ivarhash( cls, info->classivarhash);
   mulle_objc_class_unfailing_add_ivarlist( cls, info->instancevariables);
   mulle_objc_class_unfailing_add_methodlist( cls, info->instancemethods);
   mulle_objc_class_unfailing_add_propertylist( cls, info->properties);
   mulle_objc_class_unfailing_add_protocols( cls, info->protocolids);

   if( info->fastclassindex >= 0)
      _mulle_objc_runtime_set_fastclass( runtime, meta, info->fastclassindex);

   mulle_objc_class_unfailing_add_methodlist( meta, info->classmethods);
   mulle_objc_class_unfailing_add_protocols( meta, info->protocolids);

   mulle_objc_methodlist_unfailing_execute_load( info->classmethods, meta, &loads);

   if( mulle_objc_runtime_add_class( runtime, cls))
   {
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: duplicate class \"%s\" with id %08x.\n", runtime, cls->name, cls->classid);
   }
   //
   // check if categories or classes are waiting for us ?
   //
   map_f( &runtime->categoriestoload, info->classid, (void (*)()) mulle_objc_loadcategory_unfailing_enqueue, &loads);
   map_f( &runtime->classestoload, info->classid, (void (*)())  mulle_objc_loadclass_unfailing_enqueue, &loads);

   mulle_objc_callqueue_execute( &loads);
   mulle_objc_callqueue_done( &loads);
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

   fprintf( stderr, "\n");
}



#pragma mark - classlists

static void   mulle_objc_loadclasslist_unfailing_enqueue( struct _mulle_objc_loadclasslist *list, int need_sort)
{
   struct _mulle_objc_loadclass   **p_class;
   struct _mulle_objc_loadclass   **sentinel;

   p_class = list->loadclasses;
   sentinel = &p_class[ list->n_loadclasses];
   while( p_class < sentinel)
   {
      if( need_sort)
         mulle_objc_loadclass_listssort( *p_class);

      mulle_objc_loadclass_unfailing_enqueue( *p_class);
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

   list = _mulle_concurrent_hashmap_lookup( &runtime->categoriestoload, missingclassid);
   if( ! list)
   {
      list = _mulle_allocator_calloc( &runtime->memory.allocator, 1, sizeof( *list));
      _mulle_concurrent_pointerarray_init( list, 16, &runtime->memory.allocator);

      if( _mulle_concurrent_hashmap_insert( &runtime->categoriestoload, missingclassid, list))
      {
         _mulle_concurrent_pointerarray_done( list);
         _mulle_allocator_free( &runtime->memory.allocator, list);  // assume it was never alive
         return( -1);
      }
   }

   if( runtime->debug.trace.delayed_category_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: category %s( %s) waits for class with id %08x to load\n", runtime, info->classname, info->categoryname, missingclassid);

   _mulle_concurrent_pointerarray_add( list, info);
   return( 0);
}


int   mulle_objc_loadcategory_is_categorycomplete( struct _mulle_objc_loadcategory *info,
                                                   struct _mulle_objc_class *cls)
{
   struct _mulle_objc_method           *method;
   struct _mulle_objc_runtime          *runtime;
   mulle_objc_categoryid_t             *categoryids;
   mulle_objc_methodimplementation_t   imp;

   if( ! info->classmethods)
      return( 1);

   method = mulle_objc_method_bsearch( info->classmethods->methods,
                                       info->classmethods->n_methods,
                                       MULLE_OBJC_CATEGORY_DEPENDENCIES_METHODID);
   if( ! method)
      return( 1);

   runtime = _mulle_objc_class_get_runtime( cls);
   if( runtime->debug.trace.load_calls)
      fprintf( stderr, "mulle_objc_runtime %p trace: call +[%s(%s) categoryDependencies]\n", runtime, cls->name, info->categoryname);

   imp         = _mulle_objc_method_get_implementation( method);
   categoryids = (*imp)( cls, MULLE_OBJC_CATEGORY_DEPENDENCIES_METHODID, cls);
   if( ! categoryids)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: %s(%s) returned NULL for +categoryDependencies\n",
                                               runtime, info->classname, info->categoryname);

   while( *categoryids)
   {
      if( _mulle_objc_class_has_category( cls, *categoryids))
         return( 1);
      ++categoryids;
   }

   return( 0);
}


mulle_objc_classid_t   mulle_objc_loadcategory_missingclassid( struct _mulle_objc_loadcategory *info,
                                                               struct _mulle_objc_runtime *runtime,
                                                               struct _mulle_objc_class **p_class)
{
   struct _mulle_objc_class     *cls;
   struct _mulle_objc_class     *protocolclass;
   mulle_objc_classid_t         *classid_p;

   // check class
   cls = _mulle_objc_runtime_lookup_class( runtime, info->classid);
   if( ! cls)
      return( info->classid);

   // required categories present ?
   if( ! mulle_objc_loadcategory_is_categorycomplete( info, cls))
      return( info->classid);

   // protocol classes present ?
   if( info->protocolclassids)
   {
      for( classid_p = info->protocolclassids; *classid_p; ++classid_p)
      {
         // avoid duplication
         if( *classid_p == info->classid)
            continue;

         protocolclass = _mulle_objc_runtime_lookup_class( runtime, *classid_p);
         if( ! protocolclass)
            return( *classid_p);
      }
   }

   if( p_class)
      *p_class = cls;

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


void   mulle_objc_loadcategory_unfailing_enqueue( struct _mulle_objc_loadcategory *info,
                                                  struct _mulle_objc_callqueue *loads)
{
   struct _mulle_objc_class       *cls;
   struct _mulle_objc_class       *meta;
   struct _mulle_objc_runtime     *runtime;
   mulle_objc_classid_t           missingclassid;

   runtime = __get_or_create_objc_runtime();

   if( ! mulle_objc_loadcategory_is_sane( info))
   {
      errno = EINVAL;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   missingclassid = mulle_objc_loadcategory_missingclassid( info, runtime, &cls);
   if( missingclassid)
   {
      if( mulle_objc_loadcategory_delayedadd( info, missingclassid, runtime))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
      return;
   }

   if( strcmp( info->classname, cls->name))
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: duplicate classes \"%s\" \"%s\" with id %08x\n",
                                               runtime, info->classname, cls->name, info->classid);

   if( info->classivarhash != cls->ivarhash)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: class \"%s\" of category \"%s( %s)\" has changed. Recompile the category.\n", runtime, info->classname, info->classname, info->categoryname ? info->categoryname : "");

   if( info->categoryid && _mulle_objc_class_has_category( cls, info->categoryid))
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: a category of the same name \"%s(%s)\" has already been loaded.\n", runtime, info->classname, info->classname, info->categoryname ? info->categoryname : "");

   meta = _mulle_objc_class_get_metaclass( cls);

   // the loader sets the categoryid as owner
   if( info->instancemethods && info->instancemethods->n_methods)
   {
      info->instancemethods->owner = (void *) info->categoryid;
      if( mulle_objc_class_add_methodlist( cls, info->instancemethods))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }
   if( info->classmethods && info->classmethods->n_methods)
   {
      info->classmethods->owner = (void *) info->categoryid;
      if( mulle_objc_class_add_methodlist( meta, info->classmethods))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }
   if( info->properties && info->properties->n_properties)
      if( mulle_objc_class_add_propertylist( cls, info->properties))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);

   mulle_objc_class_unfailing_add_protocols( cls, info->protocolids);
   mulle_objc_class_unfailing_add_protocols( meta, info->protocolids);
   if( info->categoryid)
   {
      mulle_objc_class_unfailing_add_category( cls, info->categoryid);
      mulle_objc_class_unfailing_add_category( meta, info->categoryid);
   }

   if( runtime->debug.trace.category_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: add methods and protocols of category \"%s( %s)\"\n", runtime, cls->name, info->categoryname);

   //
   // retrigger "special" categories, that are waiting for their dependencies
   //
   map_f( &runtime->categoriestoload, info->classid, (void (*)()) mulle_objc_loadcategory_unfailing_enqueue, loads);

   mulle_objc_methodlist_unfailing_execute_load( info->classmethods, meta, loads);
}


static void   loadcategory_dump( struct _mulle_objc_loadcategory *p,
                                 char *prefix,
                                 struct _mulle_objc_loadhashedstringlist *strings)
{
   fprintf( stderr, "%s@implementation %s( %s)", prefix, p->classname, p->categoryname);

   if( p->protocolids)
      loadprotocols_dump( p->protocolids, p->protocolclassids, strings);

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


static void   mulle_objc_loadcategorylist_unfailing_enqueue( struct _mulle_objc_loadcategorylist *list, int need_sort)
{
   struct _mulle_objc_loadcategory   **p_category;
   struct _mulle_objc_loadcategory   **sentinel;

   p_category = list->loadcategories;
   sentinel   = &p_category[ list->n_loadcategories];
   while( p_category < sentinel)
   {
      if( need_sort)
         mulle_objc_loadcategory_listssort( *p_category);

      mulle_objc_loadcategory_unfailing_enqueue( *p_category, NULL);
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


void   mulle_objc_loadinfo_unfailing_enqueue( struct _mulle_objc_loadinfo *info)
{
   struct _mulle_objc_runtime   *runtime;
   int         need_sort;
   int         optlevel;
   int         load_tps;
   int         mismatch;
   uintptr_t   bits;

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

   if( getenv( "MULLE_OBJC_TRACE_LOADINFO"))
   {
      fprintf( stderr, "mulle-objc: enqueues loadinfo %p\n", info);
      assert( info);
      loadinfo_dump( info, "   ");
   }
   assert( info);

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

   // load strings in first
   if( info->loadstringlist)
      mulle_objc_loadstringlist_unfailing_enqueue( info->loadstringlist);
   if( info->loadhashedstringlist)
      mulle_objc_loadhashedstringlist_unfailing_enqueue( info->loadhashedstringlist, need_sort);

   if( info->loadclasslist)
      mulle_objc_loadclasslist_unfailing_enqueue( info->loadclasslist, need_sort);
   if( info->loadcategorylist)
      mulle_objc_loadcategorylist_unfailing_enqueue( info->loadcategorylist, need_sort);
}

