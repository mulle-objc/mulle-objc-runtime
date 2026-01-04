//
//  mulle_objc_universe_csvdump.c
//  mulle-objc-debug-universe
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
#include "mulle-objc-csvdump.h"


#include "include-private.h"

#include <errno.h>

#include "mulle-objc-class.h"
#include "mulle-objc-class-search.h"
#include "mulle-objc-classpair.h"
#include "mulle-objc-infraclass.h"
#include "mulle-objc-metaclass.h"
#include "mulle-objc-methodlist.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-loadinfo.h"
#include "mulle-objc-class-impcache.h"


static const char   *tmpdir_names[] =
{
   "TMPDIR",
   "TMP",
   "TEMP",
   "TEMPDIR"
};

char   *_mulle_objc_get_tmpdir( void)
{
   char     *s;
   size_t   i;

   for( i = 0; i < sizeof( tmpdir_names) / sizeof( tmpdir_names[ 0]); i++)
   {
      s = getenv( tmpdir_names[ i]);
      if( s)
         return( s);
   }

#ifdef _WIN32
   return( ".");
#else
   return( "/tmp");
#endif
}



static void
  mulle_objc_searchresult_csvdump_to_fp( struct _mulle_objc_searchresult  *result,
                                         FILE *fp)
{
   mulle_objc_categoryid_t   categoryid;
   char                      *classname;
   char                      *categoryname;

   classname  = _mulle_objc_class_get_name( result->class);
   categoryid = mulle_objc_methodlist_get_categoryid( result->list);
   if( categoryid)
   {
      categoryname = mulle_objc_methodlist_get_categoryname( result->list);
      fprintf( fp, "%08lx;%s;%08lx;%s;",
              (unsigned long) _mulle_objc_class_get_classid( result->class),
              classname,
              (unsigned long) categoryid,
              categoryname);
   }
   else
      fprintf( fp, "%08lx;%s;;;",
              (unsigned long) _mulle_objc_class_get_classid( result->class),
              classname);

   fprintf( fp, "%08lx;%c%s\n",
           (unsigned long) _mulle_objc_method_get_methodid( result->method),
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

void
  mulle_objc_class_csvdump_cachedmethodcoverage_to_fp( struct _mulle_objc_class *cls,
                                                       FILE *fp)
{
   struct _mulle_objc_cache             *cache;
   struct _mulle_objc_cacheentry        *p;
   struct _mulle_objc_cacheentry        *sentinel;
   struct _mulle_objc_method            *method;
   struct _mulle_objc_searcharguments   search;
   struct _mulle_objc_searchresult      result;
   mulle_objc_methodid_t                methodid;

   if( ! cls || ! fp)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   cache = _mulle_objc_cachepivot_get_cache_atomic( &cls->cachepivot.pivot);
   if( ! _mulle_atomic_pointer_read( &cache->n))
      return;

   p        = _mulle_atomic_pointer_read_nonatomic( &cls->cachepivot.pivot.entries);
   sentinel = &p[ cache->size];

   while( p < sentinel)
   {
      methodid = (mulle_objc_methodid_t) (intptr_t) _mulle_atomic_pointer_read( &p->key.pointer);
      ++p;

      if( methodid == MULLE_OBJC_NO_METHODID)
         continue;

      search = mulle_objc_searcharguments_make_default( methodid);
      method = mulle_objc_class_search_method( cls,
                                               &search,
                                               _mulle_objc_class_get_inheritance( cls),
                                               &result);
      if( ! method)
         mulle_objc_universe_fail_inconsistency( mulle_objc_class_get_universe( cls),
                                                 "universe information has disappeared");

      fprintf( fp, "%08lx;%s;",
              (unsigned long) _mulle_objc_class_get_classid( cls),
              _mulle_objc_class_get_name( cls));

      mulle_objc_searchresult_csvdump_to_fp( &result, fp);
   }
}


void
  mulle_objc_class_csvdump_methodcoverage_to_fp( struct _mulle_objc_class *cls,
                                                 FILE *fp)
{
   struct  _mulle_objc_methodlistenumerator                enumerator;
   struct _mulle_objc_method                               *method;
   struct _mulle_objc_methodlist                           *list;
   struct _mulle_objc_searchresult                         result;
   struct mulle_concurrent_pointerarrayreverseenumerator   rover;
   unsigned int                                            n;

   if( ! cls || ! fp)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   n = mulle_concurrent_pointerarray_get_count( &cls->methodlists);
   assert( n);
   if( _mulle_objc_class_get_inheritance( cls) & MULLE_OBJC_CLASS_DONT_INHERIT_CATEGORIES)
      n = 1; // this works though it its a reverse enumerator, will get 0

   result.class = cls;

   rover = mulle_concurrent_pointerarray_reverseenumerate( &cls->methodlists, n);
   while( (list = _mulle_concurrent_pointerarrayreverseenumerator_next( &rover)))
   {
      result.list = list;

      enumerator = mulle_objc_methodlist_enumerate( list);
      while( method = _mulle_objc_methodlistenumerator_next( &enumerator))
      {
         if( ! (method->descriptor.bits & _mulle_objc_method_searched_and_found))
            continue;

         result.method = method;

         mulle_objc_searchresult_csvdump_to_fp( &result, fp);
      }
      mulle_objc_methodlistenumerator_done( &enumerator);
   }
   mulle_concurrent_pointerarrayreverseenumerator_done( &rover);
}


void
  mulle_objc_universe_csvdump_cachedmethodcoverage_to_fp( struct _mulle_objc_universe *universe,
                                                          FILE *fp)
{
   intptr_t                        classid;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;

   if( ! universe || ! fp)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   mulle_concurrent_hashmap_for( &universe->classtable, classid, infra)
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);

      mulle_objc_class_csvdump_cachedmethodcoverage_to_fp( _mulle_objc_metaclass_as_class( meta), fp);
      mulle_objc_class_csvdump_cachedmethodcoverage_to_fp( _mulle_objc_infraclass_as_class( infra), fp);
   }
}



void
  mulle_objc_universe_csvdump_methodcoverage_to_fp( struct _mulle_objc_universe *universe,
                                                    FILE *fp)
{
   intptr_t                        classid;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;

   if( ! universe || ! fp)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   mulle_concurrent_hashmap_for( &universe->classtable, classid, infra)
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);

      mulle_objc_class_csvdump_methodcoverage_to_fp( _mulle_objc_metaclass_as_class( meta), fp);
      mulle_objc_class_csvdump_methodcoverage_to_fp( _mulle_objc_infraclass_as_class( infra), fp);
   }
}


#define MULLE_OBJC_CLASS_CLUSTER_PROTOCOLID  ((mulle_objc_protocolid_t) 0x6ed73da1)

static int   mulle_objc_infraclass_is_classcluster_subclass( struct _mulle_objc_infraclass *infra)
{
   struct _mulle_objc_infraclass   *superclass;
   struct _mulle_objc_classpair    *pair;

   superclass = _mulle_objc_infraclass_get_superclass( infra);
   if( ! superclass)
      return( 0);
   pair = _mulle_objc_infraclass_get_classpair( superclass);
   return( _mulle_objc_classpair_has_protocolid( pair, MULLE_OBJC_CLASS_CLUSTER_PROTOCOLID));
}

void
  mulle_objc_universe_csvdump_classcoverage_to_fp( struct _mulle_objc_universe *universe,
                                                   FILE *fp)
{
   int                             nada;
   intptr_t                        classid;
   struct _mulle_objc_infraclass   *infra;

   if( ! universe || ! fp)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   //
   // here we just go through all installed classes and check the state
   // bit (this also captures fastclasses)
   //
   // if the class is part of a class cluster and the class cluster is
   // marked also use this class (except if not wanted). This makes coverage
   // easier for cases, where the tests miss a specific class, like lets
   // say _MulleJapaneseStringLength721.
   //
   nada  = 1;
   mulle_concurrent_hashmap_for( &universe->classtable, classid, infra)
   {
      if( ! _mulle_objc_infraclass_get_state_bit( infra, MULLE_OBJC_INFRACLASS_INITIALIZE_DONE))
      {
         if( universe->config.no_classcuster_coverage)
            continue;
         if( ! mulle_objc_infraclass_is_classcluster_subclass( infra))
            continue;
      }

      nada = 0;
      fprintf( fp, "%08lx;%s\n",
              (unsigned long) _mulle_objc_infraclass_get_classid( infra),
              _mulle_objc_infraclass_get_name( infra));
   }

   if( nada)
      fprintf( stderr, "mulle_objc_universe %p warning: no coverage generated "
                       "as no Objective-C code has run yet\n", universe);
}


//
// should dump collision statistics and "preferred" cache size here
//
static void  dump_cachesize( struct _mulle_objc_class *cls,
                             char prefix,
                             FILE *fp)
{
   struct _mulle_objc_cache   *cache;

   cache = _mulle_objc_class_get_impcache_cache_atomic( cls);

   if( ! _mulle_objc_cache_get_count( cache) &&
       ! _mulle_objc_cache_get_size( cache))
   {
       return;
   }

   fprintf( fp, "%08lx;%c%s;%u;%u;%x\n",
           (unsigned long) _mulle_objc_class_get_classid( cls),
           prefix,
           _mulle_objc_class_get_name( cls),
           _mulle_objc_cache_get_count( cache),
           _mulle_objc_cache_get_size( cache),
           _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_ALWAYS_EMPTY_CACHE) |
           _mulle_objc_class_get_state_bit( cls, MULLE_OBJC_CLASS_FIXED_SIZE_CACHE));
}


void   mulle_objc_universe_csvdump_cachesizes_to_fp( struct _mulle_objc_universe *universe,
                                                     FILE *fp)
{
   intptr_t                        classid;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;

   if( ! universe || ! fp)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   //
   // here we just go through all installed classes and check the state
   // bit (this also captures fastclasses)
   //

   mulle_concurrent_hashmap_for( &universe->classtable, classid, infra)
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);
      dump_cachesize( _mulle_objc_metaclass_as_class( meta), '+', fp);
      dump_cachesize( _mulle_objc_infraclass_as_class( infra), '-', fp);
   }
}

#pragma mark - dump starters

void
  mulle_objc_universe_csvdump_methodcoverage_to_filename( struct _mulle_objc_universe *universe,
                                                          char *filename)
{
   FILE   *fp;

   fp = fopen( filename, "a");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   mulle_objc_universe_csvdump_methodcoverage_to_fp( universe, fp);

   fclose( fp);

   fprintf( stderr, "Dumped method coverage to \"%s\"\n", filename);
}


void
  mulle_objc_universe_csvdump_classcoverage_to_filename( struct _mulle_objc_universe *universe,
                                                         char *filename)
{
   FILE   *fp;

   fp = fopen( filename, "a");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   mulle_objc_universe_csvdump_classcoverage_to_fp( universe, fp);

   fclose( fp);

   fprintf( stderr, "Dumped class coverage to \"%s\"\n", filename);
}


void
  mulle_objc_universe_csvdump_cachesizes_to_filename( struct _mulle_objc_universe *universe,
                                                      char *filename)
{
   FILE   *fp;

   fp = fopen( filename, "w");  // append makes no sense
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   mulle_objc_universe_csvdump_cachesizes_to_fp( universe, fp);

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


void   mulle_objc_loadinfo_csvdump_terse_to_fp( struct _mulle_objc_loadinfo *info,
                                                FILE *fp)
{
   char   *s;

   if( ! fp)
      mulle_objc_universe_fail_code( NULL, EINVAL);

   s = mulle_objc_loadinfo_get_origin( info);
   fprintf( fp, "%s;", s ? s : "");
   _fprint_csv_version( fp, info->version.runtime);
   _fprint_csv_version( fp, info->version.foundation);
   _fprint_csv_version( fp, info->version.user);
   fprintf( fp, "%u;",  (unsigned int) ((info->version.bits >> 8) & 0x7));
   fprintf( fp, "0x%lx", (unsigned long) info->version.bits);
   fprintf( fp, "\n");
}

