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


#pragma mark -
#pragma mark categories

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


#pragma mark -
#pragma mark categories

// TODO: make this a runtime method
//
// WARNING: the "dependency" check only checks for the superclass. This is not
// enough, it should also wait for the protocol class to appear. This is a
// harder problem though. And not yet implemented. The solution would be to
// query a +(mulle_objc_classid_t *)additionalRequirements method, that returns a list
// of classids, that should delay the addition of the class further.
//
static int  mulle_objc_loadcategory_add_delayed( struct _mulle_objc_loadcategory *info,
                                                 struct _mulle_objc_runtime *runtime)
{
   struct mulle_concurrent_pointerarray   *list;

   if( ! info)
   {
      errno = EINVAL;
      return( -1);
   }

   list = _mulle_concurrent_hashmap_lookup( &runtime->categoriestoload, info->classid);
   if( ! list)
   {
      list = _mulle_allocator_calloc( &runtime->memory.allocator, 1, sizeof( *list));
      _mulle_concurrent_pointerarray_init( list, 16, &runtime->memory.allocator);

      if( _mulle_concurrent_hashmap_insert( &runtime->categoriestoload, info->classid, list))
      {
         _mulle_concurrent_pointerarray_done( list);
         _mulle_allocator_free( &runtime->memory.allocator, list);  // assume it was never alive
         return( -1);
      }
   }

   if( runtime->debug.trace.delayed_category_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: category %s( %s) waits for it's class with id %08x to load\n", runtime, info->class_name, info->category_name, info->classid);

   _mulle_concurrent_pointerarray_add( list, info);
   return( 0);
}


void   mulle_objc_loadcategory_unfailing_enqueue( struct _mulle_objc_loadcategory *info,
                                                  struct _mulle_objc_callqueue *loads)
{
   struct _mulle_objc_class   *cls;
   struct _mulle_objc_class   *meta;
   struct _mulle_objc_runtime *runtime;

   // possibly get or create runtime..

   runtime = __get_or_create_objc_runtime();

   if( ! info)
   {
      errno = EINVAL;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   cls = _mulle_objc_runtime_lookup_class( runtime, info->classid);
   if( ! cls)
   {
      if( mulle_objc_loadcategory_add_delayed( info, runtime))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
      return;
   }

   if( strcmp( info->class_name, cls->name))
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in  mulle_objc_runtime %p: duplicate classes \"%s\" \"%s\" with id %08x\n",
      runtime, info->class_name, cls->name, info->classid);

   if( info->class_ivarhash != cls->ivarhash)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: class \"%s\" of category \"%s( %s)\" has changed. Recompile the category.\n", runtime, info->class_name, info->class_name, info->category_name ? info->category_name : "");

   meta = _mulle_objc_class_get_metaclass( cls);

   if( info->instance_methods && info->instance_methods->n_methods)
   {
      info->instance_methods->owner = info->category_name;
      if( mulle_objc_class_add_methodlist( cls, info->instance_methods))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   if( info->properties && info->properties->n_properties)
      if( mulle_objc_class_add_propertylist( cls, info->properties))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);

   if( info->class_methods && info->class_methods->n_methods)
   {
      info->class_methods->owner = info->category_name;
      if( mulle_objc_class_add_methodlist( meta, info->class_methods))
         _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   mulle_objc_class_unfailing_add_protocols( cls, info->protocol_uniqueids);
   mulle_objc_class_unfailing_add_protocols( meta, info->protocol_uniqueids);

   if( runtime->debug.trace.category_adds)
      fprintf( stderr, "mulle_objc_runtime %p trace: add methods and protocols of category \"%s( %s)\"\n", runtime, cls->name, info->category_name);

   mulle_objc_methodlist_unfailing_execute_load( info->class_methods, meta, loads);
}


#pragma mark -
#pragma mark classes

// TODO: make this a runtime method
static int   mulle_objc_loadclass_add_delayed( struct _mulle_objc_loadclass *info)
{
   struct mulle_concurrent_pointerarray   *list;
   struct _mulle_objc_runtime             *runtime;
   int                                    rval;

   // possibly get or create runtime..

   runtime = __get_or_create_objc_runtime();

   if( ! info)
   {
      errno = EINVAL;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   for(;;)
   {
      list = _mulle_concurrent_hashmap_lookup( &runtime->classestoload, info->superclassid);
      if( ! list)
      {
         list = _mulle_allocator_calloc( &runtime->memory.allocator, 1, sizeof( *list));
         _mulle_concurrent_pointerarray_init( list, 16, &runtime->memory.allocator);

         rval =  _mulle_concurrent_hashmap_insert( &runtime->classestoload, info->superclassid, list);
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
      fprintf( stderr, "mulle_objc_runtime %p trace: class %s with id %08x waits for superclass %s with id %08x to load\n", runtime, info->class_name, info->classid, info->superclass_name, info->superclassid);

   _mulle_concurrent_pointerarray_add( list, info);
   return( 0);
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


void   mulle_objc_loadclass_unfailing_enqueue( struct _mulle_objc_loadclass *info)
{
   struct _mulle_objc_callqueue   loads;
   struct _mulle_objc_classpair   *pair;
   struct _mulle_objc_class       *cls;
   struct _mulle_objc_class       *meta;
   struct _mulle_objc_class       *superclass;
   struct _mulle_objc_runtime     *runtime;

   // possibly get or create runtime..

   runtime = __get_or_create_objc_runtime();

   if( ! info)
   {
      errno = EINVAL;
      _mulle_objc_runtime_raise_fail_errno_exception( runtime);
   }

   // root ?
   superclass = 0;
   if( info->superclassid)
   {
      superclass = _mulle_objc_runtime_lookup_class( runtime, info->superclassid);

      if( ! superclass)
      {
         if( mulle_objc_loadclass_add_delayed( info))
            _mulle_objc_runtime_raise_fail_errno_exception( runtime);
         return;
      }

      if( strcmp( info->superclass_name, superclass->name))
         _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: duplicate classes \"%s\" \"%s\" with id %08x\n", runtime, info->superclass_name, superclass->name, info->superclassid);
   }

   if( superclass && superclass->ivarhash != info->superclass_ivarhash)
   {
      if( ! runtime->config.ignore_ivarhash_mismatch)
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: superclass \"%s\" of \"%s\" has changed. Recompile \"%s\".\n", runtime, info->superclass_name, info->class_name, info->class_name);
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
   pair = mulle_objc_unfailing_new_classpair( info->classid, info->class_name, info->instance_size, superclass);

   cls  = _mulle_objc_classpair_get_infraclass( pair);
   meta = _mulle_objc_classpair_get_metaclass( pair);
   assert( meta == _mulle_objc_class_get_metaclass( cls));

   _mulle_objc_class_set_ivarhash( cls, info->class_ivarhash);
   mulle_objc_class_unfailing_add_ivarlist( cls, info->instance_variables);
   mulle_objc_class_unfailing_add_methodlist( cls, info->instance_methods);
   mulle_objc_class_unfailing_add_propertylist( cls, info->properties);
   mulle_objc_class_unfailing_add_protocols( cls, info->protocol_uniqueids);

   if( info->fastclassindex >= 0)
      _mulle_objc_runtime_set_fastclass( runtime, meta, info->fastclassindex);

   mulle_objc_class_unfailing_add_methodlist( meta, info->class_methods);
   mulle_objc_class_unfailing_add_protocols( meta, info->protocol_uniqueids);

   mulle_objc_methodlist_unfailing_execute_load( info->class_methods, meta, &loads);

   if( mulle_objc_runtime_add_class( runtime, cls))
   {
      _mulle_objc_runtime_raise_fail_exception( runtime, "error in mulle_objc_runtime %p: duplicate class \"%s\" with id %08x.\n", runtime, cls->name, cls->classid);
   }
   //
   // check if categories or classes are waiting for us ?
   //
   map_f( &runtime->categoriestoload, info->classid, (void (*)()) mulle_objc_loadcategory_unfailing_enqueue, &loads);
   map_f( &runtime->classestoload, info->classid, (void (*)())  mulle_objc_loadclass_unfailing_enqueue, &loads);

   //
   // this fails, if you have a +load in a category that references a method
   // of a category that is not yet loaded. Even that of a subclass in the same
   // shared library.
   // The proper solution and that's why  I am not dicking around with this
   // more, is to put the +load code into a shared library that depends on the
   // previous.
   //
   mulle_objc_callqueue_execute( &loads);
   mulle_objc_callqueue_done( &loads);
}


# pragma mark -
# pragma mark info

static int  _mulle_objc_protocolid_compare( mulle_objc_protocolid_t *a, mulle_objc_protocolid_t *b)
{
   intptr_t   diff;

   diff = (intptr_t) *a - (intptr_t) *b;
   if( diff < 0)
      return( -1);
   return( ! ! diff);
}


static size_t   mulle_objc_count_protocolids( mulle_objc_protocolid_t *ids)
{
   size_t   n;

   n = 0;
   if( ids)
      while( *ids++)
         ++n;
   return( n);
}


static void   mulle_objc_loadclass_sort_lists( struct _mulle_objc_loadclass *lcls)
{
   qsort( lcls->protocol_uniqueids, mulle_objc_count_protocolids( lcls->protocol_uniqueids), sizeof( mulle_objc_protocolid_t), (int (*)()) _mulle_objc_protocolid_compare);

   mulle_objc_ivarlist_sort( lcls->instance_variables);
   mulle_objc_methodlist_sort( lcls->instance_methods);
   mulle_objc_methodlist_sort( lcls->class_methods);
   mulle_objc_propertylist_sort( lcls->properties);
}


static void   mulle_objc_loadclasslist_unfailing_enqueue( struct _mulle_objc_loadclasslist *list, int need_sort)
{
   struct _mulle_objc_loadclass   **p_class;
   struct _mulle_objc_loadclass   **sentinel;

   p_class = list->loadclasses;
   sentinel = &p_class[ list->n_loadclasses];
   while( p_class < sentinel)
   {
      if( need_sort)
         mulle_objc_loadclass_sort_lists( *p_class);

      mulle_objc_loadclass_unfailing_enqueue( *p_class);
      p_class++;
   }
}


static void   mulle_objc_loadcategory_sort_lists( struct _mulle_objc_loadcategory *lcat)
{
   qsort( lcat->protocol_uniqueids, mulle_objc_count_protocolids( lcat->protocol_uniqueids), sizeof( mulle_objc_protocolid_t), (int (*)()) _mulle_objc_protocolid_compare);

   mulle_objc_methodlist_sort( lcat->instance_methods);
   mulle_objc_methodlist_sort( lcat->class_methods);
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
         mulle_objc_loadcategory_sort_lists( *p_category);

      mulle_objc_loadcategory_unfailing_enqueue( *p_category, NULL);
      p_category++;
   }
}


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


static void   mulle_objc_loadhashedstringlist_unfailing_enqueue( struct _mulle_objc_loadhashedstringlist *map, int need_sort)
{
   struct _mulle_objc_runtime   *runtime;

   if( need_sort)
      mulle_objc_loadhashedstringlist_sort( map);

   runtime = __get_or_create_objc_runtime();
   _mulle_objc_runtime_add_loadhashedstringlist( runtime, map);
}


static void   dump_loadclasslist( struct _mulle_objc_loadclasslist *list, char *prefix)
{
   struct _mulle_objc_loadclass   **p;
   struct _mulle_objc_loadclass   **sentinel;

   if( ! list)
      return;

   p        = list->loadclasses;
   sentinel = &p[ list->n_loadclasses];
   while( p < sentinel)
   {
      fprintf( stderr, "%s@implementation %s\n", prefix, (*p)->class_name);
      ++p;
   }
}


static void   dump_loadcategorylist( struct _mulle_objc_loadcategorylist *list, char *prefix)
{
   struct _mulle_objc_loadcategory   **p;
   struct _mulle_objc_loadcategory   **sentinel;

   if( ! list)
      return;

   p        = list->loadcategories;
   sentinel = &p[ list->n_loadcategories];
   while( p < sentinel)
   {
      fprintf( stderr, "%s@implementation %s( %s)\n", prefix, (*p)->class_name, (*p)->category_name);
      ++p;
   }
}


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

   fprintf( stderr, "%s-O%d", delim, (bits >> 4) & 0x7);
}


static void   dump_loadinfo( struct _mulle_objc_loadinfo *info, char *prefix)
{
   fprintf( stderr, "%sversion: v%d.%d.%d (", prefix,
         info->version.user,
         info->version.foundation,
         info->version.runtime);
   dump_bits( info->version.bits);
   fprintf( stderr, ")\n");

   dump_loadclasslist( info->loadclasslist, prefix);
   dump_loadcategorylist( info->loadcategorylist, prefix);
}


void   mulle_objc_loadinfo_unfailing_enqueue( struct _mulle_objc_loadinfo *info)
{
   struct _mulle_objc_runtime   *runtime;
   int         need_sort;
   int         optlevel;
   int         load_tps;
   int         mismatch;
   uintptr_t   bits;
   
   if( getenv( "MULLE_OBJC_TRACE_LOADINFO"))
   {
      fprintf( stderr, "mulle-objc: enqueues loadinfo %p\n", info);
      assert( info);
      dump_loadinfo( info, "   ");
   }
   assert( info);

   runtime = __get_or_create_objc_runtime();
   if( ! mulle_objc_class_is_current_thread_registered( NULL))
   {
      dump_loadinfo( info, "loadinfo:   ");
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: The function \"mulle_objc_loadinfo_unfailing_enqueue\" is called from a non-registered thread.", runtime, info->version.foundation);
   }

   _mulle_objc_runtime_assert_version( runtime, &info->version);

   /* 1848 is test standard, pass thru */
   if( info->version.foundation != 1848)
   {
      if( ! runtime->foundation.runtimefriend.versionassert)
      {
         dump_loadinfo( info, "loadinfo:   ");
         _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: foundation version set to %d but foundation provides no versionassert", runtime, info->version.foundation);
      }
      (*runtime->foundation.runtimefriend.versionassert)( runtime, &runtime->foundation, &info->version);
   }

   if( info->version.user)
   {
      if( ! runtime->foundation.runtimefriend.versionassert)
      {
         dump_loadinfo( info, "loadinfo:   ");
         _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: loadinfo user version set to %d but userinfo provides no versionassert", runtime, info->version.foundation);
      }
      (*runtime->userinfo.versionassert)( runtime, &runtime->userinfo, &info->version);
   }

   // pass runtime thru...
   need_sort = info->version.bits & _mulle_objc_loadinfo_unsorted;

   //
   // check for tagged pointers. What can happen ?
   // Remember that static strings can be tps!
   //
   //                | Class TPS | Class NO TPS
   // --------------------------------------------------------------
   // Runtime TPS    |       OK  | FAIL
   // Runtime NO TPS |      FAIL | OK
   //
   // For convenience in testing we allow loading of "NO TPS"-code
   // into a "TPS" aware runtime as long as no "TPS"-code is loaded also.
   //
   load_tps = ! (info->version.bits & _mulle_objc_loadinfo_notaggedptrs);

   bits     = _mulle_objc_runtime_get_loadbits( runtime);
   mismatch = (load_tps && (bits & MULLE_OBJC_RUNTIME_HAVE_NO_TPS_LOADS)) ||
              (! load_tps && (bits & MULLE_OBJC_RUNTIME_HAVE_TPS_LOADS));
   mismatch |= runtime->config.no_tagged_pointers && load_tps;
   if( mismatch)
   {
      dump_loadinfo( info, "loadinfo:   ");
      _mulle_objc_runtime_raise_inconsistency_exception( runtime, "mulle_objc_runtime %p: the runtime is %sconfigured for tagged pointers, but classes are compiled differently",
                                                        runtime,
                                                        runtime->config.no_tagged_pointers ? "not " : "");
   }
   _mulle_objc_runtime_set_loadbit( runtime, load_tps ? MULLE_OBJC_RUNTIME_HAVE_TPS_LOADS
                                   : MULLE_OBJC_RUNTIME_HAVE_NO_TPS_LOADS);

   // make sure everything is compiled with say -O0 (or -O1 at least)
   // if u want to...
   optlevel = ((info->version.bits >> 16) & 0x7);
   if( optlevel < runtime->config.min_optlevel || optlevel > runtime->config.max_optlevel)
   {
      dump_loadinfo( info, "loadinfo:   ");
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

