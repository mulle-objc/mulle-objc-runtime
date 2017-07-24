//
//  mulle_objc_csvdump.c
//  mulle-objc-universe
//
//  Created by Nat! on 16.05.17.
//  Copyright © 2017 Mulle kybernetiK. All rights reserved.
//  Copyright © 2017 Codeon GmbH. All rights reserved.
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
#include "mulle_objc_csvdump.h"

#include "mulle_objc_cache.h"
#include "mulle_objc_class.h"
#include "mulle_objc_class_universe.h"
#include "mulle_objc_class_struct.h"
#include "mulle_objc_classpair.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_load.h"
#include "mulle_objc_metaclass.h"
#include "mulle_objc_method.h"
#include "mulle_objc_universe.h"

#include <errno.h>


static void   mulle_objc_searchresult_csvdump( struct _mulle_objc_searchresult  *result,
                                               FILE *fp)
{
   mulle_objc_categoryid_t          categoryid;
   struct _mulle_objc_universe      *universe;
   char                             *classname;
   char                             *categoryname;
   
   universe = _mulle_objc_class_get_universe( result->class);
   
   categoryid = (mulle_objc_categoryid_t) (intptr_t) result->list->owner;
   classname  = _mulle_objc_class_get_name( result->class);
   if( categoryid)
   {
      categoryname = _mulle_objc_universe_string_for_categoryid( universe, categoryid);
      fprintf( fp, "%08x;%s;%08x;%s;",
              _mulle_objc_class_get_classid( result->class),
              classname,
              categoryid,
              categoryname);
   }
   else
      fprintf( fp, "%08x;%s;;;",
              _mulle_objc_class_get_classid( result->class),
              classname);
   
   fprintf( fp, "%08x;%c%s\n",
           _mulle_objc_method_get_methodid( result->method),
           _mulle_objc_class_is_metaclass( result->class) ? '+' : '-',
           _mulle_objc_method_get_name( result->method));
}


//
// remember a classpair needs to dump both meta and infra (this can handle both
// but only dumps either)
//
// currently we look for methods that are in the cache, and we never really
// empty the cache. BUT! This doesn't catch calls to methods, that are
// overridden later. If we need this we should use the
// _mulle_objc_method_searched_and_found method descriptor bit.
// Then we "just" traverse all methodlists of all classes and dump them out.
// The format would be different though, since  we don't have the cache
// information.
//

void   mulle_objc_class_csvdump_cachedmethodcoverage( struct _mulle_objc_class *cls,
                                                      FILE *fp)
{
   struct _mulle_objc_cache          *cache;
   struct _mulle_objc_cacheentry     *p;
   struct _mulle_objc_cacheentry     *sentinel;
   struct _mulle_objc_method         *method;
   struct _mulle_objc_searchresult   result;
   mulle_objc_methodid_t             methodid;
   
   if( ! cls || ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }
   
   cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   if( ! _mulle_atomic_pointer_read( &cache->n))
      return;

   p        = _mulle_atomic_pointer_nonatomic_read( &cls->cachepivot.pivot.entries);
   sentinel = &p[ cache->size];
   
   while( p < sentinel)
   {
      methodid = (mulle_objc_methodid_t) (intptr_t) _mulle_atomic_pointer_read( &p->key.pointer);
      ++p;
      
      if( methodid == MULLE_OBJC_NO_METHODID)
         continue;
      
      method = _mulle_objc_class_search_method( cls, methodid,
                                                NULL, MULLE_OBJC_ANY_OWNER,
                                                _mulle_objc_class_get_inheritance( cls),
                                                &result);
      if( ! method)
         mulle_objc_raise_inconsistency_exception( "universe information has disappeared");
      
      fprintf( fp, "%08x;%s;",
              _mulle_objc_class_get_classid( cls),
              _mulle_objc_class_get_name( cls));
      
      mulle_objc_searchresult_csvdump( &result, fp);
   }
}



void   mulle_objc_class_csvdump_methodcoverage( struct _mulle_objc_class *cls,
                                                FILE *fp)
{
   struct  _mulle_objc_methodlistenumerator                enumerator;
   struct _mulle_objc_method                               *method;
   struct _mulle_objc_methodlist                           *list;
   struct _mulle_objc_searchresult                         result;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                            n;

   if( ! cls || ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }
   
   n = mulle_concurrent_pointerarray_get_count( &cls->methodlists);
   assert( n);
   if( _mulle_objc_class_get_inheritance( cls) & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1;

   result.class = cls;

   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->methodlists, n);
   while( list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover))
   {
      result.list = list;
      
      enumerator = mulle_objc_methodlist_enumerate( list);
      while( method = _mulle_objc_methodlistenumerator_next( &enumerator))
      {
         if( ! (method->descriptor.bits & _mulle_objc_method_searched_and_found))
            continue;

         result.method = method;

         mulle_objc_searchresult_csvdump( &result, fp);
      }
      mulle_objc_methodlistenumerator_done( &enumerator);
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
}


void   mulle_objc_universe_csvdump_cachedmethodcoverage( struct _mulle_objc_universe *universe,
                                                  FILE *fp)
{
   intptr_t                                    classid;
   struct _mulle_objc_infraclass               *infra;
   struct _mulle_objc_metaclass                *meta;
   struct mulle_concurrent_hashmapenumerator   rover;

   if( ! universe || ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }

   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &classid, (void **) &infra))
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);

      mulle_objc_class_csvdump_cachedmethodcoverage( _mulle_objc_metaclass_as_class( meta), fp);
      mulle_objc_class_csvdump_cachedmethodcoverage( _mulle_objc_infraclass_as_class( infra), fp);
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
}



void   mulle_objc_universe_csvdump_methodcoverage( struct _mulle_objc_universe *universe,
                                                   FILE *fp)
{
   intptr_t                                    classid;
   struct _mulle_objc_infraclass               *infra;
   struct _mulle_objc_metaclass                *meta;
   struct mulle_concurrent_hashmapenumerator   rover;
   
   if( ! universe || ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }
   
   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &classid, (void **) &infra))
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);
      
      mulle_objc_class_csvdump_methodcoverage( _mulle_objc_metaclass_as_class( meta), fp);
      mulle_objc_class_csvdump_methodcoverage( _mulle_objc_infraclass_as_class( infra), fp);
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
}



void   mulle_objc_universe_csvdump_classcoverage( struct _mulle_objc_universe *universe,
                                                FILE *fp)
{
   intptr_t                                    classid;
   struct _mulle_objc_infraclass               *infra;
   struct mulle_concurrent_hashmapenumerator   rover;
   
   if( ! universe || ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }

   //
   // here we just go through all installed classes and check the state
   // bit (this also captures fastclasses)
   //
   
   universe = mulle_objc_get_universe();
   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &classid, (void **) &infra))
   {
      if( ! _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE))
         continue;
      
      fprintf( fp, "%08x;%s\n",
              _mulle_objc_infraclass_get_classid( infra),
              _mulle_objc_infraclass_get_name( infra));
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
}


//
// should dump collision statistics and "preferred" cache size here
//
static void  dump_cachesize( struct _mulle_objc_class *cls,
                             char prefix,
                             FILE *fp)
{
   struct _mulle_objc_cache   *cache;

   cache = _mulle_objc_class_get_methodcache( cls);
   
   if( ! _mulle_objc_cache_get_count( cache) &&
       ! _mulle_objc_cache_get_size( cache))
   {
       return;
   }
   
   fprintf( fp, "%08x;%c%s;%u;%u;%x\n",
           _mulle_objc_class_get_classid( cls),
           prefix,
           _mulle_objc_class_get_name( cls),
           _mulle_objc_cache_get_count( cache),
           _mulle_objc_cache_get_size( cache),
           _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE) |
           _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_FIXED_SIZE_CACHE));
}


static void   mulle_objc_universe_csvdump_cachesizes( struct _mulle_objc_universe *universe,
                                                     FILE *fp)
{
   intptr_t                                    classid;
   struct _mulle_objc_infraclass               *infra;
   struct _mulle_objc_metaclass                *meta;
   struct mulle_concurrent_hashmapenumerator   rover;
   
   if( ! universe || ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }

   //
   // here we just go through all installed classes and check the state
   // bit (this also captures fastclasses)
   //
   
   universe = mulle_objc_get_universe();
   rover = mulle_concurrent_hashmap_enumerate( &universe->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &classid, (void **) &infra))
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);
      dump_cachesize( _mulle_objc_metaclass_as_class( meta), '+', fp);
      dump_cachesize( _mulle_objc_infraclass_as_class( infra), '-', fp);
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
}

#pragma mark - dump starters

void   mulle_objc_csvdump_methodcoverage_to_file( char *filename)
{
   struct _mulle_objc_universe   *universe;
   FILE                         *fp;

   fp = fopen( filename, "a");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }
   
   universe = mulle_objc_get_universe();
   mulle_objc_universe_csvdump_methodcoverage( universe, fp);
   
   fclose( fp);

   fprintf( stderr, "Dumped method coverage to \"%s\"\n", filename);
}


void   mulle_objc_csvdump_classcoverage_to_file( char *filename)
{
   struct _mulle_objc_universe   *universe;
   FILE                         *fp;
   
   fp = fopen( filename, "a");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   universe = mulle_objc_get_universe();
   mulle_objc_universe_csvdump_classcoverage( universe, fp);

   fclose( fp);

   fprintf( stderr, "Dumped class coverage to \"%s\"\n", filename);
}


void   mulle_objc_csvdump_cachesizes_to_file( char *filename)
{
   struct _mulle_objc_universe   *universe;
   FILE                         *fp;
   
   fp = fopen( filename, "a");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   universe = mulle_objc_get_universe();
   mulle_objc_universe_csvdump_cachesizes( universe, fp);

   fclose( fp);

   fprintf( stderr, "Dumped cache sizes to \"%s\"\n", filename);
}


#pragma mark - loadinfo

static void   _fprint_csv_version( FILE *fp, uint32_t version)
{
   fprintf( fp, "%u.%u.%u;",
            mulle_objc_version_get_major( version),
            mulle_objc_version_get_minor( version),
            mulle_objc_version_get_patch( version));
}


void   mulle_objc_loadinfo_csvdump_terse( struct _mulle_objc_loadinfo *info, FILE *fp)
{
   char   *s;
   
   if( ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }
   
   s = mulle_objc_loadinfo_get_originator( info);
   fprintf( fp, "%s;", s ? s : "");
   _fprint_csv_version( fp, info->version.universe);
   _fprint_csv_version( fp, info->version.foundation);
   _fprint_csv_version( fp, info->version.user);
   fprintf( fp, "%d;",  (info->version.bits >> 8) & 0x7);
   fprintf( fp, "0x%x", info->version.bits);
   fprintf( fp, "\n");
}


#pragma mark - dump to /tmp

void   mulle_objc_csvdump_methodcoverage_to_tmp( void)
{
   mulle_objc_csvdump_methodcoverage_to_file( "/tmp/method-coverage.csv");
}


void   mulle_objc_csvdump_classcoverage_to_tmp( void)
{
   mulle_objc_csvdump_classcoverage_to_file( "/tmp/class-coverage.csv");
}


void   mulle_objc_csvdump_cachesizes_to_tmp( void)
{
   mulle_objc_csvdump_cachesizes_to_file( "/tmp/cache-sizes.csv");
}


#pragma mark - dump to working directory (or user defined)

void   mulle_objc_csvdump_methodcoverage( void)
{
   char   *filename;
   
   filename = getenv( "MULLE_OBJC_METHOD_COVERAGE_FILENAME");
   if( ! filename)
      filename = "method-coverage.csv";
   mulle_objc_csvdump_methodcoverage_to_file( filename);
}


void   mulle_objc_csvdump_classcoverage( void)
{
   char   *filename;
   
   filename = getenv( "MULLE_OBJC_CLASS_COVERAGE_FILENAME");
   if( ! filename)
      filename = "class-coverage.csv";

   mulle_objc_csvdump_classcoverage_to_file( filename);
}


void   mulle_objc_csvdump_cachesizes( void)
{
   char   *filename;
   
   filename = getenv( "MULLE_OBJC_CLASS_CACHESIZES_FILENAME");
   if( ! filename)
      filename = "class-cachesizes.csv";

   mulle_objc_csvdump_cachesizes_to_file( filename);
}
