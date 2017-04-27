//
//  mulle_objc_runtime_dotdump.c
//  mulle-objc
//
//  Created by Nat! on 25.10.15.
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

#include "mulle_objc_runtime_dotdump.h"

#include "mulle_objc.h"
#include "mulle_objc_html.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mulle_thread/mulle_thread.h>
#include <mulle_concurrent/mulle_concurrent.h>

#include "c_set.inc"


//
// just don't output stuff with ampersands for now
// What is this used for ?
//
void   mulle_objc_methodlist_dump( struct _mulle_objc_methodlist *list)
{
   unsigned int   i;

   for( i = 0; i < list->n_methods; i++)
      printf( "{ "
              "name = \"%s\""
              "signature = \"%s\""
              "methodid = %08x"
              "bits = 0x%x"
              "implementation = %p"
              " }",
               list->methods[ i].descriptor.name,
               list->methods[ i].descriptor.signature,
               list->methods[ i].descriptor.methodid,
               list->methods[ i].descriptor.bits,
               list->methods[ i].implementation);
}


# pragma mark - "styling"

static struct _mulle_objc_colored_string    descriptortable_title =
{
   "descriptors",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    staticstringtable_title =
{
   "static strings",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    categorytable_title =
{
   "categories",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    protocoltable_title =
{
   "protocols",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    classestoload_title =
{
   "classes to load",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    categoriestoload_title =
{
   "categories to load",
   "black",
   "white"
};



# pragma mark - walker runtime callback

static char  *mulle_objc_loadclasslist_html_row_description( intptr_t  classid, void *value)
{
   struct mulle_concurrent_pointerarray   *array = value;

   return( mulle_concurrent_pointerarray_html_description( array,
                                                           mulle_objc_loadclass_html_row_description,
                                                           NULL));
}


static char  *mulle_objc_loadcategorylist_html_row_description( intptr_t  classid, void *value)
{
   struct mulle_concurrent_pointerarray   *array = value;
   
   return( mulle_concurrent_pointerarray_html_description( array,
                                                           mulle_objc_loadcategory_html_row_description,
                                                           NULL));
}


struct dump_info
{
   c_set  set;
   FILE   *fp;
   int    terse_rootclass;
   int    draw_runtime;
};


static void   print_runtime( struct _mulle_objc_runtime *runtime,
                             struct dump_info *info)
{
   char   *label;
   int    i;
   
   label = mulle_objc_runtime_html_description( runtime);
   fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n", runtime, label, "component");
   free( label);

   if( mulle_concurrent_hashmap_count( &runtime->descriptortable))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"descriptortable\" ];\n",
              runtime, &runtime->descriptortable);
      
      label = mulle_concurrent_hashmap_html_description( &runtime->descriptortable,
                                                         mulle_objc_methoddescriptor_html_row_description,
                                                         &descriptortable_title);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &runtime->descriptortable, label, "box");
      free( label);
   }
   
   for( i = 0; i < MULLE_OBJC_S_FASTCLASSES; i++)
      if( _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer))
         fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"fastclass #%d\" ];\n",
            runtime, _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer), i);

   if( mulle_concurrent_pointerarray_get_count( &runtime->staticstrings))
   {
      label = mulle_concurrent_pointerarray_html_description( &runtime->staticstrings,
                                                              mulle_objc_staticstring_html_row_description,
                                                              &staticstringtable_title);
   
      fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"staticstrings\" ];\n",
              runtime, &runtime->staticstrings);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &runtime->staticstrings, label, "box");
      free( label);
   }

   if( mulle_concurrent_hashmap_count( &runtime->waitqueues.classestoload))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"classestoload\" ];\n",
               runtime, &runtime->waitqueues.classestoload);
      
      label = mulle_concurrent_hashmap_html_description( &runtime->waitqueues.classestoload,
                                                         mulle_objc_loadclasslist_html_row_description,
                                                         &classestoload_title);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
               &runtime->waitqueues.classestoload, label, "box");
      free( label);
   }

   if( mulle_concurrent_hashmap_count( &runtime->waitqueues.categoriestoload))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\" [ label=\"categoriestoload\" ];\n",
               runtime, &runtime->waitqueues.categoriestoload);
      
      label = mulle_concurrent_hashmap_html_description( &runtime->waitqueues.categoriestoload,
                                                         mulle_objc_loadcategorylist_html_row_description,
                                                         &categoriestoload_title);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &runtime->waitqueues.categoriestoload, label, "box");
      free( label);
   }
   
   fprintf( info->fp, "\n\n");
}


# pragma mark - walker class callback


static void   print_infraclass( struct _mulle_objc_infraclass *infra,
                                struct dump_info *info);
static void   print_metaclass( struct _mulle_objc_metaclass *meta,
                              struct dump_info *info);

static void   print_class( struct _mulle_objc_class *cls,
                           struct dump_info *info,
                           int is_meta)
{
   char                                             *label;
   struct mulle_concurrent_pointerarrayenumerator   rover;
   struct _mulle_objc_cache                         *cache;
   struct _mulle_objc_class                         *superclass;
   struct _mulle_objc_classpair                     *pair;
   struct _mulle_objc_infraclass                    *infra;
   struct _mulle_objc_metaclass                     *meta;
   struct _mulle_objc_methodlist                    *methodlist;
   struct _mulle_objc_runtime                       *runtime;
   unsigned int                                     i;

   label = mulle_objc_class_html_description( cls, is_meta ? "goldenrod" : "blue");
   fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n", cls, label, is_meta ? "component" : "box");
   free( label);

   if( info->draw_runtime)
      if( _mulle_objc_class_get_runtime( cls))
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"runtime\" ];\n", cls,  _mulle_objc_class_get_runtime( cls));

   superclass = _mulle_objc_class_get_superclass( cls);
   if( superclass)
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"super\"; penwidth=\"%d\" ];\n", cls, superclass,
              _mulle_objc_class_is_infraclass( cls)
              ? 3 : 1);
   meta = _mulle_objc_class_get_metaclass( cls);
   if( meta)
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"meta\"; color=\"goldenrod\"; fontcolor=\"goldenrod\" ];\n", cls, meta);

   infra = _mulle_objc_class_get_infraclass( cls);
   if( infra)
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"infra\"; color=\"blue\"; fontcolor=\"blue\" ];\n", cls, infra);

   if( ! (_mulle_objc_class_get_inheritance( cls) & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
   {
      struct _mulle_objc_protocolclassenumerator  rover;
      struct _mulle_objc_infraclass               *prop_cls;
      
      i = 0;
      pair  = _mulle_objc_class_get_classpair( cls);
      rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
      while( prop_cls = _mulle_objc_protocolclassenumerator_next( &rover))
      {
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"protocol class #%u\" ];\n",
                 cls, prop_cls, i++);
      
         if( ! c_set_member( &info->set, prop_cls))
         {
            c_set_add( &info->set, prop_cls);
            print_infraclass( prop_cls, info);
            c_set_add( &info->set, prop_cls);
            print_metaclass( _mulle_objc_infraclass_get_metaclass( prop_cls), info);
         }
      }
      _mulle_objc_protocolclassenumerator_done( &rover);
   }
   fprintf( info->fp, "\n\n");
   
   runtime = _mulle_objc_class_get_runtime( cls);
   if( ! info->terse_rootclass || cls->classid != _mulle_objc_runtime_get_rootclassid( runtime))
   {
      i = 0;
      rover = mulle_concurrent_pointerarray_enumerate( &cls->methodlists);
      while( methodlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
      {
         if( methodlist->n_methods)
         {
            fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"methodlist #%d\" ];\n",
                    cls, methodlist, i++);
            
            label = mulle_objc_methodlist_html_description( methodlist);
            fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", methodlist, label);
            free( label);
         }
      }
      mulle_concurrent_pointerarrayenumerator_done( &rover);
      
      cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
      if( cache->n)
      {
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"cache\" ];\n",
                 cls, cache);
         
         label = mulle_objc_cache_html_description( cache);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", cache, label);
         free( label);
      }
   }
}


static void   print_classpair( struct _mulle_objc_classpair *pair,
                               struct _mulle_objc_class *cls,
                               struct dump_info *info)
{
   char   *label;

   if( mulle_concurrent_pointerarray_get_count( &pair->protocolids))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"protocolids\" ];\n",
              cls, &pair->protocolids);

      label = mulle_objc_protocols_html_description( &pair->protocolids,
                                                     &protocoltable_title);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", &pair->protocolids, label);
      free( label);
   }

   if( mulle_concurrent_pointerarray_get_count( &pair->categoryids))
   {
      fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"categoryids\" ];\n",
              cls, &pair->categoryids);

      label = mulle_objc_categories_html_description( &pair->categoryids,
                                                      &categorytable_title);
      fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", &pair->categoryids, label);
      free( label);
   }
}


static void   print_infraclass( struct _mulle_objc_infraclass *infra,
                                struct dump_info *info)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   char                                             *label;
   struct _mulle_objc_ivarlist                      *ivarlist;
   struct _mulle_objc_propertylist                  *propertylist;
   unsigned int                                     i;

   print_class( _mulle_objc_infraclass_as_class( infra), info, 0);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &infra->ivarlists);
   while( ivarlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( ivarlist->n_ivars)
      {
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"ivarlist #%d\" ];\n",
                 infra, ivarlist, i++);
         
         label = mulle_objc_ivarlist_html_description( ivarlist);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", ivarlist, label);
         free( label);
      }
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &infra->propertylists);
   while( propertylist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( propertylist->n_properties)
      {
         fprintf( info->fp, "\"%p\" -> \"%p\"  [ label=\"propertylist #%d\" ];\n",
                 infra, propertylist, i++);
         
         label = mulle_objc_propertylist_html_description( propertylist);
         fprintf( info->fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", propertylist, label);
         free( label);
      }
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   print_classpair( _mulle_objc_infraclass_get_classpair( infra),
                    _mulle_objc_infraclass_as_class( infra),
                    info);
}


static void   print_metaclass( struct _mulle_objc_metaclass *meta,
                               struct dump_info *info)
{
   print_class( _mulle_objc_metaclass_as_class( meta), info, 1);

   print_classpair( _mulle_objc_metaclass_get_classpair( meta),
                    _mulle_objc_metaclass_as_class( meta),
                    info);
}


static int   callback( struct _mulle_objc_runtime *runtime,
                       void *p,
                       enum mulle_objc_walkpointertype_t type,
                       char *key,
                       void *parent,
                       void *userinfo)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct dump_info                *info;

   assert( p);

   if( key)
      return( mulle_objc_walk_ok);

   info = userinfo;

   if( c_set_member( &info->set, p))
      return( mulle_objc_walk_dont_descend);
   c_set_add( &info->set, p);

   switch( type)
   {
   case mulle_objc_walkpointer_is_category  :
   case mulle_objc_walkpointer_is_protocol  :
   case mulle_objc_walkpointer_is_classpair :
      break;

   case mulle_objc_walkpointer_is_runtime  :
      runtime = p;
      print_runtime( runtime, info);
      break;

   case mulle_objc_walkpointer_is_infraclass :
      infra = p;
      print_infraclass( infra, info);
      break;

   case mulle_objc_walkpointer_is_metaclass :
      meta = p;
      print_metaclass( meta, info);
      break;

   case mulle_objc_walkpointer_is_method :
   case mulle_objc_walkpointer_is_property :
   case mulle_objc_walkpointer_is_ivar :
      break;
   }

   return( mulle_objc_walk_ok);
}


# pragma mark - runtime dump


void   _mulle_objc_runtime_dotdump( struct _mulle_objc_runtime *runtime, FILE *fp)
{
   struct dump_info  info;

   c_set_init( &info.set);
   info.fp             = fp;
   info.terse_rootclass = 0;
   info.draw_runtime   = 1;
   
   fprintf( fp, "digraph mulle_objc_runtime\n{\n");
   mulle_objc_runtime_walk( runtime, callback, &info);
   fprintf( fp, "}\n");

   c_set_done( &info.set);
}


void   mulle_objc_dotdump_runtime( void)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   if( ! runtime)
      return;

   _mulle_objc_runtime_dotdump( runtime, stdout);
}


void   mulle_objc_dotdump_runtime_to_file( char *filename)
{
   struct _mulle_objc_runtime   *runtime;
   FILE                         *fp;

   runtime = mulle_objc_inlined_get_runtime();
   if( ! runtime)
   {
      fprintf( stderr, "No runtime found!\n");
      return;
   }

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   _mulle_objc_runtime_dotdump( runtime, fp);
   fclose( fp);

   fprintf( stderr, "Written dot file \"%s\"\n", filename);
}


//
// create successive number of dumps
// for conversion into a movie
//
void   mulle_objc_dotdump_runtime_to_tmp( void)
{
   static mulle_atomic_pointer_t   counter;
   auto char                       buf[ 32];
   int                             nr;
   int                             max;
   char                            *s;
   
   nr = (int) (intptr_t) _mulle_atomic_pointer_increment( &counter);
   s = getenv( "MULLE_OBJC_DOTDUMP_MAX");
   if( s)
   {
      max = atoi( s);
      if( max && nr >= max)
         return;
   }

   sprintf( buf, "/tmp/runtime_%06d.dot", nr);
   mulle_objc_dotdump_runtime_to_file( buf);
}


# pragma mark - class dump


void   mulle_objc_classpair_dotdump( struct _mulle_objc_classpair *pair, FILE *fp)
{
   extern mulle_objc_walkcommand_t
      mulle_objc_classpair_walk( struct _mulle_objc_classpair *,
                                 mulle_objc_walkcallback_t,
                                 void *);
   struct dump_info              info;

   c_set_init( &info.set);
   info.fp             = fp;
   info.terse_rootclass = 1;
   info.draw_runtime   = 0;

   mulle_objc_classpair_walk( pair, callback, &info);

   c_set_done( &info.set);
}


void   mulle_objc_class_dotdump_to_file( struct _mulle_objc_class *cls,
                                         char *filename)
{
   FILE                          *fp;
   struct _mulle_objc_classpair  *pair;

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   fprintf( fp, "digraph mulle_objc_class\n{\n");

   pair = NULL;
   do
   {
      pair = _mulle_objc_class_get_classpair( cls);
      mulle_objc_classpair_dotdump( pair, fp);
   }
   while( cls = _mulle_objc_class_get_superclass( cls));
   
   fprintf( fp, "}\n");
   fclose( fp);

   fprintf( stderr, "Written dot file \"%s\"\n", filename);
}


void   mulle_objc_dotdump_classname_to_file( char *classname, char *filename)
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
   infra   = _mulle_objc_runtime_lookup_infraclass( runtime, classid);
   if( ! infra)
   {
      fprintf( stderr, "Class \"%s\" is unknown to the runtime\n", classname);
      return;
   }
   
   cls = _mulle_objc_infraclass_as_class( infra);
   mulle_objc_class_dotdump_to_file( cls, filename);
}


void   mulle_objc_class_dotdump_to_tmp( struct _mulle_objc_class *cls)
{
   mulle_objc_class_dotdump_to_file( cls, "/tmp/mulle_objc_class.dot");
}


void   mulle_objc_dotdump_classname_to_tmp( char *classname)
{
   mulle_objc_dotdump_classname_to_file( classname, "/tmp/mulle_objc_class.dot");
}


