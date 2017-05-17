//
//  mulle_objc_csvdump.c
//  mulle-objc-runtime
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
#include "mulle_objc_class_runtime.h"
#include "mulle_objc_class_struct.h"
#include "mulle_objc_classpair.h"
#include "mulle_objc_infraclass.h"
#include "mulle_objc_metaclass.h"
#include "mulle_objc_method.h"
#include "mulle_objc_runtime.h"

#include <errno.h>


//
// remember a classpair needs to dump both meta and infra (this can handle both
// but only dumps either)
//
// currently we look for methods that are in the cache, and we never really
// empty the cache. BUT! This doesn't catch calls to methods, that are
// overridden later. If we need this sometime, the solution is to turn
// methoddescriptor bits into an atomic void * and set a bit on it, whenever
// the method is added to the cache. Then we "just" traverse all methodlists
// of all classes and dump them out. The format is different though, since
// we don't have the cache information.
//
void   mulle_objc_class_csvdump_methodcoverage( struct _mulle_objc_class *cls,
                                                FILE *fp)
{
   struct _mulle_objc_cache         *cache;
   struct _mulle_objc_cacheentry    *p;
   struct _mulle_objc_cacheentry    *sentinel;
   struct _mulle_objc_method        *method;
   mulle_objc_methodid_t            methodid;
   mulle_objc_categoryid_t          categoryid;
   struct _mulle_objc_searchresult  result;
   char                             *classname;
   char                             *categoryname;
   
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
         mulle_objc_raise_inconsistency_exception( "runtime information has disappeared");
      
      fprintf( fp, "%08x;%s;",
              _mulle_objc_class_get_classid( cls),
              _mulle_objc_class_get_name( cls));
      
      categoryid = (mulle_objc_categoryid_t) (intptr_t) result.list->owner;
      classname  = _mulle_objc_class_get_name( result.class);
      if( categoryid)
      {
         categoryname = mulle_objc_string_for_categoryid( categoryid);
         fprintf( fp, "%08x;%08x;%s;%s;",
                 _mulle_objc_class_get_classid( result.class),
                 categoryid,
                 classname,
                 categoryname);
      }
      else
         fprintf( fp, "%08x;;%s;;",
                       _mulle_objc_class_get_classid( result.class),
                       classname);
      
      fprintf( fp, "%08x;%c%s\n",
              _mulle_objc_method_get_methodid( method),
              _mulle_objc_class_is_metaclass( cls) ? '+' : '-',
              _mulle_objc_method_get_name( method));
   }
}


void   mulle_objc_runtime_csvdump_methodcoverage( struct _mulle_objc_runtime *runtime,
                                                 FILE *fp)
{
   intptr_t                                    classid;
   struct _mulle_objc_infraclass               *infra;
   struct _mulle_objc_metaclass                *meta;
   struct mulle_concurrent_hashmapenumerator   rover;

   if( ! runtime || ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }

   rover = mulle_concurrent_hashmap_enumerate( &runtime->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &classid, (void **) &infra))
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);

      mulle_objc_class_csvdump_methodcoverage( _mulle_objc_metaclass_as_class( meta), fp);
      mulle_objc_class_csvdump_methodcoverage( _mulle_objc_infraclass_as_class( infra), fp);
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
}



void   mulle_objc_runtime_csvdump_classcoverage( struct _mulle_objc_runtime *runtime,
                                                FILE *fp)
{
   intptr_t                                    classid;
   struct _mulle_objc_infraclass               *infra;
   struct _mulle_objc_metaclass                *meta;
   struct mulle_concurrent_hashmapenumerator   rover;
   
   if( ! runtime || ! fp)
   {
      errno = EINVAL;
      mulle_objc_raise_fail_errno_exception();
   }

   //
   // here we just go through all installed classes and check the state
   // bit (this also captures fastclasses)
   //
   
   runtime = mulle_objc_get_runtime();
   rover = mulle_concurrent_hashmap_enumerate( &runtime->classtable);
   while( _mulle_concurrent_hashmapenumerator_next( &rover, &classid, (void **) &infra))
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);
      if( ! _mulle_objc_metaclass_get_state_bit( meta, MULLE_OBJC_META_INITIALIZE_DONE))
         continue;
      
      fprintf( fp, "%08x;%s\n",
              _mulle_objc_infraclass_get_classid( infra),
              _mulle_objc_infraclass_get_name( infra));
   }
   mulle_concurrent_hashmapenumerator_done( &rover);
}

#pragma mark - conveniences

void   mulle_objc_csvdump_methodcoverage_to_file( char *filename)
{
   struct _mulle_objc_runtime   *runtime;
   FILE                         *fp;

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }
   
   runtime = mulle_objc_get_runtime();
   mulle_objc_runtime_csvdump_methodcoverage( runtime, fp);
   
   fclose( fp);
}


void   mulle_objc_csvdump_classcoverage_to_file( char *filename)
{
   struct _mulle_objc_runtime   *runtime;
   FILE                         *fp;
   
   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   runtime = mulle_objc_get_runtime();
   mulle_objc_runtime_csvdump_classcoverage( runtime, fp);

   fclose( fp);
}


void   mulle_objc_csvdump_methodcoverage_to_tmp( void)
{
   mulle_objc_csvdump_methodcoverage_to_file( "/tmp/method-coverage.csv");
}


void   mulle_objc_csvdump_classcoverage_to_tmp( void)
{
   mulle_objc_csvdump_classcoverage_to_file( "/tmp/class-coverage.csv");
}


void   mulle_objc_csvdump_methodcoverage( void)
{
   mulle_objc_csvdump_methodcoverage_to_file( "method-coverage.csv");
}


void   mulle_objc_csvdump_classcoverage( void)
{
   mulle_objc_csvdump_classcoverage_to_file( "class-coverage.csv");
}
