//
//  mulle_objc_htmldump.c
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
#include "mulle_objc_htmldump.h"

#include "mulle_objc_html.h"
#include "mulle_objc.h"
#include "mulle_objc_class.h"
#include "mulle_objc_infraclass.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mulle_thread/mulle_thread.h>
#include <mulle_allocator/mulle_allocator.h>
#include <mulle_concurrent/mulle_concurrent.h>

#include "c_set.inc"


static struct _mulle_objc_htmltablestyle    categorytable_style =
{
   "categories",
   "category",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle    protocoltable_style =
{
   "protocols",
   "protocol",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle    infraclass_style =
{
   "infraclass",
   "infra",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle    cachetable_style =
{
   "cache",
   "cache",
   NULL,
   NULL,
   0
};



static struct _mulle_objc_htmltablestyle    ivartable_style =
{
   "ivars",
   "ivar",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle    propertytable_style =
{
   "properties",
   "property",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle  classtable_style =
{
   "classes",
   "class",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle  descriptortable_style =
{
   "selectors",
   "selector",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle  runtime_style =
{
   "runtime",
   "runtime",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle  stringtable_style =
{
   "strings",
   "string",
   NULL,
   NULL,
   0
};


static struct _mulle_objc_htmltablestyle  methodlisttable_style =
{
   "methods",
   "method",
   NULL,
   NULL,
   0
};


# pragma mark - small routines to output the html

static char   *html_escape( char *s)
{
   if( ! strchr( s, '&') && ! strchr( s, '<'))
      return( s);

   return( "bad-html");
}


static char   *html_filename_for_classname( char *name, char *directory)
{
   char     *buf;
   size_t   len;

   assert( name);
   assert( directory);

   len = strlen( name) + strlen( directory) + 16;
   buf = mulle_allocator_malloc( &mulle_stdlib_allocator, len);
   sprintf( buf, "%s/%s.html", directory, html_escape( name));
   return( buf);
}


static char   *filename_for_runtime( struct _mulle_objc_runtime  *runtime,
                                     char *directory)
{
   char     *buf;
   size_t   len;

   assert( directory);
   
   len = strlen( directory) + 16;
   buf = mulle_allocator_malloc( &mulle_stdlib_allocator, len);
   sprintf( buf, "%s/index.html", directory);
   return( buf);
}


static FILE  *open_and_print_start( char *name, char *title)
{
   FILE  *fp;
   char  *cssurl;

   fp = fopen( name, "w");
   if( ! fp)
   {
      perror( "fopen");
      abort();
   }

   cssurl = getenv( "MULLE_OBJC_CSS_URL");
   if( ! cssurl)
      cssurl = "mulle-objc.css";

   fprintf( fp,
           "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n" // pedantic
           "<HTML>\n"
           "<HEAD>\n"
           "<TITLE>%s</TITLE>\n"
           "<LINK TYPE=\"text/css\" REL=\"stylesheet\" HREF=\"%s\">\n"
           "</HEAD>\n"
           "<BODY>\n",
           title,
           cssurl);

   return( fp);
}


static void  print_to_body_with_level( char *title, char *s, int level, FILE *fp)
{
   if( title)
      fprintf( fp, "\n<H%d>%s</H%d>\n", level, title, level);
   if( s)
      fprintf( fp, "%s\n", s);
}


static void  print_to_body( char *title, char *s, FILE *fp)
{
   print_to_body_with_level( title, s, 2, fp);
}


static void  print_end_and_close( FILE *fp)
{
   fprintf( fp, "\
</BODY>\n\
</HTML>\n\
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

   fprintf( fp, "\n<DIV CLASS=\"runtime_values\">\n");
   {
      label = mulle_objc_runtime_html_description( runtime, &runtime_style);
      print_to_body( "Values", label, fp);
      mulle_allocator_free( &mulle_stdlib_allocator, label);
   }
   fprintf( fp, "</DIV>\n");

   // need to sort this in the future
   fprintf( fp, "\n<DIV CLASS=\"runtime_classes\">\n");
   {
      label = mulle_concurrent_hashmap_html_description( &runtime->classtable,
                                                      mulle_objc_class_html_row_description,
                                                        &classtable_style);
      print_to_body( "Classes", label, fp);
      mulle_allocator_free( &mulle_stdlib_allocator, label);
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"runtime_fastclasses\">\n");
   {
      for( i = 0; i < MULLE_OBJC_S_FASTCLASSES; i++)
         if( _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer))
            break;

      if( i < MULLE_OBJC_S_FASTCLASSES)
      {
         print_to_body( "Fast Classes", NULL, fp);

         fprintf( fp, "<TABLE CLASS=\"runtime_fastclass_table\">\n");

         for( i = 0; i < MULLE_OBJC_S_FASTCLASSES; i++)
            if( _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer))
            {
               cls = _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer);
               label = mulle_objc_class_short_html_description( cls, &classtable_style);
               fprintf( fp, "<TR><TH>%u</TH><TD>%s</TD></TR>\n", i, label);
               mulle_allocator_free( &mulle_stdlib_allocator, label);
            }
         fprintf( fp, "</TABLE>\n");
      }
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"runtime_methoddescriptors\">\n");
   {
      if( mulle_concurrent_hashmap_count( &runtime->descriptortable))
      {
         fprintf( fp, "<TABLE CLASS=\"runtime_methoddescriptor_table\">\n");
         label = mulle_concurrent_hashmap_html_description( &runtime->descriptortable,
                                                           mulle_objc_methoddescriptor_html_row_description,
                                                           &descriptortable_style);
         print_to_body( "Method Descriptors", label, fp);
         mulle_allocator_free( &mulle_stdlib_allocator, label);
         fprintf( fp, "</TABLE>\n");
      }
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"runtime_strings\">\n");
   {
      if( mulle_concurrent_pointerarray_get_count( &runtime->staticstrings))
      {
         print_to_body( "Strings", NULL, fp);

         fprintf( fp, "<TABLE CLASS=\"runtime_string_table\">\n");
         rover = mulle_concurrent_pointerarray_enumerate( &runtime->staticstrings);
         while( string = _mulle_concurrent_pointerarrayenumerator_next( &rover))
         {
            label = mulle_objc_staticstring_html_description( string, &stringtable_style);
            fprintf( fp, "<li>%s\n", label);
            mulle_allocator_free( &mulle_stdlib_allocator, label);
         }
         mulle_concurrent_pointerarrayenumerator_done( &rover);
         fprintf( fp, "</TABLE>\n");
      }
   }
   fprintf( fp, "</DIV>\n");
}


static void   print_runtime( struct _mulle_objc_runtime *runtime,
                             char *directory)
{
   char   *path;
   FILE   *fp;

   path = filename_for_runtime( runtime, directory);

   fp = open_and_print_start( path, "Runtime");
   _print_runtime( runtime, fp);
   print_end_and_close( fp);

   mulle_allocator_free( &mulle_stdlib_allocator, path);
}


# pragma mark - walker class callback

struct dump_info
{
   c_set  set;
   char   *directory;
};


static void   _print_infraclass( struct _mulle_objc_infraclass *infra, FILE *fp)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_protocolclassenumerator       prover;
   char                                             *label;
   struct _mulle_objc_cache                         *cache;
   struct _mulle_objc_class                         *cls;
   struct _mulle_objc_infraclass                    *superclass;
   struct _mulle_objc_metaclass                     *meta;
   struct _mulle_objc_classpair                     *pair;
   struct _mulle_objc_infraclass                    *prop_cls;
   struct _mulle_objc_ivarlist                      *ivarlist;
   struct _mulle_objc_methodlist                    *methodlist;
   struct _mulle_objc_propertylist                  *propertylist;
   struct _mulle_objc_htmltablestyle                style;
   struct _mulle_objc_uniqueidarray                 *array;
   mulle_objc_categoryid_t                          categoryid;
   
   cls = _mulle_objc_infraclass_as_class( infra);

   print_to_body_with_level( cls->name, NULL, 1, fp);

   print_to_body( NULL, "<DIV CLASS=\"class_links\">", fp);
   {
      print_to_body( NULL, "<DIV CLASS=\"class_runtime_link\">", fp);
      {
         print_to_body( "Runtime","<A HREF=\"index.html\">Runtime</a>", fp);
      }
      print_to_body( NULL, "</DIV>\n", fp);

      print_to_body( NULL, "<DIV CLASS=\"class_superclass_link\">", fp);
      {
         superclass = _mulle_objc_infraclass_get_superclass( infra);
         if( superclass)
         {
            label = mulle_objc_class_short_html_description( _mulle_objc_infraclass_as_class( superclass),
                                                            &classtable_style);
            print_to_body( "Superclass", label, fp);
            mulle_allocator_free( &mulle_stdlib_allocator, label);
         }
      }
      print_to_body( NULL, "</DIV>\n", fp);
   }
   print_to_body( NULL, "</DIV>\n", fp);

   fprintf( fp, "\n<DIV CLASS=\"class_protocolclass_links\">\n");
   {
      pair = _mulle_objc_infraclass_get_classpair( infra);
      if( ! (_mulle_objc_infraclass_get_inheritance( infra) & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS) &&
         mulle_concurrent_pointerarray_get_count( &pair->protocolclasses))
      {
         print_to_body( "Inherited Protocol Classes", NULL, fp);
         fprintf( fp, "<OL>\n");

         prover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
         while( prop_cls = _mulle_objc_protocolclassenumerator_next( &prover))
         {
            label = mulle_objc_class_short_html_description( _mulle_objc_infraclass_as_class( prop_cls),
                                                            &classtable_style);
            fprintf( fp, "<LI>%s\n", label);
            mulle_allocator_free( &mulle_stdlib_allocator, label);
         }
         mulle_concurrent_pointerarrayenumerator_done( &rover);

         fprintf( fp, "</OL>\n");
      }
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"class_values\">\n");
   {
      style       = infraclass_style;
      style.title = cls->name;
      label = mulle_objc_class_html_description( cls, &style);

      print_to_body( "Values", label, fp);
      mulle_allocator_free( &mulle_stdlib_allocator, label);
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"class_properties\">\n");
   {
      if( infra && mulle_concurrent_pointerarray_get_count( &infra->propertylists))
      {
         print_to_body( "Property Lists", NULL, fp);

         rover = mulle_concurrent_pointerarray_enumerate( &infra->propertylists);
         while( propertylist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
         {
            label = mulle_objc_propertylist_html_description( propertylist, &propertytable_style);
            fprintf( fp, "%s\n", label);
            mulle_allocator_free( &mulle_stdlib_allocator, label);
         }
         mulle_concurrent_pointerarrayenumerator_done( &rover);
      }
   }
   fprintf( fp, "</DIV>\n");


   fprintf( fp, "\n<DIV CLASS=\"class_ivars\">\n");
   {
      if( infra && mulle_concurrent_pointerarray_get_count( &infra->ivarlists))
      {
         print_to_body( "Instance Variable Lists", NULL, fp);

         rover = mulle_concurrent_pointerarray_enumerate( &infra->ivarlists);
         while( ivarlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
         {
            label = mulle_objc_ivarlist_html_hor_description( ivarlist, &ivartable_style);
            fprintf( fp, "%s\n", label);
            mulle_allocator_free( &mulle_stdlib_allocator, label);
         }
         mulle_concurrent_pointerarrayenumerator_done( &rover);
      }
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"class_classmethods\">\n");
   {
      meta = _mulle_objc_infraclass_get_metaclass( infra);
      if( mulle_concurrent_pointerarray_get_count( &meta->base.methodlists))
      {
         print_to_body( "+ Method Lists", NULL, fp);

         rover = mulle_concurrent_pointerarray_enumerate( &meta->base.methodlists);
         while( methodlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
         {
            style       = methodlisttable_style;
            categoryid  = (mulle_objc_categoryid_t) (uintptr_t) methodlist->owner;
            if( categoryid)
               style.title = mulle_objc_string_for_categoryid( categoryid);
            else
               style.title = "";
            label       = mulle_objc_methodlist_html_hor_description( methodlist, &methodlisttable_style);
            fprintf( fp, "%s\n", label);
            mulle_allocator_free( &mulle_stdlib_allocator, label);
         }
         mulle_concurrent_pointerarrayenumerator_done( &rover);
      }
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"class_instancemethods\">\n");
   {
      if( mulle_concurrent_pointerarray_get_count( &infra->base.methodlists))
      {
         print_to_body( "- Method Lists", NULL, fp);

         rover = mulle_concurrent_pointerarray_enumerate( &infra->base.methodlists);
         while( methodlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
         {
            style = methodlisttable_style;
            style.title = methodlist->owner ? methodlist->owner : "";
            label = mulle_objc_methodlist_html_hor_description( methodlist, &methodlisttable_style);
            fprintf( fp, "%s\n", label);
            mulle_allocator_free( &mulle_stdlib_allocator, label);
         }
         mulle_concurrent_pointerarrayenumerator_done( &rover);
      }
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"class_protocols\">\n");
   {
      array = _mulle_atomic_pointer_read( &pair->p_protocolids.pointer);
      if( array->n)
      {
         label = mulle_objc_protocols_html_description( array,
                                                        &protocoltable_style);
         print_to_body( "Protocols", label, fp);
         mulle_allocator_free( &mulle_stdlib_allocator, label);
      }
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"class_categories\">\n");
   {
      array = _mulle_atomic_pointer_read( &pair->p_categoryids.pointer);
      if( array->n)
      {
         label = mulle_objc_categories_html_description( array,
                                                         &categorytable_style);

         print_to_body( "Categories", label, fp);
         mulle_allocator_free( &mulle_stdlib_allocator, label);
      }
   }
   fprintf( fp, "</DIV>\n");

   fprintf( fp, "\n<DIV CLASS=\"class_cache\">\n");
   {
      cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
      label = mulle_objc_cache_html_description( cache, &cachetable_style);
      print_to_body( "Cache", label, fp);
      mulle_allocator_free( &mulle_stdlib_allocator, label);
   }
   fprintf( fp, "</DIV>\n");
}


static void   print_infraclass( struct _mulle_objc_infraclass *infra, char *directory)
{
   char   *path;
   FILE   *fp;

   path = html_filename_for_classname( infra->base.name, directory);

   fp = open_and_print_start( path, infra->base.name);
   _print_infraclass( infra, fp);
   print_end_and_close( fp);

   mulle_allocator_free( &mulle_stdlib_allocator, path);
}


static int   callback( struct _mulle_objc_runtime *runtime,
                       void *p,
                       enum mulle_objc_walkpointertype_t type,
                       char *key,
                       void *parent,
                       void *userinfo)
{
   char                            *directory;
   struct _mulle_objc_infraclass   *infra;
   struct dump_info                *info;

   assert( p);

   if( key)
      return( mulle_objc_walk_ok);

   info      = userinfo;
   directory = info->directory;

   if( c_set_member( &info->set, p))
      return( mulle_objc_walk_dont_descend);
   c_set_add( &info->set, p);

   switch( type)
   {
   case mulle_objc_walkpointer_is_category :
   case mulle_objc_walkpointer_is_protocol :
   case mulle_objc_walkpointer_is_classpair :
      break;

   case mulle_objc_walkpointer_is_runtime  :
      runtime = p;
      print_runtime( runtime, directory);
      break;

   case mulle_objc_walkpointer_is_infraclass :
      infra = p;
      print_infraclass( infra, directory);
      break;

   case mulle_objc_walkpointer_is_metaclass :
   case mulle_objc_walkpointer_is_method :
   case mulle_objc_walkpointer_is_property :
   case mulle_objc_walkpointer_is_ivar :
      break;
   }

   return( mulle_objc_walk_ok);
}


# pragma mark - runtime dump

void   mulle_objc_runtime_htmldump_to_directory( struct _mulle_objc_runtime *runtime,
                                                 char *directory)
{
   struct dump_info  info;

   c_set_init( &info.set);
   info.directory = directory;

   mulle_objc_runtime_walk( runtime, callback, &info);

   c_set_done( &info.set);

   fprintf( stderr, "Dumped HTML to \"%s\"\n", directory);
}


void   mulle_objc_htmldump_runtime_to_directory( char *directory)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_get_runtime();
   mulle_objc_runtime_htmldump_to_directory( runtime, directory);
}


#pragma mark - class dump

void   mulle_objc_classpair_htmldump_to_directory( struct _mulle_objc_classpair *pair,
                                                   char *directory)
{
   struct dump_info               info;

   c_set_init( &info.set);
   info.directory = directory;

   mulle_objc_classpair_walk( pair, callback, &info);

   c_set_done( &info.set);
}


void   mulle_objc_class_htmldump_to_directory( struct _mulle_objc_class *cls,
                                               char *directory)
{
   struct _mulle_objc_classpair   *pair;

   do
   {
      pair = _mulle_objc_class_get_classpair( cls);
      mulle_objc_classpair_htmldump_to_directory( pair, directory);
   }
   while( cls = _mulle_objc_class_get_superclass( cls));

   fprintf( stderr, "Dumped HTML to \"/%s\"\n", directory);
}


void   mulle_objc_htmldump_classname_to_directory( char *classname, char *directory)
{
   struct _mulle_objc_runtime     *runtime;
   struct _mulle_objc_class       *cls;
   struct _mulle_objc_infraclass  *infra;
   mulle_objc_classid_t           classid;

   if( ! classname || ! *classname)
   {
      fprintf( stderr, "Invalid classname\n");
      return;
   }

   runtime = mulle_objc_get_runtime();
   classid = mulle_objc_classid_from_string( classname);
   infra   = _mulle_objc_runtime_get_or_lookup_infraclass( runtime, classid);
   if( ! infra)
   {
      fprintf( stderr, "Class \"%s\" is unknown to the runtime\n", classname);
      return;
   }

   cls = _mulle_objc_infraclass_as_class( infra);
   mulle_objc_class_htmldump_to_directory( cls, directory);
}

#pragma mark - dump to /tmp

void   mulle_objc_class_htmldump_to_tmp( struct _mulle_objc_class *cls)
{
   mulle_objc_class_htmldump_to_directory( cls, "/tmp");
}


void   mulle_objc_htmldump_classname_to_tmp( char *classname)
{
   mulle_objc_htmldump_classname_to_directory( classname, "/tmp");
}


// we have no mkdir, chdir getcwd just
void   mulle_objc_htmldump_runtime_to_tmp( void)
{
   mulle_objc_htmldump_runtime_to_directory( "/tmp");
}


#pragma mark - dump to working directory

void   mulle_objc_htmldump_runtime( void)
{
   mulle_objc_htmldump_runtime_to_directory( ".");
}


void   mulle_objc_class_htmldump( struct _mulle_objc_class *cls)
{
   mulle_objc_class_htmldump_to_directory( cls, ".");
}


void   mulle_objc_htmldump_classname( char *classname)
{
   mulle_objc_htmldump_classname_to_directory( classname, ".");
}
