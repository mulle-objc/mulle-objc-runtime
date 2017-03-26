//
//  mulle_objc_runtime_dump_html.c
//  mulle-objc
//
//  Created by Nat! on 10.05.16.
//  Copyright (c) 2016 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
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
#include "mulle_objc_runtime_dump_html.h"

#include "mulle_objc_html.h"
#include "mulle_objc.h"
#include "mulle_objc_class.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mulle_thread/mulle_thread.h>
#include <mulle_allocator/mulle_allocator.h>

#include "c_set.inc"


# pragma mark - small routines to output the html

static char   *filename_for_class( struct _mulle_objc_class  *cls, char *directory, int is_meta)
{
   char     *buf;
   size_t   len;

   len = strlen( cls->name) + strlen( directory) + 16;
   buf = mulle_malloc( len);
   sprintf( buf, "%s/%s%s.html", directory, is_meta ? "+" : "", cls->name);
   return( buf);
}


static char   *filename_for_runtime( struct _mulle_objc_runtime  *runtime, char *directory)
{
   char     *buf;
   size_t   len;

   len = strlen( directory) + 16;
   buf = mulle_malloc( len);
   sprintf( buf, "%s/index.html", directory);
   return( buf);
}


static FILE  *open_and_print_start( char *name, char *title)
{
   FILE  *fp;

   fp = fopen( name, "w");
   if( ! fp)
   {
      perror( "fopen");
      abort();
   }

   fprintf( fp, "\
<html>\n\
<header>\n\
<title>%s</title>\n\
</header>\n\
<body>\n\
", title);

   return( fp);
}


static void  print_to_body( char *title, char *s, FILE *fp)
{
   if( title)
      fprintf( fp, "\n<h1>%s</h1>\n", title);
   if( s)
      fprintf( fp, "%s\n", s);
}


static void  print_end_and_close( FILE *fp)
{
   fprintf( fp, "\
</body>\n\
</html>\n\
");
   fclose( fp);
}


# pragma mark - walker runtime callback

static void   _print_runtime( struct _mulle_objc_runtime *runtime, FILE *fp)
{
   char                                             *label;
   int                                              i;
   struct _mulle_objc_staticstring                  *string;
   struct _mulle_objc_class                         *cls;
   struct mulle_concurrent_pointerarrayenumerator   rover;

   label = mulle_objc_runtime_html_description( runtime);
   print_to_body( "Values", label, fp);
   free( label);

   label = mulle_concurrent_hashmap_html_description( &runtime->classtable, (char *(*)()) mulle_objc_class_short_html_description);
   print_to_body( "Classes", label, fp);
   free( label);


   print_to_body( "Fast Classes", NULL, fp);
   fprintf( fp, "<ol>\n");
   for( i = 0; i < MULLE_OBJC_S_FASTCLASSES; i++)
      if( _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer))
      {
         cls = _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer);
         label = mulle_objc_class_short_html_description( cls);
         fprintf( fp, "<li>%s\n", label);
         free( label);
      }
   fprintf( fp, "</ol>\n");


   label = mulle_concurrent_hashmap_html_description( &runtime->descriptortable, (char *(*)()) mulle_objc_methoddescriptor_html_hor_description);
   print_to_body( "Descriptor Table", label, fp);
   free( label);


   print_to_body( "Static Strings", NULL, fp);
   fprintf( fp, "<ul>\n");

   rover = mulle_concurrent_pointerarray_enumerate( &runtime->staticstrings);
   while( string = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      label = mulle_objc_staticstring_html_description( string);
      fprintf( fp, "<li>%s\n", label);
      free( label);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);
   fprintf( fp, "</ul>\n");
}


static void   print_runtime( struct _mulle_objc_runtime *runtime, char *directory)
{
   char   *path;
   FILE   *fp;

   path = filename_for_runtime( runtime, directory);

   fp = open_and_print_start( path, "Runtime");
   _print_runtime( runtime, fp);
   print_end_and_close( fp);

   mulle_free( path);
}


# pragma mark - walker class callback

struct dump_info
{
   c_set  set;
   char   *directory;
};


static void   _print_class( struct _mulle_objc_class *cls, FILE *fp, int is_meta)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_protocolclassenumerator       prover;
   char                                             *label;
   struct _mulle_objc_cache                         *cache;
   struct _mulle_objc_class                         *superclass;
   struct _mulle_objc_class                         *metaclass;
   struct _mulle_objc_class                         *infraclass;
   struct _mulle_objc_class                         *propertyclass;
   struct _mulle_objc_ivarlist                      *ivarlist;
   struct _mulle_objc_methodlist                    *methodlist;
   struct _mulle_objc_propertylist                  *propertylist;

   print_to_body( "Runtime","<a href=\"index.html\">Runtime</a>", fp);

   superclass = _mulle_objc_class_get_superclass( cls);
   if( superclass)
   {
      label = mulle_objc_class_short_html_description( superclass);
      print_to_body( "Superclass", label, fp);
      free( label);
   }

   infraclass = _mulle_objc_class_get_infraclass( cls);
   if( infraclass)
   {
      label = mulle_objc_class_short_html_description( infraclass);
      print_to_body( "Infraclass", label, fp);
      free( label);
   }

   metaclass = _mulle_objc_class_get_metaclass( cls);
   if( metaclass)
   {
      label = mulle_objc_class_short_html_description( metaclass);
      print_to_body( "Metaclass", label, fp);
      free( label);
   }

   label = mulle_objc_class_html_description( cls, is_meta ? "purple" : "blue");
   print_to_body( "Values", label, fp);
   free( label);

   print_to_body( "Instance Variables", NULL, fp);
   fprintf( fp, "<ol>\n");

   rover = mulle_concurrent_pointerarray_enumerate( &cls->ivarlists);
   while( ivarlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      label = mulle_objc_ivarlist_html_hor_description( ivarlist);
      fprintf( fp, "<li> %s\n", label);
      free( label);
   }
   fprintf( fp, "</ol>\n");

   mulle_concurrent_pointerarrayenumerator_done( &rover);

   print_to_body( "Properties", NULL, fp);
   fprintf( fp, "<ol>\n");

   rover = mulle_concurrent_pointerarray_enumerate( &cls->propertylists);
   while( propertylist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      label = mulle_objc_propertylist_html_description( propertylist);
      fprintf( fp, "<li> %s\n", label);
      free( label);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);
   fprintf( fp, "</ol>\n");


   print_to_body( "Method Lists", NULL, fp);
   fprintf( fp, "<ol>\n");

   rover = mulle_concurrent_pointerarray_enumerate( &cls->methodlists);
   while( methodlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      label = mulle_objc_methodlist_html_hor_description( methodlist);
      fprintf( fp, "<li> %s\n", label);
      free( label);
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);
   fprintf( fp, "</ol>\n");


   print_to_body( "Inherited Protocols", NULL, fp);
   fprintf( fp, "<ol>\n");
   if( ! (cls->inheritance & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
   {
      prover = _mulle_objc_class_enumerate_protocolclasses( cls);
      while( propertyclass = _mulle_objc_protocolclassenumerator_next( &prover))
      {
         label = mulle_objc_class_short_html_description( propertyclass);
         fprintf( fp, "<li>%s\n", label);
         free( label);
      }
      mulle_concurrent_pointerarrayenumerator_done( &rover);
   }
   fprintf( fp, "</ol>\n");

   if( mulle_concurrent_pointerarray_get_count( &cls->protocolids))
   {
      label = mulle_concurrent_pointerarray_html_description( &cls->protocolids);
      print_to_body( "Conforming to Protocols", label, fp);
      free( label);
   }

   cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   label = mulle_objc_cache_html_description( cache);
   print_to_body( "Cache", label, fp);
   free( label);
}


static void   print_class( struct _mulle_objc_class *cls, char *directory, int is_meta)
{
   char   *path;
   FILE   *fp;

   path = filename_for_class( cls, directory, is_meta);

   fp = open_and_print_start( path, cls->name);
   _print_class( cls, fp, is_meta);
   print_end_and_close( fp);

   mulle_free( path);
}


static int   callback( struct _mulle_objc_runtime *runtime,
                       void *p,
                       enum mulle_objc_runtime_type_t type,
                       char *key,
                       void *parent,
                       void *userinfo)
{
   char                       *directory;
   struct _mulle_objc_class   *cls;
   struct dump_info           *info;

   assert( p);

   if( key)
      return( mulle_objc_runtime_walk_ok);

   info      = userinfo;
   directory = info->directory;

   if( c_set_member( &info->set, p))
      return( mulle_objc_runtime_walk_dont_descend);
   c_set_add( &info->set, p);

   switch( type)
   {
   case mulle_objc_runtime_is_category :
   case mulle_objc_runtime_is_protocol :
      break;

   case mulle_objc_runtime_is_runtime  :
      runtime = p;
      print_runtime( runtime, directory);
      break;

   case mulle_objc_runtime_is_class :
      cls = p;
      print_class( cls, directory, 0);
      break;

   case mulle_objc_runtime_is_meta_class :
      cls = p;
      print_class( cls, directory, 1);
      break;

   case mulle_objc_runtime_is_method :
   case mulle_objc_runtime_is_property :
   case mulle_objc_runtime_is_ivar :
      break;
   }

   return( mulle_objc_runtime_walk_ok);
}


# pragma mark - runtime dump

void   mulle_objc_runtime_dump_as_html_to_directory( struct _mulle_objc_runtime *runtime, char *directory)
{
   struct dump_info  info;

   c_set_init( &info.set);
   info.directory = directory;

   mulle_objc_runtime_walk( runtime, callback, &info);

   c_set_done( &info.set);
   fprintf( stderr, "Dumped to \"%s\"\n", directory);
}


void   mulle_objc_dump_runtime_as_html_to_directory( char *directory)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_get_runtime();
   if( ! _mulle_objc_runtime_trylock( runtime))
   {
      mulle_objc_runtime_dump_as_html_to_directory( runtime, directory);

      _mulle_objc_runtime_unlock( runtime);
   }
   else
      fprintf( stderr, "runtime locked\n");
}


// we have no mkdir, chdir getcwd just
void   mulle_objc_dump_runtime_as_html_to_tmp( void)
{
   mulle_objc_dump_runtime_as_html_to_directory( "/tmp");
}




